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
  */

#ifndef MASTER_COMMUNICATION_UOUTGOINGMESSAGEHANDLER_H_
#define MASTER_COMMUNICATION_UOUTGOINGMESSAGEHANDLER_H_

#include "UrabrosTypeDef.h"

extern uint8_t* uMsgOutWaitingNumPtr; /**< Extern variable for showing that there is at least one message waiting in the outgoing queue*/

/** Initilaize the outgoing message queue, and creates the mutex.\n
 *  The Queue works as a FILO (First in Last out).
 *  @return Returns #uStatusOk  
 */
Urabros_StatusTypeDef uMsgOutInit();

// Take out the last message in queue ( First in last out)
/** Pops the last message from the #uOutgoinggBuffer
 *  @param uMsgPtr This is a pointer to an #Urabros_Msg variable.\n
 *                 If the pop sucseeded than it will load the correct values to the pointer variable.\n
 *                 If the queue was empty it will not make any modifications on the pointed variable.
 *  @return Returns #uMsg_Ok if it went well, returns #uMsg_BufferIsEmpty if the queue was empty
 */
Urabros_MsgStatus uMsgOutPop(Urabros_MsgPtr uMsgPtr);

// Put a message in to the buffer
/** Puts an #Urabros_Msg tot the outgoing queue.
 *  @param uMsgPtr The pointer to the message we want to put into the queue.
 *  @returnReturns #uMsg_Ok if it went well, returns #uMsg_BufferIsFull if the queue was full. If this happens oftenly than the processing time is not fast enough.
 */
Urabros_MsgStatus uMsgOutPut(Urabros_MsgPtr uMsgPtr);

// Send the message via DMA
/** Sends out the given #Urabros_Msg using the HAL_UART_Transmit_DMA() function.
 *  IT tries to send out the data 5 times.
 *  @param uMsgPtr pointer to the message we want to send out
 *  @return #uMsg_Ok - if it went well\n
 *          #uMsg_Error - If hardware error happened with HAL_UART_Transmit_DMA.\n
 *          #uMsg_Busy - If the DMA peripheral is busy sending out the previously data.
 */
Urabros_MsgStatus uMsgSend(Urabros_MsgPtr uMsgPtr);

/** A debug function, prints all the element int the outgoing queue on human readeable format to debug line.
 */
void uMsgInPrintOuttBuffer();

#endif /* MASTER_COMMUNICATION_UOUTGOINGMESSAGEHANDLER_H_ */
