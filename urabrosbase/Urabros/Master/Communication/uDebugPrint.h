/**
  * @file     uDebugPrint.h
  * @author   Marton.Lorinczi
  * @date     Aug 10, 2020
  *
  * @brief    uDebugPrint header and source file are responsible for giving functionality for ASCII debug messages.
  * 
  *           Every file where this header is included can use it's features.
  *           Depending what #defines were made before including this file, same named functions will behave differently.
  *           Explanation for defines:          
  *           #define DPRINT_LOCAL_ENABLE 1 or 0    // This will enable disable the printing functions, if it is diabled teh compiler will skip the functions.
  *           #define UTASK_NAME "NAME"             // If this is defined it will put the "NAME - " to the beggingin of every debug message.
  *           #define DPRINT_LOG_TIME_LOCAL 1 or 0  // This will enable disable the timestamp at the beggining of the message. (Its a systick from start)
  *           
  *           After included correctly you can use: dprint(const char*,...); and dprintln(const char*, ...) functions.
  *           They behave like printf() so you can convert a lot of base type variables to ASCII string.
  *           Theese functions place the ASCII debug messages in to a circural buffer, what will be later processed by the @see urabrosMessageSenderFunction() function.
  */
#ifndef MASTER_COMMUNICATION_UDEBUGPRINT_H_
#define MASTER_COMMUNICATION_UDEBUGPRINT_H_

#include "UrabrosConfig.h"
#include <stdint.h>
#include <stdio.h>

#if DPRINT_ENABLE
    #if DPRINT_LOCAL_ENABLE
        /* Static variables for Dprint*/
        static char dPrintTempBuff[DPRINT_TEMP_BUFF_SIZE];
        static uint16_t dMsgLen;

        #ifndef UTASK_NAME
            #define PRESTRING ""
        #else
            #define PRESTRING UTASK_NAME": "
        #endif

        #if DPRINT_LOG_TIME_GLOBAL
            #define dprintln(f_, ...)   {\
                                        dMsgLen = sprintf(dPrintTempBuff, (" "PRESTRING f_"\n"), ##__VA_ARGS__);\
                                        uDebugPrintWriteTimeStamp(dPrintTempBuff, dMsgLen);\
                                        }
            #define dprint(f_, ...)     {\
                                        dMsgLen = sprintf(dPrintTempBuff, (f_), ##__VA_ARGS__);\
                                        uDebugPrintWriteTimeStamp(dPrintTempBuff, dMsgLen);\
                                        }
        #elif DPRINT_LOG_TIME_LOCAL
            #define dprintln(f_, ...)   {\
                                        dMsgLen = sprintf(dPrintTempBuff, (" "PRESTRING f_"\n"), ##__VA_ARGS__);\
                                        uDebugPrintWriteTimeStamp(dPrintTempBuff, dMsgLen);\
                                        }
            #define dprint(f_, ...)     {\
                                        dMsgLen = sprintf(dPrintTempBuff, (f_), ##__VA_ARGS__);\
                                        uDebugPrintWriteTimeStamp(dPrintTempBuff, dMsgLen);\
                            }
        #else
            #define dprintln(f_, ...)   {\
                                        dMsgLen = sprintf(dPrintTempBuff, (PRESTRING f_"\n"), ##__VA_ARGS__);\
                                        uDebugPrintWrite(dPrintTempBuff, dMsgLen);\
                                        }

            #define dprint(f_, ...) {\
                                    dMsgLen = sprintf(dPrintTempBuff, (f_), ##__VA_ARGS__);\
                                    uDebugPrintWrite(dPrintTempBuff, dMsgLen);\
                                    }
        #endif
    #else
        #define dprintln(f_, ...)
        #define dprint(f_, ...)
    #endif
#else
    #define dprintln(f_, ...)
    #define dprint(f_, ...)
#endif



/**
  * @param  void
  * @brief  Init the circular buffer for debugPrint
*/
void uDebugPrintInit();

/**
  * @param  dMsg - Pointer to buffer where the actual message is.
  * @param  dMsgLen - Length of the message
  * @brief  Adds the message to the uDebugPrintBuffer, if the message cannot fit in the buffer,
  * than put a '$' character instead of the message to the end of the buffer, and sets the
  * dDebugPrintOverflow flag to 1.
*/
void uDebugPrintWrite(char* dMsg, uint16_t dMsgLen);

/**
  * @brief  Adds the message to the uDebugPrintBuffer, but places a timestamp what is a systic from the beggining of the startup
  * to the beggingin. If the message cannot fit in the buffer, than put a '$' character instead of the message to the end of the buffer,
  * and sets the dDebugPrintOverflow flag to 1.
  * @param   dMsg - Pointer to buffer where the actual message is.
  * @param   dMsgLen - Length of the message
*/
void uDebugPrintWriteimeStamp(char* dMsg, uint16_t dMsgLen);

/**
  * @return void -
  * @brief  Sends out the uDebugPrintBuffer using the defined peripheral.
  * Clears the buffer, sets the dDebugPrintOverflow flag to 0.
*/
void uDebugPrintRead(void);

#endif /* MASTER_COMMUNICATION_UDEBUGPRINT_H_ */
