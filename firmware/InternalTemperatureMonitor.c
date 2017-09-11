//
//  Internal Temperature Monitor
//
//  Reads the AtTiny84's internal temperature sensor through the ADC
//

#include "InternalTemperatureMonitor.h"

#include "ADCManager.h"
#include "SystemTime.h"
#include "DataHistory.h"

#define SENSOR_ADC_CHANNEL 8

#define REFERENCE_VOLTAGE 1.1

// at 20 deg C we got 349 counts from the AtTiny84
#define ESTIMATED_ZERO_C_COUNTS 286

#define T_OFFSET (-266)

#define SENSOR_POWERUP_DELAY SYSTEMTIME_TICKS_PER_SECOND / 5
#define SENSOR_SAMPLE_TIME SYSTEMTIME_TICKS_PER_SECOND / 10
#define SENSOR_SAMPLES 10

typedef enum {
    tms_idle,
    tms_waitingForADCStart,
    tms_waitingForADCCompletion
} TemperatureMonitor_state;

static TemperatureMonitor_state tmState = tms_idle;
static SystemTime_Timer sampleTimer;
DataHistory_define(SENSOR_SAMPLES, temperatureHistory);

void InternalTemperatureMonitor_Initialize (void)
{
    tmState = tms_idle;
    // start sampling in 1/50 second (20ms, to let power stabilize)
    SystemTime_startTimer(SYSTEMTIME_TICKS_PER_SECOND, &sampleTimer);
    DataHistory_clear(&temperatureHistory);

    // set up the ADC channel for measuring battery voltage
    ADCManager_setupChannel(SENSOR_ADC_CHANNEL, ADC_REF_INTERNAL, false);
}

bool InternalTemperatureMonitor_haveValidSample (void)
{
    return DataHistory_length(&temperatureHistory) >= SENSOR_SAMPLES;
}

int16_t InternalTemperatureMonitor_currentTemperature (void)
{
    uint16_t avgTemperature = 0;
    if (DataHistory_length(&temperatureHistory) >= SENSOR_SAMPLES) {
        uint16_t minTemmp;
        uint16_t maxTemp;
        DataHistory_getStatistics(
            &temperatureHistory, SENSOR_SAMPLES,
            &minTemmp, &maxTemp, &avgTemperature);
    }

    // counts to degrees C
    return avgTemperature + T_OFFSET;
}

void InternalTemperatureMonitor_task (void)
{
    switch (tmState) {
        case tms_idle :
            if (SystemTime_timerHasExpired(&sampleTimer)) {
                SystemTime_startTimer(SENSOR_SAMPLE_TIME, &sampleTimer);
                tmState = tms_waitingForADCStart;
            }
            break;
        case tms_waitingForADCStart :
            if (ADCManager_StartConversion(SENSOR_ADC_CHANNEL)) {
                // successfully started conversion
                tmState = tms_waitingForADCCompletion;
            }
            break;    
        case tms_waitingForADCCompletion : {
            uint16_t temperature;
            if (ADCManager_ConversionIsComplete(&temperature)) {
                DataHistory_insertValue(temperature, &temperatureHistory);

                tmState = tms_idle;
            }
            }
            break;
    }
}

