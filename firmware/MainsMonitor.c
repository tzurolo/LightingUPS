//
//  AC Mains Monitor
//
//  Monitors the state of mains power.
//
//  How it works:
//      This is done using an AC optoisolator. We sample the input pin
//      every iteration and count when it is low (AC driving LEDs in
//      optoisolator). If the mains are on we expect the input pin to be
//      low at least twice during the sample period (50mS).
//
//  Pin usage:
//     PA7 - input from optoisolator
//
#include "MainsMonitor.h"

#include "SystemTime.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define SAMPLE_INTERVAL (SYSTEMTIME_TICKS_PER_SECOND / 15)

#define OPTOISOLATOR_DDR    DDRA
#define OPTOISOLATOR_INPORT PINA
#define OPTOISOLATOR_PORT   PORTA
#define OPTOISOLATOR_PIN    PA7

static bool mainsOn = false;
static uint16_t numSamples;
static uint16_t numAssertedSamples;
static SystemTime_Timer sampleTimer;
static MainsMonitor_Notification notification = NULL;

void MainsMonitor_Initialize (void)
{
    mainsOn = false;
    numSamples = 0;
    numAssertedSamples = 0;
    notification = NULL;

    // set up pin as input, turn on pull-up
    OPTOISOLATOR_DDR &= (~(1 << OPTOISOLATOR_PIN));
    OPTOISOLATOR_PORT |= (1 << OPTOISOLATOR_PIN);

    // start sample interval timer
    SystemTime_startTimer(SAMPLE_INTERVAL, &sampleTimer);
}

void MainsMonitor_registerForNotification (
    MainsMonitor_Notification notificationFunction)
{
    notification = notificationFunction;
}

bool MainsMonitor_mainsOn (void)
{
    return mainsOn;
}

void MainsMonitor_task (void)
{
    if (SystemTime_timerHasExpired(&sampleTimer)) {
        SystemTime_startTimer(SAMPLE_INTERVAL, &sampleTimer);

        if (numSamples >= 8) {
            // check for at least 2 samples asserted
            mainsOn = (numAssertedSamples >= 2);
        }

        numSamples = 0;
        numAssertedSamples = 0;
    } else {
        ++numSamples;
        if ((OPTOISOLATOR_INPORT & (1 << OPTOISOLATOR_PIN)) == 0) {
            ++numAssertedSamples;
        }
    }
}

