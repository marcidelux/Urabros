/**
  * @file     ledBlinkerTask.c
  * @author   marton.lorinczi
  * @date     Sep 2, 2020
  *
  * @brief
  */

#include "LedDriver.h"
#include "ledBlinkerTask.h"
#include "UrabrosTask.h"

Urabros_TaskPtrTypeDef ledBlinkerTaskPtr = &Task;

#define DPRINT_LOCAL_ENABLE 1
#define UTASK_NAME "LED"
#include "uDebugPrint.h"

static void blinker_thread_function(void const *argument);
osThreadDef(ledBLinkerThread, blinker_thread_function, osPriorityNormal, 1, configMINIMAL_STACK_SIZE * 1);

void initLedBlinkerTask(Urabros_CommandIdTypedef responsibleID)
{
    // Init Task
    Task.mode               = uTaskMode_Continious;
    Task.queueMaster        = xQueueCreate(4, sizeof(uint8_t));
    Task.queueTask          = xQueueCreate(4, sizeof(uint8_t));
    Task.responsibleTaskId  = responsibleID;
    Task.status             = uTaskStatusSetup;
    Task.errorCode          = 0x00;
    Task.mutex              = xSemaphoreCreateMutex();
    Task.threadIdArrayLen   = 1;
    Task.threadIdArray[0]   = osThreadCreate(osThread(ledBLinkerThread), NULL);

    // Init Modules
    ledDriver_Init();

    dprint("ID: %d - "UTASK_NAME" - Init done\n", Task.responsibleTaskId);
}

static void blinker_thread_function(void const *argument)
{
    uint8_t recData = 0;
    uint8_t numOfWaiting = 0;
    uint32_t delays[4] = {100,250,100,250};

    uTaskSetStatus(uTaskStatusRunning);

    for(;;)
    {

        numOfWaiting = uxQueueMessagesWaiting(Task.queueMaster);
        if(numOfWaiting) {
            dprintln("Num of waiting: %d", numOfWaiting);
            for(uint8_t i = 0; i < numOfWaiting; i++) {
                if(xQueueReceive(Task.queueMaster, &recData, (TickType_t)10) == pdTRUE) {
                    delays[i] = recData;
                    dprintln("Delay[%d]: %X", i, recData);
                }
            }
        }

        for(uint8_t i = 0; i < 3; i++) {
          ledDriver_ON(i);
          osDelay(delays[0]);
        }

        osDelay(delays[1]);

        for(uint8_t i = 0; i < 3; i++) {
          ledDriver_OFF(i);
          osDelay(delays[2]);
        }

        osDelay(delays[3]);
    }
}
