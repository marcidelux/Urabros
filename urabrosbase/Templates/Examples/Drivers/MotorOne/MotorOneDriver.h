/*
 * MotorOneDriver.h
 *
 *  Created on: Feb 26, 2021
 *      Author: NapoLion
 */


#ifndef DRIVERS_MOTORONE_MOTORONEDRIVER_H_
#define DRIVERS_MOTORONE_MOTORONEDRIVER_H_

#include "UrabrosTypeDef.h"
#include "StepMotorDriver.h"

extern Urabros_DriverPtrTypeDef motorOneDriverPtr;

#define MOTOR_ONE_MUTEX_TIMEOUT (TickType_t)100

void motorOneDriver_Init(void);

Urabros_StatusTypeDef motorOneDriver_StopMotor();
Urabros_StatusTypeDef motorOneDriver_StartMotor(MotorCommandMove command);
Urabros_StatusTypeDef motorOneDriver_GoThreeRound();
Urabros_StatusTypeDef motorOneDriver_StartMotorTimeout(MotorCommandMove command, uint32_t timeout);
Urabros_StatusTypeDef motorOneDriver_StartMotorSpeed(MotorCommandMove command, uint32_t speedMax);

#endif /* DRIVERS_MOTORONE_MOTORONEDRIVER_H_ */
