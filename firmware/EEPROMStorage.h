//
// EEPROM Storage
//
// Storage of non-volatile settings and data
//

#ifndef EEPROMSTORAGE_H
#define EEPROMSTORAGE_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>

#include "EEPROM.h"

// storage address map
// 0 is initialization flag. Unprogrammed EE comes up as all one's
#define ADDR_ID 1
#define LENGTH_ID 1
#define ADDR_MODE 2
#define LENGTH_MODE 1
#define ADDR_ECHO 3
#define LENGTH_ECHO 1
#define ADDR_DARK_LEVEL 4
#define LENGTH_DARK_LEVEL 1
#define ADDR_AUTO_TIME_ON 5
#define LENGTH_AUTO_TIME_ON 2
#define ADDR_MANUAL_TIME_ON 7
#define LENGTH_MANUAL_TIME_ON 2

extern void EEPROMStorage_Initialize (void);

// device ID
#define EEPROMStorage_setDeviceId(id) EEPROM_write(ADDR_ID, id)
#define EEPROMStorage_deviceID (EEPROM_read(ADDR_ID))

// mode 'P', 'B', 'S'
#define EEPROMStorage_setMode(mode) EEPROM_write(ADDR_MODE, mode)
#define EEPROMStorage_mode (EEPROM_read(ADDR_MODE))

// console echo state
#define EEPROMStorage_setEcho(echo) EEPROM_write(ADDR_ECHO, echo ? 1 : 0)
#define EEPROMStorage_echo (EEPROM_read(ADDR_ECHO) == 1)

// level representing a dark room
#define EEPROMStorage_setDarkLevel(darkLevel)  EEPROM_write(ADDR_DARK_LEVEL, darkLevel)
#define EEPROMStorage_darkLevel EEPROM_read(ADDR_DARK_LEVEL)

// length of time to keep LED lights on when they came on automatically
#define EEPROMStorage_setAutoTime(autoTimeOn) EEPROM_writeWord(ADDR_AUTO_TIME_ON, autoTimeOn)
#define EEPROMStorage_autoTime EEPROM_readWord(ADDR_AUTO_TIME_ON)

// max length of time to for LED lights to stay on when turned on manually
#define EEPROMStorage_setManualTime(manualTimeOn) EEPROM_writeWord(ADDR_MANUAL_TIME_ON, manualTimeOn)
#define EEPROMStorage_manualTime EEPROM_readWord(ADDR_MANUAL_TIME_ON)

#endif		// EEPROMSTORAGE