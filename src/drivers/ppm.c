#include "../../include/drivers/ppm.h"
#include "../../include/register.h"
#include "../../include/gpio.h"

bool ppmActive = false;

void initTIM3() {
    // TIM3 auf dem APB1 Bus aktivieren
    *APB1ENR |= 1 << 1;

    // alle Flags zurücksetzen
    *(TIM3 + TIMx_SR) = 0;

    // der Timer soll keine Interrupts auslösen (Bit 0 leeren)
    *(TIM3 + TIMx_DIER) &= ~0b1;

    // der längste zu erwartende Abstand zwischen Flanken ist eine Pause von 12ms zwischen Packeten (laut https://wiki.rc-network.de/wiki/PPM)
    // um ein wenig Luft zu haben: von 20ms ausgehen
    // daraus folgt eine Frequenz von 1000ms/20ms = 50Hz
    // um möglichst viel Auflösung zu bekommen sollte der Zähler möglichst hoch zählen, bis er überläuft
    // für 100MHz = 100,000,000 / 40 (prescaler) / 50000 (reload value) = 50Hz
    // dann entspricht ein Schritt vom Timer (also im TIMx_CNT Register) 0,4 Mikrosekunden

    *(TIM3 + TIMx_PSC) = 40 - 1;
    *(TIM3 + TIMx_CNT) = 0;
    *(TIM3 + TIMx_ARR) = 50000 - 1;
    *(TIM3 + TIMx_CR1) |= 0b1;
}

void initPPM() {
    initTIM3();

    // welcher Pin (also A0, B0, C0, ...) für EXTI0 verwendet wird, wird vom SYSCFG_EXTICR1 Register gesteuert
    // standardmäßig wird immer Port A verwendet

    setPinMode(A0, INPUT);

    // die Interrupts standardmäßig maskieren
    *EXTI_IMR &= ~1;

    // nur auf die steigende Flanke achten!
    // der Startpuls ist immer 0,5ms lang
    *EXTI_RTSR |= 1;

    // EXTI0 ist IRQ6
    int32_t *NVIC_ISER0 = (int32_t *)0xE000E100;
    *NVIC_ISER0 |= 1 << 6;

    // ab jetzt können Interrupts entgegengenommen werden
    ppmActive = true;
}

uint16_t ppmValues[PPM_MAX_CHANNELS];
bool ppmAvailable = false;
uint8_t nextPPMChannel = 0;

void ppmEdge() {
    uint32_t val = *(TIM3 + TIMx_CNT) & 0xFFFF;
    *(TIM3 + TIMx_CNT) = 0;
    ppmAvailable = false;

    uint32_t us = val * 4 / 10;

    // TODO: wenn mehr als 100 Millisekunden vergangen sind, ist das Paket ungültig

    if (us > PPM_MINIMAL_PULSE) {
        if (us > PPM_TIMEOUT_US) {
            if (nextPPMChannel == PPM_MAX_CHANNELS) {
                // PPM Interrupts wieder deaktivieren
                *EXTI_IMR &= ~1;

                // dieses Paket ist komplett
                ppmAvailable = true;
            }
            nextPPMChannel = 0;
        } else {
            if (nextPPMChannel < PPM_MAX_CHANNELS) {
                if (us < PPM_MIN_VALUE + PPM_CLAMP) us = PPM_MIN_VALUE;
                if (us > PPM_MAX_VALUE - PPM_CLAMP) us = PPM_MAX_VALUE;
                ppmValues[nextPPMChannel] = us - PPM_MIN_VALUE;
                nextPPMChannel++;
            }
        }
    }

    // die Interrupt-Routine ist fertig: damit ein neuer Interrupt entgegengenommen werden kann
    // muss das Pending Bit geleert werden (im EXTI-Gerät, nicht im NVIC!)
    uint32_t *EXTI_PR = (uint32_t*) 0x40013C14;
    *(EXTI_PR) |= 1;
}
