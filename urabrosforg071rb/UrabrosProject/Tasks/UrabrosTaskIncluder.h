/**
  * @file     UrabrosTaskIncluder.h
  * @author   marton.lorinczi
  * @date     May 22, 2021
  *
  * @brief
  */

#ifndef TASKS_URABROSTASKINCLUDER_H_
#define TASKS_URABROSTASKINCLUDER_H_

#include "UrabrosConfig.h"

void uAddTaskToArray(Urabros_TaskPtrTypeDef taskPtr);

/* TASK INCLUDES */
#if TASK_TESTER
    #include "testerTask.h"
#endif


/** Static function for fillint the #uTasks array with the correct pointers.
 *  This has to be modified manually by the project"s architect.
 *  @param  void
 *  @return void
 */void urabrosFillTasksArray(void)
{
    #if TASK_TESTER
        uAddTaskToArray(testerTaskPtr);
    #endif
}

/** Static function for calling all the init functions of uTasks\n
 *  This has to be modified manually by the project"s architect.
 *  @param
 *  @return
 */
void urabrosInitTasks(void)
{
    #if TASK_TESTER
        initTesterTask(TASK_ID_TEST);
    #endif
}

/** Static function for adding a uTask pointer to the #uTasks array.
 *  @param  Urabros_TaskPtrTypeDef pointer to the uTask.
 *  @return void
 */
void uAddTaskToArray(Urabros_TaskPtrTypeDef taskPtr)
{
    static uint8_t uTaskCntr = 0;
    extern Urabros_TaskPtrTypeDef uTasks[TASK_COUNT];

    // If reached maximum number of utasks
    if(uTaskCntr >= TASK_COUNT) {
        return;
    }

    uTasks[uTaskCntr] = taskPtr;
    uTaskCntr++;
}


#endif /* TASKS_URABROSTASKINCLUDER_H_ */
