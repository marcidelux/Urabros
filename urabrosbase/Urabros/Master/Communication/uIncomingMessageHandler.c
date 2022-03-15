/**
 * @file     uIncomingMessageHandler.c
 * @author   Marton.Lorinczi
 * @date     Sep 14, 2020
 *
 * @brief  IncomingMessageHandler is responsible for handling the incoming message queue.
 *         Further informations in the header file.
 */
#include "uIncomingMessageHandler.h"
#include "uMessageCommon.h"
#include <usart.h>
#include "crc16.h"
#include "string.h"

#define DPRINT_LOCAL_ENABLE 1
#include "uDebugPrint.h"

/** Buffer for holding incoming messages.
 */
static Urabros_MsgInBuffer uIncomingBuffer;

/** Temporally byte buffer for DMA input.
*/
static uint8_t dmaRxBuffer[DMA_RX_BUFFER_SIZE];

/** Put a message in to #uIncomingBuffer
 */ 
static Urabros_MsgStatus uMsgInPut(Urabros_MsgPtr uMsgPtr);

/* PUBLIC FUNCTIONS */
Urabros_StatusTypeDef uMsgInInit()
{

    // Clear the buffer
    uIncomingBuffer.numOfMsg = 0;
    for(uint16_t msgIndex = 0; msgIndex < MESSAGE_IN_ARRAY_LENGTH; msgIndex++) {
        uMsgReset(uIncomingBuffer.msgBuff + msgIndex);
    }

    //enable IDLE detection
    __HAL_UART_ENABLE_IT(MESSAGE_UART_MAIN_PTR, UART_IT_IDLE);
    //start receiving MESSAGE_RX_BUFFER_SIZE amount of bytes
    while(HAL_UART_Receive_DMA(MESSAGE_UART_MAIN_PTR, dmaRxBuffer, DMA_RX_BUFFER_SIZE));

    return uStatusOk;
}

// Going from the last element pops out
Urabros_MsgStatus uMsgInPop(Urabros_MsgPtr uMsgPtr)
{
    //xSemaphoreTake(uIncomingBuffer.mutex, portMAX_DELAY); //TODO TIMEOUT
    if(!uIncomingBuffer.numOfMsg) {
        //xSemaphoreGive(uIncomingBuffer.mutex);
        return uMsg_BufferIsEmpty;
    }

    // Copy the last message to the give pointed message struct.
    uMsgCopy(uMsgPtr, uIncomingBuffer.msgBuff + uIncomingBuffer.numOfMsg - 1);
    // Reset the message in the buffer
    uMsgReset(uIncomingBuffer.msgBuff + uIncomingBuffer.numOfMsg - 1);
    // Decrease the number of messages in buffer by one. So we get the index in array.
    uIncomingBuffer.numOfMsg--;

    //xSemaphoreGive(uIncomingBuffer.mutex);
    return uMsg_Ok;
}

Urabros_MsgStatus uMsgInPut(Urabros_MsgPtr uMsgPtr)
{
    if(uIncomingBuffer.numOfMsg == MESSAGE_IN_ARRAY_LENGTH) {
        return uMsg_BufferIsFull;
    }

    uMsgCopy(uIncomingBuffer.msgBuff + uIncomingBuffer.numOfMsg, uMsgPtr);
    uIncomingBuffer.numOfMsg++;

    return uMsg_Ok;
}


