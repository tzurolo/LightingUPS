//
// Command processor
//
// Interprets and executes commands from the console
//

#include "CommandProcessor.h"

#include <stdlib.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include "SystemTime.h"
#include "Console.h"
#include "CharString.h"
#include "StringUtils.h"
#include "EEPROM.h"
#include "EEPROMStorage.h"
#include "StatusIndicators.h"
#include "PowerCommand.h"

#define CMD_TOKEN_BUFFER_LEN 20

char swver[] PROGMEM = "V2.0";

static const char tokenDelimiters[] = " \n\r";

typedef enum Reply_enum {
    r_none,
    r_ok,
    r_error
} Reply;

void CommandProcessor_processCommand (
    const char* command)
{
#if 0
    char msgbuf[80];
    sprintf(msgbuf, "cmd: '%s'", command);
    Console_print(msgbuf);
#endif
    Reply reply = r_none;
    char cmdTokenBuf[CMD_TOKEN_BUFFER_LEN];
    strncpy(cmdTokenBuf, command, CMD_TOKEN_BUFFER_LEN-1);
    cmdTokenBuf[CMD_TOKEN_BUFFER_LEN-1] = 0;
    const char* cmdToken = strtok(cmdTokenBuf, tokenDelimiters);
    if (cmdToken != NULL) {
	if (strcasecmp_P(cmdToken, PSTR("status")) == 0) {
            StatusIndicators_sendStatusMesssage();
        } else if (strcasecmp_P(cmdToken, PSTR("leds")) == 0) {
            cmdToken = strtok(NULL, tokenDelimiters);
            if (cmdToken != NULL) {
                reply = r_ok;
                if (strcasecmp_P(cmdToken, PSTR("on")) == 0) {
                    PowerCommand_turnOn();
                } else if (strcasecmp_P(cmdToken, PSTR("off")) == 0) {
                    PowerCommand_turnOff(AUTO_ON_LOCKOUT_TIME);
                } else {
                    reply = r_error;
                }
            } else {
                reply = r_error;
            }
        } else if (strcasecmp_P(cmdToken, PSTR("settings")) == 0) {
            CharString_define(16, settingStr);
            Console_printP(PSTR("{"));
            CharString_copyP(PSTR("\"ID\":"), &settingStr);
            StringUtils_appendDecimal(EEPROMStorage_deviceID, 0, &settingStr);
            Console_printCS(&settingStr);
            CharString_copyP(PSTR(",\"Mode\":\""), &settingStr);
            CharString_appendC((char)EEPROMStorage_mode, &settingStr);
            Console_printCS(&settingStr);
            CharString_copyP(PSTR("\",\"Dark\":"), &settingStr);
            StringUtils_appendDecimal(EEPROMStorage_darkLevel, 0, &settingStr);
            Console_printCS(&settingStr);
            CharString_copyP(PSTR(",\"Auto\":"), &settingStr);
            StringUtils_appendDecimal(EEPROMStorage_autoTime, 0, &settingStr);
            Console_printCS(&settingStr);
            CharString_copyP(PSTR(",\"Manual\":"), &settingStr);
            StringUtils_appendDecimal(EEPROMStorage_manualTime, 0, &settingStr);
            Console_printCS(&settingStr);
            CharString_copyP(PSTR("}"), &settingStr);
            Console_printLineCS(&settingStr);
        } else if (strcasecmp_P(cmdToken, PSTR("set")) == 0) {
            cmdToken = strtok(NULL, tokenDelimiters);
            if (cmdToken != NULL) {
                reply = r_ok;
                if (strcasecmp_P(cmdToken, PSTR("id")) == 0) {
                    cmdToken = strtok(NULL, tokenDelimiters);
                    if (cmdToken != NULL) {
                        const unsigned int deviceID = atoi(cmdToken);
                        EEPROMStorage_setDeviceId((uint8_t)deviceID);
                    } else {
                        reply = r_error;
                    }
                } else if (strcasecmp_P(cmdToken, PSTR("mode")) == 0) {
                    cmdToken = strtok(NULL, tokenDelimiters);
                    if (cmdToken != NULL) {
                        const char mode = cmdToken[0];
                        if ((mode == 'P') ||
                            (mode == 'B') ||
                            (mode == 'S')) {
                            EEPROMStorage_setMode((uint8_t)mode);
                        } else {
                            reply = r_error;
                        }
                    } else {
                        reply = r_error;
                    }
                } else if (strcasecmp_P(cmdToken, PSTR("dark")) == 0) {
                    cmdToken = strtok(NULL, tokenDelimiters);
                    if (cmdToken != NULL) {
                        const unsigned int darkLevel = atoi(cmdToken);
                        EEPROMStorage_setDarkLevel((uint8_t)darkLevel);
                    } else {
                        reply = r_error;
                    }
                } else if (strcasecmp_P(cmdToken, PSTR("auto")) == 0) {
                    cmdToken = strtok(NULL, tokenDelimiters);
                    if (cmdToken != NULL) {
                        const unsigned int autoTimeOn = atoi(cmdToken);
                        EEPROMStorage_setAutoTime((uint16_t)autoTimeOn);
                    } else {
                        reply = r_error;
                    }
                } else if (strcasecmp_P(cmdToken, PSTR("manual")) == 0) {
                    cmdToken = strtok(NULL, tokenDelimiters);
                    if (cmdToken != NULL) {
                        const unsigned int manualTimeOn = atoi(cmdToken);
                        EEPROMStorage_setManualTime((uint16_t)manualTimeOn);
                    } else {
                        reply = r_error;
                    }
                } else if (strcasecmp_P(cmdToken, PSTR("tCalOffset")) == 0) {
                    cmdToken = strtok(NULL, tokenDelimiters);
                    if (cmdToken != NULL) {
                        const int16_t tempCalOffset = atoi(cmdToken);
                        EEPROMStorage_setTempCalOffset(tempCalOffset);
                    } else {
                        reply = r_error;
                    }
                } else {
                    reply = r_error;
                }
            } else {
                reply = r_error;
            }
        } else if (strcasecmp_P(cmdToken, PSTR("get")) == 0) {
            cmdToken = strtok(NULL, tokenDelimiters);
            if (cmdToken != NULL) {
                if (strcasecmp_P(cmdToken, PSTR("tCalOffset")) == 0) {
                    CharString_define(16, offsetStr);
                    CharString_copyP(PSTR("tCalOffset: "), &offsetStr);
                    StringUtils_appendDecimal(EEPROMStorage_tempCalOffset, 0, &offsetStr);
                    Console_printLineCS(&offsetStr);
                } else {
                    reply = r_error;
                }
            } else {
                reply = r_error;
            }
        } else if (strcasecmp_P(cmdToken, PSTR("echo")) == 0) {
            cmdToken = strtok(NULL, tokenDelimiters);
            if (cmdToken != NULL) {
                reply = r_ok;
                if (strcasecmp_P(cmdToken, PSTR("on")) == 0) {
                    Console_setEcho(true);
                } else if (strcasecmp_P(cmdToken, PSTR("off")) == 0) {
                    Console_setEcho(false);
                } else {
                    reply = r_error;
                }
            } else {
                reply = r_error;
            }
        } else if (strcasecmp_P(cmdToken, PSTR("ver")) == 0) {
            Console_printLineP(swver);
        } else {
            reply = r_error;
        }
    } else {
        reply = r_ok;
    }
    switch (reply) {
        case r_none :
            break;
        case r_ok :
            Console_printLineP(PSTR("OK"));
            break;
        case r_error :
            Console_printLineP(PSTR("ERROR"));
            break;
    }
}
