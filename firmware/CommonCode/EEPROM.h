//
// EEPROM access
//
#ifndef EEPROM_H
#define EEPROM_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>

extern void EEPROM_write (
    const unsigned int uiAddress,
    const uint8_t ucData);

extern uint8_t EEPROM_read (
    const unsigned int uiAddress);

extern void EEPROM_writeWord (
    const unsigned int uiAddress,
    const uint16_t word);

extern uint16_t EEPROM_readWord (
    const unsigned int uiAddress);

#endif  // EEPROM_H
