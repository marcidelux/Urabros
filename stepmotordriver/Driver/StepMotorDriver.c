#include <StepMotorDriver.h>
#include <stdio.h> 	// For printf
#include <stdlib.h>


// For HAL_TIM functions
#include "tim.h"
//#include "stm32h7xx_hal_tim.h"

// For osDealy
//#include "FreeRTOS.h"
//#include "cmsis_os.h"

// For GPIOS
#include "main.h"

#define MAX_MOTOR_HANDLERS 5
//#define malloc(size) pvPortMalloc(size) // maybe this is needed when freeRTOS is used!

// private variables
static MotorHandlerStructPtr motorHandlerArray[MAX_MOTOR_HANDLERS]; // array with pointers to the initialized motor handlers
static uint8_t motorHandlerIndex = 0;								// the index of the next motor handler


/************************************/
/*** PRIVATE FUNCTION PROTOTYPES ***/
/************************************/
void 	MotorReadLimitSwitch				(MotorHandlerStruct *hMotor);
void 	MotorEnableMotorController 			(MotorHandlerStruct *hMotor);
void 	MotorDisableMotorController 		(MotorHandlerStruct *hMotor);
void 	MotorSetDirectionPin				(MotorHandlerStruct *hMotor);
void 	MotorCalculateActualSpeedProfile 	(MotorHandlerStruct *hMotor);
void 	MotorSetSpeed						(MotorHandlerStruct *hMotor, uint32_t prescale);
void 	MotorAccelerateSpeed				(MotorHandlerStruct *hMotor);

/*************************/
/*** PUBLIC FUNCTIONS ***/
/*************************/

/**
 * Init function of motor handler. Have to call this before any action!
 * It fills all the properties of an empty motor handler with desired data
 * @input: empty motor handler, HAL pin configuration, user parameters (speeds, gear ratio, acceleration etc.)
 */
