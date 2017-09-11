#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <math.h>
#include <util/delay.h>

#include "intlimit.h"
#include "SystemTime.h"
#include "EEPROMStorage.h"
#include "ADCManager.h"
#include "BatteryMonitor.h"
#include "PhotocellMonitor.h"
#include "PushbuttonMonitor.h"
#include "MainsMonitor.h"
#include "MotionMonitor.h"
#include "InternalTemperatureMonitor.h"
#include "SystemMode.h"
#include "PowerCommand.h"
#include "PowerSwitches.h"
#include "StatusIndicators.h"
#include "Console.h"
#include "SoftwareSerialRx.h"
#include "SoftwareSerialTx.h"
#include "RAMSentinel.h"

/** Configures the board hardware and chip peripherals for the demo's functionality. */
static void Initialize (void)
{
    // enable watchdog timer
    wdt_enable(WDTO_500MS);

    SystemTime_Initialize();
    EEPROMStorage_Initialize();
    ADCManager_Initialize();
    BatteryMonitor_Initialize();
    PhotocellMonitor_Initialize();
    PushbuttonMonitor_Initialize();
    MainsMonitor_Initialize();
    MotionMonitor_Initialize();
    InternalTemperatureMonitor_Initialize();
    SystemMode_Initialize();
    PowerCommand_Initialize();
    PowerSwitches_Initialize();
    SoftwareSerialTx_Initialize();
    SoftwareSerialRx_Initialize();
    Console_Initialize();
    StatusIndicators_Initialize();
    RAMSentinel_Initialize();
}
 
int main (void)
{
    Initialize();

    sei();

    for (;;) {
        // run all the tasks
        SystemTime_task();
        ADCManager_task();
        BatteryMonitor_task();
        PhotocellMonitor_task();
        PushbuttonMonitor_task();
        MainsMonitor_task();
        MotionMonitor_task();
        InternalTemperatureMonitor_task();
        PowerCommand_task();
        PowerSwitches_task();
        StatusIndicators_task();
        Console_task();

        if (!RAMSentinel_sentinelIntact()) {
            SystemTime_commenceShutdown();
        }

#if COUNT_MAJOR_CYCLES
        ++majorCycleCounter;
#endif
    }

    return (0);
}
