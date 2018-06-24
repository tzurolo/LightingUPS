//
//  Temperature Sensor
//
//  This unit reads the DS18B20 temperature sensor at
//  regular intervals
//
#ifndef TEMPERATURESENSOR_H
#define TEMPERATURESENSOR_H

#include <stdint.h>
#include <stdbool.h>
#include "CharString.h"

#define COUNTS_PER_DEGREE_C 16

typedef enum TemperatureSensor_measurementStatusEnum {
    ms_notStarted,
    ms_inProgress,              // measurement is still in progress
    ms_completedSuccessfully,   // measurement compeleted succesfully
    ms_error                    // 
} TemperatureSensor_measurementStatus;

extern void TemperatureSensor_Initialize (void);

extern bool TemperatureSensor_isIdle (void);

// returns true if measurement successfully started
extern bool TemperatureSensor_startMeasurement (void);

// returns status of measurement started by startMeasurement
// temperature in units of 1/16 degree C
extern TemperatureSensor_measurementStatus TemperatureSensor_getMeasurement (
    int16_t *temp);

extern void TemperatureSensor_task (void);

extern void TemperatureSensor_appendTempToString (
    const int16_t temp,
    CharString_t* string);

#endif  // TEMPERATURESENSOR_H
