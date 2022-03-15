/**
  * @file     UrabrosMaster.c
  * @author   marton.lorinczi
  * @date     Aug 10, 2020
  *
  * @brief  Main logic part of the framework.
  *         There are four important parts of this source file.
  *         @see UrabrosInit() \n
  *             Creates the uTasks array where all of the tasks pointers are stored.
  *             Calls all the necessary inits for communication handlings.
  *             Creates the three main threads.
  *         @see urabrosCommunicationFunction() \n
  *             Thread responsible for handling the incoming messages.
  *             It can adds commands to thecommand list, starts the tasks, delete command form the list, and stop the tasks.
  *         @see urabrosLogicControlFunction() \n
  *             This thread updates the statuses of the active commands, based on the statuses of the running tasks.
  *         @see urabrosMessageSenderFunction() \n
  *             This function is responsible for sending any kind of messages to the controller PC.
  */

// Include
#include "UrabrosMaster.h"
#include "UrabrosTypeDef.h"
#include "uIncomingMessageHandler.h"
#include "uOutgoingMessageHandler.h"
#include "uCommandHandler.h"
#include "uMessageCommon.h"

// Debug Print
#if DPRINT_ENABLE
    #define DPRINT_LOCAL_ENABLE DPRINT_URABROS
    #define UTASK_NAME "URABROS"
#endif
#include "uDebugPrint.h"

// Timeout
#define TIMEOUT_INSTANCES 2
#include "UrabrosTimeout.h"

#include "UrabrosTaskIncluder.h"

/* LOCAL FUNCTION PROTOTYPES */

/** Static function for getting a pointer to the given task's Id from the #uTasks array.
 *  Example: Urabros_TaskPtrTypeDef myTaskPtr = getTaskById(TASK_ID_TEST);
 *  @param  respId the task ID you want to search.
 *  @return returns NULL if it didn't find it, and returns the correct pointer if it found it.
 */
static Urabros_TaskPtrTypeDef getTaskById(Urabros_CommandIdTypedef respId);

/** Static function for sending uint8_t datas to the given uTask.
 *  @param  taskPtr pointer to the task
 *  @param dataPtr pointer to the data buffer.
 *  @param dataLen length of the data to be sent to the task
 *  @return returns #uStatusOk if it is done, returns  #uStatusError if the queue is full.
 */
static Urabros_StatusTypeDef uSendDataToTask(Urabros_TaskPtrTypeDef taskPtr, uint8_t *dataPtr, uint8_t dataLen);

/** Static function, creates a statusresponse message from the current status and errorCodes of the uTasks.
 *  @param  uTxPtr pointer to the outgoing message
 *  @return
 */
static void urabrosCreateStatusResponse(Urabros_MsgPtr uTxPtr);

/** Static function for adding all the uTasks set to #uTaskMode_Continious mode to the command list.\n
 *  This function is called once at the init phase, do not call it again.
 *  @param
 *  @return
 */
static void urabrosAddContiniousTasksToCommandList(void);

/* LOCAL VARIABLES */
Urabros_TaskPtrTypeDef uTasks[TASK_COUNT];   /**< This variable holds all of the #Urabros_TaskPtrTypeDef pointers, whom are pointing to all the uTasks*/
const uint8_t signalStart   = uSignalStart;         /**< Signal variable for starting an uTask*/
const uint8_t signalACK     = uSignalACK;           /**< Signal variable for sending ACK to uTask*/
const uint8_t signalData    = uSignalSendData;      /**< Signal variable for determine sending, this is not used at the moment.*/
const uint8_t signalStop    = uSignalStop;          /**< Signal variable for sending Stop to uTask, this is not implmented jet.*/
const uint8_t signalResume  = uSignalResume;        /**< Signal variable for sending Resume to uTask, this is not implmented jet.*/
const uint8_t* const start  = &signalStart;         /**< Pointer to start signal*/
const uint8_t* const end    = &signalACK;           /**< Pointer to ack signal*/
const uint8_t* const data   = &signalData;          /**< Pointer to send datasignal*/
const uint8_t* const stop   = &signalStop;          /**< Pointer to stop signal*/
const uint8_t* const resume = &signalResume;        /**< Pointer to resumesignal*/


/* URABROS THREADS */
osThreadId urabrosCommunicationId;  /**< Thread def for FreeRTOS*/
osThreadId urabrosLogicControlId;   /**< Thread def for FreeRTOS*/
osThreadId urabrosMessageSenderId;  /**< Thread def for FreeRTOS*/

/** Thread function prototype for handling incoming commands from driving PC or MC.
 *  @param  argunents arguments to the thread we pass NULL theese cases.
 *  @return
 */
