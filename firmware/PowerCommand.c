//
//  Power Command
//
#include "PowerCommand.h"

#include "PushbuttonMonitor.h"
#include "PhotocellMonitor.h"
#include "MainsMonitor.h"
#include "MotionMonitor.h"
#include "PowerSwitches.h"
#include "SystemMode.h"
#include "SystemTime.h"
#include "EEPROMStorage.h"

#define PHOTOCELL_RESPONSE_DELAY (SYSTEMTIME_TICKS_PER_SECOND / 8)

typedef enum PushbuttonTransition_enum {
    pt_none,
    pt_pressed,
    pt_released
} PushbuttonTransition;

typedef enum MainsTransition_enum {
    mt_none,
    mt_cameOn,
    mt_wentOff
} MainsTransition;

static PowerCommand_CommandState cmdState = cs_off;
static bool pushbuttonWasPressed = false;
static bool mainsWereOn = false;
static SystemTime_Timer onOffTimer; // used for auto on, manual on, and manual off
static uint8_t prevLightLevel = 0;
static SystemTime_Timer photocellTimer;

static void turnOnAutomatic (void)
{
    SystemTime_startTimer(
        ((int32_t)SYSTEMTIME_TICKS_PER_SECOND) * 60 * EEPROMStorage_autoTime,
        &onOffTimer);
    PowerSwitches_command(true);
    cmdState = cs_onAutomatic;
}

void PowerCommand_Initialize (void)
{
    cmdState = cs_off;
    pushbuttonWasPressed = false;
    mainsWereOn = false;
    prevLightLevel = 0;
}

void PowerCommand_task (void)
{
    // check for a transition in the pushbutton
    const bool pushbuttonNowPressed = PushbuttonMonitor_buttonIsPressed();
    const PushbuttonTransition pbTransition =
        (pushbuttonNowPressed && !pushbuttonWasPressed)
        ? pt_pressed
        : ((!pushbuttonNowPressed) && pushbuttonWasPressed)
            ? pt_released
            : pt_none;
    pushbuttonWasPressed = pushbuttonNowPressed;

    // check for a transition in the mains
    const bool mainsAreOn = MainsMonitor_mainsOn();
    const MainsTransition mTransition =
        (mainsAreOn && !mainsWereOn)
        ? mt_cameOn
        : ((!mainsAreOn) && mainsWereOn)
            ? mt_wentOff
            : mt_none;
    mainsWereOn = mainsAreOn;

    switch (cmdState) {
        case cs_off: 
            if (pbTransition == pt_pressed) {
                PowerCommand_turnOn();
            } else if (mTransition == mt_wentOff) {
                // capture light level before mains went off
                prevLightLevel = PhotocellMonitor_averageLightLevel();
                PhotocellMonitor_clearStatistics();
                SystemTime_startTimer(PHOTOCELL_RESPONSE_DELAY, &photocellTimer);
                cmdState = cs_waitingForPhotocellAfterMainsOff;
            } else if (((!MainsMonitor_mainsOn()) || 
                        (SystemMode_currentMode() == m_primary)) && // mains off or in primary mode
                       (PhotocellMonitor_haveValidSample()) &&
                       (PhotocellMonitor_averageLightLevel() <= EEPROMStorage_darkLevel)  && // it's dark
                       MotionMonitor_motionDetected() && // motion in room
                       SystemTime_timerHasExpired(&onOffTimer)) { // lockout time expired
                turnOnAutomatic();
            }
            break;
        case cs_waitingForPhotocellAfterMainsOff :
            if (SystemTime_timerHasExpired(&photocellTimer)) {
                // response delay time has passed
                const uint8_t currLightLevel = PhotocellMonitor_currentLightLevel();
                if (currLightLevel < (prevLightLevel - (prevLightLevel / 4))) {
                    turnOnAutomatic();
                } else {
                    cmdState = cs_off;
                }
            }
            break;
        case cs_onManual:
            if (pbTransition == pt_pressed) {
                // lockout automatic turn-on briefly
                PowerCommand_turnOff(AUTO_ON_LOCKOUT_TIME);
            } else if (SystemTime_timerHasExpired(&onOffTimer)) {
                PowerCommand_turnOff(0);
            }
            break;
        case cs_onAutomatic:
            if (pbTransition == pt_pressed) {
                // lockout automatic turn-on briefly
                PowerCommand_turnOff(AUTO_ON_LOCKOUT_TIME);
            } else if (mTransition == mt_cameOn) {
                prevLightLevel = PhotocellMonitor_averageLightLevel();
                PhotocellMonitor_clearStatistics();
                SystemTime_startTimer(PHOTOCELL_RESPONSE_DELAY, &photocellTimer);
                if (PowerSwitches_currentState() > pss_undervoltage) {
                    // power just came on. turn off LEDs, but re-check light level
                    // in cs_waitingForPhotocellAfterMainsOn
                    PowerSwitches_command(false);
                    cmdState = cs_waitingForPhotocellAfterMainsOn;
                } else {
                    // power just came on, but we are in undervoltage condition
                    PowerSwitches_command(false);
                    cmdState = cs_waitingForPhotocellAfterMainsOnUndervoltage;
                }
            } else if (MotionMonitor_motionDetected()) { // motion in room
                // extend auto period
                SystemTime_startTimer(
                    ((int32_t)SYSTEMTIME_TICKS_PER_SECOND) * 60 * EEPROMStorage_autoTime,
                    &onOffTimer);
            } else {
                if (SystemTime_timerHasExpired(&onOffTimer)) {
                    PowerCommand_turnOff(0);
                }
            }
            break;
        case cs_waitingForPhotocellAfterMainsOn :
            if (SystemTime_timerHasExpired(&photocellTimer)) {
                // response delay time has passed
                const uint8_t currLightLevel = PhotocellMonitor_currentLightLevel();
                if (currLightLevel < (prevLightLevel - (prevLightLevel / 4))) {
                    // power came back on, but light level dropped when we
                    // turned LEDs off. Turn LEDs back on
                    turnOnAutomatic();
                } else {
                    PowerCommand_turnOff(0);
                }
            }
            break;
        case cs_waitingForPhotocellAfterMainsOnUndervoltage :
            if (SystemTime_timerHasExpired(&photocellTimer)) {
                // response delay time has passed
                const uint8_t currLightLevel = PhotocellMonitor_currentLightLevel();
                if (currLightLevel <= (prevLightLevel + (prevLightLevel / 4))) {
                    // power came back on, but light level did not increase
                    // Turn LEDs back on
                    turnOnAutomatic();
                } else {
                    PowerCommand_turnOff(0);
                }
            }
            break;
    }
}

void PowerCommand_turnOn (void)
{
    SystemTime_startTimer(
        ((int32_t)SYSTEMTIME_TICKS_PER_SECOND) * 60 * EEPROMStorage_manualTime,
        &onOffTimer);
    PowerSwitches_command(true);
    cmdState = cs_onManual;
}

void PowerCommand_turnOff (
    const uint8_t autoOnLockoutTime)
{
    if (autoOnLockoutTime > 0) {
        SystemTime_startTimer(
            ((int32_t)SYSTEMTIME_TICKS_PER_SECOND) * autoOnLockoutTime,
            &onOffTimer);
    } else {
        SystemTime_cancelTimer(&onOffTimer);
    }
    PowerSwitches_command(false);
    cmdState = cs_off;
}

PowerCommand_CommandState PowerCommand_current (void)
{
    return cmdState;
}

