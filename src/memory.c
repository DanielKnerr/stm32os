#include "../include/memory.h"

#include <stddef.h>

#define NUM_BLOCKS 2048
#define BLOCK_SIZE 64

uint8_t *memory_map;

uint8_t getAlloc(int block) {
    uint16_t memory_index = block / 4;
    uint8_t offset = (block - memory_index * 4) * 2;
    uint8_t value = *(memory_map + memory_index);
    value = (value >> offset) & 0b11;
    return value;
}

void setAlloc(uint16_t block, uint8_t val) {
    uint16_t memory_index = block / 4;
    uint8_t offset = (block - memory_index * 4) * 2;
    uint8_t newVal = *(memory_map + memory_index);
    newVal = newVal & ~(0b11 << offset);
    newVal |= val << offset;
    *(memory_map + memory_index) = newVal;
}

void initMemoryMap(uint32_t usedBytes) {
    memory_map = (uint8_t*) MMRAM;
    memory_map += usedBytes;

    for (int i = 0; i < NUM_BLOCKS; i++) {
        setAlloc(i, 0);
    }

    allocate(usedBytes + NUM_BLOCKS / 4);
}

uint8_t valWrap(int8_t val) {
    if (val < 1) return 3;
    if (val > 3) return 1;
    return val;
}

uint8_t *allocate(uint16_t numBytes) {
    int reqBlocks = (numBytes + BLOCK_SIZE - 1) / BLOCK_SIZE;

    for (int i = 0; i < NUM_BLOCKS - reqBlocks + 1; i++) {
        int valid = 1;
        for (int j = 0; j < reqBlocks; j++) {
            if (getAlloc(i + j) != 0) {
                valid = 0;
                break;
            }
        }

        if (valid == 1) {
            int8_t left, right, thisVal;
            if (i == 0) {
                left = 0;
            } else {
                left = getAlloc(i - 1);
            }

            if (i + reqBlocks == NUM_BLOCKS) {
                right = 0;
            } else {
                right = getAlloc(i + reqBlocks);
            }

            if (left == 0) {
                thisVal = valWrap(right + 1);
            } else if (right == 0) {
                thisVal = valWrap(left + 1);
            } else {
                if (left + right == 3) {
                    thisVal = 3;
                } else if (left + right == 4) {
                    thisVal = 2;
                } else if (left + right == 5) {
                    thisVal = 1;
                }
            }

            for (int j = 0; j < reqBlocks; j++) {
                setAlloc(i + j, thisVal);
            }
            return (uint8_t *)MMRAM + i * BLOCK_SIZE;
        }
    }
    return NULL;
}
