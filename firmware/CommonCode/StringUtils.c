//
//  String
//

#include "StringUtils.h"

#include <stdio.h>
#include <stdlib.h>

// scans for " and then puts everything up to the next " in
// quotedString. returns the updated source ptr
const char* StringUtils_scanQuotedString (
    const char* sourcePtr,
    CharString_t* quotedString)
{
    CharString_clear(quotedString);

    // advance to the next "
    while ((*sourcePtr != 0) && (*sourcePtr != '\"')) {
        ++sourcePtr;
    }
    if (*sourcePtr == '\"') {
        // step over opening "
        ++sourcePtr;
    }

    // read characters between quotes
    while ((*sourcePtr != 0) && (*sourcePtr != '\"')) {
        CharString_appendC(*sourcePtr, quotedString);
        ++sourcePtr;
    }

    if (*sourcePtr == '\"') {
        // step over closing "
        ++sourcePtr;
    }
    
    return sourcePtr;
}

void StringUtils_appendDecimal (
    const int16_t value,
    const uint8_t numDecimalDigits,
    CharString_t* destStr)
{
    char decimalValueBuffer[8];
    uint16_t workingValue;
    if (value < 0) {
        CharString_appendC('-', destStr);
        workingValue = -value;
    } else {
        workingValue = value;
    }
    decimalValueBuffer[numDecimalDigits] = 0;  // null-terminate
    for (int d = numDecimalDigits; d > 0; --d) {
        decimalValueBuffer[d-1] = (workingValue % 10) + '0';
        workingValue /= 10;
    }
    char valueBuffer[8];
    itoa(workingValue, valueBuffer, 10);
    CharString_append(valueBuffer, destStr);
    if (numDecimalDigits > 0) {
        CharString_appendC('.', destStr);
        CharString_append(decimalValueBuffer, destStr);
    }
}

