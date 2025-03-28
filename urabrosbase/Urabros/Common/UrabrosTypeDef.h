/**
  * @file     UrabrosTypeDef.h
  * @author   Marton.Lorinczi
  * @date     Aug 3, 2020
  *
  * @brief  This file holds all the typedefs for Urabros Framework\n
  *         Many of the Tasks and other files are using the same typedefs,\n
  *         placing typedefs to separate files would overcomplicate include tree.
  */

#ifndef URABROSTYPEDEF_H_
#define URABROSTYPEDEF_H_

#include "UrabrosConfig.h"
#include "stdint.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"

/**
 * An enum for the basic Urabros function returns.
 */
typedef enum
{
    uStatusOk               = 0x00, /**< Function executed successfully. */
    uStatusError            = 0x01, /**< Something went wrong. */
    uStatusBusy             = 0x02, /**< Something is working, can't execute function. */
    uStatusTimeOut          = 0x03, /**< If time was given for function to execute it ran of. */
}Urabros_StatusTypeDef;

/**
 * An enum for determine the physical communication layer with the controlling unit
 * At this version only UART is created
 */
typedef enum
{
    uPeripheralUART         = 0x00, /**< UART peripheal */
    uPeripheralUSB          = 0x01, /**< USB peripheal*/
    uPeripheralETHERNET     = 0x02, /**< ETHERNET peripheral*/
}Urabros_MessagePeripheral;


// URABROS COMMAND RELEVANT TYPEDEFS
/**
 * An enum for the return status of the processed command
 */
typedef enum
{
    uCommandOk              = 0x00, /**< Everything is fine*/
    uCommandAdded           = 0x01, /**< Command added to the command list with the given Task ID*/
    uCommandNotFinished     = 0x02, /**< Task is still running, can't execute command*/
    uCommandNotFound        = 0x03, /**< Task ID is not found on the command list*/
    uCommandDeleted         = 0x04, /**< Task is deleted from the command list*/
    uCommandTimedOut        = 0x05, /**< Can't execute command it timed out*/
    uCommandOwerFlow        = 0x06, /**< The command list is full, or the Tasks Queue is full*/
    uCommandIdAlreadyUsed   = 0x07, /**< The given ID is already on the command list*/
    uCommandIdOutOfRange    = 0x08, /**< The given task ID is not valid*/
    uCommandCantReceiveData = 0x09, /**< Task is finished or waiting for start, at this point It cannot receive data*/
    uCommandIdDisabledTask  = 0x0A, /**< If the given ID points to a disabled task.*/
    uCommandError           = 0xFF, /**< Other error*/
}Urabros_CommandReturnStatus;

/**
 *  An enum for the Command IDs
 */
typedef enum
{
    uCommand_GET_STATUS     = 0x01, /**< Gets all the Tasks on the command list.*/
    uCommand_START          = 0x02, /**< Put a Task with given ID on the command list and send start signal to it.*/
    uCommand_DELETE         = 0x03, /**< Remove a Task with given ID from the command list.*/
    uCommand_SEND_DATA      = 0x04, /**< Sends data to a Task with the given ID.*/
    uCommand_PAUSE          = 0x05, /**< Pause a Task with a given ID.*/
    uCommand_RESUME         = 0x06, /**< Resume a Task with a given ID.*/
    uCommand_DATA_FROM_TASK = 0x07, /**< If a task sends data to PC directly.*/
    uCommand_RECEIVE_ERROR  = 0xFE, /**< If one of the incoming data were corrupted or badly designed, this indicates its failure.*/
    uCommand_EMERGENCY_STOP = 0xFF, /**< Calls emergency stop function*/
}Urabros_CommandType;

/** @struct Urabros_CommandTypedef
 *  @brief This structure is holding the status of a command it's size is 1 byte, because main is 3 bit and minor is 5 bit long.
 *  @var Urabros_CommandStatusTypeDef::main
 *  Holds the main status of the Task (Running, waiting, etc...)
 *  @var Urabros_CommandStatusTypeDef::minor
 *  Holds the minor status of the Task
 */