void urabrosCommunicationFunction(void const *argument);

/** Thread function prototype for updating the command statuses, depending on the uTasks statuses
 *  @param  argunents arguments to the thread we pass NULL theese cases.
 *  @return
 */
void urabrosLogicControlFunction(void const *argument);

/** Thread function prototype for sending out the uMessages and DebugMessages.
 *  @param  argunents arguments to the thread we pass NULL theese cases.
 *  @return
 */
void urabrosMessageSenderFunction(void const *argument);

// Thread definitions
/** Mcaro define for register the thread for FreeRTOS.
 *  @param name name of the thread it has to be individual for all the threads. 
 *  @param thread function pointer where the actual thread will run.
 *  @param priority priority of the thread
 *  @param instances instances of the thread, usually we use 1
 *  @param stacksz srack size, if you goes to a problem when the software stops running usually the stack size is too small.
 */
osThreadDef(urabrosCommunication,   urabrosCommunicationFunction,   osPriorityAboveNormal,  1, configMINIMAL_STACK_SIZE * 2);

/** Mcaro define for register the thread for FreeRTOS.
 *  @param name name of the thread it has to be individual for all the threads. 
 *  @param thread function pointer where the actual thread will run.
 *  @param priority priority of the thread
 *  @param instances instances of the thread, usually we use 1
 *  @param stacksz srack size, if you goes to a problem when the software stops running usually the stack size is too small.
 */
osThreadDef(urabrosLogicControl,    urabrosLogicControlFunction,    osPriorityNormal,  1, configMINIMAL_STACK_SIZE * 2);

/** Mcaro define for register the thread for FreeRTOS.
 *  @param name name of the thread it has to be individual for all the threads. 
 *  @param thread function pointer where the actual thread will run.
 *  @param priority priority of the thread
 *  @param instances instances of the thread, usually we use 1
 *  @param stacksz srack size, if you goes to a problem when the software stops running usually the stack size is too small.
 */
osThreadDef(urabrosMessageSender,   urabrosMessageSenderFunction,   osPriorityAboveNormal,  1, configMINIMAL_STACK_SIZE * 2);

/** Initialize all the uTasks and important threads Urabros needs.
 */
void UrabrosInit(void)
{
    // DONT CHANGE THE ORDER OF THEESE !

    // Message relevant inits
    uMsgInInit();
    uMsgOutInit();
    uDebugPrintInit();

    // Task relevant inits
    urabrosFillTasksArray();
    urabrosInitTasks();

    // Command handler init
    uCommandHandlerInit();
    urabrosAddContiniousTasksToCommandList();

    urabrosCommunicationId  = osThreadCreate(osThread(urabrosCommunication), NULL);
    urabrosLogicControlId   = osThreadCreate(osThread(urabrosLogicControl), NULL);
    urabrosMessageSenderId  = osThreadCreate(osThread(urabrosMessageSender), NULL);

#if DPRINT_ENABLE
#endif

}

