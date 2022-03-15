/**
  * @file     UrabrosTimeout.h
  * @author   marton.lorinczi
  * @date     Aug 12, 2020
  *
  * @brief  This header contains static functions for Timeout checks, based on HAL_GetTick() function.\n
  *         Include this library to any of the Urabros Tasks source  or Urabros Modules source.\n
  *         Each file will have its own copy of the static functions.\n
  *         Before including it to a source file you have to define some preprocessor defines\n
  *         #TIMEOUT_ENABLED - Define this to 1 for enable timeout functions or 0 to disable them\n
  *         #TIMEOUT_INSTANCES - Define this with a number of instances you would like to use it.\n
  *                           - Usually the number of threads in Task is the correct number\n
  *                           - If it isn't defined than one instance will be created\n
  *         IMPORTANT - Read the source file, because the two kind of behaviour the other part cannot be represented in the documentation.
  */

#ifndef COMMON_URABROSTIMEOUT_H_
#define COMMON_URABROSTIMEOUT_H_

#include "UrabrosConfig.h"
#include "UrabrosTypeDef.h"
#include <stdint.h>
#include BOARD_HAL_HEADER

#if TIMEOUT_ENABLED
    #if TIMEOUT_INSTANCES
        /**
         * An array of uint32_t for save the starting times of the given instances
         */
        static uint32_t uTimeoutStartArray[TIMEOUT_INSTANCES];
        /*
         * An array of uint32_t for save the deadline times of the given instances
         */
        static uint32_t uTimeoutDeadLineArray[TIMEOUT_INSTANCES];

        /**
         * Starts the timer for checking timeout.
         * @param instanceId an uint8_t for witch instance to start.
         * #param deadLine an uint32_t is the deadline of the timer. If the uTimeoutCheck is called
         *        after deadLine miliseconds passed it will return uStatusTimeOut.
         * @return void
         */
        static inline void uTimeoutStart(uint8_t instanceId, uint32_t deadLine)
        {
            uTimeoutStartArray[instanceId]      = HAL_GetTick();
            uTimeoutDeadLineArray[instanceId]   = deadLine;
        }

        /**
         * @brief Calling this function will return uStatusTimeOut if at least deadLine time has passed
         *        or it returns uStatusOk if we are in time.
         * @param instanceId an uint8_t for witch instance to check.
         * @return Urabros_StatusTypeDef uStatusTimeOut or uStatusOk
         */
        static inline Urabros_StatusTypeDef uTimeoutCheck(uint8_t instanceId)
        {
            if((HAL_GetTick() - uTimeoutStartArray[instanceId]) > uTimeoutDeadLineArray[instanceId]) {
                return uStatusTimeOut;
            } else {
                return uStatusOk;
            }
        }

    #else
        /**
         * An uint32_t for save the starting time.
         */
        static uint32_t startingTick;
        /*
         * An uint32_t for save the deadline time.
         */
        static uint32_t deadlineCount;

        /**
         * Starts the timer for checking timeout.
         * #param deadLine an uint32_t is the deadline of the timer. If the uTimeoutCheck is called
         *        after deadLine miliseconds passed it will return uStatusTimeOut.
         * @return void
         */
        static inline void uTimeoutStart(uint32_t deadLine)
        {
            startingTick = HAL_GetTick();
            deadlineCount = deadLine;
        }

        /**
         * @brief Calling this function will return uStatusTimeOut if at least deadLine time has passed
         *        or it returns uStatusOk if we are in time.
         * @return Urabros_StatusTypeDef uStatusTimeOut or uStatusOk
         */
        static inline Urabros_StatusTypeDef uTimeoutCheck()
        {
            if((HAL_GetTick() - startingTick) > deadlineCount) {
                return uStatusTimeOut;
            } else {
                return uStatusOk;
            }
        }
    #endif
#else
    #if TIMEOUT_INSTANCES
        static inline void uTimeoutStart(uint8_t instanceId, uint32_t deadLine) {}
        static inline Urabros_StatusTypeDef uTimeoutCheck(uint8_t instanceId) {return uStatusOk;}
    #else
        static inline void uTimeoutStart(uint32_t deadLine) {}
        static inline Urabros_StatusTypeDef uTimeoutCheck() {return uStatusOk;}
#endif
#endif


#endif /* COMMON_URABROSTIMEOUT_H_ */
