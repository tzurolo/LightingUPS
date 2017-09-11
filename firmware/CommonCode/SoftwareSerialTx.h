//
//  Software Serial Transmit
//
//  Software implementation of UART
//
#ifndef SOFTWARESERIALTX_H
#define SOFTWARESERIALTX_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include <avr/pgmspace.h>

extern void SoftwareSerialTx_Initialize (void);

extern void SoftwareSerialTx_enable (void);
extern void SoftwareSerialTx_disable (void);

extern bool SoftwareSerialTx_isIdle (void);

extern void SoftwareSerialTx_send (
    const char* text);

extern bool SoftwareSerialTx_sendP (
   PGM_P string);

extern void SoftwareSerialTx_sendChar (
    const char ch);

#endif  /* SOFTWARESERIALTX_H */

