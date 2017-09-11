//
//  SystemTime
//
//  Counts out ticks of time at 300Hz. This is used as the baud clock
//  for software serial tx.
//  Resets the watchdog timer
//
#ifndef SYSTEMTIME_H
#define SYSTEMTIME_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>

#define SYSTEMTIME_TICKS_PER_SECOND 300

#define COUNT_MAJOR_CYCLES false

typedef uint16_t SystemTime_tick;

typedef struct Timer_struct {
    uint32_t timeRemaining;
    uint16_t lastCheckTime;
} SystemTime_Timer;

// prototype for functions that clients supply to
// get notification when a tick occurs
typedef void (*SystemTime_TickNotification)(void);

extern void SystemTime_Initialize (void);

extern void SystemTime_task (void);

extern void SystemTime_registerForTickNotification (
    SystemTime_TickNotification notificationFcn);

extern SystemTime_tick SystemTime_currentTick (void);

extern void SystemTime_startTimer (
    const uint32_t duration,
    SystemTime_Timer *timer);
extern void SystemTime_cancelTimer (
    SystemTime_Timer *timer);
extern bool SystemTime_timerHasExpired (
    SystemTime_Timer *timer);

extern void SystemTime_commenceShutdown (void);
extern bool SystemTime_shuttingDown (void);

#if COUNT_MAJOR_CYCLES
extern uint32_t majorCycleCounter;
#endif

#endif  // SYSTEMTIME_H