typedef struct {
    uint8_t main    : 3;
    uint8_t minor   : 5;
}Urabros_CommandStatusTypeDef, *Urabros_CommandStatusPtrTypeDef;

/** @struct Urabros_CommandTypedef
 *  @brief The commandType structure, the command list is made of theese structs.
 *  @var Urabros_CommandTypedef::id
 *  Id of the command and the connected Task
 *  @var Urabros_CommandTypedef::status
 *  Main and Minor status of the connected Task
 */
typedef struct {
    Urabros_CommandIdTypedef        id;
    Urabros_CommandStatusTypeDef    status;
}Urabros_CommandTypedef, *Urabros_CommandPtrTypedef;

/* URABROS TASK RELEVANT TYPEDEFS */
/**
 *  An enum conatins the most important signals what can be sent from UrabrosMaster to target Task.
 */
typedef enum
{
    uSignalStart    = 0xAA, /**< This signal starts the Task if its in uTaskStatusWaitingForStartSignal state*/
    uSignalACK      = 0xBB, /**< This signal is sent to Task if it's in uTaskStatusWaitingForACKSignal,\nthan the Task goes to waitng for start signal phase*/
    uSignalSendData = 0xCC, /**< This signal is tells that data will be arriving from Urabros Master,\nthis is not used at the moment.*/
    uSignalStop     = 0XDD, /**< This signal stops the Task.\nThis not implemented jet.*/
    uSignalResume   = 0XEE, /**< This signal resumes the Task.\nThis isnot implemented jet.*/
}Urabros_SignalMasterTypeDef;

/**
 *  An enum for describing the current status of a uTask
 */
typedef enum
{
    uTaskStatusSetup                    = 0, /**< This is the status before the Task is started. Think it as an InitStatus.*/
    uTaskStatusRunning                  = 1, /**< When the task is doing it's job.*/
    uTaskStatusWaitingForStartSignal    = 2, /**< When the task finished it's job, and ACK arrived it is waiting for the Urabros to send the Start signal.*/
    uTaskStatusWaitingForACKSignal      = 3, /**< When the task finished it's job waits for the ACK signal form the Urabros.*/
    uTaskStatusWaitingForInnerSignal    = 4, /**< When the task is waiting for an another signal usually from an another Task.*/
    uTaskStatusStopped                  = 5, /**< When the task is stopped by the Urabros.*/
    uTaskStatusError                    = 6, /**< When smoething went wrong, than this is the error state.*/
}Urabros_TaskStatusTypeDef;

/**
 *  An enum for describing the current status of a uDriver
 */
typedef enum
{
    uDriverStatus_Running               = 0, /**< When the driver is doing it's job*/
    uDriverStatus_Finished              = 1, /**< When the  job of the driver is done*/
    uDriverStatus_TimeOut               = 2, /**< When the driver's job did not end in the given time it goes to this state.*/
    uDriverStatus_Waiting               = 3, /**< When the driver is waiting for something*/
    uDriverStatus_InitDone              = 4, /**< When the driver's initphase successfully ended.*/
    uDriverStatus_Error                 = 255, /**< When something wrong happend.*/
}Urabros_DriverStatusTypeDef;

/**
 * An enum for sensor input mode for waiting function @see uDriverWaitUntil()
 */
typedef enum
{
  uSensorMode_WaitHigh                  = 0, /**< Sensor is waiting for a High level input*/
  uSensorMode_WaitLow                   = 1, /**< Sensor is waiting for a Low level input*/
}Urabros_DriverSensorMode;

/**
 * An enum for descibing the mode of the given task.
 */
typedef enum
{
    uTaskMode_OneTime                   = 0, /**< The task is waiting to be started, do its job once, than it waits for the ACK signal and this circle goes in a loop*/
    uTaskMode_Continious                = 1, /**< The task doesn't wait for any signal form the Urabros. It is continuously doing it's job*/
}Urabros_TaskMode;

