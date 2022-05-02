#pragma once
#include <stdbool.h>
#include <stdint.h>

#define PPM_MAX_CHANNELS 8
#define PPM_TIMEOUT_US 2400
#define PPM_MIN_VALUE 1000
#define PPM_MAX_VALUE 2000
#define PPM_CLAMP 20
#define PPM_MINIMAL_PULSE 850

extern bool ppmActive;
extern uint16_t ppmValues[];
extern bool ppmAvailable;

void initPPM();
void ppmEdge();
