#include "../include/clock.h"
#include "../include/register.h"
#include "../include/math.h"

void initClock100MHz() {
    // für mehr Infos https://controllerstech.com/stm32-clock-setup-using-registers/

    // HSE starten
    // (HSE ist stabiler als HSI)
    *RCC_CR |= 1 << 16;

    // warten, bis HSE bereit ist
    while (!(*RCC_CR & (1 << 17)));

    // PLL Konfiguration:
    // um höhere Taktgeschwindigkeiten zu erreichen muss durch einen PLL die Frequenz einer "Eingabe-Uhr" erhöht werden
    // das Programm STM32CubeMX kann für einen gegebenen Zieltakt (HCLK) die Parameter für den PLL ausrechnen
    // da die HSE genutzt wird (siehe oben) und die HSE extern ist, muss man die Frequenz raussuchen und in dem Programm eintragen
    // für das aktuelle Board sind es 25MHz (https://stm32-base.org/boards/STM32F411CEU6-WeAct-Black-Pill-V2.0.html)
    // außerdem muss man die HSE im Tab "Pinout & Configuration" aktivieren

    // diese Werte wurden von STM32CubeMX für einen Takt von HCLK = 100MHz gewählt
    uint16_t PLL_M = 12;
    uint16_t PLL_N = 96;
    uint16_t PLL_P = 2;
    // Q = 4, wird aber nicht gesetzt

    uint16_t AHBprescale = 1;
    uint16_t APB1prescale = 2;
    uint16_t APB2prescale = 1;

    // die APBx und AHB Prescaler in Bits übersetzen (siehe das RCC_CFGR Register)
    uint8_t PPRE1 = intLog2(APB1prescale) + 3;
    uint8_t PPRE2 = intLog2(APB2prescale) + 3;
    uint8_t HPRE = intLog2(AHBprescale) + 7;

    // das Power Interface aktivieren
    *APB1ENR |= 1 << 28;

    // die Skalierung des Spannungsreglers auswählen (0b11 = Scale 1 Modus für bis zu 100MHz)
    *PWR_CR |= 3 << 14;

    // aktiviert den Data und Instruction Cache, sowie Prefetch
    // setzt die Latenz auf "five wait states"
    *FLASH_ACR = (1<<8) | (1<<9)| (1<<10)| (5<<0);

    // AHB
    *RCC_CFGR &= ~(0xF0);
    *RCC_CFGR |= HPRE << 4;

    // APB1
    *RCC_CFGR |= PPRE1<<10;

    // APB2
    *RCC_CFGR |= PPRE2<<13;

    // PLL konfigurieren
    *RCC_PLLCFGR = (PLL_M << 0) | (PLL_N << 6) | (((PLL_P - 1) / 2) << 16) | (1 << 22);

    // PLL aktivieren
    *RCC_CR |= (1 << 24);
    // warten, bis PLL bereit ist
    while (!(*RCC_CR & (1 << 25)));

    // PLL als System Clock auswählen
    *RCC_CFGR |= (2 << 0);
    // warten, bis PLL als System Clock genutzt wird
    while (!(*RCC_CFGR & (2 << 2)));
}

void initSysTick() {
    // Zitat aus dem Cortex-M4 Devices Generic User Guide 4.4.5, Seite 252:
    // This means the correct initialization sequence for the SysTick counter is:
    // 1. Program reload value.
    // 2. Clear current value.
    // 3. Program Control and Status register.

    // if we're using the AHB/8 clock and AHB is 100 MHZ and we want one systick per 1ms
    // wenn die AHB/8 Uhr genutzt wird und Systick eine Frequenz von 1kHZ haben soll
    // 100,000,000 / 8 = 12,500,000 (8 ist der prescaler)
    // 12,500,000 / 1000 = 12,500 (1000 ist die Frequenz, 12500 ist die Reload Value)

    *MMREG_STK_LOAD = 12500 - 1;
    *MMREG_STK_VAL = 0;
    *MMREG_STK_CTRL = 0b011;
}
