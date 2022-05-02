#pragma once
#include <stdint.h>

struct interCustom {
    uint32_t PSP;
    uint32_t currentIPSR;
    uint32_t EXC_RETURN;
    uint32_t R4;
    uint32_t R5;
    uint32_t R6;
    uint32_t R7;
    uint32_t R8;
    uint32_t R9;
    uint32_t R10;
    uint32_t R11;
};

struct interByCPU {
    uint32_t R0;
    uint32_t R1;
    uint32_t R2;
    uint32_t R3;
    uint32_t R12;
    uint32_t LR;
    uint32_t PC;
    uint32_t xPSR;
};

struct inter {
    struct interCustom *custom;
    struct interByCPU *cpu;
};

uint32_t *interruptHandler(uint32_t *i);
void initTIM2();
