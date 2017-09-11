//
//  Console interface
//
//  How it works:
//     Collects incoming characters from the UART until a cr is received
//     and then passes the string to the command processor.
//     Puts message strings out to the UART
//
//  I/O Pin assignments
//
#include "Console.h"

#include "SystemTime.h"
#include "SoftwareSerialRx.h"
#include "SoftwareSerialTx.h"
#include "CommandProcessor.h"
#include "EEPROMStorage.h"
#include <avr/io.h>
#include <avr/pgmspace.h>

// commands
#define cmdOK       '?'

#define commandBufferSize 20

// state variables
static char commandBuffer[commandBufferSize+1];

void Console_Initialize (void)
{
    commandBuffer[0] = 0;
}

void Console_task (void)
{
    ByteQueue *rxQueue = SoftwareSerial_rxQueue();
    if (!ByteQueue_is_empty(rxQueue)) {
        const char cmdByte = (char)ByteQueue_pop(rxQueue);
        switch (cmdByte) {
            case '\r' : {
            Console_printNewline();
                // command complete. execute it
                CommandProcessor_processCommand(commandBuffer);

                commandBuffer[0] = 0;
                }
                break;
            case 0x12 : {
                if (EEPROMStorage_echo) {
                    Console_printP(PSTR("\r\033[K"));   // carriage return, erase to end of line
                    Console_print(commandBuffer);       // reprint entire line
                }
                }
                break;
            case 0x7f : {
                // delete last char
                const size_t cmdBufLen = strlen(commandBuffer);
                if (cmdBufLen > 0) {
                    commandBuffer[cmdBufLen - 1] = 0;
                    if (EEPROMStorage_echo) {
                        Console_printP(PSTR("\033[D \033[D"));   // backspace, blank, backspace
                    }
                }
                }
                break;
            default : {
                // command not complete yet. append to command buffer
                const size_t cmdBufLen = strlen(commandBuffer);
                if (cmdBufLen < commandBufferSize) {
                    commandBuffer[cmdBufLen] = cmdByte;
                    commandBuffer[cmdBufLen+1] = 0;
                    if (EEPROMStorage_echo) {
                        Console_print(&commandBuffer[cmdBufLen]);
                    }
                }
                }
                break;
        }
    }
}

void Console_setEcho (
    const bool newEcho)
{
    EEPROMStorage_setEcho(newEcho);
}

void Console_print (
	const char* text)
{
    // not checking if write was successful.
    // will need to revisit this. Maybe console has
    // a buffer to hold text that didn't get written.
    // Would need a console task to push it out.
    SoftwareSerialTx_send(text);
}

void Console_printLine (
	const char* text)
{
    Console_print(text);
    Console_printNewline();
}

void Console_printLineCS (
	const CharString_t* text)
{
    Console_print(CharString_cstr(text));
    Console_printNewline();
}

void Console_printNewline (void)
{
    Console_printP(PSTR("\r\n"));
}

void Console_printP (
	PGM_P text)
{
    // not checking if write was successful.
    // will need to revisit this. Maybe console has
    // a buffer to hold text that didn't get written.
    // Would need a console task to push it out.
    SoftwareSerialTx_sendP(text);
}

void Console_printLineP (
	PGM_P text)
{
    Console_printP(text);
    Console_printNewline();
}
