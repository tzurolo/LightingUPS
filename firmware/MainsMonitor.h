//
//  AC Mains Monitor
//
//  Monitors the state of the AC Mains (to detect power failure)
//
#ifndef MAINSMONITOR_H
#define MAINSMONITOR_H

#include <stdint.h>
#include <stdbool.h>

// prototypes for functions that clients supply to
// get notification of mains state change event
typedef void (*MainsMonitor_Notification)(bool mainsOn);

extern void MainsMonitor_Initialize (void);

extern void MainsMonitor_registerForNotification (
    MainsMonitor_Notification notificationFunction);

extern bool MainsMonitor_mainsOn (void);

extern void MainsMonitor_task (void);

#endif      // MAINSMONITOR_H