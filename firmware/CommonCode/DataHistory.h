//
//  Data History
//
//  The object defined here is used to retain the latest
//  n data readings, and provide a min, max, and
//  average value over the readings
//
#ifndef DATAHISTORY_H
#define DATAHISTORY_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t tail;
    uint8_t length;
    uint8_t capacity;
    uint16_t* dataBuffer;
} DataHistory_t;

#define DataHistory_define(capacity, name) \
    uint16_t name##_buf[capacity] = {0}; \
    DataHistory_t name = {0, 0, capacity, name##_buf};

inline void DataHistory_clear (
    DataHistory_t* dataHistory)
{
    dataHistory->tail = 0;
    dataHistory->length = 0;
}

extern void DataHistory_insertValue (
    const uint16_t value,
    DataHistory_t* dataHistory);

inline uint8_t DataHistory_length (
    DataHistory_t* dataHistory)
{
    return dataHistory->length;
}

extern uint16_t DataHistory_getLatest (
    const DataHistory_t* dataHistory);

// computes statistics on the latest numSamples data
extern void DataHistory_getStatistics (
    const DataHistory_t* dataHistory,
    const uint8_t numSamples,
    uint16_t* min,
    uint16_t* max,
    uint16_t* avg);

#endif		// DATAHISTORY_H