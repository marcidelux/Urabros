/**
  * @file     uMessageCommon.h
  * @author   Marton.Lorinczi
  * @date     Feb 8, 2021
  *
  * @brief  This file contains the common uMessage relevant functions.
  *         
  *         This had to be created because uOutgoingMessageHandler.h and "uIncomingMessageHandler.h" are uses the same
  *         functions, and to avoid duplicates, this file had to be made.
  */

#ifndef MASTER_COMMUNICATION_UMESSAGECOMMON_H_
#define MASTER_COMMUNICATION_UMESSAGECOMMON_H_

#include "UrabrosTypeDef.h"

/**
  * @brief  Clears the target #Urabros_Msg by the given pointer.
  * @param  uMsgTarget - Target of the message to be cleared
*/
void uMsgReset(Urabros_MsgPtr uMsgPtr);

/**
  * @brief  Copies the Base Urabros_Msg to the Target
  * @param  uMsgTarget - Target of the copy function
  * @param  uMsgBase - Base of the copy function
*/
void uMsgCopy(Urabros_MsgPtr uMsgTarget, Urabros_MsgPtr uMsgBase);

/**
  * @brief  Calculates and sets the Crc filed of the gien #Urabros_Msg
  * @param  uMsgPtr - The pointed message.
*/
void uMsgSetCrc(Urabros_MsgPtr uMsgPtr);

/**
  * @brief  Checks if the crcCode in UrabrosMsg struct pointed by uMsgPtr is correct or not.
  * 
  *         This function is used in the @see uMsgPutFromDMA() what is called by the DMA intteruupt.
  * @return Status of the function process. Currently this return value is not processed.\n
  *         TODO: Do something with the return value. The problem with it the ret val is in an interrupt handling, this is not so easy :D
*/
Urabros_MsgStatus uMsgCheckCrc(Urabros_MsgPtr uMsgPtr);

/**
  * @brief  Appends the pointed #Urabros_Msg with the given one byte of data.
  * @param  uMsgPtr - The pointed message.
  * @param  data - One byte of data to be added to the message's buffer.
  * @return returns #uMsg_Ok if the data was added.\n
  *         returns #uMsg_BufferIsFull if the buffer was full, and data cannot be added to it. 
*/
Urabros_MsgStatus uMsgAppend(Urabros_MsgPtr uMsgPtr, uint8_t data);

/**
  * @brief  Appends the pointed #Urabros_Msg with the given datas. Maximum datas to be added is #MESSAGE_BUFFER_LENGTH
  * @param  uMsgPtr - The pointed message.
  * @param  buff - Pointer to the databuffer where the desired datas are waiting to be added to the message.
  * @param  buffLen - How many datas to be copied from the buffer to the message's buffer.
  * @return returns #uMsg_Ok if the data was added.\n
  *         returns #uMsg_CopyBufferTooBig if there is no enough space in the message's buffer.
*/
Urabros_MsgStatus uMsgAppendBuffer(Urabros_MsgPtr uMsgPtr, uint8_t *buff, uint8_t buffLen);

/**
  * @brief  This is a user friendly function, usefull at uTasks skope.
  * 
  *         Normally the UrabrosMaster makes the outgoing message, but if a task has to put a message directly in to the queue
  *         than use this function. The first parameter has to be &Task, sadly this cannot be hided inside this function.
  *         There are some other functions needed to be called to sucsesfully create and put a message to the outgoing queue.
  *         Example:
  *         Urabros_Msg msgFromTask;
  *         uint8_t dataBuffer[10]  = {0,1,2,3,4,5,6,7,8,9};
  * 
  *         uMsgReset(&msgFromTask);                                        // Resets the uMsg
  *         uMsgAppendBufferFromTask(&Task, &msgFromTask, dataBuffer, 10);  // Appends the uMsg with the message type Task Id and the given data
  *         uMsgSetCrc(&msgFromTask);                                       // Sets the CRC
  *         uMsgOutPut(&msgFromTask);                                       // Puts the message to the outgoing buffer.
  * @param  uTaskPtr - Pointer of the caller uTask, alwazs use: &Task here.
  * @param  uMsgPtr - The pointed message.
  * @param  buff - Pointer to the databuffer where the desired datas are waiting to be added to the message.
  * @param  buffLen - How many datas to be copied from the buffer to the message's buffer.
  * @return returns #uMsg_Ok if the data was added.\n
  *         returns #uMsg_CopyBufferTooBig if there is no enough space in the message's buffer.
*/
Urabros_MsgStatus uMsgAppendBufferFromTask(Urabros_TaskPtrTypeDef uTaskPtr, Urabros_MsgPtr uMsgPtr, uint8_t *buff, uint8_t buffLen);

/**
  * @brief  This is more user friendly function than @see uMsgAppendBufferFromTask().
  *         
  *         With this function an uint8_t buffer can be directly sent via the outgoing mesage queue.
  *         You dont have to call uMsgReset(), uMsgSetCrc(), uMsgOutPut() functions.
  *         Using it is very easy, here is an example:
  *         uint8_t dataBuffer[10]  = {0,1,2,3,4,5,6,7,8,9};
  *         uMsgSendMessageFromTask(&Task, dataBuffer, 10);
  * 
  * @param  uTaskPtr - Pointer of the caller uTask, alwazs use: &Task here.
  * @param  buff - Pointer to the databuffer where the desired datas are waiting to be added to the message.
  * @param  buffLen - How many datas to be copied from the buffer to the message's buffer.
  * @return returns #uMsg_Ok if the data was added.\n
  *         returns #uMsg_CopyBufferTooBig if there is no enough space in the message's buffer.
*/
Urabros_MsgStatus uMsgSendMessageFromTask(Urabros_TaskPtrTypeDef uTaskPtr, uint8_t *buff, uint8_t buffLen);

/**
  * @brief  A debug print function to write to the debug line the message in human readeable format.
  * @param  uMsgPtr - Pointer to the message to be printed.
*/
void uMsgPrint(Urabros_MsgPtr uMsgPtr);

#endif /* MASTER_COMMUNICATION_UMESSAGECOMMON_H_ */