void urabrosCommunicationFunction(void const *argument)
{
    Urabros_Msg                 uMsgRx      = {0};
    Urabros_MsgPtr              uMegRxPtr   = &uMsgRx;
    Urabros_Msg                 uMsgTx      = {0};
    Urabros_MsgPtr              uMegTxPtr   = &uMsgTx;
    Urabros_MsgStatus           uMsgStatus  = uMsg_Ok;
    Urabros_CommandTypedef      uCommand    = {0};
    Urabros_CommandPtrTypedef   uComandmPtr = &uCommand;
    Urabros_TaskPtrTypeDef      uTask       = NULL;

    for(;;)
    {
        // If an msg arrived pops it from the queue
        uMsgStatus = uMsgInPop(uMegRxPtr);
        if(uMsgStatus == uMsg_Ok) {

            // Append type if command we receive
            uMsgAppend(uMegTxPtr, uMsgRx.data[0]);

            // Swithc the command type
            switch(uMsgRx.data[0]) {

                // Creating the status response 
                case  uCommand_GET_STATUS :
                    urabrosCreateStatusResponse(uMegTxPtr);
                    uCommandPrintList();
                    break;

                case uCommand_START :
                    uCommand.id = uMsgRx.data[1];
                    uMsgAppend(uMegTxPtr, uMsgRx.data[1]); // Append Tx with Task ID

                    // Check if the given ID is pointing to a disabled task.
                    if(!isTaskIDValid(uCommand.id)) {
                        uMsgAppend(uMegTxPtr, uCommandIdDisabledTask);
                        dprintln("Disabled task");
                        break;
                    }

                    switch(uCommandAppend(uComandmPtr)) {
                        case uCommandAdded:
                            uTask = getTaskById(uCommand.id);
                            if(uTask->status == uTaskStatusWaitingForStartSignal) {
                                if(uSendDataToTask(uTask, start, 1) == uStatusOk) {
                                    uMsgAppend(uMegTxPtr, uCommandAdded);
                                    dprintln("Command added");
                                } else {
                                    uMsgAppend(uMegTxPtr, uCommandError);
                                    dprintln("Task Queue overflow");
                                }
                            } else {
                                uMsgAppend(uMegTxPtr, uCommandNotFinished);
                                dprintln("Task is not waiting to start");
                            }
                            break;
                        case uCommandIdAlreadyUsed:
                            uMsgAppend(uMegTxPtr, uCommandIdAlreadyUsed);
                            dprintln("Command ID already used");
                            break;
                        case uCommandTimedOut:
                            uMsgAppend(uMegTxPtr, uCommandTimedOut);
                            dprintln("Command addition timeout");
                            break;
                        case uCommandOwerFlow:
                            uMsgAppend(uMegTxPtr, uCommandOwerFlow);
                            dprintln("Command Overflow");
                            break;
                        case uCommandIdOutOfRange:
                            uMsgAppend(uMegTxPtr, uCommandIdOutOfRange);
                            dprintln("Command out of range");
                            break;
                        default:
                            uMsgAppend(uMegTxPtr, uCommandError);
                            dprintln("Addition error");
                            break;
                    }
                    break;

                case uCommand_DELETE :
                    uMsgAppend(uMegTxPtr, uMsgRx.data[1]); // Append Tx with Task ID
                    switch(uCommandRemoveById((Urabros_CommandIdTypedef)uMsgRx.data[1])) {
                        case uCommandDeleted :
                            uMsgAppend(uMegTxPtr, uCommandDeleted);
                            uSendDataToTask(getTaskById(uMsgRx.data[1]), end, 1);
                            dprintln("Command deleted");
                            break;
                        case uCommandNotFound :
                            uMsgAppend(uMegTxPtr, uCommandNotFound);
                            dprintln("Command delete not found");
                            break;
                        case uCommandNotFinished :
                            uMsgAppend(uMegTxPtr, uCommandNotFinished);
                            dprintln("Command delete not finished");
                            break;
                        case uCommandTimedOut :
                            uMsgAppend(uMegTxPtr, uCommandTimedOut);
                            dprintln("Command delete timeout");
                            break;
                        default:
                            uMsgAppend(uMegTxPtr, uCommandError);
                            dprintln("Removing error");
                            break;
                    }
                    break;

                case uCommand_SEND_DATA :
                    uMsgAppend(uMegTxPtr, uMsgRx.data[1]); // Append Tx with Task ID
                    uTask = getTaskById(uMsgRx.data[1]);
                    if(uTask == NULL){
                        uMsgAppend(uMegTxPtr, uCommandIdOutOfRange);
                        dprintln("Command out of range");
                        break;
                    }

                    // If the task is in Waiting for Start or ACK than Cant send data to it.
                    if(uTask->status == uTaskStatusWaitingForACKSignal || uTask->status == uTaskStatusWaitingForStartSignal) {
                        uMsgAppend(uMegTxPtr, uCommandCantReceiveData);
                        dprintln("Task cant receive data");
                        break;
                    }

                    if(uSendDataToTask(uTask, uMsgRx.data + 2, uMsgRx.dataLen - 2) == uStatusOk) {
                        uMsgAppend(uMegTxPtr, uStatusOk);
                        dprintln("Sent data to task")
                    } else {
                        uMsgAppend(uMegTxPtr, uCommandOwerFlow);
                        dprintln("Queue is full")
                    }
                    break;

               case uCommand_RECEIVE_ERROR :
                   switch(uMsgRx.data[1]) {
                       case uMSg_IdleError :
                           uMsgAppend(uMegTxPtr, uMSg_IdleError);
                           break;

                       case uMsg_DataLenError :
                           uMsgAppend(uMegTxPtr, uMsg_DataLenError);
                           break;

                       case uMsg_CrcError :
                           uMsgAppend(uMegTxPtr, uMsg_CrcError);
                           break;

                       default:
                           break;
                   }

                   break;

                case uCommand_EMERGENCY_STOP :
                    //TODO call emergency stop function
                    break;

                default :
                    uMsgPrint(uMegRxPtr);
                    break;

            }

            uMsgSetCrc(uMegTxPtr);
            uMsgOutPut(uMegTxPtr);

            // Reset RX message and temp command
            uMsgReset(uMegRxPtr);
            uMsgReset(uMegTxPtr);
            uCommandClear(uComandmPtr);
        }
        osDelay(COMMUNICATION_DELAY);
    }
}

