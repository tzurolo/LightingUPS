//
//  Pushbutton Monitor
//
//  Monitors the state of the Pushbutton
//
//  How it works:
//      The pushbutton is sampled 150 times per second, and the last
//      8 samples are retained in a queue. If all samples are the
//      same the pushbutton is considered to be in a stable state (no
//      more contact bounce). Contact bounce usually settles in 50mS
//      The pushed button is active-low
//
//  Pin usage:
//     PA4 - input from Pushbutton (normally pulled up, low when pushbutton pushed)
//
#include "PushbuttonMonitor.h"

#include "SystemTime.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define PUSHBUTTON_DDR    DDRA
#define PUSHBUTTON_INPORT PINA
#define PUSHBUTTON_PORT   PORTA
#define PUSHBUTTON_PIN    PA4

#define SAMPLE_TIME (SYSTEMTIME_TICKS_PER_SECOND / 150)

static uint8_t samples;
static bool pushbuttonStatus = false;
static SystemTime_Timer sampleTimer;

void PushbuttonMonitor_Initialize (void)
{
    // make pushbutton pin an input and enable pullup
    PUSHBUTTON_DDR &= (~(1 << PUSHBUTTON_PIN));
    PUSHBUTTON_PORT |= (1 << PUSHBUTTON_PIN);

    SystemTime_startTimer(0, &sampleTimer); // start sampling immediately
    samples = 0xFF;    // assume button starts off not pressed
    pushbuttonStatus = false;
}

bool PushbuttonMonitor_buttonIsPressed (void)
{
    return pushbuttonStatus;
}

void PushbuttonMonitor_task (void)
{
    if (SystemTime_timerHasExpired(&sampleTimer)) {
        SystemTime_startTimer(SAMPLE_TIME, &sampleTimer);

        // push pushbutton state onto sample "queue"
        samples <<= 1;
        if ((PUSHBUTTON_INPORT & (1 << PUSHBUTTON_PIN)) != 0) {
            samples |= 1;
        }

        // check all samples
        if (samples == 0) {
            pushbuttonStatus = true;
        } else if (samples == 0xFF) {
            pushbuttonStatus = false;
        }
    }
}

