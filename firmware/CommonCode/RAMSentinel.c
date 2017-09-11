//
//  RAM Sentinel
//
//  How it works:
//      Defines a byte of storage and initializes it to a particular bit pattern.
//      when RAMSentinel_checkIntegrity() is called it checks to see if the storage
//      still has the pattern. If the stack overflowed it will likely have a different
//      value.
//

#include "RAMSentinel.h"

#define SENTINEL_VALUE 0xAA

static uint8_t sentinel;

void RAMSentinel_Initialize (void)
{
    sentinel = SENTINEL_VALUE;
}

extern bool RAMSentinel_sentinelIntact (void)
{
    if (sentinel == SENTINEL_VALUE) {
        return true;
    } else {
        // reset sentinel
        sentinel = SENTINEL_VALUE;
        return false;
    }
}

