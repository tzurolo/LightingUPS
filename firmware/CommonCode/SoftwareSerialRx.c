//
//  Software Serial Receiver
//
//   Uses AtTiny84 16-bit Timer 1
//
//  Pin usage:
//      PA5 - serial data in
//
//  How it works:
//    Upon initializaation it sets a pin change interrupt on the serial
//    data input pin. When the interrupt fires a timer is used to clock
//    a state machine to read each bit and push the resulting byte into
//    the queue.
//

#include "SoftwareSerialRx.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#define SERIAL_RX_DDR      DDRA
#define SERIAL_RX_PORT     PORTA
#define SERIAL_RX_INPORT   PINA
#define SERIAL_RX_PIN      PA5

// with 1MHz clock and 64 prescale this 1/300 second
#define BIT_CLOCK_TIME 52

typedef enum RxState_enum {
    rs_idle,
    rs_waitingForStartBit,
    rs_readingDataBits,
    rs_waitingForStopBit
} RxState;
// state variables
static volatile bool isEnabled;
static volatile RxState rxState;
static ByteQueueElement dataByte;
static uint8_t bitMask;
ByteQueue_define(16, rxQueue);

static bool rxBit (void)
{
    return (SERIAL_RX_INPORT & (1 << SERIAL_RX_PIN)) != 0;
}

void SoftwareSerialRx_Initialize (void)
{
    // make rx pin an input and enable pullup
    SERIAL_RX_DDR &= (~(1 << SERIAL_RX_PIN));
    SERIAL_RX_PORT |= (1 << SERIAL_RX_PIN);

    // set up timer1 to fire interrupt 300 Hz (baud clock)
    TCCR1A &= 0xFC;                         // set CTC mode (WGM11:10)
    TCCR1B = (TCCR1B & 0xE7) | (1 << 3);    // set CTC mode (WGM13:12)
    TCCR1B = (TCCR1B & 0xF8) | 3; // prescale by 64
    OCR1A = BIT_CLOCK_TIME;  // with 1MHz clock and 64 prescale this is 1/300 second
    TCNT1 = 0;  // start the time counter at 0
    TIFR1 |= (1 << OCF1A);  // "clear" the timer compare flag

    // enable pin change interrupts
    GIMSK |= (1 << PCIE0);

    SoftwareSerialRx_enable();
}

void SoftwareSerialRx_enable (void)
{
    if (!isEnabled) {
        rxState = rs_idle;
        // set up pin change interrupt to detect start bit
        PCMSK0 |= (1 << PCINT5);
        isEnabled = true;
    }
}

void SoftwareSerialRx_disable (void)
{
    if (isEnabled) {
        TIMSK1 &= ~(1 << OCIE1A);   // disable timer compare match interrupt
        PCMSK0 &= ~(1 << PCINT5);
        isEnabled = false;
    }
}

ByteQueue* SoftwareSerial_rxQueue (void)
{
    return &rxQueue;
}

ISR(PCINT0_vect, ISR_BLOCK)
{
    if (!rxBit()) {
        TIFR1 |= (1 << OCF1A);  // "clear" the timer compare flag
        TIMSK1 |= (1 << OCIE1A);// enable timer compare match interrupt
        TCNT1 = 0;  // start the time counter at 0
        // set timer for 1/2 bit time to get in the middle of the start bit
        OCR1A = BIT_CLOCK_TIME / 2;

        // disable this interrupt
        PCMSK0 &= ~(1 << PCINT5);
        rxState = rs_waitingForStartBit;
    }
}

ISR(SIG_OUTPUT_COMPARE1A, ISR_BLOCK)
{
    switch (rxState) {
        case rs_waitingForStartBit :
            if (!rxBit()) {
                // rx bit is low - got start bit
                dataByte = 0;
                bitMask = 1;
                OCR1A = BIT_CLOCK_TIME;
                rxState = rs_readingDataBits;
            } else {
                // expected low for start bit but didn't get it
                TIMSK1 &= ~(1 << OCIE1A);// disable timer compare match interrupt
                // re-enable pin change interrupt
                PCMSK0 |= (1 << PCINT5);
            }
            break;
        case rs_readingDataBits :
            if (rxBit()) {
                dataByte |= bitMask;
            }
            if (bitMask == 0x80) {
                rxState = rs_waitingForStopBit;
            } else {
                bitMask <<= 1;
            }
            break;
        case rs_waitingForStopBit :
            if (rxBit()) {
                // got stop bit.
                ByteQueue_push(dataByte, &rxQueue);
            }
            TIMSK1 &= ~(1 << OCIE1A);// disable timer compare match interrupt
            rxState = rs_idle;

            // re-enable pin change interrupt
            PCMSK0 |= (1 << PCINT5);
            break;
        default :
            break;
    }
}
