//
//  Battery Monitor
//
//  Monitors the state of the UPS's battery
//
#ifndef BATTERYMONITOR_H
#define BATTERYMONITOR_H

#include <stdint.h>

typedef enum {
    bs_unknown,
    bs_underVoltage,        // below minimun voltage (10.8V)
    bs_lowVoltage,          // low (between 11 and 12V)
    bs_goodVoltage,         // good (between 12 and 13V)
    bs_fullVoltage          // full (over 13V)
} BatteryMonitor_batteryStatus;

extern void BatteryMonitor_Initialize (void);

extern BatteryMonitor_batteryStatus BatteryMonitor_currentStatus (void);

// voltage in units of 1/100 volt
extern int16_t BatteryMonitor_currentVoltage (void);

extern void BatteryMonitor_task (void);

#endif      // BATTERYMONITOR_H