Urabros_MsgStatus uMsgPutFromDMA(void)
{
    static uint8_t first = 1;
    if(first) {
        first = 0;
        return uMsg_Ok;
    }

    Urabros_Msg tempMsg = {0};
    static uint8_t old_pos;
    static uint8_t tempRxBuffer[MESSAGE_BUFFER_LENGTH + 3];
    uint16_t pos;
    uint16_t recDataLen;
    Urabros_MsgStatus status = uMsg_Ok;
    /* Calculate current position in buffer */
    pos = DMA_RX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(MESSAGE_UART_MAIN.hdmarx);


    if (pos != old_pos) {                       /* Check change in received data */
        if (pos > old_pos) {                    /* Current position is over previous one */
            /* We are in "linear" mode */
            /* Process data directly by subtracting "pointers" */

            // Length of data received
            recDataLen = pos - old_pos;

            // Minimum received data length size is 4 --> | datalen | commandID | CRC 1 | CRC 2 |
            if(recDataLen < 4) {
                tempMsg.dataLen = 2;
                tempMsg.data[0] = uCommand_RECEIVE_ERROR;
                tempMsg.data[1] = uMSg_IdleError;
                uMsgInPut(&tempMsg);
                status = uMSg_IdleError;
                goto end;
            }

            if(recDataLen - 3 != dmaRxBuffer[old_pos]) {
                tempMsg.dataLen = 2;
                tempMsg.data[0] = uCommand_RECEIVE_ERROR;
                tempMsg.data[1] = uMsg_DataLenError;
                uMsgInPut(&tempMsg);
                status = uMsg_DataLenError;
                goto end;
            }

            // Fill temp message
            tempMsg.dataLen = dmaRxBuffer[old_pos];
            for(uint8_t i = 0; i < (recDataLen - 3); i++) {
                tempMsg.data[i] = dmaRxBuffer[old_pos + 1 + i];
            }
            tempMsg.crc16Code += (uint16_t)dmaRxBuffer[pos - 2] << 8;
            tempMsg.crc16Code += dmaRxBuffer[pos - 1];

            if(uMsgCheckCrc(&tempMsg) == uMsg_Ok) {
                status = uMsgInPut(&tempMsg);
            } else {
                tempMsg.dataLen = 2;
                tempMsg.data[0] = uCommand_RECEIVE_ERROR;
                tempMsg.data[1] = uMsg_CrcError;
                uMsgInPut(&tempMsg);
                status = uMsg_CrcError;
            }


        } else {
            /* We are in "overflow" mode */

            //easy way:
            uint16_t recDataLenPart1 = DMA_RX_BUFFER_SIZE - old_pos;
            uint16_t recDataLenPart2 = pos;
            recDataLen = recDataLenPart1 + recDataLenPart2;

            if(recDataLen < 4) {
                tempMsg.dataLen = 2;
                tempMsg.data[0] = uCommand_RECEIVE_ERROR;
                tempMsg.data[1] = uMSg_IdleError;
                uMsgInPut(&tempMsg);
                status = uMSg_IdleError;
                status = uMSg_IdleError;
                goto end;
            }

            if(recDataLen - 3 != dmaRxBuffer[old_pos]) {
                tempMsg.dataLen = 2;
                tempMsg.data[0] = uCommand_RECEIVE_ERROR;
                tempMsg.data[1] = uMsg_DataLenError;
                uMsgInPut(&tempMsg);
                status = uMsg_DataLenError;
                goto end;
            }

            memcpy(tempRxBuffer, dmaRxBuffer + old_pos, recDataLenPart1);
            /* Check and continue with beginning of buffer */
            if (recDataLenPart2 > 0) {
                memcpy(tempRxBuffer + DMA_RX_BUFFER_SIZE - old_pos, dmaRxBuffer, recDataLenPart2);
            }

            // Fill temp message
            tempMsg.dataLen = tempRxBuffer[0];
            for(uint16_t i = 0; i < recDataLen - 3; i++) {
                tempMsg.data[i] = tempRxBuffer[i + 1];
            }
            tempMsg.crc16Code += (uint16_t)tempRxBuffer[recDataLen - 2] << 8;
            tempMsg.crc16Code += tempRxBuffer[recDataLen - 1];

            if(uMsgCheckCrc(&tempMsg) == uMsg_Ok) {
                status = uMsgInPut(&tempMsg);
            } else {
                tempMsg.dataLen = 2;
                tempMsg.data[0] = uCommand_RECEIVE_ERROR;
                tempMsg.data[1] = uMsg_CrcError;
                uMsgInPut(&tempMsg);
                status = uMsg_CrcError;
            }
        }
    }

end:
    old_pos = pos;                              /* Save current position as old */

    /* Check and manually update if we reached end of buffer */
    if (old_pos == DMA_RX_BUFFER_SIZE) {
        old_pos = 0;
    }
    return status;
}

/**
  * @param  UART_HandleTypeDef *huart - pointer to the uart handler
  * @return void -
  * @brief This function is depending on the communication peripheral
  * @note This function is called by HAL.
*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart == MESSAGE_UART_MAIN_PTR)
    {
        //TODO: some code here if we want to send signal to a message handler!
        /*
        for(uint8_t i = 0; i < 10; i++) {
            dprint("dma full interrupt\n");
            if(HAL_UART_Receive_DMA(MESSAGE_UART_MAIN_PTR, dmaRxBuffer, DMA_RX_BUFFER_SIZE) == HAL_OK) {
                break;
            }
        }
        */
    }
}


void uMsgInPrintBuffer()
{
    for(uint16_t i = 0; i < uIncomingBuffer.numOfMsg; i++) {
        dprintln("IN Index: %d", i);
        uMsgPrint(uIncomingBuffer.msgBuff + i);
    }
}

