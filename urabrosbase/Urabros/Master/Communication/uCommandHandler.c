/**
  * @file     uCommandHandler.c
  * @author   Marton.Lorinczi
  * @date     Feb 10, 2020
  *
  * @brief    Further information in header file.
*/

#include "uCommandHandler.h"

#define DPRINT_LOCAL_ENABLE 1
#include "uDebugPrint.h"

Urabros_CommandTypedef   uCommandList[TASK_COUNT];
SemaphoreHandle_t        uCommandListMutex;
uint8_t                  uCommandListStatusBuffer[TASK_COUNT * 2];
uint8_t                  uCommandNumber;


static Urabros_CommandReturnStatus uCommandCheck(Urabros_CommandPtrTypedef commandPtr);

Urabros_StatusTypeDef uCommandHandlerInit(void)
{
    // Define global variables
    uCommandListMutex = xSemaphoreCreateMutex();

    // Clear command list
    for(uint16_t i = 0; i < TASK_COUNT; i++) {
        uCommandClear(uCommandList + i);
    }

    uCommandNumber = 0;

    return uStatusOk;
}

Urabros_CommandReturnStatus uCommandAppend(Urabros_CommandPtrTypedef commandPtr)
{
    Urabros_CommandReturnStatus retStat = uCommandCheck(commandPtr);

    // If there was a problem return with status
    if(retStat != uCommandOk) {
        return retStat;
    }

    if(uCommandNumber >= TASK_COUNT) {
        return uCommandOwerFlow;
    }

    if(commandPtr->id > TASK_ID_LAST && commandPtr->id != TASK_ID_TEST) {
        return uCommandIdOutOfRange;
    }

    if (xSemaphoreTake(uCommandListMutex, COMMAND_TIMEOUT) == pdTRUE) {
        uCommandList[uCommandNumber].id             = commandPtr->id;
        uCommandList[uCommandNumber].status.main    = 0x00;
        uCommandList[uCommandNumber].status.minor   = 0x00;
        xSemaphoreGive(uCommandListMutex);
        uCommandNumber++;
        return uCommandAdded;
    } else {
        return uCommandTimedOut;
    }
}

Urabros_CommandReturnStatus uCommandCheck(Urabros_CommandPtrTypedef commandPtr)
{
    if (xSemaphoreTake(uCommandListMutex, COMMAND_TIMEOUT) == pdTRUE) {

        uint8_t idAlreadyExists  = 0;
        for(uint16_t i = 0; i < uCommandNumber; i++) {
           if(uCommandList[i].id == commandPtr->id) {
               idAlreadyExists = 1;
               break;
           }
        }
        xSemaphoreGive(uCommandListMutex);
        if(idAlreadyExists)
           return uCommandIdAlreadyUsed;
        else
           return uCommandOk;
   } else {
       return uCommandTimedOut;
   }
}

Urabros_CommandReturnStatus uCommandRemoveById(Urabros_CommandIdTypedef commandId)
{
    if (xSemaphoreTake(uCommandListMutex, COMMAND_TIMEOUT) == pdTRUE) {
        uint8_t foundStatus = 0;
        for(uint8_t i = 0; i < uCommandNumber; i++) {
            if(uCommandList[i].id == commandId) {
                if(uCommandList[i].status.main != uTaskStatusWaitingForACKSignal) {
                    foundStatus = 1;
                    break;
                }
                foundStatus = 2;
                // If it is the last of the array than resetit.
                if(i == TASK_COUNT -1) {
                    uCommandClear(uCommandList + i);
                } else {
                    for(uint8_t j = i; j <= uCommandNumber; j++) {
                        uCommandList[j].id              = uCommandList[j+1].id;
                        uCommandList[j].status.main     = uCommandList[j+1].status.main;
                        uCommandList[j].status.minor    = uCommandList[j+1].status.minor;
                    }
                }
                uCommandNumber--;
            }
        }
        xSemaphoreGive(uCommandListMutex);

        if(foundStatus == 0)
            return uCommandNotFound;
        else if(foundStatus == 1)
            return uCommandNotFinished;
        else if(foundStatus == 2)
            return uCommandDeleted;
    } else {
        return uCommandTimedOut;
    }
}

Urabros_CommandReturnStatus uCommandGetIndexById(Urabros_CommandIdTypedef commandId, uint8_t *cmdIndex)
{
    if (xSemaphoreTake(uCommandListMutex, COMMAND_TIMEOUT) == pdTRUE) {
        uint8_t foundIt = 0;
        for(uint8_t i = 0; i < uCommandNumber; i++) {
            if(uCommandList[i].id == commandId) {
                foundIt = 1;
                *cmdIndex = i;
                break;
            }
        }
        xSemaphoreGive(uCommandListMutex);
        if(foundIt) {

            return uCommandOk;
        } else {
            return uCommandNotFound;
        }
    } else {
        return uCommandTimedOut;
    }
}

Urabros_CommandReturnStatus uCommandGetById(Urabros_CommandIdTypedef commandId, Urabros_CommandPtrTypedef commandRetPtr)
{
    if (xSemaphoreTake(uCommandListMutex, COMMAND_TIMEOUT) == pdTRUE) {
        uint8_t foundIt = 0;
        for(uint8_t i = 0; i < uCommandNumber; i++) {
            if(uCommandList[i].id == commandId) {
                foundIt = 1;
                commandRetPtr = uCommandList + i;
                break;
            }
        }
        xSemaphoreGive(uCommandListMutex);
        if(foundIt) {
            return uCommandOk;
        } else {
            return uCommandNotFound;
        }
    } else {
        return uCommandTimedOut;
    }
}

Urabros_CommandReturnStatus uCommandClear(Urabros_CommandPtrTypedef commandPtr)
{
    commandPtr->id              = TASK_ID_NONE;
    commandPtr->status.main     = 0x00;
    commandPtr->status.minor    = 0x00;
    return uCommandOk;
}

Urabros_CommandReturnStatus uCommandPrint(Urabros_CommandPtrTypedef commandPtr)
{
    if(commandPtr == NULL)
        return;

    dprintln("Id: %d | main: %d | minor: %d", commandPtr->id, commandPtr->status.main, commandPtr->status.minor);
}

Urabros_CommandReturnStatus uCommandPrintList(void)
{
    dprintln("Command list:");
    if (xSemaphoreTake(uCommandListMutex, COMMAND_TIMEOUT) == pdTRUE) {
        for(uint16_t i = 0; i < uCommandNumber; i++) {
            uCommandPrint(uCommandList + i);
        }
        xSemaphoreGive(uCommandListMutex);
    } else {
        return uCommandTimedOut;
    }
}