void MotorDriverInit(MotorHandlerStruct *hMotor, MotorPinsStruct *pins, MotorUserParametersStruct *input)
{
	// Add handler to the array
	motorHandlerArray[motorHandlerIndex] = hMotor;
	motorHandlerIndex++;


	// Set Timer handler
	hMotor->mPins.timerSourceFreq 			= pins->timerSourceFreq;
	hMotor->mPins.Tim        				= pins->Tim;
	hMotor->mPins.TimChannel 				= pins->TimChannel;

	//Set Driving mode
	hMotor->mMovement.drivingMode 			= DRIVE_MODE_FREERUN_BLOCKING;

	// Set direction
	hMotor->mMovement.direction 			= DIRECTION_FORWARD;

	// Set status
	hMotor->mStatus.runningStatus 			= MOTOR_STATUS_STANDING;
	hMotor->mStatus.position 				= 0;

	// Set timeout
    hMotor->mTimeout.timeOutCntr        	= 0;
    hMotor->mTimeout.timeOutLimit       	= 0;

	// Set the electric gear ratio
	hMotor->mProfile.gearNum				= input->gearNum;
	hMotor->mProfile.gearDenom				= input->gearDenom;

	// Set Acceleration
	uint32_t speedCalculatorConst 			= pins->timerSourceFreq / ( ( pins->Tim->Init.Period + 1 ) *  hMotor->mProfile.gearNum / hMotor->mProfile.gearDenom);
	hMotor->mProfile.speedMin               = speedCalculatorConst / input->speedMin 	- 1;
	hMotor->mProfile.speedMax 	    	    = speedCalculatorConst / input->speedMax 	- 1;
	hMotor->mProfile.speedFixed             = speedCalculatorConst / input->speedFixed 	- 1;
	hMotor->mProfile.accelerationSteps		= input->accelerationSteps;
	hMotor->mProfile.status 				= ACCELERATION_OFF;

	// Allocate memory for the arrays
	//TODO: what if freeRtos?
	hMotor->mProfile.tickArray 				= (uint32_t*) 	malloc(sizeof(uint32_t) * hMotor->mProfile.accelerationSteps);
	hMotor->mProfile.speedArray 			= (uint32_t*) 	malloc(sizeof(uint32_t) * hMotor->mProfile.accelerationSteps);
	hMotor->mProfile.accelPulseArray 		= (float*)		malloc(sizeof(float) 	* hMotor->mProfile.accelerationSteps);
	hMotor->mProfile.decelPulseArray 		= (float*)		malloc(sizeof(float) 	* hMotor->mProfile.accelerationSteps);

    // Set motor controller pins and logic
    hMotor->mPins.directionPort            	= pins->directionPort;
    hMotor->mPins.directionPin             	= pins->directionPin;
    hMotor->mPins.enablePort            	= pins->enablePort;
    hMotor->mPins.enablePin             	= pins->enablePin;
    hMotor->mPins.holdingTorque				= input->holdingTorque;
    hMotor->mPins.driverInputLogic			= input->driverInputLogic;

	// Set Limit switch
	hMotor->mPins.limitSwitchMode          	= pins->limitSwitchMode;
	hMotor->mPins.LimitSwitchPort_fwd       = pins->LimitSwitchPort_fwd;
	hMotor->mPins.LimitSwitchPin_fwd        = pins->LimitSwitchPin_fwd;
    hMotor->mPins.LimitSwitchPort_bwd       = pins->LimitSwitchPort_bwd;
    hMotor->mPins.LimitSwitchPin_bwd        = pins->LimitSwitchPin_bwd;

    // Read limit switches based on mode
    MotorReadLimitSwitch(hMotor,hMotor->mPins.limitSwitchMode);

    // Set holding torque if needed
    if (hMotor->mPins.holdingTorque == HOLDING_TORQUE_ON)
    	MotorEnableMotorController(hMotor);
	else
    	MotorDisableMotorController(hMotor);

    // If acceleration is turned on: calculate the acceleration and decceleration profile
	if(hMotor->mProfile.accelerationSteps != 0)
	{
		// based on desired steps and time calculate the delta speed and delta time
		float differentialSpeed = (hMotor->mProfile.speedMin - hMotor->mProfile.speedMax) / hMotor->mProfile.accelerationSteps;
		float differentialTime = input->accelerationTime / hMotor->mProfile.accelerationSteps;

		// fill the speed- and time arrays
		for(uint16_t i = 0; i  < hMotor->mProfile.accelerationSteps; i++)
		{
			hMotor->mProfile.tickArray[i]  = (uint32_t) (i+1) * differentialTime;
			hMotor->mProfile.speedArray[i] = (uint32_t) hMotor->mProfile.speedMin - i * differentialSpeed;
		}

		// constant value to change the speed [UU/s] to prescale of the motor PWM timer
		uint32_t prescaleToFreqConst = hMotor->mPins.timerSourceFreq / ( pins->Tim->Init.Period + 1 );

		// zero index
		hMotor->mProfile.accelPulseArray[0] = (float) prescaleToFreqConst / ( hMotor->mProfile.speedArray[0] + 1 ) * hMotor->mProfile.tickArray[0] / 1000;

		// pulses taken at different speeds
		float distancesArray[hMotor->mProfile.accelerationSteps];
		distancesArray[0] = hMotor->mProfile.accelPulseArray[0];

		for (uint16_t i = 1; i < hMotor->mProfile.accelerationSteps; i++)
		{
			distancesArray[i] = (float) prescaleToFreqConst / ( hMotor->mProfile.speedArray[i] + 1 ) *
					(hMotor->mProfile.tickArray[i] - hMotor->mProfile.tickArray[i-1]) / 1000;
			// accelPulseArray is a commulative array!
			hMotor->mProfile.accelPulseArray[i] = (float) distancesArray[i] + hMotor->mProfile.accelPulseArray[i-1];
		}

		// zero index
		hMotor->mProfile.decelPulseArray[0] = distancesArray[hMotor->mProfile.accelerationSteps-1];

		// fill the decceleraion array too
		for (uint16_t i = 1 ; i < hMotor->mProfile.accelerationSteps; i++)
			hMotor->mProfile.decelPulseArray[i] = distancesArray[hMotor->mProfile.accelerationSteps - i - 1] + hMotor->mProfile.decelPulseArray[i - 1];

		MotorSetSpeed(hMotor, hMotor->mProfile.speedMin);
	}
	// If acceleration turned off set the speed to fixed
	else
		MotorSetSpeed(hMotor, hMotor->mProfile.speedFixed);

}

