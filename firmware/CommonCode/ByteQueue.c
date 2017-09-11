//
// Byte Queue
//

#include "ByteQueue.h"

void ByteQueue_clear (
    ByteQueue *q)
    {
    char SREGSave;
    SREGSave = SREG;
    cli();

    q->head = 0;
    q->tail = 0;
    q->length = 0;

    SREG = SREGSave;
    }

bool ByteQueue_push (
    const ByteQueueElement byte,
    ByteQueue *q)
    {
    bool push_successful = false;

    char SREGSave;
    SREGSave = SREG;
    cli();

    if (q->length < q->capacity)
        {  // the queue is not full
        // put the byte in the queue
        q->bytes[q->tail] = byte;

        // advance the tail pointer
        if (q->tail < (q->capacity - 1))
            ++q->tail;
        else  // wrap around
            q->tail = 0;

        // increment length
        ++q->length;

        push_successful = true;
        }

    SREG = SREGSave;

    return push_successful;
    }

ByteQueueElement ByteQueue_pop (
    ByteQueue *q)
{
    ByteQueueElement byte = 0;

    char SREGSave;
    SREGSave = SREG;
    cli();

    if (q->length > 0) {  // the queue is not empty
        // get the byte from the queue
        byte = q->bytes[q->head];

        // advance the head pointer
        if (q->head < (q->capacity - 1))
            ++q->head;
        else  // wrap around
            q->head = 0;

        // decrement length
        --q->length;
    }

    SREG = SREGSave;

    return byte;
}
