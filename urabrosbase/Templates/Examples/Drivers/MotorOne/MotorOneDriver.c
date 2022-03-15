/*
 * MotorOneDriver.c
 *
 *  Created on: Feb 26, 2021
 *      Author: NapoLion
 */

#include "MotorOneDriver.h"
#include "UrabrosDriver.h"

extern TIM_HandleTypeDef htim15;

// Include dprint functions
#define DPRINT_LOCAL_ENABLE 1
#include "uDebugPrint.h"

// Include Timeout functions
#include "UrabrosTimeout.h"

Urabros_DriverPtrTypeDef motorOneDriverPtr = &Driver;

MotorHandlerStruct          MotorOneHandler;
MotorPinsStruct             MotorOnePins;
MotorUserParametersStruct   MotorOneUserParam;

void motorOneDriver_Init(void)
{
    // If it was already inited.
    if(Driver.status == uDriverStatus_InitDone)
        return;

    // Create the mutex
    Driver.mutex        = xSemaphoreCreateMutex();
    Driver.mutexTimeout = MOTOR_ONE_MUTEX_TIMEOUT;

    // Init Motor Driver Pins
    MotorOnePins.timerSourceFreq        = 64000000;
    MotorOnePins.Tim                    = &htim15;
    MotorOnePins.TimChannel             = TIM_CHANNEL_2;
    MotorOnePins.directionPort          = MotorOne_Dir_GPIO_Port;
    MotorOnePins.directionPin           = MotorOne_Dir_Pin;
    MotorOnePins.holdingTorque          = HOLDING_TORQUE_OFF;
    MotorOnePins.driverInputLogic       = LOGIC_POSITIVE;
    MotorOnePins.enablePort             = MotorOne_Enable_GPIO_Port;
    MotorOnePins.enablePin              = MotorOne_Enable_Pin;
    MotorOnePins.limitSwitchMode        = LIMIT_SWITCH_FORWARD;
    MotorOnePins.LimitSwitchPort_fwd    = BlueButton_GPIO_Port;
    MotorOnePins.LimitSwitchPin_fwd     = BlueButton_Pin;
    MotorOnePins.LimitSwitchPort_bwd    = NULL;
    MotorOnePins.LimitSwitchPin_bwd     = 0x00;

    // Init Motor User Param
    MotorOneUserParam.speedMin          = 5;
    MotorOneUserParam.speedMax          = 30;
    MotorOneUserParam.accelerationSteps = 60;
    MotorOneUserParam.accelerationTime  = 1000;
    MotorOneUserParam.speedFixed        = 5;
    MotorOneUserParam.driverInputLogic  = LOGIC_POSITIVE;
    MotorOneUserParam.holdingTorque     = HOLDING_TORQUE_OFF;
    MotorOneUserParam.gearNum           = 6000; // whole round
    MotorOneUserParam.gearDenom         = 10;

    MotorDriverInit(&MotorOneHandler, &MotorOnePins, &MotorOneUserParam);

    // Set state to init Done.
    Driver.status       = uDriverStatus_InitDone;
}

Urabros_StatusTypeDef motorOneDriver_StartMotor(MotorCommandMove command)
{
    if(uDriverMutexTake() == uStatusOk) {
        dprintln("Position before start: %d", MotorOneHandler.mStatus.position / MotorOneHandler.mProfile.gearNum * MotorOneHandler.mProfile.gearDenom);
        uDriverSetStatus(uDriverStatus_Running);
        MotorStart(&MotorOneHandler, command);
        uDriverSetStatus(uDriverStatus_Finished);
        dprintln("Position after end: %d", MotorOneHandler.mStatus.position / MotorOneHandler.mProfile.gearNum * MotorOneHandler.mProfile.gearDenom);
        return uDriverMutexGive();
    } else {
        return uStatusTimeOut;
    }
}

Urabros_StatusTypeDef motorOneDriver_StopMotor()
{
    MotorStop(&MotorOneHandler);
    dprintln("Motor position: %d", MotorOneHandler.mStatus.position);

}

Urabros_StatusTypeDef motorOneDriver_StartMotorSpeed(MotorCommandMove command, uint32_t speedMax)
{
    if(uDriverMutexTake() == uStatusOk) {
        dprintln("Position before start: %d", MotorOneHandler.mStatus.position / MotorOneHandler.mProfile.gearNum * MotorOneHandler.mProfile.gearDenom);
        MotorOneUserParam.speedMax          = speedMax;
        MotorDriverInit(&MotorOneHandler, &MotorOnePins, &MotorOneUserParam);
        uDriverSetStatus(uDriverStatus_Running);
        MotorStart(&MotorOneHandler, command);
        uDriverSetStatus(uDriverStatus_Finished);
        dprintln("Position after end: %d", MotorOneHandler.mStatus.position / MotorOneHandler.mProfile.gearNum * MotorOneHandler.mProfile.gearDenom);
        return uDriverMutexGive();
    } else {
        return uStatusTimeOut;
    }
}


Urabros_StatusTypeDef motorOneDriver_StartMotorTimeout(MotorCommandMove command, uint32_t timeout)
{
    MotorStatusEnum retMotStatus    = MOTOR_STATUS_STANDING;
    MotorOneHandler.mStatus.runningStatus   = MOTOR_STATUS_STANDING;
    if(uDriverMutexTake() == uStatusOk) {
        dprintln("Position before start: %d", MotorOneHandler.mStatus.position / MotorOneHandler.mProfile.gearNum * MotorOneHandler.mProfile.gearDenom);
        MotorStart(&MotorOneHandler, command);
        retMotStatus = MotorWaitUntilFinishTimeout(&MotorOneHandler, timeout);
        if(retMotStatus == MOTOR_STATUS_ERROR_TIMEOUT) {
            dprintln("Timeout hapend, stop Motor.");
        } else {
            dprintln("Motor Job done.")
        }
        dprintln("Position after end: %d", MotorOneHandler.mStatus.position / MotorOneHandler.mProfile.gearNum * MotorOneHandler.mProfile.gearDenom);
        return uDriverMutexGive();
    } else {
        return uStatusTimeOut;
    }
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
    MotorPulseCallback(htim);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    MotorLimitSwitchCallback(GPIO_Pin);
}
