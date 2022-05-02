#pragma once
#include <stdint.h>

static const uint32_t CMD_DELAY = 0x1;
static const uint32_t CMD_WAIT_PPM = 0x2;
static const uint32_t CMD_EXIT_TASK = 0x3;

extern uint32_t SVCall(uint32_t r0, uint32_t r1);

void delay(uint32_t ms);
void waitForPPM();
void exitTask();
