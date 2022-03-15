
### START HEADER ####

#include "UrabrosTypeDef.h"

extern Urabros_TaskPtrTypeDef tempTaskPtr;

void initTempTask(Urabros_CommandIdTypedef responsibleID);

#### END HEADER ####



### START SOURCE ####

#include "tempTask.h"
#include "UrabrosTask.h"

#define DPRINT_LOCAL_ENABLE DPRINT_TEMP
#define DPRINT_LOG_TIME_LOCAL LOG_TIME_TEMP
#define UTASK_NAME "TEMP TASK NAME"
#include "uDebugPrint.h"

Urabros_TaskPtrTypeDef tempTaskPtr = &Task;

static void temp_thread_function(void const *argument);
osThreadDef(tempThread, temp_thread_function, osPriorityNormal, 1, configMINIMAL_STACK_SIZE * 2);


void initTempTask(Urabros_CommandIdTypedef responsibleID)
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
    Task.threadIdArray[0]   = osThreadCreate(osThread(tempThread), NULL);
}

void helloWorld_thread_function(void const *argument)
{
    for(;;)
    {
        if(uTaskWaitForSignalStart() != uStatusOk) {
            dprintln("Wrong Start signal");
            continue;
        }

        // Do Stuff

        dprintln("Wait for ACK...");
        uTaskWaitFoSignalACK();
    }
}

### END SOURCE ####

