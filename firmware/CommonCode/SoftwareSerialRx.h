//
//  Software Serial - Receiver
//
//  What it does:
//      Software implementation of UART Receiver
//      Currently hardcoded to listen for 8N1 on pin B6 at 300 baud
//      Has a 32-byte queue.
//
//  How to use it:
//     Call SoftwareSerialRx_Initialize() once at the beginning of the
//     program (upon powerup).
//     During the program you can get the byte queue using
//     SoftwareSerial_rxQueue() to check for and read incoming bytes.
//
//  Hardware resources used:
//    Serial data in - PA5
//    AtTiny84 16-bit Timer 1
//
#ifndef SOFTWARESERIALRX_H
#define SOFTWARESERIALRX_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include "ByteQueue.h"

// comes up enabled by default
extern void SoftwareSerialRx_Initialize (void);

extern void SoftwareSerialRx_enable (void);
extern void SoftwareSerialRx_disable (void);

extern ByteQueue* SoftwareSerial_rxQueue (void);

#endif  /* SOFTWARESERIALRX_H */

