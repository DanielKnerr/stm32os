#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "gpio.h"

// SPI Kommandos (Table 19, Seite 48, nRF24L01Pluss_Preliminary_Product_Specification_v1_0.pdf)
#define READ_REG 0b00000000
#define WRITE_REG 0b00100000
#define READ_RX_PAYLOAD 0b01100001
#define WRITE_TX_PAYLOAD 0b10100000
#define FLUSH_TX 0b11100001
#define FLUSH_RX 0b11100010
#define NOP 0b11111111

// SPI Register (Table 27, ab Seite 54, nRF24L01Pluss_Preliminary_Product_Specification_v1_0.pdf)
#define CONFIG 0x00
#define EN_AA 0x01
#define EN_RXADDR 0x02
#define SETUP_AW 0x03
#define SETUP_RETR 0x04
#define RF_CH 0x05
#define RF_SETUP 0x06
#define STATUS 0x07
#define OBSERVE_TX 0x08
#define RPD 0x09
#define RX_ADDR_P0 0x0A
#define RX_ADDR_P1 0x0B
#define RX_ADDR_P2 0x0C
#define RX_ADDR_P3 0x0D
#define RX_ADDR_P4 0x0E
#define RX_ADDR_P5 0x0F
#define TX_ADDR 0x10
#define RX_PW_P0 0x11
#define RX_PW_P1 0x12
#define RX_PW_P2 0x13
#define RX_PW_P3 0x14
#define RX_PW_P4 0x15
#define RX_PW_P5 0x16
#define FIFO_STATUS 0x17
#define DYNPD 0x1C
#define FEATURE 0x1D

/*
* Repräsentiert einen an einen SPI-Bus angeschlossenen nRF24L01+.
* Soll nicht direkt verwendet werden, sondern durch `initDevice` erstellt werden.
*/
typedef struct {
    enum Pins cePin, csnPin;
    uint8_t spiPort;
    uint8_t payloadSize;
    uint8_t *buffer, *sendBuffer;
    uint8_t sendBufferSize;
} nRF24L01Device;

/**
 * @brief Initialisiert ein nRF24L01+ und erstellt ein `nRF24L01Device` Objekt.
 * 
 * @param spiBus der SPI-Bus, an dem der Chip angeschlossen ist
 * @param cePin der entsprechende CE Pin
 * @param csnPin der entsprechende CSN Pin
 * @return nRF24L01Device 
 */
nRF24L01Device initDevice(uint8_t spiBus, enum Pins cePin, enum Pins csnPin);

/**
 * @brief Konfiguriert die automatische erneute Übertragung, falls ein Paket nicht angekommen ist.
 * 
 * @param device ein Zeiger auf den zu konfigurierenden Chip
 * @param retransmitDelay nach wie vielen Mikrosekunden die Übertragung als gescheitert angesehen wird und erneut versucht wird.
 *                        Dabei entspricht das Minimum `0` einem Timeout von 250us und das Maximum `15` einen Timeout von 4000us.
 * @param retransmitCount wie oft eine erneute Übertragung versucht werden soll. Durch `0` wird nie versucht, gescheiterte Übertragungen erneut zu versuchen. Das Maximum ist `15`. 
 */
void configAutoRetrans(nRF24L01Device *device, uint16_t retransmitDelay, uint8_t retransmitCount);

/**
 * @brief Konfiguriert den Chip auf eine Übetragungsgeschwindigkeit von 1Mbps und der maximalen Sendeleistung.
 * 
 * @param device ein Zeiger auf den zu konfigurierenden Chip
 */
void configPowerAndSpeed(nRF24L01Device *device);

/**
 * @brief Setzt die Anzahl an Bytes in einem Packet.
 * 
 * @param device ein Zeiger auf den zu konfigurierenden Chip
 * @param payloadSize die Anzahl an Bytes in einem Packet. Im Bereich `[1, 32]`.
 */
void setPayloadSize(nRF24L01Device *device, uint8_t payloadSize);

/**
 * @brief Setzt den Kanal, auf dem der Chip arbeitet.
 * 
 * @param device ein Zeiger auf den zu konfigurierenden Chip
 * @param channel der positive Offset in MHz von 2,4 GhZ. Im Bereich `[0, 127]`.
 */
void setChannel(nRF24L01Device *device, uint8_t channel);

/**
 * @brief Leert die drei Interrupt-Flags im STATUS Register.
 * 
 * @param device ein Zeiger auf den zu konfigurierenden Chip
 */
