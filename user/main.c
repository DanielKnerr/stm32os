#include "../include/multitask.h"
#include "../include/gpio.h"
#include "../include/memory.h"
#include "../include/syscall.h"
#include "../include/drivers/bno055.h"
#include "../include/drivers/nrf24l01.h"
#include <string.h>

#define DISPLAY_BUFFER_SIZE 256

// der ASCII "End of Medium" Charakter
#define ASCII_END_OF_MEDIUM 0x19
uint8_t displayBuffer[DISPLAY_BUFFER_SIZE];

// Variablen zum Ringspeicher, der den Display Puffer speichert 
uint16_t writeHead = 0;
uint16_t readHead = 0;
uint16_t count = 0;
int bufferOverflow = 0;

// Fügt einen String zum Puffer hinzu.
void addBuffer(char *characters) {
    int i = 0;
    while (true) {
        uint8_t c = characters[i];
        if (c == 0) break;
        if (count < DISPLAY_BUFFER_SIZE) {
            displayBuffer[writeHead] = c;
            writeHead++;
            count++;
            if (writeHead >= DISPLAY_BUFFER_SIZE) writeHead = 0;
        } else {
            bufferOverflow = 1;
            displayBuffer[DISPLAY_BUFFER_SIZE - 1] = 0;
        }

        i++;
    }
}

// Liest ein Byte aus dem Puffer. Gibt ein EM Zeichen zurück, wenn kein Byte im Puffer ist.
uint8_t readFromBuffer() {
    if (count > 0) {
        uint8_t val = displayBuffer[readHead];
        readHead++;
        count--;
        if (readHead >= DISPLAY_BUFFER_SIZE) readHead = 0;
        return val;
    } else {
        return ASCII_END_OF_MEDIUM;
    }
}

// Terminologie:
// - "Paket": eine Payload die z.B. von einem nRF24L01 übertragen wird. Besitzt einen Typ, eine ID, ggf. eine Länge und Daten.
// - "Paketserie": nachfolgende Pakete mit dem gleichen Typ und aufsteigender ID, die eine Datenstruktur übertragen.

// Paketformat:
// +-----+----+--------+---------------------------+
// | Typ | ID | [Länge] | Data (29 / 30 Byte)     |
// +-----+----+--------+---------------------------+
//
// Beim Startpaket (mit der ID = 0) wird im dritten Byte die Anzahl an zu erwartenden Paketen
// (inklusive des Startpakets) der Paketserie gespeichert. In diesem Fall gibt es 29 Datenbytes. Bei allen nachfolgenden
// Paketen (mit ID >= 1) wird diese Information weggelassen. Dann gibt es 30 Datenbytes.

// Typ:
// - 0x00: Aufforderung an den Empfänger den Puffer der Shell sowie das nächste Prompt zu senden
// - 0x01: Shellbefehl
// - 0x02: sendet den Puffer der Shell sowie das nächste Prompt

uint8_t promptArr[] = "prompt: ";

int packetSeriesType = 0;
int packetSeriesLastID = 0;

// Überträgt ein Byte einer Paketserie
void transmitByteInSeries(nRF24L01Device *dev, uint8_t data) {
    dev->sendBuffer[dev->sendBufferSize] = data;
    dev->sendBufferSize++;

    if (dev->sendBufferSize >= dev->payloadSize) {
        sendBuffer(dev);
        transmitByteInSeries(dev, packetSeriesType);
        packetSeriesLastID++;
        transmitByteInSeries(dev, packetSeriesLastID);
    }
}

// Startet eine Paketserie
void beginPacketSeries(nRF24L01Device *dev, uint8_t type, uint16_t numBytes) {
    resetBuffer(dev);
    transmitByteInSeries(dev, type);
    transmitByteInSeries(dev, 0);
    packetSeriesType = type;
    packetSeriesLastID = 0;

    // das Startpaket kann bis zu 29 Datenbytes enthalten, alle nachfolgenen bis zu 30
    // um die Rechnung zu vereinfachen wird die kürzere Länge des Startpakets ignoriert, dafür aber ein Byte auf die
    // Anzahl an Bytes addiert
    numBytes++;

    // siehe https://stackoverflow.com/a/503201/17278981
    uint16_t numPackets = (numBytes - 1) / 30 + 1;
    transmitByteInSeries(dev, numBytes);
}

uint8_t cmdBuffer[64];
void processUserRequest(uint8_t typ, nRF24L01Device *dev) {
    if (typ == 0x0) {
        uint16_t numBytes = 0;
        // am Anfang stehen zwei Byte mit der Länge des Puffers der Shell und der Länge des Prompts
        numBytes += 2;
        // danach folgt das Prompt
        numBytes += strlen(promptArr);
        // und dann der Puffer der Shell
        numBytes += count;

        beginPacketSeries(dev, 0x02, numBytes);

        transmitByteInSeries(dev, count);
        transmitByteInSeries(dev, strlen(promptArr));

        for (int i = 0; i < strlen(promptArr); i++) {
            transmitByteInSeries(dev, promptArr[i]);
        }

        // kein Nullzeichen zwischen Prompt und Puffer, da die Länge aus den ersten zwei Bytes bekannt ist
        uint8_t data;
        while ((data = readFromBuffer()) != ASCII_END_OF_MEDIUM) {
            transmitByteInSeries(dev, data);
        }

        sendBuffer(dev);
        delay(2);
        setReceiver(dev);
    } else if (typ == 0x1) {
        if (strcmp(cmdBuffer, "ping") == 0) {
            addBuffer("pong");
        }
    }
}

uint8_t cmdBufferIdx = 0;
uint8_t packetsLeft = 0;
bool packetsInit = false;
int prevPacketType = 0;
int prevPacketID = 0;
int expectedPackets = 0;

void readPacket(nRF24L01Device *dev) {
    int thisPacketType = dev->buffer[0];
    int thisPacketID = dev->buffer[1];
    bool firstPacketInSeries = false;

    if (thisPacketType != prevPacketType) {
        // eine neue Paketserie wurde gestartet, dieses Paket muss die ID 0 haben
        prevPacketID = -1;
        prevPacketType = thisPacketType;
        firstPacketInSeries = true;
    }

    if (thisPacketID != prevPacketID + 1) {
        // das vorherige Paket ist nicht angekommen: die prevPacketID auf -1 setzen,
        // damit erst bei einem Startpaket (also ID = 0) weitergemacht wird
    } else {
        prevPacketID = thisPacketID;
        int startPos = 2;
        if (firstPacketInSeries) {
            startPos = 3;
            expectedPackets = dev->buffer[2];
        }

        for (int i = startPos; i < 32; i++) {
            cmdBuffer[cmdBufferIdx] = dev->buffer[i];
            cmdBufferIdx++;
        }

        if (thisPacketID == expectedPackets - 1) {
            // diese Paketserie ist komplett
            prevPacketID = -1;
            prevPacketType = -1;
            processUserRequest(thisPacketID, dev);
            cmdBufferIdx = 0;
        }
    }
}

void myTaskA() {
    bno055Device bnoDev = initBNO055(0x28, 1);

    nRF24L01Device dev;
    dev = initDevice(1, A9, A8);

    delay(50);
    setReceiver(&dev);
    while (1) {
        bool dataAvail = hasData(&dev);
        if (dataAvail) {
            loadDataFromReceiver(&dev);
            clearInterrupts(&dev);

            readPacket(&dev);
        }
        delay(2);
    }
}

void start() {
    addTask(myTaskA, 0);
}
