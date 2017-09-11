//
//  String
//

#include "CharString.h"

void CharString_append (
    const char* srcStr,
    CharString_t* destStr)
{
    const uint8_t capacity = destStr->capacity;
    const uint8_t srcStrLen = strlen(srcStr);
    const uint8_t remainingCapacity = capacity - destStr->length;
    const uint8_t charsToAppend =
        (remainingCapacity < srcStrLen)
        ? remainingCapacity
        : srcStrLen;
    strncpy(destStr->body + destStr->length, srcStr, charsToAppend);
    destStr->length += charsToAppend;
    destStr->body[destStr->length] = 0;
}

void CharString_appendP (
    PGM_P srcStr,
    CharString_t* destStr)
{
    const uint8_t capacity = destStr->capacity;
    const uint8_t srcStrLen = strlen_P(srcStr);
    const uint8_t remainingCapacity = capacity - destStr->length;
    const uint8_t charsToAppend =
        (remainingCapacity < srcStrLen)
        ? remainingCapacity
        : srcStrLen;
    strncpy_P(destStr->body + destStr->length, srcStr, charsToAppend);
    destStr->length += charsToAppend;
    destStr->body[destStr->length] = 0;
}

void CharString_appendCS (
    const CharString_t* srcStr,
    CharString_t* destStr)
{
    CharString_append(CharString_cstr(srcStr), destStr);
}

void CharString_appendC (
    const char ch,
    CharString_t* destStr)
{
    if (destStr->length < destStr->capacity) {
        char* cp = destStr->body + destStr->length;
        *cp++ = ch;
        *cp = 0;
        ++destStr->length;
    }
}

void CharString_appendNewline (
    CharString_t* destStr)
{
    CharString_appendP(PSTR("\r\n"), destStr);
}

void CharString_truncate (
    const uint8_t truncatedLength,
    CharString_t* destStr)
{
    if (truncatedLength < destStr->length) {
        destStr->length = truncatedLength;
        destStr->body[truncatedLength] = 0;
    }
}
