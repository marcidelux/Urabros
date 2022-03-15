/**
  * @file     UrabrosTask.h
  * @author   Marton.Lorinczi
  * @date     Aug 3, 2020
  *
  * @brief  This header file contains the main functions an #Urabros_TaskTypeDef has.\n
  *         Include this header to a Tasks source file, don't include it to it's header!\n
  *         All functions described here are inline functions, thats how every separate\n
  *         uTask will have it's own copy of the function.\n
  *         It has an important static variable called Task, with including this header\n
  *         every task will has it's own handler. It has to be asigned to the actual task's\n
  *         #Urabros_TaskPtrTypeDef extern variable what is in the actual task's header.
  */

#ifndef URABROSTASK_H_INCLUDED
#define URABROSTASK_H_INCLUDED

#include "UrabrosSharedResources.h"
#include "UrabrosTypeDef.h"

/** A static handler variable, the other source files make modifications on this varaible.
 */
static Urabros_TaskTypeDef Task;

/** Pauses all threads in the uTask.
 *  @param void
 *  @return executing status:\n
 *  - uStatusError if the IdArray length is 0\n
 *  - uStatusOk otherwise
 */
static inline Urabros_StatusTypeDef uTaskPause(void)
{
    if(!Task.threadIdArrayLen)
        return uStatusError;

    xSemaphoreTake(Task.mutex, portMAX_DELAY);
    for(uint8_t taskIndex = 0; taskIndex < Task.threadIdArrayLen; taskIndex++) {
        vTaskSuspend(Task.threadIdArray[taskIndex]);
    }
    xSemaphoreGive(Task.mutex);

    return uStatusOk;
}

/** Resumes all threads in the uTask.
 *  @param void
 *  @return executing status:\n
 *  - uStatusError if the IdArray length is 0\n
 *  - uStatusOk otherwise
 */
static inline Urabros_StatusTypeDef uTaskResume(void)
{
    if(!Task.threadIdArrayLen)
        return uStatusError;

    xSemaphoreTake(Task.mutex, portMAX_DELAY);
    for(uint8_t taskIndex = 0; taskIndex < Task.threadIdArrayLen; taskIndex++) {
        vTaskResume(Task.threadIdArray[taskIndex]);
    }
    xSemaphoreGive(Task.mutex);

    return uStatusOk;
}

/** Sets the status of the uTask.
 *  @param the desired status of the uTask
 *  @return executing status:\n
 *  - uStatusOk every time. Maybe a timeout should be added if we don't use portMAX_DELAY
 */
static inline Urabros_StatusTypeDef uTaskSetStatus(Urabros_TaskStatusTypeDef status)
{
    xSemaphoreTake(Task.mutex, portMAX_DELAY);
    Task.status = status;
    xSemaphoreGive(Task.mutex);
    return uStatusOk;
}

/** Sets the errorCode of the uTask.
 *  @param the desired error code of the uTask
 *  @return executing status:\n
 *  - uStatusOk every time. Maybe a timeout should be added if we don't use portMAX_DELAY
 */
static inline Urabros_StatusTypeDef uTaskSetErrorCode(uint8_t errCode)
{
    xSemaphoreTake(Task.mutex, portMAX_DELAY);
    Task.errorCode = errCode;
    xSemaphoreGive(Task.mutex);
    return uStatusOk;
}

/** Sets the status and the error code of the uTask.
 *  @param the desired status of the uTask
 *  @param the desired error code of the uTask
 *  @return executing status:\n
 *  - uStatusOk every time. Maybe a timeout should be added if we don't use portMAX_DELAY
 */
static inline Urabros_StatusTypeDef uTaskSetStatusAndErrorCode(Urabros_TaskStatusTypeDef status, uint8_t errCode)
{
    xSemaphoreTake(Task.mutex, portMAX_DELAY);
    Task.status = status;
    Task.errorCode = errCode;
    xSemaphoreGive(Task.mutex);
    return uStatusOk;
}

/** Set uTask's status to #uTaskStatusWaitingForStartSignal\n
 *  than it is waiting until the #uSignalStart arrives, than set it's status to #uTaskStatusRunning
 *  @return executing status:\n
 *  - uStatusError if the received signal is not #uSignalStart\n
 *  - uStatusOk if it went well.
 */
static inline Urabros_StatusTypeDef uTaskWaitForSignalStart()
{
    uint8_t recData = 0;
    uTaskSetStatus(uTaskStatusWaitingForStartSignal);
    xQueueReceive(Task.queueMaster, &recData, portMAX_DELAY);
    if(recData == uSignalStart) {
        uTaskSetStatus(uTaskStatusRunning);
        return uStatusOk;
    } else {
        return uStatusError;
    }
}

/** Set uTask's status to #uTaskStatusWaitingForACKSignal\n
 *  than it is waiting until the #uSignalACK arrives.
 *  @return executing status:\n
 *  - uStatusError if the received signal is not #uSignalStart\n
 *  - uStatusOk if it went well.
 */
static inline Urabros_StatusTypeDef uTaskWaitFoSignalACK()
{
    uint8_t recData = 0;
    uTaskSetStatus(uTaskStatusWaitingForACKSignal);

    while(recData != uSignalACK) {
        xQueueReceive(Task.queueMaster, &recData, portMAX_DELAY);
    }

    return uStatusOk;
}

#endif // URABROSTASK_H_INCLUDED
