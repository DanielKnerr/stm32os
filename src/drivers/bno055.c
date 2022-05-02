#include "../../include/drivers/bno055.h"
#include "../../include/drivers/i2c.h"
#include "../../include/memory.h"
#include "../../include/syscall.h"

void writeReg(uint8_t addr, uint8_t reg, uint8_t value) {
    startCondition();
    writeAddress(addr);
    writeByte(reg);
    writeByte(value);
    stopCondition();
}

void readReg(bno055Device *dev, uint8_t startReg, uint8_t size) {
    startCondition();
    writeAddress(dev->writeAddress);
    writeByte(startReg);
    startCondition();
    i2cReadBytes(dev->readAddress, size, dev->i2cBuffer);
    stopCondition();
}

#define BNO055_OPR_MODE 0x3D
#define BNO055_UNIT_SEL 0x3B
#define BNO055_TEMP 0x34
#define BNO055_CALIB_STAT 0x35
#define BNO055_SYS_STATUS 0x39
#define BNO055_TEMP_SOURCE 0x40

#define BNO055_EUL_DATA_Z_LSB 0x1E
#define BNO055_EUL_DATA_Z_MSB 0x1F
#define BNO055_EUL_DATA_START 0x1A

bno055Device initBNO055(uint8_t rawAddress, uint8_t i2cBus) {
    initI2C1();

    bno055Device dev;
    dev.writeAddress = rawAddress << 1;
    dev.readAddress = (rawAddress << 1) | 0b1;
    dev.i2cBus = i2cBus;
    dev.i2cBuffer = allocate(32);

    // in den CONFIGMODE wechseln (hilfreich beim Debugging, falls das Gerät bereits konfiguriert wurde)
    writeReg(dev.writeAddress, BNO055_OPR_MODE, 0x00);
    delay(20);

    // by setting UNIT_SEL to 0x00, the chip uses the following units:
    // UNIT_SEL = 0x00 wählt folgende Einheiten aus:
    // - m/s² für Beschleunigung and lineare Beschleunigung
    // - dps ür Winkelgeschwindigkeit
    // - Grad für Eulersche Winkel
    // - Celsius für Temperatur
    // - Wertebereich der Winkel im Modus "windows orientation"
    writeReg(dev.writeAddress, BNO055_UNIT_SEL, 0x00);

    // der Beschleunigungssensor scheint die genaueren Temperaturwerte zu liefern
    writeReg(dev.writeAddress, BNO055_TEMP_SOURCE, 0b00);

    // den "NDOF Fusion Mode" auswählen
    writeReg(dev.writeAddress, BNO055_OPR_MODE, 0b1100);
    delay(20);

    // warten bis der System Status "Sensor fusion algorithm running" ist
    while (1) {
        readReg(&dev, BNO055_SYS_STATUS, 1);
        if (dev.i2cBuffer[0] == 5) break;
    }

    // den Sensor sich selbst kalibrieren lassen
    delay(500);

    return dev;
}

EulerAngles getEulerAngles(bno055Device *dev) {
    // lese 6 Bytes ab dem LSB von X
    readReg(dev, BNO055_EUL_DATA_START, 6);

    // die 6 Bytes bestehen aus drei Paaren mit je zwei Byte, welche einen signed Integer beschreiben
    // die so berechneten Ganzzahl-Werte müssen durch 16 geteilt werden um den tatächlichen Winkel zu berechnen (siehe Datasheet Table 3-28)
    EulerAngles ret;
    ret.x = dev->i2cBuffer[1] << 8 | dev->i2cBuffer[0];
    ret.y = dev->i2cBuffer[3] << 8 | dev->i2cBuffer[2];
    ret.z = dev->i2cBuffer[5] << 8 | dev->i2cBuffer[4];

    return ret;
}

uint8_t getTempCelsius(bno055Device *dev) {
    // wenn Fahrenheit als Einheit gewählt wäre, müsste man hier den Wert mit 2 multiplizieren (Table 3-37)
    readReg(dev, BNO055_TEMP, 1);
    return dev->i2cBuffer[0];
}
