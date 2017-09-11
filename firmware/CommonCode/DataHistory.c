//
//  Data History
//

#include "DataHistory.h"


void DataHistory_insertValue (
	const uint16_t value,
	DataHistory_t* dataHistory)
{
    // put data in buffer
    dataHistory->dataBuffer[dataHistory->tail] = value;
	
    // advance tail
    if (dataHistory->tail >= (dataHistory->capacity - 1)) {
        // wrap around
        dataHistory->tail = 0;
    } else {
        ++dataHistory->tail;
    }

    // increment length
    if (dataHistory->length < dataHistory->capacity) {
        ++dataHistory->length;
    }
}

uint16_t DataHistory_getLatest (
    const DataHistory_t* dataHistory)
{
    uint16_t latest = 0;

    if (dataHistory->length > 0) {
        uint8_t dataIndex = ((dataHistory->tail == 0)
            ? dataHistory->capacity
            : dataHistory->tail) - 1;
        latest = dataHistory->dataBuffer[dataIndex];
    }

    return latest;
}

void DataHistory_getStatistics (
    const DataHistory_t* dataHistory,
    const uint8_t numSamples,
    uint16_t* min,
    uint16_t* max,
    uint16_t* avg)
{
    *min = 65535;
    *max = 0;
    *avg = 0;

    if (dataHistory->length > 0) {
        uint32_t sum = 0;
        uint8_t dataIndex = ((dataHistory->tail == 0)
            ? dataHistory->capacity
            : dataHistory->tail) - 1;
        for (int i = 0; i < numSamples; ++i) {
            const uint16_t sample = dataHistory->dataBuffer[dataIndex];
            if (sample < *min) {
                *min = sample;
            }
            if (sample > *max) {
                *max = sample;
            }
            sum += sample;

            dataIndex = (
                (dataIndex == 0)
                    ? dataHistory->capacity
                    : dataIndex) - 1;
        }
        *avg = (uint16_t)(sum / numSamples);
    }
}
