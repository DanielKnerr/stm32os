#pragma once
#include <stdint.h>
#include <stdbool.h>

// aus Table 1 im Reference Manual
#define SPI1_BASE 0x40013000
#define SPI2_BASE 0x40003800
#define SPI3_BASE 0x40003C00

enum SPIBaudRateDivisor { BAUD_DIV_2, BAUD_DIV_4, BAUD_DIV_8, BAUD_DIV_16, BAUD_DIV_32, BAUD_DIV_64, BAUD_DIV_128, BAUD_DIV_256 };

/**
 * @brief Initialisiert das SPI1 Peripheral im Master-Modus auf den Pins A5, A6, A7. Verwaltet *nicht* NSS.
 * 
 * @param baudRateDivisor der Teiler für f_PCLK um die Baud-Rate zu erhalten
 */
void initSPI1Master(enum SPIBaudRateDivisor baudRateDivisor);

// sends out one byte and returns the byte that is parallely shifted out on MISO
/**
 * @brief Überträgt ein Byte auf SPI1 und gibt das Byte zurück, das parallel zurückgegeben wurde.
 * 
 * @param data das zu übertragende Byte
 * @return uint8_t das parallel zurückgegebende Byte
 */
uint8_t transmitByteSPI1(uint8_t data);
