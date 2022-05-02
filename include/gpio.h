#pragma once
#include <stdint.h>

// für das STM32F411 "Black Pill" Board:
// - A13 und A14 sind mit dem Software Debugger verbunden und sollten nicht genutzt werden
// - B11 und C0 - C12 sind nicht verfügbar
enum Pins {
    A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15,
    B0, B1, B2, B3, B4, B5, B6, B7, B8, B9, B10, B11, B12, B13, B14, B15,
    C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15
};

enum PinModes { INPUT = 0b00, OUTPUT = 0b01, ALTERNATE_FUNCTION = 0b10, ANALOG = 0b11 };
enum DigitalLevels { LOW, HIGH };
enum PinSpeed { LOW_SPEED = 0b00, MEDIUM_SPEED = 0b01, FAST_SPEED = 0b10, HIGH_SPEED = 0b11};
enum PullupPulldown { NO_PULL = 0b00, PULL_UP = 0b01, PULL_DOWN = 0b10};
enum OutputType { PUSH_PULL = 0b0, OPEN_DRAIN = 0b1};

void setPinMode(enum Pins pin, enum PinModes pinMode);
void digitalWrite(enum Pins pin, enum DigitalLevels level);
enum DigitalLevels digitalRead(enum Pins pin);
void initGPIO();
void setOutputSpeed(enum Pins pin, enum PinSpeed speed);
void selectAlternateFunction(enum Pins pin, uint8_t alternateFunctionNumber);
void selectPUPD(enum Pins pin, enum PullupPulldown v);
void selectOutputType(enum Pins pin, enum OutputType t);
