/**
  * @file     UrabrosConfig.h
  * @author   Marton.Lorinczi
  * @date     Aug 3, 2020
  *
  * @brief  This file holds the most important settings of the Urabros framework.
  *         Only the architect of the project should modify this file.
  */

#ifndef URABROSCONFIG_H_
#define URABROSCONFIG_H_

#include "stdint.h"

/* URABROS COMMON DEFINES */
#define TIMEOUT_ENABLED             1                   /**< Enable = 1 / Disable = 0 timeout function globally.*/
#define LOGIC_CONTROL_DELAY         1000                /**< The delay in miliseconds of the refreshing loop @see urabrosLogicControlFunction() function*/
#define COMMUNICATION_DELAY         100                 /**< The delay in miliseconds of the message handler loop @see urabrosCommunicationFunction()*/
#define COMMAND_TIMEOUT             (TickType_t) 100    /**< The time limit in miliseconds to trying to take the #uCommandListMutex*/

/* URABROS TASK DEFINES */
#define MAX_SUBTHREADS              2   /**< This define sets the maximum numer of subthreads per Tasks*/

/* URABROS TASK IDs */
/** This is an Urabros command typedef, even if it is an uint8_t this what is secured to not mess it up in the code.\n
 *  Every high level jobs has to own one command id, and connected it to its own uTask.\n
 *  Leave the TASK_ID_NON as 0 and TASK_ID_TEST as 255, so the user defined task can be between 1 - 254.\n
 *  Do it in an increasing order. Here we can see some examples:
*/
typedef uint8_t Urabros_CommandIdTypedef;
#define TASK_ID_NONE                (Urabros_CommandIdTypedef) 0x00 /**< Leave this as it is*/
#define TASK_ID_TEST                (Urabros_CommandIdTypedef) 0xFF /**< Leave this as it is.*/
#define TASK_ID_LAST                TASK_ID_TEST

/* URABROS TASK SWITCHIES */
#define TASK_TESTER                 1   /**< Example of including an uTask to the project or not.*/

/** This define is calcualted at preprocess time, it counts how many uTasks will be included in the project.*/
#define TASK_COUNT  (TASK_TESTER)

/** Define for timeout module */
#define BOARD_HAL_HEADER "stm32h7xx_hal.h"

/* URABROS MESSAGE */
#define MESSAGE_PERIPHERAL          URABROS_PERIPHERAL_UART                                     /**< Which peripheral the Urabros is using, Currently UART is implemented*/
#define MESSAGE_BAUD_RATE           115200                                                      /**< UART speed, this has to be equal what was set in the CubeMX*/
#define MESSAGE_UART_MAIN_PTR       &huart3                                                     /**< UART handler, this has to be equal what was set in the CubeMX*/
#define MESSAGE_UART_MAIN           huart3
#define MESSAGE_BUFFER_LENGTH       64                                                          /**< Maximal length of an incoming and outgoing #Urabros_Msg data field, this dosn't count the message length and Crc code and data type, so the actual buffer will be 4 byte longer. Maximal value is 251*/
#define MESSAGE_IN_ARRAY_LENGTH     4                                                           /**< Size of the incoming message queue see at #Urabros_MsgInBuffer*/
#define MESSAGE_OUT_ARRAY_LENGTH    4                                                           /**< Size of the outgoing message queue see at #Urabros_MsgOutBuffer*/
#define DMA_RX_BUFFER_MULTIPLIER    2                                                           /**< This is a security multiplier for DMA buffer size, 2 is enough, if it still overflows than the process time is too slow*/
#define DMA_RX_BUFFER_SIZE          (MESSAGE_BUFFER_LENGTH + 4) * DMA_RX_BUFFER_MULTIPLIER      /**< Calculated define, leave it as it is. It's value: (BufferLen +Message ID + datalen + 2Crc) * security multiplie*/
#define MESSAGE_START_OF_TEXT       2                                                           /**< Define for determine debug ASCII message starting byte*/
#define MESSAGE_END_OF_TEXT         3                                                           /**< Define for determine debug ASCII message end byte*/
#define MESSAGE_URABROS             0xFF                                                        /**< Define for determine UrabrosMessage type byte*/
#define MESSAGE_SENDING_TIME        ((MESSAGE_BUFFER_LENGTH / (MESSAGE_BAUD_RATE / 8000)) + 10) /**< Calculated define, leave it as it is.*/

/* URABROS DEBUG PRINT */
#define DPRINT_ENABLE               1 /**< Enable Debug print globally*/
#if DPRINT_ENABLE
    #define DPRINT_LOG_TIME_GLOBAL  0       /**< Enable timestamp (Systic from the start) globally on debug messages*/
    #define DPRINT_BUFF_SIZE        1024    /**< Size of the debug ASCII message buffer*/
    #define DPRINT_TEMP_BUFF_SIZE   60      /**< Size of one debug ASCII message*/
    #define DPRINT_SENDING_TIME     ((DPRINT_BUFF_SIZE / (MESSAGE_BAUD_RATE / 8000)) + 10) /**< Calculated define for debug message sending timeout, leave it as it is.*/

    //ENABLE DEBUG TASK BY TASKS
    #define DPRINT_URABROS          1       /**< Enable / Disable ASCII debug messages locally*/
    #define DPRINT_TESTER           1       /**< Enable / Disable ASCII debug messages locally*/

    //ENABLE DEBUG LOG BY TASKS
    #define LOG_TIME_URABROS          0     /**< Enable / Disable timestamp locally*/
    #define LOG_TIME_TESTER           0     /**< Enable / Disable timestamp locally*/
#endif

#endif /* URABROSCONFIG_H_ */
