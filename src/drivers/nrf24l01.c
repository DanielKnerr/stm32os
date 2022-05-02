#include "../../include/drivers/nrf24l01.h"
#include "../../include/gpio.h"
#include "../../include/register.h"
#include "../../include/memory.h"
#include "../../include/drivers/spi.h"
#include "../../include/syscall.h"

// TODO: den Puffer für die Pipes dynamisch erstellen und das Auswählen von Pipes ermöglichen

uint8_t pipe0Buffer[32];

void beginSPItransaction(nRF24L01Device *device) {
    digitalWrite(device->csnPin, LOW);
}

void endSPItransaction(nRF24L01Device *device) {
    digitalWrite(device->csnPin, HIGH);
}

void sendCommand(nRF24L01Device *dev, uint8_t command) {
    beginSPItransaction(dev);
    transmitByteSPI1(command);
    endSPItransaction(dev);
}

void sendCommandAndData(nRF24L01Device *dev, uint8_t command, uint8_t data) {
    beginSPItransaction(dev);
    transmitByteSPI1(command);
    transmitByteSPI1(data);
    endSPItransaction(dev);
}

nRF24L01Device initDevice(uint8_t spiBus, enum Pins cePin, enum Pins csnPin) {
    nRF24L01Device dev;
    dev.spiPort = spiBus;
    dev.cePin = cePin;
    dev.csnPin = csnPin;
    dev.payloadSize = 0;
    dev.buffer = pipe0Buffer;
    dev.sendBuffer = allocate(32);
    dev.sendBufferSize = 0;

    setPinMode(cePin, OUTPUT);
    setPinMode(csnPin, OUTPUT);
    digitalWrite(cePin, LOW);
    digitalWrite(csnPin, HIGH);

    initSPI1Master(BAUD_DIV_256);

    disableChip(&dev);
    powerDown(&dev);

    configAutoRetrans(&dev, 1500, 15);
    configPowerAndSpeed(&dev);
    setPayloadSize(&dev, 32);
    setChannel(&dev, 76);

    clearInterrupts(&dev);

    flushRx(&dev);
    flushTx(&dev);

    configureCRC(&dev);
    sendCommandAndData(&dev, WRITE_REG | CONFIG, 0b01111100);

    powerUp(&dev);

    setAddressPipe0(&dev);

    return dev;
}

void configAutoRetrans(nRF24L01Device *dev, uint16_t retransmitDelay, uint8_t retransmitCount) {
    uint8_t ARD = (retransmitDelay / 250) - 1;
    sendCommandAndData(dev, WRITE_REG | SETUP_RETR, ((ARD << 4) & 0xF0) | (retransmitCount & 0xF));
}

void configPowerAndSpeed(nRF24L01Device *dev) {
    // Bit 3 und 5 setzen die Geschwindigkeit (`0b00` entspricht 1Mbps)
    // Bit 1:2 setzt die Leistung des Radios beim Senden (`0b11` entspricht 0dBm, der höchsten Leistung)
    sendCommandAndData(dev, WRITE_REG | RF_SETUP, 0b110);
}

void setPayloadSize(nRF24L01Device *dev, uint8_t payloadSize) {
    // TODO: payloadSize muss im Bereich [1, 32] sein
    dev->payloadSize = payloadSize;
    sendCommandAndData(dev, WRITE_REG | RX_PW_P0, payloadSize & 0x3F);
}

void setChannel(nRF24L01Device *dev, uint8_t channel) {
    sendCommandAndData(dev, WRITE_REG | RF_CH, channel & 0b01111111);
}

void clearInterrupts(nRF24L01Device *dev) {
    sendCommandAndData(dev, WRITE_REG | STATUS, 0b01110000);
}

void flushRx(nRF24L01Device *dev) {
    sendCommand(dev, FLUSH_RX);
}

void flushTx(nRF24L01Device *dev) {
    sendCommand(dev, FLUSH_TX);
}

void configureCRC(nRF24L01Device *dev) {
    sendCommandAndData(dev, WRITE_REG | CONFIG, 0b00001100);
}

void powerUp(nRF24L01Device *dev) {
    sendCommandAndData(dev, WRITE_REG | CONFIG, 0b00001110);
    // der Start braucht 1.5ms
    delay(2);
}

void powerDown(nRF24L01Device *dev) {
    sendCommandAndData(dev, WRITE_REG | CONFIG, 0b00001100);
    delay(1);
}

void setAddressPipe0(nRF24L01Device *dev) {
    // TODO: die Addressbreite beachten
    beginSPItransaction(dev);
    transmitByteSPI1(WRITE_REG | RX_ADDR_P0);
    transmitByteSPI1(0xE6);
    transmitByteSPI1(0xE6);
    transmitByteSPI1(0xE6);
    transmitByteSPI1(0xE6);
    transmitByteSPI1(0xE6);
    endSPItransaction(dev);
}

