//
// Byte Queue
//
// supports the usual push to tail and pop from head operations
//
#ifndef BYTEQUEUE_LOADED
#define BYTEQUEUE_LOADED

#include <stdbool.h>
#include "inttypes.h"

#include <avr/interrupt.h>

typedef uint8_t ByteQueueElement;

typedef struct {
    uint16_t head;
    uint16_t tail;
    uint16_t length;
    uint16_t capacity;
    ByteQueueElement *bytes;
    } ByteQueue;

#define ByteQueue_define(capacity, queueName) \
    ByteQueueElement queueName##_buf[capacity] = {0}; \
    ByteQueue queueName = {0, 0, 0, capacity, queueName##_buf};

extern void ByteQueue_clear (
    ByteQueue *q);

// returns the current length of the queue
inline uint16_t ByteQueue_length (
    const ByteQueue *q)
    {
    char SREGSave;
    SREGSave = SREG;
    cli();

    const uint16_t len = q->length;

    SREG = SREGSave;

    return len;
    }

// returns the length available in the queue
inline uint16_t ByteQueue_spaceRemaining (
    const ByteQueue *q)
    {
    char SREGSave;
    SREGSave = SREG;
    cli();

    const uint16_t remaining = q->capacity - q->length;

    SREG = SREGSave;

    return remaining;
    }

// returns true if the queue is currently empty
inline bool ByteQueue_is_empty (
   const ByteQueue *q)
   {
    char SREGSave;
    SREGSave = SREG;
    cli();

    const bool empty = q->length == 0;

    SREG = SREGSave;

    return empty;
    }

// returns true if the queue is currently full
inline bool ByteQueue_is_full (
   const ByteQueue *q)
   {
    char SREGSave;
    SREGSave = SREG;
    cli();

    const bool full = q->length == q->capacity;

    SREG = SREGSave;

    return full;
    }

// assumes the queue is not empty
inline ByteQueueElement ByteQueue_head (
   const ByteQueue *q)
{
    char SREGSave;
    SREGSave = SREG;
    cli();

    const ByteQueueElement h = q->bytes[q->head];

    SREG = SREGSave;

    return h;
}

// pushes a byte onto the tail of the queue, if it's not full. returns
// true if successful
extern bool ByteQueue_push (
   const ByteQueueElement byte,
   ByteQueue *q);

// pops a byte from the head of the queue, expects it's not empty
extern ByteQueueElement ByteQueue_pop (
   ByteQueue *q);

#endif   // BYTEQUEUE_LOADED
