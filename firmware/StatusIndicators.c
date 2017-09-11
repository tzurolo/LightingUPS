//
//  Status Indicators
//
//  Controls outputs that indicate the internal status of the UPS.
//  The first set of output is connected to an LEDs and gives a
//  visual indication of battery charge and whether AC power is present.
//
//  The second function of this unit is status reporting. This is a
//  serial ASCII stream that reports the state of the UPS (e.g. on AC,
//  on battery, undervoltage warning, etc.)
//
//  uses pins
//      PB0 - battery voltage indicator - 0-4 blinks
//

#include "StatusIndicators.h"

#include "SystemTime.h"
#include "BatteryMonitor.h"
#include "PowerCommand.h"
#include "PowerSwitches.h"
#include "PhotocellMonitor.h"
#include "MainsMonitor.h"
#include "MotionMonitor.h"
#include "InternalTemperatureMonitor.h"
#include "PushbuttonMonitor.h"
#include "SystemMode.h"
#include "SoftwareSerialTx.h"
#include "CharString.h"
#include "StringUtils.h"
#include <avr/io.h>

#define BATTERY_STATUS_DDR      DDRB
#define BATTERY_STATUS_PORT     PORTB
#define BATTERY_STATUS_PIN      PB0

#define TICK_TIMER_DURATION (SYSTEMTIME_TICKS_PER_SECOND / 20)
#define TICKS_IN_MAJOR_CYCLE 160

static SystemTime_Timer tickTimer;
static uint8_t curTick;  // zero-based (0..40, takes 2 seconds to reach 40)
static uint8_t ticksInCurrentState;
static BatteryMonitor_batteryStatus latestBatteryStatus;

void StatusIndicators_Initialize (void)
{
    // set status pin to output
    BATTERY_STATUS_DDR      |= (1 << BATTERY_STATUS_PIN);

    SystemTime_startTimer(TICK_TIMER_DURATION, &tickTimer);
    curTick = 0;
    ticksInCurrentState = 0;
    latestBatteryStatus = bs_unknown;

    SoftwareSerialTx_enable();
}

void StatusIndicators_task (void)
{
    // count ticks in major cycle
    bool tickHasElapsed = false;
    if (SystemTime_timerHasExpired(&tickTimer)) {
        SystemTime_startTimer(TICK_TIMER_DURATION, &tickTimer);
        if (curTick < (TICKS_IN_MAJOR_CYCLE - 1)) {
            ++curTick;
        } else {
            curTick = 0;
        }
        tickHasElapsed = true;
    }

    //
    // Battery status indicator
    //
    // create 6 blink slots (0-5) per two seconds
    if (curTick == 0) {
        latestBatteryStatus = BatteryMonitor_currentStatus();
    }
    const uint8_t interval = curTick / 6;
    const uint8_t subinterval = curTick % 6;
    const bool ledOn = // blink to indicate...
        ((interval < latestBatteryStatus) && (subinterval == 0)) || // ...battery voltage
        (MainsMonitor_mainsOn() && (interval == 10));               // ...mains status
    if (ledOn) {
        BATTERY_STATUS_PORT |= 1 << BATTERY_STATUS_PIN;
    } else {
        BATTERY_STATUS_PORT &= ~(1 << BATTERY_STATUS_PIN);
    }
}

void StatusIndicators_sendStatusMesssage (void)
{
    CharString_define(30, msg);

    // UPS status
    CharString_appendP(PSTR("U"), &msg);
    char upsChar = ' ';
    switch (PowerSwitches_currentState()) {
        case pss_initial                : upsChar = 'I'; break;
        case pss_undervoltage           : upsChar = 'U'; break;
        case pss_undervoltageWarning    : upsChar = 'W'; break;
        case pss_onBattery              : upsChar = 'B'; break;
        case pss_onAdapter              : upsChar = 'A'; break;
    }
    CharString_appendC(upsChar, &msg);

    // Battery voltage
    CharString_appendP(PSTR(" B"), &msg);
    StringUtils_appendDecimal(BatteryMonitor_currentVoltage(), 2, &msg);

    // mains status
    CharString_appendP(PSTR(" A"), &msg);
    StringUtils_appendDecimal((uint16_t)MainsMonitor_mainsOn(), 0, &msg);

    // Power command
    CharString_appendP(PSTR(" C"), &msg);
    CharString_appendC((char)SystemMode_currentMode(), &msg);
    char cmdChar = ' ';
    switch (PowerCommand_current()) {
        case cs_off                              : cmdChar = '0'; break;
        case cs_onManual                         : cmdChar = 'M'; break;
        case cs_waitingForPhotocellAfterMainsOff : cmdChar = 'f'; break;
        case cs_onAutomatic                      : cmdChar = 'A'; break;
        case cs_waitingForPhotocellAfterMainsOn  : cmdChar = 'n'; break;
        case cs_waitingForPhotocellAfterMainsOnUndervoltage  : cmdChar = 'n'; break;
    }
    CharString_appendC(cmdChar, &msg);

    // Photocell
    CharString_appendP(PSTR(" L"), &msg);
    const uint8_t lightLevel = PhotocellMonitor_currentLightLevel();
    StringUtils_appendDecimal(lightLevel, 0, &msg);

    // Motion
    CharString_appendP(PSTR(" M"), &msg);
    StringUtils_appendDecimal((uint16_t)MotionMonitor_motionDetected(), 0, &msg);

    // Temperature
    CharString_appendP(PSTR(" T"), &msg);
    StringUtils_appendDecimal((uint16_t)InternalTemperatureMonitor_currentTemperature(), 0, &msg);

#if 0
    // pushbutton
    CharString_appendP(PSTR(" P"), &msg);
    StringUtils_appendDecimal((int16_t)PushbuttonMonitor_buttonIsPressed(), 0, &msg);
#endif

#if COUNT_MAJOR_CYCLES
    // Cycle counter
    CharString_appendP(PSTR(" C"), &msg);
    if (majorCycleCounter > 65535L) {
        CharString_appendP(PSTR("oflo"), &msg);
    } else {
        StringUtils_appendDecimal((int16_t)majorCycleCounter, 0, &msg);
    }
    majorCycleCounter = 0;
#endif

    CharString_appendP(PSTR("\r\n"), &msg);

    SoftwareSerialTx_send(CharString_cstr(&msg));
}