void setTransmitAddress(nRF24L01Device *dev) {
    // TODO: die Addressbreite beachten (gilt auch für TX)
    beginSPItransaction(dev);
    transmitByteSPI1(WRITE_REG | TX_ADDR);
    transmitByteSPI1(0xE6);
    transmitByteSPI1(0xE6);
    transmitByteSPI1(0xE6);
    transmitByteSPI1(0xE6);
    transmitByteSPI1(0xE6);
    endSPItransaction(dev);
}

void setReceiver(nRF24L01Device *dev) {
    disableChip(dev);
    sendCommandAndData(dev, WRITE_REG | CONFIG, 0b00001111);

    clearInterrupts(dev);

    enableChip(dev);

    // 150us für den Start des Receivers
    delay(1);

    // die Addresse erneut setzen
    // CHECKME: wird das wirklich benötigt? wodurch wird die Addresse von Pipe 0 zurückgesetzt?
    setAddressPipe0(dev);
}

// die Daten sollten bereits im TX FIFO sein
void setTransmitter(nRF24L01Device *dev) {
    sendCommandAndData(dev, WRITE_REG | CONFIG, 0b00001110);

    // 150us für den Start des Transmitters
    delay(1);
}

void enableChip(nRF24L01Device *dev) {
    digitalWrite(dev->cePin, HIGH);
    delay(1);
}

void disableChip(nRF24L01Device *dev) {
    digitalWrite(dev->cePin, LOW);
    delay(1);
}

uint8_t getData(nRF24L01Device *dev, uint8_t reg) {
    beginSPItransaction(dev);
    transmitByteSPI1(READ_REG | reg);
    uint8_t val = transmitByteSPI1(NOP);
    endSPItransaction(dev);
    return val;
}

bool hasData(nRF24L01Device *dev) {
    beginSPItransaction(dev);
    uint8_t status = transmitByteSPI1(NOP);
    endSPItransaction(dev);

    if (((status >> 1) & 0b111) != 0b111) {
        return true;
    } else {
        return false;
    }
}

void writePacket(nRF24L01Device *dev, uint8_t *packetData, uint8_t packetSize) {
    beginSPItransaction(dev);
    transmitByteSPI1(WRITE_TX_PAYLOAD);
    int realPacketSize = packetSize;
    if (packetSize > dev->payloadSize) realPacketSize = dev->payloadSize;

    for (int i = 0; i < realPacketSize; i++) {
        transmitByteSPI1(packetData[i]);
    }

    for (int i = 0; i < (dev->payloadSize - realPacketSize); i++) {
        transmitByteSPI1(0x00);
    }

    endSPItransaction(dev);
}

void loadDataFromReceiver(nRF24L01Device *dev) {
    beginSPItransaction(dev);
    transmitByteSPI1(READ_RX_PAYLOAD);
    for (int i = 0; i < dev->payloadSize; i++) {
        dev->buffer[i] = transmitByteSPI1(0xFF);
    }
    endSPItransaction(dev);
}

void transmitPacket(nRF24L01Device *dev, uint8_t *packetData, uint8_t packetSize) {
    disableChip(dev);

    // leert das Data Sent Bit
    sendCommandAndData(dev, WRITE_REG | STATUS, 0b100000);

    flushTx(dev);

    setTransmitAddress(dev);
    
    // die Daten in den TX FIFO schreiben
    writePacket(dev, packetData, packetSize);

    // in den Sendemodus wechseln
    setTransmitter(dev);

    // den Chip für >10us aktivieren
    enableChip(dev);

    // den Chip wieder deaktivieren, damit er nicht für mehr als 4ms im TX Modus ist
    // (im Manual wird diese Begrenzung genannt)
    disableChip(dev);

    // warten bis der TX FIFO leer ist (dann wird u.a. automatisch in Standby-I gewechselt)
    bool waitForAck = true;
    while (1) {
        uint8_t fifo = getData(dev, FIFO_STATUS);
        uint8_t status = getData(dev, STATUS);
        if ((fifo & (1 << 4))) break;
        // wenn MAX_RT gesetzt ist konnte das Paket nicht zugestellt werden
        if ((status & (1 << 4))) {
            waitForAck = false;
            break;
        }
    }

    // warten bis das ACK zurückkommt
    while (waitForAck) {
        uint8_t status = getData(dev, STATUS);
        if ((status & (1 << 5))) {
            // leert das Data Sent Bit
            sendCommandAndData(dev, WRITE_REG | STATUS, 0b100000);
            break;
        }
    }
}

void resetBuffer(nRF24L01Device *device) {
    device->sendBufferSize = 0;
}

void sendBuffer(nRF24L01Device *device) {
    transmitPacket(device, device->sendBuffer, device->sendBufferSize);
    device->sendBufferSize = 0;
    delay(1);
}
