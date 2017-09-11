//
//  String Utilities
//
//  Utility functions for operating on strings, with overrun protection
//
#ifndef StringUtils_H
#define StringUtils_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include <avr/pgmspace.h>
#include "CharString.h"

// scans for " and then puts everything up to the next " in
// quotedString. returns the updated source ptr
extern const char* StringUtils_scanQuotedString (
    const char* sourcePtr,
    CharString_t* quotedString);

// sets destStr to the decimal string for the given value
extern void StringUtils_appendDecimal (
    const int16_t value,
    const uint8_t numDecimalDigits,
    CharString_t* destStr);

#endif  // StringUtils_H