void clearInterrupts(nRF24L01Device *device);

/**
 * @brief Leert den RX Puffer.
 * 
 * @param device ein Zeiger auf den zu konfigurierenden Chip
 */
void flushRx(nRF24L01Device *device);

/**
 * @brief Leert den TX Puffer.
 * 
 * @param device ein Zeiger auf den zu konfigurierenden Chip
 */
void flushTx(nRF24L01Device *device);

/**
 * @brief Aktiviert CRC im 2 byte Encoding Schema.
 * 
 * @param device ein Zeiger auf den zu konfigurierenden Chip
 */
void configureCRC(nRF24L01Device *device);

/**
 * @brief Fährt den Chip in den Standby-I Modus hoch.
 * 
 * @param device ein Zeiger auf den zu konfigurierenden Chip
 */
void powerUp(nRF24L01Device *device);

/**
 * @brief Fährt den Chip in den "Power Down" Modus herunter.
 * 
 * @param device ein Zeiger auf den zu konfigurierenden Chip
 */
void powerDown(nRF24L01Device *device);

/**
 * @brief Setzt die Adresse von Pipe 0 auf 0xE6E6E6E6E6.
 * 
 * @param device ein Zeiger auf den zu konfigurierenden Chip
 */
void setAddressPipe0(nRF24L01Device *device);

/**
 * @brief Setzt die Sende-Adresse auf 0xE6E6E6E6E6.
 * 
 * @param device ein Zeiger auf den zu konfigurierenden Chip
 */
void setTransmitAddress(nRF24L01Device *device);

/**
 * @brief Bewegt den Chip in den RX Modus.
 * 
 * @param device ein Zeiger auf den zu konfigurierenden Chip
 */
void setReceiver(nRF24L01Device *device);

/**
 * @brief Bewegt den Chip in den TX Modus. Die zu übertragenden Daten sollten bereits im TX FIFO sein.
 * 
 * @param device ein Zeiger auf den zu konfigurierenden Chip
 */
void setTransmitter(nRF24L01Device *device);

/**
 * @brief Aktiviert den CE Pin.
 * 
 * @param device ein Zeiger auf den zu konfigurierenden Chip
 */
void enableChip(nRF24L01Device *device);

/**
 * @brief Deaktiviert den CE Pin.
 * 
 * @param device ein Zeiger auf den zu konfigurierenden Chip
 */
void disableChip(nRF24L01Device *device);

/**
 * @brief Prüft, ob es eine Pipe gibt, in der sich zu lesende Daten befinden.
 * 
 * @param device ein Zeiger auf den zu konfigurierenden Chip
 * @return true Es existieren zu lesende Daten in einer Pipe.
 * @return false Es gibt keine zu lesenden Daten.
 */
bool hasData(nRF24L01Device *device);

/**
 * @brief Liest ein Packet aus dem RX FIFO und speichert die Daten in `device.buffer`
 * 
 * @param device ein Zeiger auf den zu konfigurierenden Chip
 */
void loadDataFromReceiver(nRF24L01Device *device);

/**
 * @brief Schreibt ein Packet in den TX FIFO Puffer. Soll nicht direkt verwendet werden.
 * 
 * @param device ein Zeiger auf den zu konfigurierenden Chip
 * @param packetData ein Zeiger auf eine Payload
 * @param packetSize die Anzahl an Bytes, die die Payload enthält. Wenn die Payload kleiner als die konfigurierte Payload Size ist, wird der Rest mit `0x00` aufgefüllt.
 */
void writePacket(nRF24L01Device *device, uint8_t *packetData, uint8_t packetSize);

/**
 * @brief Sendet ein Packet.
 * 
 * @param device ein Zeiger auf den zu konfigurierenden Chip
 * @param packetData ein Zeiger auf eine Payload
 * @param packetSize die Anzahl an Bytes, die die Payload enthält. Wenn die Payload kleiner als die konfigurierte Payload Size ist, wird der Rest mit `0x00` aufgefüllt.
 */
void transmitPacket(nRF24L01Device *device, uint8_t *packetData, uint8_t packetSize);

/**
 * @brief Leert den Puffer.
 * 
 * @param device 
 * @param data 
 */
void resetBuffer(nRF24L01Device *device);

/**
 * @brief Sendet den Puffer.
 * 
 * @param device mit welchem Gerät gearbeitet werden soll
 */
void sendBuffer(nRF24L01Device *device);

uint8_t getData(nRF24L01Device *dev, uint8_t reg);
