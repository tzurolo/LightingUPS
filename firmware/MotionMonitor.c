//
//  Motion Detector Monitor
//
//  Monitors the state of the motion detector
//
//  Pin usage:
//     PB2 - input from motion detector
//

#include "MotionMonitor.h"

#include "SystemTime.h"
#include <avr/io.h>

#define MOTION_DETECTOR_DDR    DDRB
#define MOTION_DETECTOR_INPORT PINB
#define MOTION_DETECTOR_PIN    PB2

// sample 20 times per second
#define MOTION_DETECTOR_WARMUP_TIME (((uint16_t)SYSTEMTIME_TICKS_PER_SECOND) * 90)

typedef enum {
    mss_warmingUp,
    mss_ready
} MotionSensorState;

static MotionSensorState sensorState = mss_warmingUp;
static SystemTime_Timer warmupTimer;

void MotionMonitor_Initialize (void)
{
    // set motion detector pin to be an input
    MOTION_DETECTOR_DDR &= ~(1 << MOTION_DETECTOR_PIN);

    SystemTime_startTimer(MOTION_DETECTOR_WARMUP_TIME, &warmupTimer);
    sensorState = mss_warmingUp;
}

bool MotionMonitor_motionDetected (void)
{
    // read motion detector
    return (sensorState == mss_ready) &&
           ((MOTION_DETECTOR_INPORT & (1 << MOTION_DETECTOR_PIN)) != 0);
}

void MotionMonitor_task (void)
{
    switch (sensorState) {
        case mss_warmingUp :
            if (SystemTime_timerHasExpired(&warmupTimer)) {
                sensorState = mss_ready;
            }
            break;
        case mss_ready :
            break;    
    }
}

