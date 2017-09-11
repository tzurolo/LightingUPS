//
//  Motion Detector Monitor
//
//  Monitors the state of the motion detector
//
#ifndef MOTIONMONITOR_H
#define MOTIONMONITOR_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>

extern void MotionMonitor_Initialize (void);

extern bool MotionMonitor_motionDetected (void);

extern void MotionMonitor_task (void);

#endif      // MOTIONMONITOR_H