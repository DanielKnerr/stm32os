#pragma once
#include <stdint.h>

typedef struct {
    uint8_t readAddress, writeAddress;
    uint8_t i2cBus;
    uint8_t *i2cBuffer;
} bno055Device;

typedef struct {
    int16_t x, y, z;
} EulerAngles;

bno055Device initBNO055(uint8_t address, uint8_t i2cBus);
uint8_t getTempCelsius(bno055Device *dev);
EulerAngles getEulerAngles(bno055Device *dev);
