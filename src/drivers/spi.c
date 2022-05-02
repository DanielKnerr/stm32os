#include "../../include/drivers/spi.h"
#include "../../include/register.h"
#include "../../include/gpio.h"

void initSPI1Master(enum SPIBaudRateDivisor baudRateDivisor) {
    enum Pins pins[3];
    uint8_t alternateFunctionNumber;

    // im Clock Control Register die Clock für SPI1 aktivieren (SPI1 ist auf dem APB2 Bus)
    *APB2ENR |= (1 << 12);

    // SPI1 ist AF05 (SPI3 ist ebenfalls AF05)
    alternateFunctionNumber = 5;

    // in Table 9 ab Seite 47 im Datenblatt des STM32F411 steht, welche Alternative Functions auf welchen Pins verfügbar sind
    // für SPI1 sind u.a. folgende Pins verfügbar
    // - A5 als SCK
    // - A6 als MISO
    // - A7 als MOSI
    pins[0] = A5;
    pins[1] = A6;
    pins[2] = A7;

    // Master-Modus wählen
    *SPI1_CR1 |= 1 << 2;

    // Baud-Rate wählen
    *SPI1_CR1 |= baudRateDivisor << 3;
    
    // Software Slave Management aktivieren und den NSS Pin auf HIGH setzen
    // CHECKME: NSS (bzw. CSN) wird in nrf24l01.c manuell gesetzt um SPI Transaktionen zu starten / zu beenden.
    //          Vielleicht sollte NSS besser hier verwaltet werden?
    *SPI1_CR1 |= 0b11 << 8;

    for (int i = 0; i < 3; i++) {
        setPinMode(pins[i], ALTERNATE_FUNCTION);
        setOutputSpeed(pins[i], HIGH_SPEED);
        selectAlternateFunction(pins[i], alternateFunctionNumber);
    }

    // SPI1 aktivieren
    *SPI1_CR1 |= 1 << 6;
}

uint8_t transmitByteSPI1(uint8_t data) {
    // warten bis der Transmit Puffer leer ist
    while (!((*SPI1_SR) & 0b10)) {}

    // ein Byte schreiben
    *SPI1_DR = data;

    // warten bis der Receive Puffer Daten hat
    // while (!((*SPI1_SR) & 0b1)) {}
    while (true) {
        uint32_t val = *SPI1_SR;
        val &= 0b1;
        if (val == 1) break;
    }

    // ein Byte lesen und zurückgeben
    return *SPI1_DR;
}
