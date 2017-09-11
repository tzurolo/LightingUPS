//
//  Power Command
//
//  Determines whether the LEDs should be powered on or off. This
//  decision is based on the following inputs:
//      The pushbutton - manual on/off
//      The mains monitor and the photocell - automatic on/off
//  When this determination is made it sets the on/off command of
//  the power COMMAND (PowerCOMMAND.h)
//
#ifndef POWERCOMMAND_H
#define POWERCOMMAND_H

#include <stdint.h>
#include <stdbool.h>

// suggested time (in seconds) to re-enable auto-on after manual-off
#define AUTO_ON_LOCKOUT_TIME 20

typedef enum CommandState_enum {
    cs_off,
    cs_onManual,
    cs_waitingForPhotocellAfterMainsOff,
    cs_onAutomatic,
    cs_waitingForPhotocellAfterMainsOn,
    cs_waitingForPhotocellAfterMainsOnUndervoltage
} PowerCommand_CommandState;

extern void PowerCommand_Initialize (void);

extern void PowerCommand_task (void);

extern void PowerCommand_turnOn (void);
extern void PowerCommand_turnOff (
    const uint8_t autoOnLockoutTime);   // don't turn on automatically for given seconds

extern PowerCommand_CommandState PowerCommand_current (void);

#endif      // POWERCOMMAND_H