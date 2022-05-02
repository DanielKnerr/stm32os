#include <stdint.h>
#include "../include/memory.h"

// https://downloads.ti.com/docs/esd/SPRUI03C/using-linker-symbols-in-c-c-applications-slau1318080.html
// => linker symbols are 32-bit unsigned integers
extern uint32_t _data_vma;
extern uint32_t _data_lma;
extern uint32_t _data_size;
extern uint32_t _rodata_vma;
extern uint32_t _rodata_lma;
extern uint32_t _rodata_size;
extern uint32_t _bss_vma;
extern uint32_t _bss_lma;
extern uint32_t _bss_size;

int initVars() {
    int occupiedRAM = 0;
    uint32_t *dvp = &_data_vma;
    uint32_t *dlp = &_data_lma;
    uint32_t *dsp = &_data_size;
    uint32_t *bss_dvp = &_bss_vma;
    uint32_t *bss_dsp = &_bss_size;

    for (uint32_t *ptr = dvp; ptr < dsp; ptr++) {
        *ptr = *dlp;
        dlp++;
    }

    occupiedRAM += (uint32_t)dsp - (uint32_t)dvp;

    for (uint32_t *ptr = bss_dvp; ptr < bss_dsp; ptr++) {
        *ptr = 0;
    }
    occupiedRAM += (uint32_t)bss_dsp - (uint32_t)bss_dvp;

    return occupiedRAM;
}

void _entry() {
    uint32_t usedBytes = initVars();
    initMemoryMap(usedBytes);
}