/**
 * Set the timeout of motor
 * @input: motorhandler, timeoutlimit
 */
void MotorSetTimeout(MotorHandlerStruct *hMotor, uint16_t timeOutLimit)
{
	hMotor->mTimeout.timeOutCntr 	= 0;
	hMotor->mTimeout.timeOutLimit 	= timeOutLimit;
}

/**
 * This function can be called in while loops.
 * It adds 1 to the timeoutcounter and if it reaches the limit the motor status will be changed to ERROR.
 * @input: motor handler
 */
void MotorCheckTimeout(MotorHandlerStruct *hMotor)
{
	if(hMotor->mStatus.runningStatus == MOTOR_STATUS_ERROR_TIMEOUT || hMotor->mStatus.runningStatus == MOTOR_STATUS_STANDING)
		return;

	hMotor->mTimeout.timeOutCntr++;
	if(hMotor->mTimeout.timeOutCntr >= hMotor->mTimeout.timeOutLimit)
		hMotor->mStatus.runningStatus = MOTOR_STATUS_ERROR_TIMEOUT;
}

/**
 * This blocks the thread until MotorStop was called
 * Use only when driving mode is set to STEP
 */
void MotorWaitUntilFinish(MotorHandlerStruct *hMotor, uint16_t waitDelay)
{
    if(hMotor->mStatus.runningStatus == MOTOR_STATUS_ERROR_TIMEOUT)
        return;

    // Wait until the motor stops
    while(hMotor->mStatus.runningStatus != MOTOR_STATUS_STANDING) {
        //osDelay(waitDelay);
    }
}

/**
 * Starts the motor with desired conditions of command
 * @input: motor handler, command of the user
 */
void MotorStart(MotorHandlerStruct *hMotor, MotorCommandMove command)
{
    // If the status is not standing than it wont take the command.
    if(hMotor->mStatus.runningStatus != MOTOR_STATUS_STANDING)
        return;

    // Set command vars
    hMotor->mMovement.drivingMode   	= command.cMode;
    hMotor->mMovement.direction			= command.cDirection;
    hMotor->mMovement.pulseDifference  	= command.cUserUnitDifference * hMotor->mProfile.gearNum / hMotor->mProfile.gearDenom;
    hMotor->mMovement.pulseCounter 		= 0;
    hMotor->mMovement.arrayIndex 		= 0;

    // It is clever because mMode 1 - 4 is equal to status 1 - 4
    hMotor->mStatus.runningStatus = hMotor->mMovement.drivingMode;

    if (hMotor->mStatus.runningStatus == MOTOR_STATUS_FREERUN_BLOCKING || hMotor->mStatus.runningStatus == MOTOR_STATUS_FREERUN_NON_BLOCKING)
    	hMotor->mProfile.status = ACCELERATION_OFF;
    else
    	hMotor->mProfile.status = ACCELERATION_SPEED_UP;

    // Set Enable pin
    if (hMotor->mPins.holdingTorque == HOLDING_TORQUE_OFF)
    	MotorEnableMotorController(hMotor);

    // Set Direction pin
    MotorSetDirectionPin(hMotor);

    // calculate the actual movement profile if needed and set the speed
    if(hMotor->mProfile.status != ACCELERATION_OFF)
    {
    	MotorCalculateActualSpeedProfile(hMotor);
    	MotorSetSpeed(hMotor, hMotor->mProfile.speedMin);
    }
    else
    	MotorSetSpeed(hMotor, hMotor->mProfile.speedFixed);

    // Start the PWM
    HAL_TIM_PWM_Start_IT(hMotor->mPins.Tim, hMotor->mPins.TimChannel);

    // If it is blocking mode than wait until it finished.
    if(hMotor->mMovement.drivingMode == DRIVE_MODE_FREERUN_BLOCKING || hMotor->mMovement.drivingMode == DRIVE_MODE_STEP_BLOCKING)
        MotorWaitUntilFinish(hMotor,100);
}

