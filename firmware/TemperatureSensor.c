//
//  Water tank water level sensor
//
//
//  I/O Pin assignments
//      PA6  - DS18B20 data pin
//
#include "TemperatureSensor.h"

#include <avr/io.h>
#include <avr/pgmspace.h>
#include "SystemTime.h"
#include "OneWire.h"
#include "crc8.h"
#include "StringUtils.h"

// controller states
typedef enum TemperatureSensorState_enum {
    ts_idle,
    ts_waitingForConversionComplete
} TemperatureSensorState;
// state variables
static TemperatureSensorState tsState = ts_idle;
static TemperatureSensor_measurementStatus latestStatus;
static int16_t latestTemp;

void TemperatureSensor_Initialize (void)
{
    tsState = ts_idle;
    latestStatus = ms_notStarted;
    latestTemp = 0;
}

bool TemperatureSensor_isIdle (void)
{
    return tsState == ts_idle;
}

bool TemperatureSensor_startMeasurement (void)
{
    bool started = false;

    if (tsState == ts_idle) {
        if (OneWire_reset()) {
            OneWire_writeByte(0xCC);    // skip ROM
            OneWire_writeByte(0x44);    // convert T
            tsState = ts_waitingForConversionComplete;
            latestStatus = ms_inProgress;
            started = true;
        } else {
            // reset failed
            latestStatus = ms_error;
        }
    }

    return started;
}

TemperatureSensor_measurementStatus TemperatureSensor_getMeasurement (
    int16_t *temp)
{
    if (latestStatus == ms_completedSuccessfully) {
        *temp = latestTemp;
    }
    return latestStatus;
}

void TemperatureSensor_task (void)
{
    if (tsState == ts_waitingForConversionComplete) {
        if (OneWire_readBit()) {
            // conversion is complete
            if (OneWire_reset()) {
                uint8_t scratchpadData[9];
                OneWire_writeByte(0xCC);    // skip ROM
                OneWire_writeByte(0xBE);    // read scratchpad
                for (int b = 0; b < 9; ++b) {
                    scratchpadData[b] = OneWire_readByte();
                }
                const int16_t newTemp =
                    scratchpadData[0] |
                    (((int16_t)scratchpadData[1]) << 8);
                const uint8_t crc = crc8(scratchpadData, 8);
                if (crc == scratchpadData[8]) {
                    // skip power-on reset scratchpad contents that reads 85 C
			        if (newTemp != 0x0550) {
				        latestTemp = newTemp;
                    }
                    latestStatus = ms_completedSuccessfully;
                } else {
                    // crc check failed
                    latestStatus = ms_error;
                }
            } else {
                // reset failed
                latestStatus = ms_error;
            }
            tsState = ts_idle;
        }
    }
}

void TemperatureSensor_appendTempToString (
    const int16_t temp,
    CharString_t* string)
{
    // convert 1/16 degree C to 1/10 degree C
    int16_t degF = (temp * 10) / 16;
    StringUtils_appendDecimal(degF, 1, string);
}
