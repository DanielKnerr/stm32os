#include <stdint.h>

#include "../include/gpio.h"
#include "../include/memory.h"
#include "../include/interrupts.h"
#include "../include/multitask.h"
#include "../include/register.h"
#include "../include/clock.h"
#include "../include/drivers/nrf24l01.h"
#include "../include/drivers/ppm.h"

extern void start();

void main() {
    // die Uhr des System Configuration Controllers aktivieren
    // der verwaltet u.a. externe Interrupts
    *APB2ENR |= (1 << 14);

    initGPIO();

    initTasks();

    initTIM2();
    initPPM();
    initClock100MHz();
    initSysTick();

    start();

    asm ("cpsie if");

    // in einem unendlichem Loop bleiben bis der Scheduler zum ersten Task wechselt
    while (1) { }
}
