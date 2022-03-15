/**
  * @file     uOutgoingMessageHandler.h
  * @author   Marton.Lorinczi
  * @date     Sep 14, 2020
  *
  * @brief  OutgoingMessageHandler is responsible for handling the outgoing message queue.
  * 
  *         Only the UrabrosMaster.c is allowed to use the @see uMsgOutPop() and @see uMsgSend() functions.\n
  *         The @see uMsgOutPut() can be used from uTasks. But for easier use there are some gigher level functions for\n
  *         putting a message to the outgoing buffer.
  *         The outgoing buffer is a static variable of this source file, so it's a private variable.
  *         UrabrosMaster.c one of it's field so an extern variable #uMsgOutWaitingNumPtr was created for that purpose.
  */

#include "uOutgoingMessageHandler.h"
#include "uMessageCommon.h"
#include <usart.h>
#include <string.h>

#define DPRINT_LOCAL_ENABLE 1
#include "uDebugPrint.h"

/** Tx buffer for outgoing messages, its size 4 byte bigger than the #MESSAGE_BUFFER_LENGTH.
 * Pos0: Message Type, #MESSAGE_URABROS or #MESSAGE_START_OF_TEXT\n
 * Pos1: Datalen max value is #MESSAGE_BUFFER_LENGTH - 4\n
 * n-1: CRC16 Top 8\n
 * n: CRC16 bot 8
 */
static uint8_t tempTxBuffer[MESSAGE_BUFFER_LENGTH + 4];
static Urabros_MsgOutBuffer uOutgoinggBuffer;               /**< The outgoing buffer.*/
static SemaphoreHandle_t mutex;                             /**< */
uint8_t *uMsgOutWaitingNumPtr = &uOutgoinggBuffer.numOfMsg; /**< Pointer to the outgoing buffers numOfMsg field, because the outgoing buffer is a private variable, with this pointer it's field can be accessed.*/

Urabros_StatusTypeDef uMsgOutInit()
{
    uOutgoinggBuffer.numOfMsg = 0;
    mutex = xSemaphoreCreateMutex();
    for(uint16_t msgIndex = 0; msgIndex < MESSAGE_OUT_ARRAY_LENGTH; msgIndex++) {
        uMsgReset(uOutgoinggBuffer.msgBuff + msgIndex);
    }
    return uStatusOk;
}

// Take out the last message in queue ( First in last out)
Urabros_MsgStatus uMsgOutPop(Urabros_MsgPtr uMsgPtr)
{
    xSemaphoreTake(mutex, portMAX_DELAY);

    //xSemaphoreTake(uIncomingBuffer.mutex, portMAX_DELAY); //TODO TIMEOUT
    if(!uOutgoinggBuffer.numOfMsg) {
        xSemaphoreGive(mutex);
        return uMsg_BufferIsEmpty;
    }

    // Copy the last message to the give pointed message struct.
    uMsgCopy(uMsgPtr, uOutgoinggBuffer.msgBuff + uOutgoinggBuffer.numOfMsg - 1);
    // Reset the message in the buffer
    uMsgReset(uOutgoinggBuffer.msgBuff + uOutgoinggBuffer.numOfMsg - 1);
    // Decrease the number of messages in buffer by one. So we get the index in array.
    uOutgoinggBuffer.numOfMsg--;

    xSemaphoreGive(mutex);
    return uMsg_Ok;
}

// Put a message in to the buffer
Urabros_MsgStatus uMsgOutPut(Urabros_MsgPtr uMsgPtr)
{
    xSemaphoreTake(mutex, portMAX_DELAY);

    if(uOutgoinggBuffer.numOfMsg == MESSAGE_OUT_ARRAY_LENGTH) {
        xSemaphoreGive(mutex);
        return uMsg_BufferIsFull;
    }

    uMsgCopy(uOutgoinggBuffer.msgBuff + uOutgoinggBuffer.numOfMsg, uMsgPtr);
    uOutgoinggBuffer.numOfMsg++;

    xSemaphoreGive(mutex);
    return uMsg_Ok;
}


Urabros_MsgStatus uMsgSend(Urabros_MsgPtr uMsgPtr)
{
    Urabros_StatusTypeDef sendStatus = uStatusOk;
    memset(tempTxBuffer, 0x00, MESSAGE_BUFFER_LENGTH + 4);

    // Send the length of data
    tempTxBuffer[0] = MESSAGE_URABROS;
    tempTxBuffer[1] = uMsgPtr->dataLen;
    // Copy data
    memcpy(tempTxBuffer + 2, uMsgPtr->data, uMsgPtr->dataLen);

    // copy Crc code
    tempTxBuffer[uMsgPtr->dataLen + 2] = uMsgPtr->crc16Code >> 8;
    tempTxBuffer[uMsgPtr->dataLen + 3] = uMsgPtr->crc16Code;

    for(uint8_t i = 0; i < 5; i++) {
        sendStatus = (Urabros_StatusTypeDef)HAL_UART_Transmit_DMA(MESSAGE_UART_MAIN_PTR, tempTxBuffer, uMsgPtr->dataLen + 4);
        if(sendStatus == uStatusOk) {
            break;
        }
    }

    switch (sendStatus) {
    case uStatusOk :
        return uMsg_Ok;
    case uStatusError:
        return uMsg_Error;
    case uStatusBusy:
        return uMsg_Busy;
    default:
        return uMsg_Error;
    }

}

void uMsgInPrintOuttBuffer()
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    dprintln("Outgoing Buffer size: %d", uOutgoinggBuffer.numOfMsg);
    for(uint16_t i = 0; i < uOutgoinggBuffer.numOfMsg; i++) {
        dprintln("Index: %d", i);
        uMsgPrint(uOutgoinggBuffer.msgBuff + i);
    }
    xSemaphoreGive(mutex);
}