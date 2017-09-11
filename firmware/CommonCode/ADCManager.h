//
//  Analog to Digital Converter Manager
//
//  Used to reserve the ADC and do conversions
//
#ifndef ADCMANAGER_H
#define ADCMANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>

// ADC voltage reference selection (use one only)
#define ADC_REF_VCC         0
#define ADC_REF_AREF        (1 << REFS0)
#define ADC_REF_INTERNAL    (1 << REFS1)

#define COUNTS_PER_VOLT 205

// called once at power-up
extern void ADCManager_Initialize (void);

// call once at the beginning of program after power-up for
// each channel you intend to use
extern void ADCManager_setupChannel (
    const uint8_t channelIndex,
    const uint8_t adcRef,   // one of ADC_REF_xxx
    const bool leftAdjustResult);

// schedules and reads the requested channel
// called in each iteration of the mainloop
extern void ADCManager_task (void);

// returns true and starts a conversion if the ADC is
// available (not in use by another caller).
// channel must be one of ADC_CHANNELn as defined in
// LUFA/Drivers/Peripheral/ADC.h
extern bool ADCManager_StartConversion (
    const uint8_t channelIndex);

// returns true when the conversion is complete and
// returns the analog value. only passes the value
// to the caller the first time it returns true
extern bool ADCManager_ConversionIsComplete (
    uint16_t* analogValue);

#endif  // ADCMANAGER_H