/*
 * Stops the motor and update its status
 * This function is called from IT when driving mode is STEP and desired steps are taken.
 * When you use FREERUN mode you have to call it to stop motor. Note that if you started the motor in
 * BLOCKING mode that thread will be blocked until you wont stop from another thread.
 * @input: motor handler
 */
void MotorStop(MotorHandlerStruct *hMotor)
{
	if(hMotor->mStatus.runningStatus == MOTOR_STATUS_STANDING)
		return;

    HAL_TIM_PWM_Stop_IT(hMotor->mPins.Tim, hMotor->mPins.TimChannel);

    // Reset Enable pin
	if (hMotor->mPins.holdingTorque == HOLDING_TORQUE_OFF)
		MotorDisableMotorController(hMotor);

    /*
	// Set the taken steps depending on the mode
    if(hMotor->mMovement.drivingMode == DRIVE_MODE_FREERUN_BLOCKING || hMotor->mMovement.drivingMode == DRIVE_MODE_FREERUN_NON_BLOCKING)
    {
    	if (hMotor->mMovement.direction == DIRECTION_FORWARD)
    		hMotor->mStatus.position += hMotor->mMovement.pulseCounter;
    	else
    		hMotor->mStatus.position -= hMotor->mMovement.pulseCounter;
    }
    else if(hMotor->mMovement.drivingMode == DRIVE_MODE_STEP_BLOCKING || hMotor->mMovement.drivingMode == DRIVE_MODE_STEP_NON_BLOCKING)
    {
    	if (hMotor->mMovement.direction == DIRECTION_FORWARD)
			hMotor->mStatus.position += hMotor->mMovement.pulseDifference;
		else
			hMotor->mStatus.position -= hMotor->mMovement.pulseDifference;
    }
    */

	// Update position
	if (hMotor->mMovement.direction == DIRECTION_FORWARD)
		hMotor->mStatus.position += hMotor->mMovement.sumPulseCounter;
	else
		hMotor->mStatus.position -= hMotor->mMovement.sumPulseCounter;


    // Set pulse counter to 0 to avoid the interrupt problems
    hMotor->mMovement.pulseCounter 		= 0;
    hMotor->mMovement.sumPulseCounter 	= 0;

    // update the acceleration status
    if(hMotor->mProfile.status != ACCELERATION_OFF)
        hMotor->mProfile.status = ACCELERATION_SPEED_UP;

    // update motor status
    hMotor->mStatus.runningStatus = MOTOR_STATUS_STANDING;
}

/**
 * This function must be called in HAL_TIM_PWM_PulseFinishedCallback function!
 * Updates the counters and call the acceleration manager function
 */
void MotorPulseCallback(TIM_HandleTypeDef *htim)
{
	// look for the motor handler connected to timer
	uint8_t i = 0;
	while (motorHandlerArray[i]->mPins.Tim != htim)
		i++;

	// increase the pulse counter
	motorHandlerArray[i]->mMovement.pulseCounter++;
	motorHandlerArray[i]->mMovement.sumPulseCounter++;

    // If the motor in step mode
    if(motorHandlerArray[i]->mMovement.drivingMode == DRIVE_MODE_STEP_BLOCKING || motorHandlerArray[i]->mMovement.drivingMode == DRIVE_MODE_STEP_NON_BLOCKING)
        // Decide if PWM speed change needed or not.
		MotorAccelerateSpeed(motorHandlerArray[i]);
}

/**
 * This function must be called in HAL_GPIO_EXTI_Rising_Callback or HAL_GPIO_EXTI_Falling_Callback function!
 * Stops the motor which limit switch has called the IT
 */
