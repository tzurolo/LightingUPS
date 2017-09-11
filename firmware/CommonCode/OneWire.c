//
//  OneWire interface
//
//  Implements Maxim OneWire device interface
//
#include "OneWire.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include "util/delay.h"

#define OW_DEVICE_PIN       PA6
#define OW_DEVICE_OUTPORT   PORTA
#define OW_DEVICE_INPORT    PINA
#define OW_DEVICE_DIR       DDRA

// set pin to output
#define OW_SET_OUTPUT() (OW_DEVICE_DIR |= (1<<OW_DEVICE_PIN))
// set pin to input
#define OW_SET_INPUT()  (OW_DEVICE_DIR &= ~(1<<OW_DEVICE_PIN))
// write high to pin
#define OW_WRITE_HIGH() (OW_DEVICE_OUTPORT |= (1<<OW_DEVICE_PIN))
// write low to pin
#define OW_WRITE_LOW()  (OW_DEVICE_OUTPORT &= ~(1<<OW_DEVICE_PIN))
// read pin
#define OW_READ()       ((OW_DEVICE_INPORT & (1<<OW_DEVICE_PIN)) != 0)

bool OneWire_reset (void)
{
    bool successful = false;

    cli();

    OW_WRITE_LOW();
    OW_SET_OUTPUT();

    _delay_us(500);

    OW_SET_INPUT();
    OW_WRITE_HIGH();    // enable pull-up

    _delay_us(64);
    successful = !OW_READ();

    _delay_us(416);

    successful = successful && OW_READ();

    sei();

    return successful;
}

static void write_0 (void)
{
    cli();
    OW_WRITE_LOW();
    OW_SET_OUTPUT();

    _delay_us(63);

    OW_SET_INPUT();
    OW_WRITE_HIGH();    // enable pull-up

    _delay_us(2);      // TREC
    sei();
}

static void write_1 (void)
{
    cli();
    OW_WRITE_LOW();
    OW_SET_OUTPUT();

    _delay_us(3);

    OW_SET_INPUT();
    OW_WRITE_HIGH();    // enable pull-up

    _delay_us(62);
    sei();
}

void OneWire_writeBit (
    const bool bit)
{
    if (bit) {
        write_1();
    } else {
        write_0();
    }
}

void OneWire_writeByte (
    const uint8_t byte)
{
    uint8_t residual = byte;
    for (int b = 0; b < 8; ++b) {
        OneWire_writeBit(residual & 1);
        residual >>= 1;
    }
}

bool OneWire_readBit (void)
{
    bool bit = false;

    cli();
    OW_WRITE_LOW();
    OW_SET_OUTPUT();

    _delay_us(2);

    OW_SET_INPUT();
    OW_WRITE_HIGH();    // enable pull-up

    _delay_us(11);

    bit = OW_READ();

    _delay_us(54);
    sei();

    return bit;
}

uint8_t OneWire_readByte (void)
{
    uint8_t byte = 0;

    for (int b = 0; b < 8; ++b) {
        byte >>= 1;
        if (OneWire_readBit()) {
            byte |= 0x80;
        }
    }

    return byte;
}