/** @struct Urabros_TaskTypeDef
 *  @brief Structure to describe an Urabros Tak
 *         In shoreter name: uTask, this is one of the most important variable int the framework.\n
 *         Think an uTask as a abstraction layer for a job. It can have many hardware modules etc...\n
 *         But it's job has to be one goal. For example giving you a bottle of water.
 *         For every jobs the embedded system could make an uTask must be implemented.
 * 
 *  @var Urabros_TaskTypeDef::mode
 *  This variable indicates it the task is behaves as OneTime or Continious.
 * 
 *  @var Urabros_TaskTypeDef::status
 *  This varibale holds the status of the uTask. It values are fixed.
 * 
 *  @var Urabros_TaskTypeDef::errorCode
 *  This variable holds the errorCode of the current uTask,
 *  It's values should be implemented in the tasks header.
 *  Even if it is an uint8_t the value can be between only 0 - 31 !!!
 * 
 *  @var Urabros_TaskTypeDef::threadIdArrayLen
 *  Number of the CMSIS-RTOS threads this uTasks uses.
 *  The maximum possible number is defined in the UrabrosConfig.h as #MAX_SUBTHREADS
 * 
 *  @var Urabros_TaskTypeDef::threadIdArray[MAX_SUBTHREADS]
 *  This variable holds the pointers to the inner thread ids.
 *  Theese threads are the the logic level of the uTask.
 * 
 *  @var Urabros_TaskTypeDef::responsibleTaskId
 *  This variable holds the higher abstraction level CommandID.
 *  At the init part this variable must be set to the calling commands ID
 *  For example you have a command: #define SETLEDS_COMMAND = 4;
 *  At the UrabrosMaster.c call it like this: initLedTask(SETLEDS_COMMAND);
 * 
 *  @var Urabros_TaskTypeDef::mutex
 *  This variable is the mutex of the task.
 *  UrabrosMaster.c and the Task are using the taskTypedef variable,
 *  so it is necessary to have a mutex here.
 * 
 *  @var Urabros_TaskTypeDef::queueMaster
 *  This queue variable is for the communication between the UrabrosMaster.c and this Task
 *  The Task only receive messages from UrabrosMaster.c so this is an RX queue
 * 
 *  @var Urabros_TaskTypeDef::queueTask
 *  This queue variable is for receiving data from other Tasks.
 *  From architect point of view this kind of solutions should be avoided,
 *  but if there is no other way use this. For using this variable each Tasks has to see
 *  the others taskTypeDef variables. Use extern for it.
 */
typedef struct
{
    Urabros_TaskMode            mode;
    Urabros_TaskStatusTypeDef   status;
    uint8_t                     errorCode;
    uint8_t                     threadIdArrayLen;
    osThreadId                  threadIdArray[MAX_SUBTHREADS];
    Urabros_CommandIdTypedef    responsibleTaskId;
    SemaphoreHandle_t           mutex;
    QueueHandle_t               queueMaster;
    QueueHandle_t               queueTask;
}Urabros_TaskTypeDef, *Urabros_TaskPtrTypeDef;

/** @struct Urabros_DriverTypeDef
 *  @brief Structure of a DriverType.
 *         It is similar to ::Urabros_TaskTypeDef just way simpler.\n
 *         The main difference is an uDriver only should contain a vell described hardware part.\n
 *         This part can contain more than one phisical driver, but it has to be in one group.\n
 *         For example a temperature control uDriver contains a thermal battery and some sensors.\n
 *         uDriver dosn't have a thread, it has only public functions, the mutex handling must be implemented inside thoose functions.
 *  @var Urabros_DriverTypeDef::status
 *  This variable holds the status of the driver.
 *  It's value can be the followings: #Urabros_DriverStatusTypeDef
 *  
 *  @var Urabros_DriverTypeDef::mutex
 *  It's a mutex for the driver.
 *  With this that case can be avoided when more than one Tasks are trying to use the driver
 *  
 *  @var Urabros_DriverTypeDef::mutexTimeout
 *  It's holding the timeout value.
 *  A certen job can have a time limit when it must finishies it's job.\n
 *  If that limit is over than the status will change to timeout.
 */
typedef struct
{
    Urabros_DriverStatusTypeDef status;
    SemaphoreHandle_t           mutex;
    TickType_t                  mutexTimeout;
    Urabros_StatusTypeDef(*takeMutex)(void);
    Urabros_StatusTypeDef(*giveMutex)(void);
}Urabros_DriverTypeDef, *Urabros_DriverPtrTypeDef;

