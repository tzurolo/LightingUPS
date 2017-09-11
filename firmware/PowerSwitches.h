//
//  Power Switches
//
//  Controls the FETs that route power from the mains adapter or
//  battery to the load. It also is responsible for disconnecting
//  the load (turning off both FETs) when the battery voltage
//  drops below 11 volts, and not reconnecting the load until
//  the battery voltage is charged back up to 12 volts.
//
#ifndef POWERSWITCHES_H
#define POWERSWITCHES_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    pss_initial,
    pss_undervoltage,
    pss_undervoltageWarning,
    pss_onBattery,
    pss_onAdapter
} PowerSwitches_state;

extern void PowerSwitches_Initialize (void);

extern void PowerSwitches_task (void);

extern void PowerSwitches_command (
    const bool powerOn);

extern PowerSwitches_state PowerSwitches_currentState (void);

#endif      // POWERSWITCHES_H