#include "../include/interrupts.h"
#include "../include/multitask.h"
#include "../include/register.h"
#include "../include/gpio.h"
#include "../include/syscall.h"
#include "../include/drivers/ppm.h"

void initTIM2() {
    // TIM2 ist auf dem APB1 Bus
    *APB1ENR |= 1 << 0;

    // Status Register leeren, u.a. um den Trigger Interrupt zurückzusetzen
    *(TIM2 + TIMx_SR) = 0;

    // Interrupts aktivieren
    *(TIM2 + TIMx_DIER) |= 1 << 0;

    // für 100MHz = 100,000,000 / 500 (prescaler) / 200 (reload value) = 1kHz
    // INFO zur Auto Reload:
    // der Timer erreicht den Auto Reload Wert und löst erst beim Überlauf von dem Wert auf 0 einen Interrupt aus
    // um also einen Interrupt jeden 5. Takt zu bekommen, muss ARR = 5 - 1 gewählt werden
    // das sieht man gut in Figure 90 (für ARR=0x36) im Reference Manual
    // man muss auch 1 vom PSC abziehen, da der Timer +1 auf den PSC rechnet, bevor geteilt wird
    // die Reset Value von PSC ist 0, sonst würde der Timer ja durch 0 Teilen (siehe 13.4.11 im Reference Manual)

    *(TIM2 + TIMx_PSC) = 499;
    *(TIM2 + TIMx_CNT) = 0;
    *(TIM2 + TIMx_ARR) = 199;
    *(TIM2 + TIMx_CR1) |= 0b001;

    // Interrupts für TIM2 aktivieren
    int32_t *NVIC_ISER0 = (int32_t *)0xE000E100;
    // TIM2 ist IRQ28
    *NVIC_ISER0 |= 1 << 28;
}

int ms = 0;
uint32_t *interruptHandler(uint32_t *addr) {
    // addr ist ein Pointer zu den von der Assembly-Routine gespushten Registern und liegt immer auf dem MSP

    // Wenn vor dem Interrupt der MSP verwendet wurde, ist es ein kontinuierliches struct im MSP
    // wenn davor aber der PSP verwendet wurde, liegt ein Teil (interByCPU) auf dem PSP und ein anderer Teil (interCustom) auf dem MSP
    // die Information, welcher Stack vorher verwendet wurde, ist in EXC_RETURN enthalten

    struct inter interrupt;
    interrupt.custom = (struct interCustom*) addr;

    uint32_t EXC_RETURN = interrupt.custom->EXC_RETURN;

    // EXC_RETURN lesen um die von der CPU gepushten Register zu finden
    if (((EXC_RETURN & 0xF) == 0x9) || ((EXC_RETURN & 0xF) == 0x1)) {
        // die von der CPU gepushten register liegen auf dem MSP
        interrupt.cpu = (struct interByCPU*) (addr + sizeof(struct interCustom) / 4);
    } else {
        // die von der CPU gepushten register liegen auf dem PSP
        interrupt.cpu = (struct interByCPU*) interrupt.custom->PSP;
    }

    uint8_t ISRnum = interrupt.custom->currentIPSR & 0xFF;

    if (ISRnum == 15) {
        return onSysTick(addr, ms);
    } else if (ISRnum == 23) {
        // die Interrupt-Flag leeren
        uint32_t *EXTI_PR = (uint32_t*) 0x40013C14;
        *EXTI_PR |= 2;
    } else if (ISRnum == 44) {
        // Status Register leeren, u.a. um den Trigger Interrupt zurückzusetzen
        *(TIM2 + TIMx_SR) = 0;
        ms++;
    } else if (ISRnum == 0xb) {
        uint32_t r0 = interrupt.cpu->R0;
        uint32_t r1 = interrupt.cpu->R1;

        if (r0 == CMD_DELAY) {
            delayCurrentTaskUntil(ms + r1);
        } else if (r0 == CMD_WAIT_PPM) {
            delayTaskUntilPPM();
        } else if (r0 == CMD_EXIT_TASK) {
            killCurrentTask();
        }

        return onSysTick(addr, ms);
    } else if (ISRnum == 0x16) {
        // EXTI0 (PPM)
        if (ppmActive) {
            ppmEdge();
        }
    } else {
        asm ("bkpt");
    }
    return addr;
}
