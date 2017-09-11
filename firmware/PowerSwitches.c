//
//  Power Switches
//
//  Pin usage:
//     PA1 - output that controls state of battery FET
//     PA2 - output that controls state of AC adapter FET
//

#include "PowerSwitches.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "BatteryMonitor.h"
#include "MainsMonitor.h"
#include "SystemTime.h"
#include "SoftwareSerialTx.h"

#define BATTERY_FET_DDR    DDRA
#define BATTERY_FET_PORT   PORTA
#define BATTERY_FET_PIN    PA1

#define ACADAPTER_FET_DDR  DDRA
#define ACADAPTER_FET_PORT PORTA
#define ACADAPTER_FET_PIN  PA2

#define TICKS_PER_SECOND 20
#define TICK_TIMER_DURATION (SYSTEMTIME_TICKS_PER_SECOND / TICKS_PER_SECOND)

// number of 1/20 second ticks for the warning interval
#define UNDERVOLTAGE_WARNING_DURATION (10 * TICKS_PER_SECOND)

// number of 1/20 second ticks to consider AC good
#define MIN_AC_GOOD_DURATION 5

static bool powerCommand = false;
static PowerSwitches_state pssState = pss_initial;
static PowerSwitches_state lastPssState = pss_initial;
static uint16_t timeInState;        // in units of TICK_TIMER_DURATION
static SystemTime_Timer tickTimer;  // used for counting time in state

static void setBatteryFET (
    const bool on)
{
    if (on) {
        BATTERY_FET_PORT |= 1 << BATTERY_FET_PIN;
    } else {
        BATTERY_FET_PORT &= ~(1 << BATTERY_FET_PIN);
    }

}

static void setACAdapterFET (
    const bool on)
{
    if (on) {
        ACADAPTER_FET_PORT |= 1 << ACADAPTER_FET_PIN;
    } else {
        ACADAPTER_FET_PORT &= ~(1 << ACADAPTER_FET_PIN);
    }

}

void PowerSwitches_Initialize (void)
{
    powerCommand = false;
    pssState = pss_initial;

    // set FET control pins to output
    BATTERY_FET_DDR |= (1 << BATTERY_FET_PIN);
    ACADAPTER_FET_DDR |= (1 << ACADAPTER_FET_PIN);

    setBatteryFET(false);
    setACAdapterFET(false);
}

void PowerSwitches_task (void)
{
    if (powerCommand) {
        // some states count time. we do this with a SystemTime timer.
        if ((pssState == pss_initial) ||
            (pssState != lastPssState)) {
            lastPssState = pssState;
            timeInState = 0;
            SystemTime_startTimer(TICK_TIMER_DURATION, &tickTimer);
        } else if (SystemTime_timerHasExpired(&tickTimer)) {
            SystemTime_startTimer(TICK_TIMER_DURATION, &tickTimer);
            // another tick has occurred
            if (timeInState < 65535) {
                ++timeInState;
            }
        }

        switch (pssState) {
            case pss_initial :
                if (MainsMonitor_mainsOn()) {
                    // we have AC power
                    setACAdapterFET(true);
                    setBatteryFET(false);
                    pssState = pss_onAdapter;
                } else {
                    // no AC power
                    if (BatteryMonitor_currentStatus() != bs_unknown) {
                        if (BatteryMonitor_currentStatus() > bs_underVoltage) {
                            // battery is not in undervoltage condition
                            setBatteryFET(true);
                            setACAdapterFET(false);
                            pssState = pss_onBattery;
                        } else {
                            // battery too low and no AC power. disconnect load
                            setBatteryFET(false);
                            setACAdapterFET(false);
                            pssState = pss_undervoltage;
                        }
                    }
                }
                break;
            case pss_onAdapter :
                if (MainsMonitor_mainsOn()) {
                    // still have AC power - stay on AC power
                    if (timeInState >= MIN_AC_GOOD_DURATION) {
                        // AC stabilization time has been reached
                        // we can now turn off the battery FET
                        setBatteryFET(false);
                    }
                } else {
                    // lost AC power
                    // check if battery has enough charge
                    if (BatteryMonitor_currentStatus() > bs_underVoltage) {
                        // switch over to battery power, but leave the
                        // AC adapter FET on
                        setBatteryFET(true);
                        pssState = pss_onBattery;
                    } else {
                        // no AC power, battery too low. disconnect load
                        setBatteryFET(false);
                        setACAdapterFET(false);
                        pssState = pss_undervoltage;
                    }
                }
                break;
            case pss_onBattery :
                if (MainsMonitor_mainsOn()) {
                    // we have AC power now
                    // switch to AC power, but leave battery FET on
                    setACAdapterFET(true);
                    pssState = pss_onAdapter;
                } else {
                    // no AC power, check battery status
                    if (BatteryMonitor_currentStatus() > bs_underVoltage) {
                        // still have battery power - stay on battery
                        if (timeInState >= MIN_AC_GOOD_DURATION) {
                            // we can now turn off the AC adapter FET
                            setACAdapterFET(false);
                        }
                    } else {
                        // no AC power, battery too low. start the
                        // undervoltage warning period
                        pssState = pss_undervoltageWarning;
                    }
                }
                break;
            case pss_undervoltageWarning :
                // count ticks until warning interval expires
                if ((timeInState >= UNDERVOLTAGE_WARNING_DURATION) && 
                    SoftwareSerialTx_isIdle()) {  // wait for message to finish sending
                    // warning interval has expired.
                    // go into undervoltage state
                    pssState = pss_undervoltage;
                }
                break;
            case pss_undervoltage :
                if (MainsMonitor_mainsOn()) {
                    // we have AC power now
                    setACAdapterFET(true);
                    setBatteryFET(false);
                    pssState = pss_onAdapter;
                } else {
                    if (BatteryMonitor_currentStatus() >= bs_goodVoltage) {
                        // battery power has recovered - switch back to battery
                        setBatteryFET(true);
                        setACAdapterFET(false);
                        pssState = pss_onBattery;
                    } else {
                        // battery still too low. keep load disconnected
                        setBatteryFET(false);
                        setACAdapterFET(false);
                    }
                }
                break;
        }
    } else {
        // power command is off
        setBatteryFET(false);
        setACAdapterFET(false);
        pssState = pss_initial;
    }
}

void PowerSwitches_command (
    const bool powerOn)
{
    powerCommand = powerOn;
}

PowerSwitches_state PowerSwitches_currentState (void)
{
    return pssState;
}