void urabrosLogicControlFunction(void const *argument)
{

    static Urabros_CommandPtrTypedef cmdPtr;
    static Urabros_TaskPtrTypeDef   taskPtr;

    for(;;)
    {

        if (xSemaphoreTake(uCommandListMutex, COMMAND_TIMEOUT) == pdTRUE) {
            for(uint8_t cmdIdx = 0; cmdIdx < uCommandNumber; cmdIdx++) {
                cmdPtr = uCommandList + cmdIdx;
                taskPtr = getTaskById(cmdPtr->id);

                if(taskPtr != NULL) {
                    cmdPtr->status.main  = taskPtr->status;
                    cmdPtr->status.minor = taskPtr->errorCode;
                }

            }
            xSemaphoreGive(uCommandListMutex);
        } else {
            dprintln("Cant take mutex");
        }

        osDelay(LOGIC_CONTROL_DELAY);
    }
}

void urabrosMessageSenderFunction(void const *argument)
{
    Urabros_Msg                 uMsgTx      = {0};
    Urabros_MsgPtr              uMegTxPtr   = &uMsgTx;

    for(;;)
    {
        while(1) {
            // if there is an Urabros message to be sent send it.
            if(uMsgOutPop(uMegTxPtr) == uMsg_Ok) {
                while(1) {
                    if(uMsgSend(uMegTxPtr) == uMsg_Ok) {
                        break;
                    }
                    else {
                        osDelay(1);
                    }
                }
            } else {
                break;
            }
            osDelay(MESSAGE_SENDING_TIME); //TODO decide is this needed or not, and think about using HAL_UART_TxCpltCallback() to determine we can send the next message or not.
        }

        #if DPRINT_ENABLE
            osDelay(1);
            // Only enable if no Urabros message is waiting to be sent.
            if(!(*uMsgOutWaitingNumPtr)) {
                uDebugPrintRead();
            }
        #endif

        osDelay(1);
    }
}

void urabrosAddContiniousTasksToCommandList(void)
{
    Urabros_TaskPtrTypeDef taskPtr = NULL;
    Urabros_CommandTypedef tempCommand;

    for(uint16_t taskIndex = 0; taskIndex < TASK_COUNT; taskIndex++) {
        taskPtr = uTasks[taskIndex];
        if(taskPtr->mode == uTaskMode_Continious) {
            tempCommand.id = taskPtr->responsibleTaskId;
            uCommandAppend(&tempCommand);
        }
    }
}

static Urabros_TaskPtrTypeDef getTaskById(Urabros_CommandIdTypedef respId)
{
    Urabros_TaskPtrTypeDef taskPtr = NULL;
    for(uint8_t taskIdx = 0; taskIdx < TASK_COUNT; taskIdx++) {
        taskPtr = uTasks[taskIdx];
        if(taskPtr->responsibleTaskId == respId) {
            return taskPtr;
        }
    }
    return NULL;
}

Urabros_StatusTypeDef uSendDataToTask(Urabros_TaskPtrTypeDef taskPtr, uint8_t *dataPtr, uint8_t dataLen)
{
    xSemaphoreTake(taskPtr->mutex, portMAX_DELAY);
    for(uint8_t dataIdx = 0; dataIdx < dataLen; dataIdx++) {
        if(xQueueSend(taskPtr->queueMaster, dataPtr + dataIdx, portMAX_DELAY) != pdTRUE) { //TODO maybe add timeout
            xSemaphoreGive(taskPtr->mutex);
            return uStatusError; // Queue is full
        }
    }

    xSemaphoreGive(taskPtr->mutex);
    return uStatusOk;
}

void urabrosCreateStatusResponse(Urabros_MsgPtr uTxPtr)
{
    static Urabros_CommandPtrTypedef cmdPtr;
    uint8_t cmdStatus = 0;

    if (xSemaphoreTake(uCommandListMutex, COMMAND_TIMEOUT) == pdTRUE) {
        for(uint8_t cmdIdx = 0; cmdIdx < uCommandNumber; cmdIdx++) {
            cmdPtr      = uCommandList + cmdIdx;
            cmdStatus   = 0;
            cmdStatus   = cmdPtr->status.main << 5;
            cmdStatus   |= cmdPtr->status.minor;
            uMsgAppend(uTxPtr, cmdPtr->id);
            uMsgAppend(uTxPtr, cmdStatus);
        }
        xSemaphoreGive(uCommandListMutex);
    } else {
        dprintln("Cant take mutex");
    }
}

static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];
/** FREE RTOS NEEDS THIS FUNCTION */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
