/**
  * @file     uCommandHandler.h
  * @author   Marton.Lorinczi
  * @date     Feb 10, 2020
  *
  * @brief    Command handle is one of the key part of the frimware.
  * 
  *           The driver machine usually in a Master/Slave hierarchy so the drier PC asks the state of the Urabros.
  *           The Status in this system is described with the statuses of the uTasks.
  *           An uTask has two kind of "status" one of them is teh status the other is errorCode.
  *           These two values are shifted in to one 8 bit value. (Status = main = 3bit long. Error code = minor = 5 bit long)
  *           From theese two values one is created with the @see urabrosCreateStatusResponse() funcion.
  *           So the driver PC will get all of the IDs of a the currently running tasks and their stauties.
  *           The drier can add and delete tasks from the command list. The maximum size of the command list is #TASK_COUNT
  *           because every command can have only one runnin instance in this architecture. 
  */

#ifndef MASTER_COMMUNICATION_UCOMMANDHANDLER_H_
#define MASTER_COMMUNICATION_UCOMMANDHANDLER_H_

#include "UrabrosTypeDef.h"

extern Urabros_CommandTypedef   uCommandList[TASK_COUNT]; /**< Array of commands, the maximum number */
extern SemaphoreHandle_t        uCommandListMutex;
extern uint8_t                  uCommandListStatusBuffer[TASK_COUNT * 2];
extern uint8_t                  uCommandNumber;

/**
  * @brief  
*/
Urabros_StatusTypeDef uCommandHandlerInit(void);
/**
  * @brief  
  * @param   commandPtr - 
  * @return  
*/
Urabros_CommandReturnStatus uCommandAppend(Urabros_CommandPtrTypedef commandPtr);
/**
  * @brief  
  * @param   commandId - 
  * @return  
*/
Urabros_CommandReturnStatus uCommandRemoveById(Urabros_CommandIdTypedef commandId);
/**
  * @brief  
  * @param   commandId - 
  * @param   cmdIndex - 
  * @return 
*/
Urabros_CommandReturnStatus uCommandGetIndexById(Urabros_CommandIdTypedef commandId, uint8_t *cmdIndex);
/**
  * @brief  
  * @param  commandId - 
  * @param  commandRetPtr - 
  * @return 
*/
Urabros_CommandReturnStatus uCommandGetById(Urabros_CommandIdTypedef commandId, Urabros_CommandPtrTypedef commandRetPtr);
/**
  * @brief  
  * @param   commandPtr - 
  * @return 
*/
Urabros_CommandReturnStatus uCommandClear(Urabros_CommandPtrTypedef commandPtr);

/**
  * @brief  
  * @param   commandPtr - 
  * @return 
*/
Urabros_CommandReturnStatus uCommandPrint(Urabros_CommandPtrTypedef commandPtr);
/**
  * @brief  
  * @return 
*/
Urabros_CommandReturnStatus uCommandPrintList(void);


#endif /* MASTER_COMMUNICATION_UCOMMANDHANDLER_H_ */
