//
//  RAM Sentinel
//
//  What it does:
//     Detects when stack has overflowed down into RAM area.
//     This module should be last in the list of objects so the linker
//     places its memory at the end of RAM
//

#ifndef RAMSENTINEL_H
#define RAMSENTINEL_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>

extern void RAMSentinel_Initialize (void);

// returns true if the sentinel has not been trampled by the stack
// Resets it if it has.
extern bool RAMSentinel_sentinelIntact (void);

#endif      /* RAMSENTINEL_H */
