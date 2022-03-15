/**
  * @file     circularBuffer.h
  * @author   Marton.Lorinczi
  * @date     Sep 2, 2020
  *
  * @brief    A circular buffer for holding uint8_t datas.
  *           It is used with the uDebugPrint.h, but if any other project needs a circural buffer like this, it is free to use.
  */

#ifndef MASTER_COMMUNICATION_CIRCULARBUFFER_H_
#define MASTER_COMMUNICATION_CIRCULARBUFFER_H_

#include <stdint.h>

/** @struct circularBuffer
 *  @brief A Circural buffer structure, holding all the necesary values.
 *  @var circularBuffer::head
 *  Circ Buff array's first element's address
 *  @var circularBuffer::tail
 *  Circ Buff array's last element's address
 *  @var circularBuffer::reader
 *  The reader pointer
 *  @var circularBuffer::writer
 *  The writer pointer
 *  @var circularBuffer::bufferSize
 *  Size of the buffer
 *  @var circularBuffer::isFull
 *  Flag for indicate is buffer is foll of data or not.
 */
typedef struct {
    uint8_t *head;
    uint8_t *tail;
    uint8_t *reader;
    uint8_t *writer;
    uint16_t bufferSize;
    uint8_t isFull;
}circularBuffer, *circularBufferPtr;

/**
  * @brief   Creates the circural buffer.
  *          The array in the buffer is not dynamically allocated. so it has to passed by this function.
  * @param   cBuff - Pointer to a cirBuffer
  * @param   buffer - Pointer for the array what will be assigned to circuralBuffer
  * @param   bufferSize - Size of the array.
*/
void circularBufferInit(circularBufferPtr cBuff, uint8_t *buffer, uint16_t bufferSize);

/**
  * @brief  Writes data to the circbuffer.
  * @param  cBuff - Pointer to the circBuffer 
  * @param  dataWrite - Ponter to the array where are the datas staying for be writen in to the circbuff
  * @param  dataReadLen - Length of the data we want to write into the buffer.
  * @return 0 - if write is done\n
  *         1 - There is not enough space in the buffer for #bufferSize of data.\n 
  *         2 - If bufer is full, and data cant be writen in.
*/
uint8_t circularBufferWrite(circularBufferPtr cBuff, uint8_t *dataWrite, uint16_t dataReadLen);

/**
  * @brief   Reads all the data stored in the circular buffer.
  * @param   cBuff - Pointer to the circBuffer 
  * @param   dataRead - Pointer for the outgoing array, this function fills this array with the readed values.
  * @param   dataReadLen - Pointer for a variable, this will be loaded with the value of how many datas were readed by this function.
  * @return  0 - Read done\n
  *          1 - Buffer is empty, cant read. 
*/
uint8_t circularBufferRead( circularBufferPtr cBuff, uint8_t *dataRead, uint16_t *dataReadLen);

#endif /* MASTER_COMMUNICATION_CIRCULARBUFFER_H_ */
