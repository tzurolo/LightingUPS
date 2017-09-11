//
//  Internal Temperature Monitor
//
//  Monitors the temperature of the AtTiny84 using its internal
//  temperature sensor
//
#ifndef INTERNALTEMPMONITOR_H
#define INTERNALTEMPMONITOR_H

#include <stdint.h>
#include <stdbool.h>

extern void InternalTemperatureMonitor_Initialize (void);

extern bool InternalTemperatureMonitor_haveValidSample (void);

// temperature in units of degree C
extern int16_t InternalTemperatureMonitor_currentTemperature (void);

extern void InternalTemperatureMonitor_task (void);

#endif      // INTERNALTEMPMONITOR_H