#include "../include/math.h"

uint32_t intLog2(uint32_t value) {
    // Formel von https://stackoverflow.com/a/994623/17278981
    int output = 0;
    while (value >>= 1) ++output;

    return output;
}
