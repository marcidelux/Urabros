/**
  * @file     uDebugPrint.c
  * @author   marton.lorinczi
  * @date     Aug 10, 2020
  *
  * @brief  uDebugPrint header and source file are responsible for giving functionality for ASCII debug messages.
  *         Further information can be found in the header file.
  */
#include "uDebugPrint.h"
#include "circularBuffer.h"

/* Includes ------------------------------------------------------------------*/
#include "string.h"
#include "usart.h"

/* Public global extern variables --------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
#if DPRINT_ENABLE
    static uint8_t uDebugPrintBuffer[DPRINT_BUFF_SIZE];
    static uint8_t uDebugPrintOutPutBuffer[DPRINT_BUFF_SIZE + 2];
    static circularBuffer uDebugCircBuffer;
    static char timeStamp[10];
#endif

#if DPRINT_ENABLE
    void uDebugPrintInit()
    {
        circularBufferInit(&uDebugCircBuffer, uDebugPrintBuffer, DPRINT_BUFF_SIZE);
    }
#else
    void uDebugPrintInit(){};
#endif

#if DPRINT_ENABLE
    void uDebugPrintWrite(char* dMsg, uint16_t dMsgLen)
    {
        uint8_t retval = circularBufferWrite(&uDebugCircBuffer, (uint8_t*)dMsg, dMsgLen);
        if(retval) {
            char full[20];
            uint8_t len = sprintf(full, "\nful:%d\n", retval);
            HAL_UART_Transmit_DMA(MESSAGE_UART_MAIN_PTR, full, len);
        }
    }
#else
    void uDebugPrintWrite(char* dMsg, uint16_t dMsgLen){};
#endif

#if DPRINT_ENABLE
void uDebugPrintWriteTimeStamp(char* dMsg, uint16_t dMsgLen)
    {
        sprintf(timeStamp, "%10d", HAL_GetTick());
        circularBufferWrite(&uDebugCircBuffer, (uint8_t*)timeStamp, 10);
        circularBufferWrite(&uDebugCircBuffer, (uint8_t*)dMsg, dMsgLen);
    }
#else
    void uDebugPrintWriteTimeStamp(char* dMsg, uint16_t dMsgLen){};
#endif

#if DPRINT_ENABLE
    void uDebugPrintRead(void)
    {
        uint16_t dataReadLen = 0;
        uDebugPrintOutPutBuffer[0] = MESSAGE_START_OF_TEXT; // Set the first byte to charset mode.
        circularBufferRead(&uDebugCircBuffer, uDebugPrintOutPutBuffer + 1, &dataReadLen);
        uDebugPrintOutPutBuffer[dataReadLen + 1] = MESSAGE_END_OF_TEXT;
        if(dataReadLen) {
            HAL_UART_Transmit_DMA(MESSAGE_UART_MAIN_PTR, uDebugPrintOutPutBuffer, dataReadLen + 2);
        }
    }
#else
    void uDebugPrintRead(void){};
#endif
