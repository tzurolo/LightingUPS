//
//  AC Adapter Monitor
//
//  Monitors the state of the UPS's AC Adapter
//
#ifndef ADAPTERMONITOR_H
#define ADAPTERMONITOR_H

#include <stdint.h>

// Analog comparator interrupt mode select
#define ACIMS_TOGGLE    0
#define ACIMS_FALLING   2
#define ACIMS_RISING    3

// prototypes for functions that clients supply to
// get notification of comparator event
typedef void (*AdapterMonitor_Notification)(void);

typedef enum {
    as_unknown,
    as_underVoltage,        // below minimun voltage (< 8V)
    as_goodVoltage,         // good (> 8V)
} AdapterMonitor_adapterStatus;

extern void AdapterMonitor_Initialize (void);

extern void AdapterMonitor_registerForNotification (
    const uint8_t interruptModeSelect,
    AdapterMonitor_Notification notificationFunction);

extern AdapterMonitor_adapterStatus AdapterMonitor_currentStatus (void);

extern void AdapterMonitor_task (void);

#endif      // ADAPTERMONITOR_H