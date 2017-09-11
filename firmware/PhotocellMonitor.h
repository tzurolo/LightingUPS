//
//  Photocell Monitor
//
//  Monitors the photocell and provides ambient light level
//
#ifndef PHOTOCELLMONITOR_H
#define PHOTOCELLMONITOR_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>

extern void PhotocellMonitor_Initialize (void);

extern bool PhotocellMonitor_haveValidSample (void);

// light levels in units of percent (higher means brighter)
extern uint8_t PhotocellMonitor_currentLightLevel (void);
extern uint8_t PhotocellMonitor_averageLightLevel (void);

extern void PhotocellMonitor_clearStatistics (void);

extern void PhotocellMonitor_task (void);

#endif      // PHOTOCELLMONITOR_H
