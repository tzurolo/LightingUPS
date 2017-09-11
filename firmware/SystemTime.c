//
//  System Time keeper
//
//  Counts out ticks of time at 300Hz. This is used as the baud clock
//  for software serial. The counter is 16 bits and rolls over at 65535.
//
//   Uses 8-bit Timer 0
//
// Two approaches considered for user-declared timers. The challenge comes
// from the fact that the system time counter rolls over  
//
//  I: timer stores start time and end time
//
//  we need to account for the case when the future time rolls over
//  the tick counter. future time must store start time and end time
// if end time < start time
//    // rollover case
//    if current time < start time && // tick counter rolled over
//       current time >= end time
//       future time has arrived
// else
//    // normal case
//    if current time >= end time
//       future time has arrived
//
// II: timer stores time remaining and when it was last checked
//
//  advantage: timer duration independent of tick counter rollover time
//  needs to be checked at least once per rollover time
//  timer struct captures
//   1. time remaining (32 bit)
//   2. last check time (16 bit)
// when we check - 
//   if time remaining > 0
//      if current time > last check time
//         delta = current time - last check time
//      else // rollover
//         delta = twos-complement of last check time + current time
//      if delta >= time remaining
//         time remaining = 0L;
//      else
//         time remaining -= delta
//         last check time = current time
//   timer expired = time remaining == 0L

#include "SystemTime.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#if COUNT_MAJOR_CYCLES
uint32_t majorCycleCounter;
#endif

static volatile SystemTime_tick ticksInMajorCycle = 0;
static SystemTime_TickNotification notificationFunction;

static bool shuttingDown = false;

void SystemTime_Initialize (void)
{
    ticksInMajorCycle = 0;
    notificationFunction = NULL;

    // set up timer0 to fire interrupt 300 Hz (baud clock)
    TCCR0A = (TCCR0A & 0xFC) | 2;   // set CTC mode
    TCCR0B = (TCCR0B & 0xF8) | 3;   // prescale by 64
    OCR0A = 52;  // with 1MHz clock and 64 prescale this is 1/300 second
    TIMSK0 |= (1 << OCIE0A);// enable timer compare match interrupt

}

SystemTime_tick SystemTime_currentTick (void)
{
    // we disable interrupts during read of secondsSinceReset because
    // it is updated in an interrupt handler
    char SREGSave;
    SREGSave = SREG;
    cli();
    const SystemTime_tick curTick = ticksInMajorCycle;
    SREG = SREGSave;

    return curTick;
}

void SystemTime_startTimer (
    const uint32_t duration,
    SystemTime_Timer *timer)
{
    timer->lastCheckTime = SystemTime_currentTick();
    timer->timeRemaining = duration;
}

void SystemTime_cancelTimer (
    SystemTime_Timer *timer)
{
    timer->timeRemaining = 0;
}

bool SystemTime_timerHasExpired (
    SystemTime_Timer *timer)
{
    bool expired = false;

    if (timer->timeRemaining > 0) {
        SystemTime_tick curTime = SystemTime_currentTick();
        if (curTime != timer->lastCheckTime) {
            // at least one tick has elapsed since last check
            uint16_t delta;
            if (curTime > timer->lastCheckTime) {
                delta = curTime - timer->lastCheckTime;
            } else {    // curTime < lastCheckTime -> rollover
                delta = (~(timer->lastCheckTime)) + 1 + curTime;
            }
            if (delta >= timer->timeRemaining) {
                timer->timeRemaining = 0L;
                expired = true;
            } else {
                timer->timeRemaining -= delta;
                timer->lastCheckTime = curTime;
            }
        }
    } else {
        expired = true;
    }

    return expired;
}

void SystemTime_commenceShutdown (void)
{
    shuttingDown = true;
    wdt_enable(WDTO_8S);
}

bool SystemTime_shuttingDown (void)
{
    return shuttingDown;
}

void SystemTime_task (void)
{
    if (shuttingDown) {
        // LED_OUTPORT |= (1 << LED_PIN);
    } else {
        // reset the watchdog timer
        wdt_reset();
    }
}

void SystemTime_registerForTickNotification (
    SystemTime_TickNotification notificationFcn)
{
    notificationFunction = notificationFcn;
}

ISR(SIG_OUTPUT_COMPARE0A, ISR_BLOCK)
{
    ++ticksInMajorCycle;

    if (notificationFunction != NULL) {
        notificationFunction();
    }
}