/**
 * Ane enum for describe statuses of the message handling.
 */
typedef enum {
    uMsg_Ok                 = 0, /**< Everything went well.*/
    uMsg_BufferIsFull       = 1, /**< This can describe many cases when a buffer is full, see at: @see uMsgInPut() @see uMsgOutPut() @see uMsgAppend()*/
    uMsg_BufferIsEmpty      = 2, /**< This can describe many cases when a bugger is empty, see at: @see uMsgInPop() @see uMsgOutPop()*/
    uMsg_CrcError           = 3, /**< If there was a crc misscalculation or data is corrupted, than this describes that state*/
    uMSg_IdleError          = 4, /**< If the transmittion broke while sending a message.*/
    uMsg_DataLenError       = 5, /**< If the first byte describing the lenght of message was wrong.*/
    uMsg_Busy               = 6, /**< If the Hal uart transmit returns with busy. @see uMsgSend()*/
    uMsg_Timeout            = 7, /**< If the sending was timeouted, if this happens an architectural mistake was made. */
    uMsg_CopyBufferTooBig   = 8, /**< If the outgoing buffer dosn't have space for the current message.*/
    uMsg_Error              = 255, /**< If some other error happend*/
}Urabros_MsgStatus, *Urabros_MsgStatusPtr;

/** @struct Urabros_Msg
 *  @brief Structure of an urabros message.
 *         The incoming data can be only this type. The outgoing can be this or a simple ASCII text format.
 * 
 *  @var Urabros_Msg::dataLen
 *  An uint8_t type variable holding the lenght of the incoming/outgoing data
 * 
 *  @var Urabros_Msg::data[MESSAGE_BUFFER_LENGTH]
 *  An uint8_t array holding the data to be sent or be processed its maxumum size can be set here: #MESSAGE_BUFFER_LENGTH
 * 
 *  @var Urabros_Msg::crc16Code
 *  an uint16_t varaible holding the calculated or received CRC16 number. The CRC only calculated to the data part.
 */
typedef struct {
    uint8_t  dataLen;
    uint8_t  data[MESSAGE_BUFFER_LENGTH];
    uint16_t crc16Code;
}Urabros_Msg, *Urabros_MsgPtr;

// Holds the incoming messages in a filo buffer

/** @struct Urabros_MsgInBuffer
 *  @brief Structure for holding the incoming messages ina filo buffer.
 * 
 *  @var Urabros_MsgInBuffer::msgBuff[MESSAGE_IN_ARRAY_LENGTH]
 *  An array of #Urabros_Msg type variables. The  size can be modified here: #MESSAGE_IN_ARRAY_LENGTH
 * 
 *  @var Urabros_MsgInBuffer::numOfMsg
 *  This variable holds the current number of messages inside the buffer. Its max value is 255.
 */
typedef struct {
    Urabros_Msg         msgBuff[MESSAGE_IN_ARRAY_LENGTH];
    uint8_t             numOfMsg;       // How many messages are in the buffer
}Urabros_MsgInBuffer, *Urabros_MsgInBufferPtr;


/** @struct Urabros_MsgOutBuffer
 *  @brief Structure for holding the outgoing messages ina filo buffer.
 * 
 *  @var Urabros_MsgOutBuffer::msgBuff[MESSAGE_OUT_ARRAY_LENGTH]
 *  An array of #Urabros_Msg type variables. The  size can be modified here: #MESSAGE_OUT_ARRAY_LENGTH
 * 
 *  @var Urabros_MsgOutBuffer::numOfMsg
 *  This variable holds the current number of messages inside the buffer. Its max value is 255.
 */
typedef struct {
    Urabros_Msg         msgBuff[MESSAGE_OUT_ARRAY_LENGTH];
    uint8_t             numOfMsg;       // How many messages are in the buffer
}Urabros_MsgOutBuffer, *Urabros_MsgOutBufferPtr;

#endif /* URABROSTYPEDEF_H_ */
