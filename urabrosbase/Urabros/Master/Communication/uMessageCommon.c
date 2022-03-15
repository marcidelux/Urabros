/**
  * @file     uMessageCommon.c
  * @author   Marton.Lorinczi
  * @date     Feb 8, 2021
  *
  * @brief  This file contains the common uMessage relevant functions.
  *         
  *         This had to be created because uOutgoingMessageHandler.h and "uIncomingMessageHandler.h" are uses the same
  *         functions, and to avoid duplicates, this file had to be made.
  */

#include "uMessageCommon.h"
#include "crc16.h"
#include "string.h"


#define DPRINT_LOCAL_ENABLE 1
#include "uDebugPrint.h"

/** Using this extern function is breaking the architecture, but to make it fast end easy this was the only solution.
 *  There was a request from a user to make a more friendly function what can place a buffer to the outgoing message,
 *  without using crc calculation and other stuffs. This method needed multiple functions from multiple files, and avoiding
 *  too much header includes, I just used the extern keyword, if you come up with a better solution dont hesitate to tell it to the owner of the framework :)
 */
extern Urabros_MsgStatus uMsgOutPut(Urabros_MsgPtr uMsgPtr);

void uMsgReset(Urabros_MsgPtr uMsgTarget)
{
    uMsgTarget->dataLen    = 0;
    uMsgTarget->crc16Code  = 0;
    memset(uMsgTarget->data, 0x00, MESSAGE_BUFFER_LENGTH);
}

void uMsgCopy(Urabros_MsgPtr uMsgTarget, Urabros_MsgPtr uMsgBase)
{
   uMsgTarget->dataLen      = uMsgBase->dataLen;
   uMsgTarget->crc16Code    = uMsgBase->crc16Code;
   memcpy(uMsgTarget->data, uMsgBase->data, MESSAGE_BUFFER_LENGTH);
}

Urabros_MsgStatus uMsgCheckCrc(Urabros_MsgPtr uMsgPtr)
{
    uint16_t calcCrc16 = crc_modbus(uMsgPtr->data, uMsgPtr->dataLen);
    if(calcCrc16 == uMsgPtr->crc16Code) {
        return uMsg_Ok;
    } else {
        return uMsg_CrcError;
    }
}


void uMsgSetCrc(Urabros_MsgPtr uMsgPtr)
{
    if(!uMsgPtr->dataLen)
        return;

    uMsgPtr->crc16Code = crc_modbus(uMsgPtr->data, uMsgPtr->dataLen);
}

Urabros_MsgStatus uMsgAppend(Urabros_MsgPtr uMsgPtr, uint8_t data)
{
    if(uMsgPtr->dataLen >= MESSAGE_BUFFER_LENGTH) {
        return uMsg_BufferIsFull;
    }

    uMsgPtr->data[uMsgPtr->dataLen] = data;
    uMsgPtr->dataLen++;

    return uMsg_Ok;
}

Urabros_MsgStatus uMsgAppendBuffer(Urabros_MsgPtr uMsgPtr, uint8_t *buff, uint8_t buffLen)
{
    if(uMsgPtr->dataLen + buffLen >= MESSAGE_BUFFER_LENGTH) {
        return uMsg_CopyBufferTooBig;
    }

    // Copy the buffer to the end of the data buffer.
    memcpy(uMsgPtr->data + uMsgPtr->dataLen, buff, buffLen);

    // Increment the datalen index with bufflen
    uMsgPtr->dataLen += buffLen;

    return uMsg_Ok;
}

Urabros_MsgStatus uMsgAppendBufferFromTask(Urabros_TaskPtrTypeDef uTaskPtr, Urabros_MsgPtr uMsgPtr, uint8_t *buff, uint8_t buffLen)
{
    // First time check
    if(!uMsgPtr->dataLen) {
        if(uMsgPtr->dataLen + buffLen + 2 >= MESSAGE_BUFFER_LENGTH) {
            return uMsg_CopyBufferTooBig;
        }
    }

    // After first time check
    if(uMsgPtr->dataLen + buffLen >= MESSAGE_BUFFER_LENGTH) {
        return uMsg_CopyBufferTooBig;
    }

    // If its the first time append it with command ID and task ID
    if(!uMsgPtr->dataLen) {
        uMsgPtr->data[0] = uCommand_DATA_FROM_TASK;
        uMsgPtr->data[1] = uTaskPtr->responsibleTaskId;
        uMsgPtr->dataLen = 2;
    }

    // Copy the buffer to the end of the data buffer.
    memcpy(uMsgPtr->data + uMsgPtr->dataLen, buff, buffLen);

    // Increment the datalen index with bufflen
    uMsgPtr->dataLen += buffLen;

    return uMsg_Ok;
}

Urabros_MsgStatus uMsgSendMessageFromTask(Urabros_TaskPtrTypeDef uTaskPtr, uint8_t *buff, uint8_t buffLen)
{
    if(buffLen + 2 >= MESSAGE_BUFFER_LENGTH) {
        return uMsg_CopyBufferTooBig;
    }

    Urabros_Msg tempMessage;

    //Reset the temp message.
    uMsgReset(&tempMessage);

    // Set the Data from and task ID and length
    tempMessage.data[0] = uCommand_DATA_FROM_TASK;
    tempMessage.data[1] = uTaskPtr->responsibleTaskId;

    // Copy the buffer to the end of the data buffer.
    memcpy(tempMessage.data + 2, buff, buffLen);

    // Set the length of message
    tempMessage.dataLen = buffLen + 2;

    // Set the CRC code
    tempMessage.crc16Code = crc_modbus(tempMessage.data, tempMessage.dataLen);

    //Put it to the outgoing queue
    return uMsgOutPut(&tempMessage);
}

void uMsgPrint(Urabros_MsgPtr uMsgPtr)
{
    dprintln("Msg len:%d", uMsgPtr->dataLen);
    dprint("Buff: |");
    for(uint8_t i = 0; i < uMsgPtr->dataLen; i++) {
        dprint("%02X|", uMsgPtr->data[i]);
    }
    dprintln("\nCrc16: %04X\n", uMsgPtr->crc16Code);
}