void MotorLimitSwitchCallback(uint16_t GPIO_Pin)
{
	// look for the motor handler connected to the IT pin
	for(uint8_t i = 0; i < motorHandlerIndex; i++)
	{
		if(motorHandlerArray[i]->mPins.limitSwitchMode != LIMIT_SWITCH_NONE)
		{
			if(motorHandlerArray[i]->mPins.LimitSwitchPin_fwd == GPIO_Pin || motorHandlerArray[i]->mPins.LimitSwitchPin_bwd == GPIO_Pin)
			{
				// Stops the motor
				MotorStop(motorHandlerArray[i]);
				break;
			}
		}
	}
}

/********************************/
/*** PRIVATE DRIVER FUNCTIONS ***/
/********************************/

/**
 * Calculates the actual speed profile of the movement.
 * It adaptively change the acceleration and deceleration profile based on the desired taking pulses
 * @input: motor handler
 */
void MotorCalculateActualSpeedProfile (MotorHandlerStruct *hMotor) {

	uint32_t pulsesDuringAccel = (uint32_t) hMotor->mProfile.accelPulseArray[hMotor->mProfile.accelerationSteps-1];
	uint16_t searchIndex = 0;

	// when desired pulses are less than the pulses attached to the lowest speed
	if (hMotor->mMovement.pulseDifference <= 2 * hMotor->mProfile.accelPulseArray[0])
	{
		hMotor->mProfile.status = ACCELERATION_OFF;
		return;
	}

	// if the desired pulses are less than the number of pulses taken during acceleration
	if (hMotor->mMovement.pulseDifference < pulsesDuringAccel)
	{
		// look forward the half index
		while (hMotor->mMovement.pulseDifference > hMotor->mProfile.accelPulseArray[searchIndex])
			searchIndex++;

		searchIndex--;
		hMotor->mMovement.maxIndex = (uint16_t) searchIndex / 2;
	}
	// when the number of desired pulses between acceleration and deceleration distance
	else if (hMotor->mMovement.pulseDifference >= pulsesDuringAccel && hMotor->mMovement.pulseDifference < (2 * pulsesDuringAccel))
		hMotor->mMovement.maxIndex = (uint16_t) (hMotor->mProfile.accelerationSteps-1) / 2;
	else
		hMotor->mMovement.maxIndex = hMotor->mProfile.accelerationSteps-1;

	// calculate the number of pulses will be taken at constant speed
	hMotor->mMovement.pulsesAtConstSpeed = hMotor->mMovement.pulseDifference - 2 * (uint32_t) hMotor->mProfile.accelPulseArray[hMotor->mMovement.maxIndex];
}

/**
 * Read the state of limit switches based on the set mode
 * @input: motor handler
 */
void MotorReadLimitSwitch(MotorHandlerStruct *hMotor)
{
	switch (hMotor->mPins.limitSwitchMode)
	{
		case LIMIT_SWITCH_NONE:
			break;
		case LIMIT_SWITCH_BOTH:
			hMotor->mStatus.LimitSwitchState_fwd = HAL_GPIO_ReadPin(hMotor->mPins.LimitSwitchPort_fwd, hMotor->mPins.LimitSwitchPin_fwd);
			hMotor->mStatus.LimitSwitchState_bwd = HAL_GPIO_ReadPin(hMotor->mPins.LimitSwitchPort_bwd, hMotor->mPins.LimitSwitchPin_bwd);
			break;
		case LIMIT_SWITCH_FORWARD:
			hMotor->mStatus.LimitSwitchState_fwd = HAL_GPIO_ReadPin(hMotor->mPins.LimitSwitchPort_fwd, hMotor->mPins.LimitSwitchPin_fwd);
			hMotor->mStatus.LimitSwitchState_bwd = 0;
			break;
		case LIMIT_SWITCH_BACKWARD:
			hMotor->mStatus.LimitSwitchState_fwd = 0;
			hMotor->mStatus.LimitSwitchState_bwd = HAL_GPIO_ReadPin(hMotor->mPins.LimitSwitchPort_bwd, hMotor->mPins.LimitSwitchPin_bwd);
			break;
		default:
			break;
	}
}

