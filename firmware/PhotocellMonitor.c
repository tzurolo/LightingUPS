//
//  Photocell Monitor
//
//  Monitors the photocell and provides ambient light level
//  The photocell has fairly slow response. Tests show it takes
//  about 100mS to settle at a new higher light level, and a
//  little more going to a lower light level.
//
//  Pin usage:
//     ADC3 (PA3) - input from photocell voltage divider
//

#include "PhotocellMonitor.h"

#include "ADCManager.h"
#include "SystemTime.h"
#include "DataHistory.h"

#define PHOTOCELL_ADC_CHANNEL 3

#define SAMPLE_INTERVAL (SYSTEMTIME_TICKS_PER_SECOND / 20)
#define PHOTOCELL_SAMPLES 5

typedef enum {
    pms_idle,
    pms_waitingForADCStart,
    pms_waitingForADCCompletion
} PhotocellMonitor_state;

static PhotocellMonitor_state pmState = pms_idle;
static SystemTime_Timer sampleTimer;
static uint16_t photocellVoltage;
DataHistory_define(PHOTOCELL_SAMPLES, lightLevelHistory);

// ADC counts to percent (1023 counts is 100%)
static inline uint8_t countsToPercent (
    const uint16_t counts)
{
    // counts to percent
    const int32_t c32 = counts;
    return (uint8_t)((c32 * 100) / 1024);
}

void PhotocellMonitor_Initialize (void)
{
    pmState = pms_idle;
    SystemTime_startTimer(0, &sampleTimer); // start sampling immediately
    DataHistory_clear(&lightLevelHistory);

    // set up the ADC channel for measuring photocell voltage
    ADCManager_setupChannel(PHOTOCELL_ADC_CHANNEL, ADC_REF_VCC, false);
}

bool PhotocellMonitor_haveValidSample (void)
{
    return (DataHistory_length(&lightLevelHistory) >= PHOTOCELL_SAMPLES);
}

uint8_t PhotocellMonitor_currentLightLevel (void)
{
    const uint16_t latestVoltage = DataHistory_getLatest(&lightLevelHistory);
    return countsToPercent(latestVoltage);
}

uint8_t PhotocellMonitor_averageLightLevel (void)
{
    uint16_t photocellVoltage = 0;
    if (PhotocellMonitor_haveValidSample()) {
        uint16_t minVoltage;
        uint16_t maxVoltage;
        DataHistory_getStatistics(
            &lightLevelHistory, PHOTOCELL_SAMPLES,
            &minVoltage, &maxVoltage, &photocellVoltage);
    }

    return countsToPercent(photocellVoltage);
}

void PhotocellMonitor_clearStatistics (void)
{
    DataHistory_clear(&lightLevelHistory);
}

void PhotocellMonitor_task (void)
{
    switch (pmState) {
        case pms_idle : {
            if (SystemTime_timerHasExpired(&sampleTimer)) {
                SystemTime_startTimer(SAMPLE_INTERVAL, &sampleTimer);
                pmState = pms_waitingForADCStart;
            }
            break;
        case pms_waitingForADCStart :
            if (ADCManager_StartConversion(PHOTOCELL_ADC_CHANNEL)) {
                // successfully started conversion
                pmState = pms_waitingForADCCompletion;
            }
            break;
        case pms_waitingForADCCompletion :
            if (ADCManager_ConversionIsComplete(&photocellVoltage)) {
                DataHistory_insertValue(photocellVoltage, &lightLevelHistory);
                pmState = pms_idle;
            }
            }
            break;
    }
}

