//
//  Software Serial Transmit
//
//   Uses SystemTime tick as clock.
//
//  Pin usage:
//      PA6 - serial data out (MOSI)
//

#include "SoftwareSerialTx.h"

#include <avr/io.h>
#include "ByteQueue.h"
#include "../SystemTime.h"

#define SERIAL_TX_DDR      DDRA
#define SERIAL_TX_PORT     PORTA
#define SERIAL_TX_PIN      PA6

typedef enum TxState_enum {
    ts_idle,
    ts_sendingDataBits,
    ts_sendingStopBit
} TxState;
// state variables
static bool isEnabled;
static SystemTime_tick lastTick;
static TxState txState = ts_idle;
static ByteQueueElement dataByte;
static uint8_t bitNumber;
ByteQueue_define(70, txQueue);

static void setTxBit (
    const uint8_t bit)
{
    if (bit != 0) {
        SERIAL_TX_PORT |= (1 << SERIAL_TX_PIN);
    } else {
        SERIAL_TX_PORT &= ~(1 << SERIAL_TX_PIN);
    }
}

static void systemTimeTickTask (void)
{
    switch (txState) {
    case ts_idle:
        if (!ByteQueue_is_empty(&txQueue)) {
            // issue start bit
            setTxBit(0);
            dataByte = ByteQueue_pop(&txQueue);
            bitNumber = 1;
            txState = ts_sendingDataBits;
        }
        break;
    case ts_sendingDataBits:
        setTxBit(dataByte & 1);
        if (bitNumber == 8) {
            txState = ts_sendingStopBit;
        } else {
            ++bitNumber;
            dataByte >>= 1;
        }
        break;
    case ts_sendingStopBit:
        setTxBit(1);
        txState = ts_idle;
        break;
    }
}

void SoftwareSerialTx_Initialize (void)
{
    // set serial tx as output
    SERIAL_TX_DDR      |= (1 << SERIAL_TX_PIN);

    isEnabled = false;
    lastTick = 0;
    txState = ts_idle;

    SystemTime_registerForTickNotification(systemTimeTickTask);
}

void SoftwareSerialTx_enable (void)
{
    isEnabled = true;
}

void SoftwareSerialTx_disable (void)
{
    isEnabled = false;
}

bool SoftwareSerialTx_isIdle (void)
{
    return ((txState == ts_idle) && ByteQueue_is_empty(&txQueue));
}

void SoftwareSerialTx_send (
    const char* text)
{
    if (isEnabled) {
        const char* cp = text;
        char ch;
        while ((ch = *cp++) != 0) {
            ByteQueue_push((ByteQueueElement)ch, &txQueue);
        }
    }
}

bool SoftwareSerialTx_sendP (
   PGM_P string)
{
    bool successful = false;

    if (isEnabled) {
        // check if there is enough space left in the tx queue
        if (strlen_P(string) <= ByteQueue_spaceRemaining(&txQueue))
            {  // there is enough space in the queue
            // push all bytes onto the queue
            PGM_P cp = string;
            char ch = 0;
            do {
                ch = pgm_read_byte(cp);
                ++cp;
                if (ch != 0) {
                    ByteQueue_push(ch, &txQueue);
                }
            } while (ch != 0);

            successful = true;
        }
    }

   return successful;
}

void SoftwareSerialTx_sendChar (
    const char ch)
{
    if (isEnabled) {
        ByteQueue_push((ByteQueueElement)ch, &txQueue);
    }
}

