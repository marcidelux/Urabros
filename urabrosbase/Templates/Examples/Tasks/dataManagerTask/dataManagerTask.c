/*
 * dataManagerTask.c
 *
 *  Created on: Feb 24, 2021
 *      Author: NapoLion
 */
#include "UrabrosTask.h"
#include "dataManagerTask.h"

Urabros_TaskPtrTypeDef dataManagerTaskPtr = &Task;

#define DPRINT_LOCAL_ENABLE 1
#define UTASK_NAME "DATA MANAGER"
#include "uDebugPrint.h"

#include "uOutgoingMessageHandler.h"
#include "uMessageCommon.h"

static void dataManager_thread_function(void const *argument);
osThreadDef(dataManagerThread, dataManager_thread_function, osPriorityNormal, 1, configMINIMAL_STACK_SIZE * 2);

void initDataManagerTask(Urabros_CommandIdTypedef responsibleID)
{
    // Init Task
    Task.mode               = uTaskMode_OneTime;
    Task.queueMaster        = xQueueCreate(60, sizeof(uint8_t));
    Task.queueTask          = xQueueCreate(4, sizeof(uint8_t));
    Task.responsibleTaskId  = responsibleID;
    Task.status             = uTaskStatusSetup;
    Task.errorCode          = 0x00;
    Task.mutex              = xSemaphoreCreateMutex();
    Task.threadIdArrayLen   = 1;
    Task.threadIdArray[0]   = osThreadCreate(osThread(dataManagerThread), NULL);

    dprint("ID: %d - "UTASK_NAME" - Init done\n", Task.responsibleTaskId);
}

static void dataManager_thread_function(void const *argument)
{
    uint8_t         recData         = 0;
    uint8_t         numOfWaiting    = 0;
    uint8_t         dataBuffer[60]  = {0};
    Urabros_Msg     uMsgTx          = {0};
    Urabros_MsgPtr  uMsgTxPtr       = &uMsgTx;
    uint8_t         msgBlockCntr    = 0;

    for(;;)
    {
        // Waiting for start signal, if it received a wrong signal jumps on here again.
        // It sets the status first waiting than running.
        dprintln("Wait for start signal...");
        if(uTaskWaitForSignalStart() != uStatusOk) {
            dprintln("Wrong Start signal");
            continue;
        }

        // Receive two message blocks than goes to finished state.
        msgBlockCntr = 0;
        while(msgBlockCntr < 4) {
            numOfWaiting = uxQueueMessagesWaiting(Task.queueMaster);
            if(numOfWaiting) {
                dprintln("Data arrived: %d", msgBlockCntr);
                for(uint8_t i = 0; i <  numOfWaiting; i++) {
                    if(xQueueReceive(Task.queueMaster, &recData, (TickType_t)10) == pdTRUE) {
                        dataBuffer[i] = recData;
                    }
                }

                uMsgSendMessageFromTask(&Task, dataBuffer, numOfWaiting);

                uMsgReset(uMsgTxPtr);
                uMsgAppendBufferFromTask(&Task, uMsgTxPtr, dataBuffer, numOfWaiting);
                uMsgAppendBufferFromTask(&Task, uMsgTxPtr, dataBuffer, numOfWaiting);
                uMsgSetCrc(uMsgTxPtr);
                uMsgOutPut(uMsgTxPtr);

                msgBlockCntr++;
                /*
                // This is equal with the uMsgAppendBufferFromTask(...) for the first time calling, than its like uMsgAppendBuffer(...)
                uMsgAppend(uMsgTxPtr, uCommand_DATA_FROM_TASK);
                uMsgAppend(uMsgTxPtr, Task.responsibleTaskId);
                uMsgAppendBuffer(uMsgTxPtr, dataBuffer, numOfWaiting);
                */
            }
            osDelay(10);
        }

        dprintln("Wait for ACK...");
        uTaskWaitFoSignalACK();
    }
}




























































