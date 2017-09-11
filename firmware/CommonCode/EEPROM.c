//
// EEPROM access
//

#include "EEPROM.h"
#include <avr/io.h>

void EEPROM_write (
    const unsigned int uiAddress,
    const uint8_t ucData)
{
    /* Wait for completion of previous write */
    while(EECR & (1<<EEPE))
        ;
    /* Set up address and Data Registers */
    EEAR = uiAddress;
    EEDR = ucData;
    /* Write logical one to EEMPE */
    EECR |= (1<<EEMPE);
    /* Start eeprom write by setting EEPE */
    EECR |= (1<<EEPE);
}

uint8_t EEPROM_read (
    const unsigned int uiAddress)
{
    /* Wait for completion of previous write */
    while (EECR & (1<<EEPE))
        ;
    /* Set up address register */
    EEAR = uiAddress;
    /* Start eeprom read by writing EERE */
    EECR |= (1<<EERE);
    /* Return data from Data Register */
    return EEDR;
}

void EEPROM_writeWord (
    const unsigned int uiAddress,
    const uint16_t word)
{
    EEPROM_write(uiAddress, word & 0xFF);
    EEPROM_write(uiAddress + 1, (word >> 8) & 0xFF);
}

uint16_t EEPROM_readWord (
    const unsigned int uiAddress)
{
    uint16_t word = 0;
    word = EEPROM_read(uiAddress);
    word |= (EEPROM_read(uiAddress + 1) << 8);

    return word;
}
