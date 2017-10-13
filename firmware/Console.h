//
//  Console
//
//  This unit responds to characters from the serial port and
//  sends replies
//
#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>
#include <stdbool.h>
#include <avr/pgmspace.h>
#include "CharString.h"

// sets up control pins. called once at power-up
extern void Console_Initialize (void);

// reads commands from FromUSB_Buffer and writes responses to
// ToUSB_Buffer.
// called in each iteration of the mainloop
extern void Console_task (void);

extern void Console_setEcho (
    const bool newEcho);

extern void Console_print (
    const char* text);

extern void Console_printLine (
    const char* text);

extern void Console_printCS (
    const CharString_t* text);

extern void Console_printLineCS (
    const CharString_t* text);

extern void Console_printNewline (void);

extern void Console_printP (
    PGM_P text);

extern void Console_printLineP (
    PGM_P text);

#endif  // Console_H
