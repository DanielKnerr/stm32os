#pragma once
#include <stdint.h>

void initI2C1();

void i2cStart();
void writeAddress(uint8_t addr);
void writeByte(uint8_t byte);
void i2cReadBytes(uint8_t addr, uint8_t size, uint8_t *buffer);
void i2cStop();

void startCondition();
void stopCondition();
