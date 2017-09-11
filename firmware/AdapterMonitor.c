//
//  AC Adapter Monitor
//
//  Monitors the state of mains power coming in through an AC adapter.
//  This is done using the AtTiny84's analog comparator
//
//  Pin usage:
//     AIN1 (PA2) - input from AC adapter voltage divider
//
#include "AdapterMonitor.h"

#include "SystemTime.h"
#include <avr/io.h>
#include <avr/interrupt.h>

static uint16_t adapterVoltage;
static AdapterMonitor_adapterStatus adapterStatus = as_unknown;
static AdapterMonitor_Notification notification = NULL;

void AdapterMonitor_Initialize (void)
{
    adapterStatus = as_unknown;

    // set up the analog comparator for detecting loss of mains power
    ACSR |= (1 << ACBG);     // select bandgap as the comparator's positive input
    DIDR0 |= (1 << ADC2D);   // disable digital input on AIN1
}

void AdapterMonitor_registerForNotification (
    const uint8_t interruptModeSelect,
    AdapterMonitor_Notification notificationFunction)
{
    // select interrupt mode
    ACSR = (ACSR & 0xFC) | (interruptModeSelect & 0x03);

    // enable comparator interrupt
    ACSR |= (1 << ACIE);

    notification = notificationFunction;
}

AdapterMonitor_adapterStatus AdapterMonitor_currentStatus (void)
{
    return adapterStatus;
}

void AdapterMonitor_task (void)
{
    // check analog comparator
    if ((ACSR & (1 << ACO)) != 0) {
        adapterStatus = as_underVoltage;
    } else {
        adapterStatus = as_goodVoltage;
    }
}

ISR(SIG_COMPARATOR, ISR_BLOCK)
{
    AdapterMonitor_task();
    if (notification != NULL) {
        notification();
    }
}
