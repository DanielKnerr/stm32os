#include "../include/gpio.h"
#include "../include/register.h"

void initGPIO() {
    // GPIO A, B und C sind auf dem AHB1 Bus
    // alle drei Peripherals aktivieren
    *AHB1ENR |= 0b111;
}

uint32_t *getBaseReg(enum Pins pin) {
    if (pin <= A15) {
        return GPIOA;
    } else if (pin <= B15) {
        return GPIOB;
    } else if (pin <= C15) { 
        return GPIOC;
    }
}

uint8_t getPinNumber(enum Pins pin) {
    if (pin <= A15) {
        return pin - A0;
    } else if (pin <= B15) {
        return pin - B0;
    } else if (pin <= C15) { 
        return pin - C0;
    }
}

void setOutputSpeed(enum Pins pin, enum PinSpeed speed) {
    uint32_t *baseReg = getBaseReg(pin);
    uint8_t pinNumber = getPinNumber(pin);

    *(baseReg + GPIO_OSPEEDR) &= ~(0b11 << (pinNumber * 2));
    *(baseReg + GPIO_OSPEEDR) |= speed << (pinNumber * 2);
}

void selectAlternateFunction(enum Pins pin, uint8_t alternateFunctionNumber) {
    uint32_t *baseReg = getBaseReg(pin);
    uint8_t pinNumber = getPinNumber(pin);
    uint8_t pinOffset;
    uint32_t *AFreg;

    if (pinNumber <= 7) {
        AFreg = baseReg + GPIO_AFRL;
        pinOffset = pinNumber;
    } else {
        AFreg = baseReg + GPIO_AFRH;
        pinOffset = pinNumber - 8;
    }

    uint32_t val = *AFreg;
    val &= ~( 0b1111 << (pinOffset * 4));
    val |= alternateFunctionNumber << (4 * pinOffset);
    *AFreg = val;
}

void setPinMode(enum Pins pin, enum PinModes pinMode) {
    uint32_t *baseReg = getBaseReg(pin);
    uint8_t pinNumber = getPinNumber(pin);

    *(baseReg + GPIO_MODER) &= ~(0b11 << (pinNumber * 2));
    *(baseReg + GPIO_MODER) |= pinMode << (pinNumber * 2);
}

void setPortOutputData(MMREG_t *portRegister, int pinOffset, enum DigitalLevels level) {
    int newValue = *portRegister;
    if (level == HIGH) {
        newValue |= 1 << pinOffset;
    } else if (level == LOW) {
        newValue &= ~(1 << pinOffset);
    }
    *portRegister = newValue;
}

void digitalWrite(enum Pins pin, enum DigitalLevels level) {
    if (pin <= A15) {
        setPortOutputData(GPIOA_ODR, pin - A0, level);
    } else if (pin <= B15) {
        setPortOutputData(GPIOB_ODR, pin - B0, level);
    } else if (pin <= C15) { 
        setPortOutputData(GPIOC_ODR, pin - C0, level);
    }
}

enum DigitalLevels getPortInputData(MMREG_t *portRegister, int pinOffset) {
    return (*portRegister & (1 << pinOffset)) == 0 ? LOW : HIGH;
}

enum DigitalLevels digitalRead(enum Pins pin) {
    if (pin <= A15) {
        return getPortInputData(GPIOA_IDR, pin - A0);
    } else if (pin <= B15) {
        return getPortInputData(GPIOB_IDR, pin - B0);
    } else if (pin <= C15) { 
        return getPortInputData(GPIOC_IDR, pin - C0);
    }
    // TODO: das sollte niemals passieren, hier am besten eine Trap implementieren
    return LOW;
}

void selectPUPD(enum Pins pin, enum PullupPulldown v) {
    uint32_t *baseReg = getBaseReg(pin);
    uint8_t pinNumber = getPinNumber(pin);

    *(baseReg + GPIO_PUPDR) &= ~(0b11 << (pinNumber * 2));
    *(baseReg + GPIO_PUPDR) |= v << (pinNumber * 2);
}

void selectOutputType(enum Pins pin, enum OutputType t) {
    uint32_t *baseReg = getBaseReg(pin);
    uint8_t pinNumber = getPinNumber(pin);

    *(baseReg + GPIO_OTYPER) &= ~(1 << pinNumber);
    if (t == OPEN_DRAIN) {
        *(baseReg + GPIO_OTYPER) |= 1 << pinNumber;
    }
}
