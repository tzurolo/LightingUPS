//
//  Battery Monitor
//
//  Monitors the state of the UPS's battery
//
//  Pin usage:
//     ADC0 (PA0) - input from battery voltage divider
//

#include "BatteryMonitor.h"

#include "ADCManager.h"
#include "SystemTime.h"
#include "DataHistory.h"

#define BATTERY_ADC_CHANNEL 0

#define BATTERY_DIVIDER_R1 10.0
#define BATTERY_DIVIDER_R2 21.7
#define SYSTEM_VCC 4.99

#define RESISTOR_DIVIDER_COUNTS(VIN, VCC, R1, R2) ((uint16_t)((((R1/(R1+R2))*VIN)/VCC)*1024+0.5))
#define BATTERY_VOLTAGE_COUNTS(VIN) RESISTOR_DIVIDER_COUNTS(VIN, SYSTEM_VCC, BATTERY_DIVIDER_R1, BATTERY_DIVIDER_R2)

#define BATTERY_10V9 BATTERY_VOLTAGE_COUNTS(10.9)
#define BATTERY_12V  BATTERY_VOLTAGE_COUNTS(12.0)
#define BATTERY_13V  BATTERY_VOLTAGE_COUNTS(13.0)

#define BATTERY_VOLTAGE_SAMPLES 3
// sample 20 times per second
#define BATTERY_VOLTAGE_SAMPLE_TIME (SYSTEMTIME_TICKS_PER_SECOND / 20)

typedef enum {
    bms_idle,
    bms_waitingForADCStart,
    bms_waitingForADCCompletion
} BatteryMonitor_state;

static BatteryMonitor_batteryStatus battStatus = bs_unknown;
static BatteryMonitor_state bmState = bms_idle;
static SystemTime_Timer sampleTimer;
DataHistory_define(BATTERY_VOLTAGE_SAMPLES, batteryVoltageHistory);

void BatteryMonitor_Initialize (void)
{
    bmState = bms_idle;
    battStatus = bs_unknown;
    SystemTime_startTimer(0, &sampleTimer); // start sampling immediately
    DataHistory_clear(&batteryVoltageHistory);

    // set up the ADC channel for measuring battery voltage
    ADCManager_setupChannel(BATTERY_ADC_CHANNEL, ADC_REF_VCC, false);
}

BatteryMonitor_batteryStatus BatteryMonitor_currentStatus (void)
{
    return battStatus;
}

int16_t BatteryMonitor_currentVoltage (void)
{
    uint16_t batteryVoltage = 0;
    if (DataHistory_length(&batteryVoltageHistory) >= BATTERY_VOLTAGE_SAMPLES) {
        uint16_t minVoltage;
        uint16_t maxVoltage;
        DataHistory_getStatistics(
            &batteryVoltageHistory, BATTERY_VOLTAGE_SAMPLES,
            &minVoltage, &maxVoltage, &batteryVoltage);
    }

    int32_t vBatt = batteryVoltage;
    // counts to 1/100s volt
    // measured resistor divider ratio: 0.315, VCC: 4.99V
    // ((4.99V / 0.315) / 1024) * 100 => 1.54
    return ((((int32_t)vBatt) * 154) / 100);
}

void BatteryMonitor_task (void)
{
    switch (bmState) {
        case bms_idle :
            if (SystemTime_timerHasExpired(&sampleTimer)) {
                SystemTime_startTimer(BATTERY_VOLTAGE_SAMPLE_TIME, &sampleTimer);
                bmState = bms_waitingForADCStart;
            }
            break;
        case bms_waitingForADCStart :
            if (ADCManager_StartConversion(BATTERY_ADC_CHANNEL)) {
                // successfully started conversion
                bmState = bms_waitingForADCCompletion;
            }
            break;    
        case bms_waitingForADCCompletion : {
            uint16_t batteryVoltage;
            if (ADCManager_ConversionIsComplete(&batteryVoltage)) {
                DataHistory_insertValue(batteryVoltage, &batteryVoltageHistory);

                if (DataHistory_length(&batteryVoltageHistory) >=
                        BATTERY_VOLTAGE_SAMPLES) {
                    uint16_t minVoltage;
                    uint16_t maxVoltage;
                    uint16_t avgVoltage;
                    DataHistory_getStatistics(
                        &batteryVoltageHistory, BATTERY_VOLTAGE_SAMPLES,
                        &minVoltage, &maxVoltage, &avgVoltage);
                    // determine status based on voltage reading
                    if (maxVoltage < BATTERY_10V9) {
                        battStatus = bs_underVoltage;
                    } else if (maxVoltage < BATTERY_12V) {
                        battStatus = bs_lowVoltage;
                    } else if (maxVoltage < BATTERY_13V) {
                        battStatus = bs_goodVoltage;
                    } else {
                        battStatus = bs_fullVoltage;
                    }
                }
                bmState = bms_idle;
            }
            }
            break;
    }
}

