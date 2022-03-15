/**
  * @file     uIncomingMessageHandler.h
  * @author   Marton.Lorinczi
  * @date     Sep 14, 2020
  *
  * @brief  IncomingMessageHandler is responsible for handling the incoming message queue.
  *
  *         Uart currently works on DMA mode, there are a lot of different detection types for ensure secure communication.
  *         There is IDLE line detection, with this we dont have to count the number of bytes arriving and assembly the message,
  *         we just wait for the idle interrupt. That on;ly fires when a data transmission ended, after that happens we call the
  *         @see uMsgPutFromDMA() function, and it handles the rest.
  *         The Uart must be configured to DMA mode and enable global interrupt for it in CUBE MX
  *         Here is where and how to call the uMsgPutFromDMA function:
  *         stm32xxxx_it.c
  *         void USARTX_IRQHandler(void)
  *         {
  *             HAL_UART_IRQHandler(&huart3);
  *             USER CODE BEGIN USART3_IRQn 1
  *             if (__HAL_UART_GET_FLAG(&huart3,UART_FLAG_IDLE)){
  *             __HAL_UART_CLEAR_IDLEFLAG(&huart3);
  *             if(uMsgPutFromDMA() != uMsg_Ok)  {}
  *         }
  */
#ifndef MASTER_COMMUNICATION_UINCOMINGMESSAGEHANDLER_H_
#define MASTER_COMMUNICATION_UINCOMINGMESSAGEHANDLER_H_

#include "UrabrosTypeDef.h"

/**
  * @brief  Initialize the incoming message queue. Sets IDLE line detection on. Starts the DMA receive function.
  * @return #uStatusOk
*/
Urabros_StatusTypeDef uMsgInInit();

/** Pops the last message from the #uIncomingBuffer
 *  @param uMsgPtr This is a pointer to an #Urabros_Msg variable.\n
 *                 If the pop sucseeded than it will load the correct values to the pointer variable.\n
 *                 If the queue was empty it will not make any modifications on the pointed variable.
 *  @return Returns #uMsg_Ok if it went well, returns #uMsg_BufferIsEmpty if the queue was empty
 */
Urabros_MsgStatus uMsgInPop(Urabros_MsgPtr uMsgPtr);

/**
  * @brief  Puts a message received by Uart to the incoming queue.
  * 
  *         It makes different checks on the message, and depending on them it creates and puts a message to the queue:
  *         - If everything is ok than the received message is placed to the buffer
  *         - If Idle Error happend (The sending got interupted, and message broke appart)
  *         ---- data[0]: uCommand_RECEIVE_ERROR
  *         ---- data[1]: uMSg_IdleError
  *         - If Data len doesnt match:
  *         ---- data[0]: uCommand_RECEIVE_ERROR
  *         ---- data[1]: uMsg_DataLenError
  *         - If CRC is not correct:
  *         ---- data[0]: uCommand_RECEIVE_ERROR
  *         ---- data[1]: uMsg_CrcError
  * @return Depending on the process it returns, #uMsg_Ok, #uMSg_IdleError, #uMsg_DataLenError, #uMsg_CrcError but theese are not needed to be handled in the interrupt.
*/
Urabros_MsgStatus uMsgPutFromDMA(void);

/** A debug function, prints all the element int the outgoing queue on human readeable format to debug line.
 */
void uMsgInPrintBuffer();

#endif /* MASTER_COMMUNICATION_UINCOMINGMESSAGEHANDLER_H_ */
