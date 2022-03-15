/*
 * timeoutTask.c
 *
 *  Created on: Feb 23, 2021
 *      Author: NapoLion
 */

#include <Drivers/LED/LedDriver.h>
#include "ledBlinkerTask.h"
#include "UrabrosTask.h"

Urabros_TaskPtrTypeDef timeOutTaskPtr = &Task;

#define DPRINT_LOCAL_ENABLE     1
#define DPRINT_LOG_TIME_LOCAL   1
#define UTASK_NAME              "TIMEOUT TASK"
#include "uDebugPrint.h"

// Set the define and include Timeout
#define TIMEOUT_INSTANCES 2
#include "UrabrosTimeout.h"

static void timeout_function_A(void const *argument);
static void timeout_function_B(void const *argument);
osThreadDef(timeOutThread_A, timeout_function_A, osPriorityNormal, 1, configMINIMAL_STACK_SIZE * 1);
osThreadDef(timeOutThread_B, timeout_function_B, osPriorityNormal, 1, configMINIMAL_STACK_SIZE * 1);

uint8_t innerCommand = 0;

void initTimeOutTask(Urabros_CommandIdTypedef responsibleID)
{
    // Init Task
    Task.mode               = uTaskMode_OneTime;
    Task.queueMaster        = xQueueCreate(4, sizeof(uint8_t));
    Task.queueTask          = xQueueCreate(4, sizeof(uint8_t));
    Task.responsibleTaskId  = responsibleID;
    Task.status             = uTaskStatusSetup;
    Task.errorCode          = 0x00;
    Task.mutex              = xSemaphoreCreateMutex();
    Task.threadIdArrayLen   = 1;
    Task.threadIdArray[0]   = osThreadCreate(osThread(timeOutThread_A), NULL);
    Task.threadIdArray[1]   = osThreadCreate(osThread(timeOutThread_B), NULL);

    dprint("ID: %d - "UTASK_NAME" - Init done\n", Task.responsibleTaskId);
}

static void timeout_function_A(void const *argument)
{
    for(;;)
    {
        // Waiting for start signal, if it received a wrong signal jumps on here again.
        // It sets the status first waiting than running.
        dprintln("Wait for start signal...");
        if(uTaskWaitForSignalStart() != uStatusOk) {
            dprintln("Wrong Start signal");
            continue;
        }

        dprintln("Function A started");
        innerCommand = 1; //Start thread B

        // Start the first instance timeout counter, after 5 seconds it will timeout.
        uTimeoutStart(0, 5000);
        for(uint8_t i = 0; i < 10; i++) {
            // Check if it timeout is true
            if(uTimeoutCheck(0)) {
                dprintln("A - Timeout");
                uTaskSetStatusAndErrorCode(uTaskStatusError, uStatusTimeOut);
                break;
            } else {
                dprintln("A - Do stuff");
            }
            osDelay(1000);
        }

        dprintln("Wait for check status");
        osDelay(4000);

        // Sets the status and err code back to normal.
        uTaskSetStatusAndErrorCode(uTaskStatusRunning, uStatusOk);

        // Wait task B to end.
        while(innerCommand) {
            osDelay(1);
        }

        dprintln("Wait for ACK...");
        uTaskWaitFoSignalACK();
    }

}

static void timeout_function_B(void const *argument)
{
    while(1) {
        if(innerCommand == 1) {
            dprintln("Function B started");

            // Start the second instance timeout counter, after 2 seconds it will timeout.
            uTimeoutStart(1, 2000);
            for(uint8_t i = 0; i < 5; i++) {
                // Check if it timeout is true
                if(uTimeoutCheck(1)) {
                    dprintln("B - Timeout");
                    uTaskSetStatusAndErrorCode(uTaskStatusError, uStatusTimeOut);
                    break;
                } else {
                    dprintln("B - Do stuff");
                }
                osDelay(1000);
            }

            innerCommand = 0;
        }
        osDelay(10);
    }
}

