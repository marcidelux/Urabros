/*
 * motorOneTask.c
 *
 *  Created on: Feb 26, 2021
 *      Author: NapoLion
 */

#include "StepMotorDriver.h"
#include "MotorOneDriver.h"
#include "motorOneTask.h"
#include "UrabrosTask.h"

Urabros_TaskPtrTypeDef motorOneTaskPtr = &Task;

#define DPRINT_LOCAL_ENABLE 1
#define UTASK_NAME "MOTOR ONE"
#include "uDebugPrint.h"

static void motor_one_thread_function(void const *argument);
osThreadDef(motorOneThread, motor_one_thread_function, osPriorityNormal, 1, configMINIMAL_STACK_SIZE * 1);

void initMotorOneTask(Urabros_CommandIdTypedef responsibleID)
{
    // Init Task
    Task.mode               = uTaskMode_Continious;
    Task.queueMaster        = xQueueCreate(13, sizeof(uint8_t));
    Task.queueTask          = xQueueCreate(4, sizeof(uint8_t));
    Task.responsibleTaskId  = responsibleID;
    Task.status             = uTaskStatusSetup;
    Task.errorCode          = 0x00;
    Task.mutex              = xSemaphoreCreateMutex();
    Task.threadIdArrayLen   = 1;
    Task.threadIdArray[0]   = osThreadCreate(osThread(motorOneThread), NULL);

    // Init Drivers
    motorOneDriver_Init();

    dprint("ID: %d - "UTASK_NAME" - Init done\n", Task.responsibleTaskId);
}

static void motor_one_thread_function(void const *argument)
{
    uint8_t         recData             = 0;
    uint8_t         numOfWaiting        = 0;
    uint8_t         dataBuffer[12]      = {0};

    for(;;)
    {
        // Waiting for start signal, if it received a wrong signal jumps on here again.
        // It sets the status first waiting than running.

        // Wait for 3 data
        dprintln("Waiting for Motor command data")

        numOfWaiting = 0;
        while(!numOfWaiting) {
            numOfWaiting = uxQueueMessagesWaiting(Task.queueMaster);
            osDelay(10);
        }

        if(numOfWaiting == 1) {
            xQueueReceive(Task.queueMaster, &recData, portMAX_DELAY);
            switch(recData){
                case 0:
                    break;
                default:
                    break;
            }

        } else if(numOfWaiting == 2) {
            //Freerun mode
        } else if(numOfWaiting > 2){
            // Mode with data.
            for(uint8_t i = 0; i <  numOfWaiting; i++) {
                if(xQueueReceive(Task.queueMaster, &recData, portMAX_DELAY) == pdTRUE) {
                    dataBuffer[i] = recData;
                }
            }
            MotorCommandMove command;
            command.cMode       = dataBuffer[0];
            command.cDirection  = dataBuffer[1];
            command.cUserUnitDifference = (dataBuffer[2] << 8) + dataBuffer[3];
            uint32_t timeoutVal = (dataBuffer[4] << 8) + dataBuffer[5];

            if(timeoutVal) {
                dprintln("With timeout mode: %d, dir: %d, Step: %d",command.cMode, command.cDirection, command.cUserUnitDifference);
                motorOneDriver_StartMotorSpeed(command, timeoutVal);
            }
            else {
                dprintln("No timeout mode: %d, dir: %d, Step: %d",command.cMode, command.cDirection, command.cUserUnitDifference);
                motorOneDriver_StartMotor(command);
            }

        }


    }
}
