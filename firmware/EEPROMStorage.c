//
// EEPROM Storage
//

#include "EEPROMStorage.h"
#include "SystemMode.h"

void EEPROMStorage_Initialize (void)
{
    // check if EE has been initialized
    const uint8_t initFlag = EEPROM_read(0);
    const uint8_t initLevel = (initFlag == 0xFF) ? 0 : initFlag;

    if (initLevel < 1) {
        // EE has not been initialized. Initialize to default settings now.

        EEPROMStorage_setDeviceId(0);       // undefined device id
        EEPROMStorage_setMode(m_switch);    // get mode from mode switch
        EEPROMStorage_setEcho(false);       // echo off
        EEPROMStorage_setDarkLevel(15);     // 15% or less is dark
        EEPROMStorage_setAutoTime(30);      // stay on for 30 minutes
                                            // when it comes on automatically
        EEPROMStorage_setManualTime(360);   // stay on no more than 6 hours
                                            // when turned on manually

        EEPROMStorage_setTempCalOffset(-266);

        // register that EEPROM is initialized
        EEPROM_write(0, 1);
    }
}

