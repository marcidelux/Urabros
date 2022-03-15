/**
  * @file     testerTask.c
  * @author   marton.lorinczi
  * @date     May 14, 2022
  *
  * @brief
  */

#include "testerTask.h"
#include "UrabrosTask.h"

Urabros_TaskPtrTypeDef testerTaskPtr = &Task;

#define DPRINT_LOCAL_ENABLE 1
#define UTASK_NAME "TESTER"
#include "uDebugPrint.h"

static void tester_thread_function(void const *argument);
osThreadDef(testerThread, tester_thread_function, osPriorityNormal, 1, configMINIMAL_STACK_SIZE * 1);

void initTesterTask(Urabros_CommandIdTypedef responsibleID)
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
    Task.threadIdArray[0]   = osThreadCreate(osThread(testerThread), NULL);

    dprint("ID: %d - "UTASK_NAME" - Init done\n", Task.responsibleTaskId);
}

static void tester_thread_function(void const *argument)
{
    for(;;)
    {
        dprintln("Waiting for start signal.");
        if (uTaskWaitForSignalStart() != uStatusOk) {
            dprintln("Wrong start signal");
            continue;
        }

        for(uint8_t i = 10; i <= 20; i++) {
            dprintln("Test Msg: %02d", i);
            osDelay(100);
        }

        dprintln("Waiting for ACK signal.");
        uTaskWaitFoSignalACK();
    }
}
