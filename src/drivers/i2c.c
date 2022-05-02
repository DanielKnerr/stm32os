#include "../../include/drivers/i2c.h"
#include "../../include/register.h"
#include "../../include/syscall.h"
#include "../../include/gpio.h"

// Die Statusregister werden geleert indem sie gelesen werden.
// Diese Funktion ist so kurz, dass es sich lohnt sie immer inline zu verwenden.
__attribute__((always_inline))
inline void clearStatusRegisters();

// für diesen Teil die Option -Wunused-variable ausschalten, da das unnötige Lesen kein ausversehen ist
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
void clearStatusRegisters() {
    // volatile damit GCC diesen "unnötigen" Lesevorgang nicht wegoptimiert
    volatile uint8_t temp = *(I2C1 + I2C_SR1) | *(I2C1 + I2C_SR2);
}
#pragma GCC diagnostic pop

void initI2C1() {
    // aus Table 8 (Datasheet) sind folgende Pins eine Möglichkeit für die Ausgabe von I2C1:
    // - PB9 (I2C1_SDA)
    // - PB8 (I2C1_SCL)

    enum Pins pins[2];
    
    pins[0] = B8;
    pins[1] = B9;

    // I2C1 auf dem APB1 Bus aktivieren
    *APB1ENR |= 1 << 21;

    for (int i = 0; i < 2; i++) {
        selectAlternateFunction(pins[i], 4);
        setPinMode(pins[i], ALTERNATE_FUNCTION);
        setOutputSpeed(pins[i], HIGH_SPEED);
        selectPUPD(pins[i], PULL_UP);
        selectOutputType(pins[i], OPEN_DRAIN);
    }

    // es wäre sinnvoll I2C1 am Anfang einmal zu resetten, jedoch muss man aufpassen:
    // "make sure the I2C lines are released and the bus is free" (Reference Manual, Seite 492)
    // *(I2C1 + I2C_CR1) |= 1 << 15;
    // *(I2C1 + I2C_CR1) &= 1 << 15;

    // die Frequenz des APB1 Bus setzen (50MHz)
    *(I2C1 + I2C_CR2) |= 50;

    // die Formel für CCR ist (T_rSCL + T_wSCLH) / T_PCLK1
    // da der Zähler = 5000ns und T_PCLK1 = 20ns => 250
    *(I2C1 + I2C_CCR) |= 250;

    // set TRISE to 51 (dec), if FREQ is 50MHz P_{CLK1} = 1/50MHz = 20ns
    // (1000ns / 20ns) = 51
    // im Standard (Sm) Modus (welche hier genutzt wird) ist die maximale SCL rise time 1000ns
    // die Formel für dieses Register lautet: (1000ns / T_PCLK1) + 1 = (1000ns / 20ns) + 1 = 51
    *(I2C1 + I2C_TRISE) |= 51;

    // Aktivieren
    *(I2C1 + I2C_CR1) |= 1;
}

void startCondition() {
    *(I2C1 + I2C_CR1) |= 1 << 10;
    *(I2C1 + I2C_CR1) |= 1 << 8;

    while (1) {
        uint16_t val = *(I2C1 + I2C_SR1);
        if ((val & 0b1) != 0) break;
    }
}

void stopCondition() {
    *(I2C1 + I2C_CR1) |= 1 << 9;
}

void writeAddress(uint8_t addr) {
    // hier nicht für TxE warten, da dieses Bit erst nachdem die Addresse gesendet wurde gesetzt wird
    *(I2C1 + I2C_DR) = addr;

    while (1) {
        uint16_t val = *(I2C1 + I2C_SR1);
        if ((val & 0b10) != 0) break;
    }
    clearStatusRegisters();
}

void writeByte(uint8_t byte) {
    *(I2C1 + I2C_DR) = byte;
    while (!(*(I2C1 + I2C_SR1) & (1 << 2)));
}

void i2cReadBytes(uint8_t addr, uint8_t size, uint8_t *buf) {
    int remaining = size;

    if (size == 1) {
        *(I2C1 + I2C_DR) = addr;
        while (!(*(I2C1 + I2C_SR1) & (1 << 1)));

        *(I2C1 + I2C_CR1) &= ~(1 << 10);
        clearStatusRegisters();
        *(I2C1 + I2C_CR1) |= 1 << 9;

        while (!(*(I2C1 + I2C_SR1) & (1 << 6)));

        uint8_t val = *(I2C1 + I2C_DR);
        buf[size - remaining] = val;
    } else {
        *(I2C1 + I2C_DR) = addr;
        while (!(*(I2C1 + I2C_SR1) & (1 << 1)));
        clearStatusRegisters();

        while (remaining > 2) {
            while (!(*(I2C1 + I2C_SR1) & (1 << 6)));
            buf[size - remaining] = *(I2C1 + I2C_DR);
            *(I2C1 + I2C_CR1) |= 1 << 10;
            remaining--;
        }

        while (!(*(I2C1 + I2C_SR1) & (1 << 6)));
        buf[size - remaining] = *(I2C1 + I2C_DR);
        *(I2C1 + I2C_CR1) &= ~(1 << 10);
        *(I2C1 + I2C_CR1) |= 1 << 9;

        remaining--;

        while (!(*(I2C1 + I2C_SR1) & (1 << 6)));
        buf[size - remaining] = *(I2C1 + I2C_DR);
    }
}
