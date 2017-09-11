//
//  OneWire interface
//
//  Implements Maxim OneWire device interface
//
#ifndef ONEWIRE_H
#define ONEWIRE_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>

// resets one-wire bus - returns true if successful
extern bool OneWire_reset (void);

extern void OneWire_writeByte (
    const uint8_t byte);

extern uint8_t OneWire_readByte (void);

extern bool OneWire_readBit (void);

#endif  // ONEWIRE_H
