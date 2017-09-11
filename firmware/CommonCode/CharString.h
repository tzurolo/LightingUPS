//
//  Character String
//
//  Provides fixed-capacity, variable-length string type, with overrun protection
//  Strings can have up to 255 char capacity
//
#ifndef CHARSTRING_H
#define CHARSTRING_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include <avr/pgmspace.h>

typedef struct CharString_struct {
    uint8_t capacity;
    uint8_t length;
    char* body;
} CharString_t;

#define CharString_define(strCapacity, strName) \
    char strName##_buf[strCapacity+1] = {0}; \
    CharString_t strName = {strCapacity, 0, strName##_buf};

inline bool CharString_isEmpty (
    const CharString_t* str)
{
    return str->length == 0;
}

inline uint8_t CharString_length (
    const CharString_t* str)
{
    return str->length;
}

inline void CharString_clear (
    CharString_t* str)
{
    str->length = 0;
    str->body[0] = 0;
}

inline const char* CharString_cstr (
    const CharString_t* str)
{
    return str->body;
}

extern void CharString_append (
    const char* srcStr,
    CharString_t* destStr);

extern void CharString_appendP (
    PGM_P srcStr,
    CharString_t* destStr);

// safely appends string (protects destination against overflow)
extern void CharString_appendCS (
    const CharString_t* srcStr,
    CharString_t* destStr);

extern void CharString_appendC (
    const char ch,
    CharString_t* destStr);

inline void CharString_copy (
    const char* srcStr,
    CharString_t* destStr)
{
    CharString_clear(destStr);
    CharString_append(srcStr, destStr);
}

inline void CharString_copyP (
    PGM_P srcStr,
    CharString_t* destStr)
{
    CharString_clear(destStr);
    CharString_appendP(srcStr, destStr);
}

inline void CharString_copyCS (
    CharString_t* srcStr,
    CharString_t* destStr)
{
    CharString_clear(destStr);
    CharString_appendCS(srcStr, destStr);
}

extern void CharString_appendNewline (
    CharString_t* destStr);

extern void CharString_truncate (
    const uint8_t truncatedLength,
    CharString_t* destStr);

inline int CharString_equalsP (
    const CharString_t* leftStr,
    PGM_P rightStr)
{
    return strcmp_P(leftStr->body, rightStr) == 0;
}

inline bool CharString_startsWithP (
    const CharString_t* str,
    PGM_P prefix)
{
    return (strstr_P(str->body, prefix) == str->body);
}

inline char CharString_at (
    const CharString_t* str,
    const int offset)
{
    return str->body[offset];
}

inline const char* CharString_right (
    const CharString_t* str,
    const int offset)
{
    return str->body + offset;
}

inline char* CharString_buffer (
    CharString_t* str)
{
    return str->body;
}

#endif  // CHARSTRING_H