#include "../include/syscall.h"

void delay(uint32_t ms) {
    SVCall(CMD_DELAY, ms);
}

void waitForPPM() {
    SVCall(CMD_WAIT_PPM, 0);
}

void exitTask() {
    SVCall(CMD_EXIT_TASK, 0);
}
