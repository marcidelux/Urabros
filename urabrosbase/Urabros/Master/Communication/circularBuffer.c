/**
  * @file     circularBuffer.c
  * @author   Marton.Lorinczi
  * @date     Sep 2, 2020
  *
  * @brief    A circular buffer for holding uint8_t datas.
  *           It is used with the uDebugPrint.h, but if any other project needs a circural buffer like this, it is free to use.
  */

#include "circularBuffer.h"

typedef enum {
    READER_TO_WRITER,
    WRITER_TO_READER,
}pointerDistanceType;

typedef enum {
    NOT_TURNED,
    TURNED,
}bufferTurnedType;

// Private function
static uint16_t distancePointers(circularBufferPtr cBuff, pointerDistanceType distanceType, uint8_t *turned )
{
    uint8_t *A;
    uint8_t *B;

    *turned = NOT_TURNED;

    if(distanceType == READER_TO_WRITER) {
        A = cBuff->reader;
        B = cBuff->writer;
        if(A == B) {
            if(cBuff->isFull) {
                if(A != cBuff->head) {
                    *turned = TURNED;
                }
                return cBuff->bufferSize;
            } else {
                return 0;
            }
        }
    } else if(distanceType == WRITER_TO_READER) {
        A = cBuff->writer;
        B = cBuff->reader;
        if(A == B) {
            if(A != cBuff->head) {
                *turned = TURNED;
            }
            return cBuff->bufferSize;
        }
    }

    if (A < B) {
        return B - A;
    } else { // A > B
        *turned = TURNED;
        return (cBuff->tail - A) + (B - cBuff->head);
    }
}


void circularBufferInit(circularBufferPtr cBuff, uint8_t *buffer, uint16_t bufferSize)
{
    cBuff->bufferSize   = bufferSize;
    cBuff->head         = buffer;
    cBuff->tail         = buffer + (bufferSize - 1);
    cBuff->reader       = buffer;
    cBuff->writer       = buffer;
    cBuff->isFull       = 0;

    for(uint16_t i = 0; i < bufferSize; i++) {
        buffer[i] = 0;
    }
}

uint8_t circularBufferWrite(circularBufferPtr cBuff, uint8_t *dataWrite, uint16_t dataWriteLen)
{
    uint8_t turned;
    uint16_t distance = distancePointers(cBuff, WRITER_TO_READER, &turned);

    if(dataWriteLen > distance) {
        return 1; // Not enough space to write
    } else if(dataWriteLen == distance) {
        cBuff->isFull = 1;
    } else if(cBuff->isFull) {
        return 2; // Buffer is full
    }

    for(uint16_t i = 0; i < dataWriteLen; i++) {
        *(cBuff->writer) = *(dataWrite + i);
        if(cBuff->writer == cBuff->tail)
            cBuff->writer = cBuff->head;
        else
            cBuff->writer++;
    }

    return 0;
}

uint8_t circularBufferRead(circularBufferPtr cBuff, uint8_t *dataRead, uint16_t *dataReadLen)
{
    uint16_t dataReadIndex = 0;
    uint8_t turned = NOT_TURNED;

    *dataReadLen = distancePointers(cBuff, READER_TO_WRITER, &turned);

    if (!*dataReadLen && !cBuff->isFull) {
        return 1; // Buffer is empty
    }

    if(turned) {
        uint16_t toTail = cBuff->tail - cBuff->reader + 1;
        uint16_t fromHead = cBuff->writer - cBuff->head;

        for(uint16_t i = 0; i < toTail; i++) {
            *(dataRead + dataReadIndex) = *cBuff->reader;
            *cBuff->reader = 0;
            cBuff->reader++;
            dataReadIndex++;
        }
        cBuff->reader = cBuff->head;

        for(uint16_t i = 0; i < fromHead; i++) {
            *(dataRead + dataReadIndex) = *cBuff->reader;
            *cBuff->reader = 0;
            cBuff->reader++;
            dataReadIndex++;
        }

    } else { // Not turned
        for(uint16_t i = 0; i < *dataReadLen; i++) {
            *(dataRead + dataReadIndex) = *cBuff->reader;
            *cBuff->reader = '\0';
            cBuff->reader++;
            dataReadIndex++;
        }
    }

    cBuff->isFull = 0;
    return 0;
}
