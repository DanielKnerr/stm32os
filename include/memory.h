#pragma once
#include "register.h"

// RAM is split into 2048 blocks, each 64 bytes large
// each block is allocated 2 bits in the memory map
// the two bits represent a number 0 to 3
// 0 indicates that this block is not used
// 1, 2, 3 serve as boundaries for an allocation

// RAM wird in 2048 Blöcke (je 64 Byte groß) geteilt. Jedem Block werden zwei Bits in der "memory map" zugewiesen.
// Die zwei Bits repräsentieren eine Zahl, wodurch zugewiesene Blöcke getrennt werden.
// - 0 ist ein freier Block 
// - 1, 2, 3 repräsentieren einen allozierten Block.
//
// Dass es drei verschiedene Werte für einen allozierten Block gibt hat den Vorteil, dass ein zugewiesener Block
// verkleinert werden kann und der so entstandene Raum trotzdem genutzt werden kann, da es immer ein Trennelement
// (also eine Zahl 1 bis 3) gibt, die sich vom linken und rechten Block unterscheidet.

void initMemoryMap(uint32_t usedBytes);
uint8_t *allocate(uint16_t numBytes);