/**
 * Enables the motor controller based on controller input logic
 * @input: motor handler
 */
void MotorEnableMotorController (MotorHandlerStruct *hMotor)
{
	if (hMotor->mPins.driverInputLogic == LOGIC_POSITIVE)
		HAL_GPIO_WritePin(hMotor->mPins.enablePort, hMotor->mPins.enablePin, GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(hMotor->mPins.enablePort, hMotor->mPins.enablePin, GPIO_PIN_RESET);
}

/**
 * Disables the motor controller based on controller input logic
 * @input: motor handler
 */
void MotorDisableMotorController (MotorHandlerStruct *hMotor)
{
	if (hMotor->mPins.driverInputLogic == LOGIC_POSITIVE)
		HAL_GPIO_WritePin(hMotor->mPins.enablePort, hMotor->mPins.enablePin, GPIO_PIN_RESET);
	else
		HAL_GPIO_WritePin(hMotor->mPins.enablePort, hMotor->mPins.enablePin, GPIO_PIN_SET);
}

/**
 * Set the direction pin to the desired
 * @input: motor handler
 */
void MotorSetDirectionPin (MotorHandlerStruct *hMotor)
{
	if(hMotor->mMovement.direction == DIRECTION_FORWARD)
		HAL_GPIO_WritePin(hMotor->mPins.directionPort, hMotor->mPins.directionPin, GPIO_PIN_SET);
	else if (hMotor->mMovement.direction == DIRECTION_BACKWARD)
		HAL_GPIO_WritePin(hMotor->mPins.directionPort, hMotor->mPins.directionPin, GPIO_PIN_RESET);
}

/*
 * Stops the PWM than set its speed to the given
 * @input: motor handler, new prescale
 */
void MotorSetSpeed(MotorHandlerStruct *hMotor, uint32_t prescale)
{
	// run if really need to run
	if (prescale != hMotor->mPins.Tim->Init.Prescaler) {

		TIM_OC_InitTypeDef sConfigOC = {0};
		HAL_TIM_PWM_Stop_IT(hMotor->mPins.Tim, hMotor->mPins.TimChannel);

		hMotor->mPins.Tim->Init.Prescaler = prescale;
		if (HAL_TIM_Base_Init(hMotor->mPins.Tim) != HAL_OK)
		{
		  Error_Handler();
		}
		if (HAL_TIM_PWM_Init(hMotor->mPins.Tim) != HAL_OK)
		{
		  Error_Handler();
		}
		sConfigOC.OCMode = TIM_OCMODE_PWM1;
		sConfigOC.Pulse = 50; // 50%
		sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
		sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
		if (HAL_TIM_PWM_ConfigChannel(hMotor->mPins.Tim, &sConfigOC, hMotor->mPins.TimChannel) != HAL_OK)
		{
		  Error_Handler();
		}
	}
}

/**
 * Manages the acceleration, deceleration and running at constant speed of the motor
 * This function is being called in IT Callback
 * This is kind of state-machine
 * @input: motor handler
 */
void MotorAccelerateSpeed(MotorHandlerStruct *hMotor)
{
    switch(hMotor->mProfile.status) {
    	// when acceleration off there is no need to manage anything except stopping in STEP driving mode
        case ACCELERATION_OFF :
            if(hMotor->mStatus.runningStatus == MOTOR_STATUS_STEP_BLOCKING || hMotor->mStatus.runningStatus == MOTOR_STATUS_STEP_NON_BLOCKING)
                if (hMotor->mMovement.pulseCounter >= hMotor->mMovement.pulseDifference)
                	MotorStop(hMotor);
            break;

        case ACCELERATION_SPEED_UP :
			// look for the actual speed state based on the taken pulses
        	while( hMotor->mMovement.pulseCounter >= (uint32_t) hMotor->mProfile.accelPulseArray[hMotor->mMovement.arrayIndex] )
        	{
        		hMotor->mMovement.arrayIndex++;
        		if(hMotor->mMovement.arrayIndex == hMotor->mMovement.maxIndex)
					break;
        	}

        	// if we reached the pulses has to be taken during acceleration
			if (hMotor->mMovement.pulseCounter >=  (uint32_t) hMotor->mProfile.accelPulseArray[hMotor->mMovement.maxIndex])
			{
				hMotor->mProfile.status = ACCELERATION_KEEP;
				hMotor->mMovement.arrayIndex = 0;
				hMotor->mMovement.pulseCounter = 0;
				// if the actual movement uses the whole acceleration profile set the speed to max
				if (hMotor->mMovement.maxIndex == hMotor->mProfile.accelerationSteps-1)
				{
					MotorSetSpeed(hMotor, hMotor->mProfile.speedMax);
				    HAL_TIM_PWM_Start_IT(hMotor->mPins.Tim, hMotor->mPins.TimChannel);
				}
				// if the actual movement uses a part of acceleration profile because of small distance to take
				else
				{
					MotorSetSpeed(hMotor, hMotor->mProfile.speedArray[hMotor->mMovement.maxIndex+1]);
				    HAL_TIM_PWM_Start_IT(hMotor->mPins.Tim, hMotor->mPins.TimChannel);
				}
				break;
			}

			// set the speed to the desired and start PWM
			MotorSetSpeed(hMotor, hMotor->mProfile.speedArray[hMotor->mMovement.arrayIndex]);
		    HAL_TIM_PWM_Start_IT(hMotor->mPins.Tim, hMotor->mPins.TimChannel);

            break;

        case ACCELERATION_KEEP :
            // If reached the point where it should start slowing down.
            if(hMotor->mMovement.pulseCounter >= hMotor->mMovement.pulsesAtConstSpeed)
            {
                hMotor->mProfile.status = ACCELERATION_SLOW_DOWN;
                // set the 'first' speed of the deceleration
                if (hMotor->mMovement.maxIndex == hMotor->mProfile.accelerationSteps-1)
                	hMotor->mMovement.pulseCounter = 0;
				else
				{
					hMotor->mMovement.arrayIndex = hMotor->mProfile.accelerationSteps - 1 - hMotor->mMovement.maxIndex;
					hMotor->mMovement.pulseCounter = (uint32_t) hMotor->mProfile.decelPulseArray[hMotor->mProfile.accelerationSteps-1] - hMotor->mProfile.accelPulseArray[hMotor->mMovement.maxIndex] + 1;
				}
                MotorSetSpeed(hMotor, hMotor->mProfile.speedArray[hMotor->mMovement.maxIndex]);
                HAL_TIM_PWM_Start_IT(hMotor->mPins.Tim, hMotor->mPins.TimChannel);
            }
            break;

        case ACCELERATION_SLOW_DOWN :

        	// look for the actual speed state based on the taken pulses
        	while( hMotor->mMovement.pulseCounter >= (uint32_t) hMotor->mProfile.decelPulseArray[hMotor->mMovement.arrayIndex] )
        	{
				hMotor->mMovement.arrayIndex++;
				if(hMotor->mMovement.arrayIndex == hMotor->mProfile.accelerationSteps-1)
					break;
			}

        	// if the motor has taken the desired pulses, stop it
			if (hMotor->mMovement.pulseCounter >=  (uint32_t) hMotor->mProfile.decelPulseArray[hMotor->mProfile.accelerationSteps-1])
			{
				MotorStop(hMotor);
				break;
			}

			// set the motor speed if it has changed and start it
			MotorSetSpeed(hMotor, hMotor->mProfile.speedArray[hMotor->mProfile.accelerationSteps-1-hMotor->mMovement.arrayIndex]);
		    HAL_TIM_PWM_Start_IT(hMotor->mPins.Tim, hMotor->mPins.TimChannel);

            break;
    }
}





























































