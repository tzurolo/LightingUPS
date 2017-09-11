//
//  Pushbutton Monitor
//
//  Monitors the state of the Pushbutton, provides debouncing
//
#ifndef PUSHBUTTONMONITOR_H
#define PUSHBUTTONMONITOR_H

#include <stdint.h>
#include <stdbool.h>

extern void PushbuttonMonitor_Initialize (void);

// returns true if pushbutton is currently pressed
extern bool PushbuttonMonitor_buttonIsPressed (void);

extern void PushbuttonMonitor_task (void);

#endif      // PUSHBUTTONMONITOR_H