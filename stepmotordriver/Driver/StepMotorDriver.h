
#ifndef INC_STEPMOTORDRIVER_H_
#define INC_STEPMOTORDRIVER_H_

#include "main.h"
/**
 * 'Freerun' means that the motor has been started and stopped whenever you want. Position will be updated when it stops
 * 'Step' means that you want to take exact number of steps
 * 'Blocking and Non Blocking' means that when you call for MotorStart it will wait until the steps has been taken or not
 */
typedef enum {
	DRIVE_MODE_FREERUN_BLOCKING 	    = 1,
	DRIVE_MODE_FREERUN_NON_BLOCKING	    = 2,
	DRIVE_MODE_STEP_BLOCKING 		    = 3,
	DRIVE_MODE_STEP_NON_BLOCKING	    = 4,
}MotorDrivingModeEnum;

/**
 * Same logic as MotorDrivingModeEnum. That enum will be mapped to the status all the time!
 */
typedef enum {
	MOTOR_STATUS_STANDING 				= 0,
	MOTOR_STATUS_FREERUN_BLOCKING 		= 1,
	MOTOR_STATUS_FREERUN_NON_BLOCKING 	= 2,
	MOTOR_STATUS_STEP_BLOCKING			= 3,
	MOTOR_STATUS_STEP_NON_BLOCKING		= 4,
	MOTOR_STATUS_ERROR_TIMEOUT			= 5,
}MotorStatusEnum;

/**
 * Actual status of motor acceleration
 */
typedef enum {
	ACCELERATION_OFF			        = 0,
	ACCELERATION_SPEED_UP		        = 1,
	ACCELERATION_SLOW_DOWN		        = 2,
	ACCELERATION_KEEP			        = 3,
}AccelerationStatusEnum;

/**
 * Enum for Direction selecion
 */
typedef enum {
    DIRECTION_FORWARD                   = 0,
    DIRECTION_BACKWARD                  = 1,
}MotorDirectionEnum;

/**
 * If you want to remain the 'holding torque' of a stepper motor after movement, you have to turn this on
 * If this is turned on the stepper motor controller wont be disabled after movement and enabled before it. It will be enabled all the time
 */
typedef enum {
	HOLDING_TORQUE_OFF					= 0,
	HOLDING_TORQUE_ON					= 1,
}MotorHoldingTorqueEnum;

/**
 * Configuration for built-in limit switches.
 */
typedef enum {
    LIMIT_SWITCH_NONE                   = 0,
    LIMIT_SWITCH_FORWARD                = 1,
    LIMIT_SWITCH_BACKWARD               = 2,
	LIMIT_SWITCH_BOTH					= 3,
}MotorLimitSwitchModeEnum;

/**
 * The logic of stepper motor controllers can be positive or negative.
 * Negative means that the controller is turned on when ENABLE pin is LOW.
 */
typedef enum {
	LOGIC_POSITIVE						= 0,
	LOGIC_NEGATIVE						= 1,
}MotorDriverLogicEnum;

/**
 * input structure for user to set up!
 * brief for UserUnit [UU], and software gear ratio:
 * UU x gearNum / gearDenom = internal pulses ; 1 Turn = PulsePerRev ; (if gearNum = pulsePerRev & gearDenom = 1 => 1 UU = 1 Turn !)
 */
typedef struct {
	uint32_t                	speedMin; 			// [UU/s]
	uint32_t                	speedMax;			// [UU/s]
	uint16_t                	accelerationSteps;
	uint16_t                	accelerationTime;	// [ms]
	uint32_t                	speedFixed;			// [UU/s]
	MotorDriverLogicEnum		driverInputLogic;
	MotorHoldingTorqueEnum		holdingTorque;
	uint32_t					gearNum;			// numerator of software gear ratio
	uint32_t					gearDenom;			// denominator of software gear ratio
}MotorUserParametersStruct;

/**
 * Struct of motor acceleration and deceleration
 * Every variable is static during movement except status.
 */
typedef struct {
	uint32_t                	speedMin;			// [prescale]
	uint32_t                	speedMax;			// [prescale]
	uint32_t                	speedFixed;			// [prescale]
	uint16_t                	accelerationSteps;
	uint32_t                	*tickArray;			// contains the commulative time of accelerationTime
	uint32_t                	*speedArray;		// static array of speed steps
	float	                	*accelPulseArray;	// same size of speedArray. contains the number of taken pulses by the end of the actual speed
	float	                	*decelPulseArray;	// same size of speedArray. contains the number of taken pulses by the end of the actual speed
	AccelerationStatusEnum  	status;				// actual status of acceleration
	uint32_t					gearNum;			// numerator of software gear ratio
	uint32_t					gearDenom;			// denominator of software gear ratio
}MotorMovingProfileStruct;

/**
 * Timeout struct of motor.
 */
typedef struct {
	uint16_t					timeOutCntr;		// counter
	uint16_t					timeOutLimit;		// limit. When 'counter' reaches the 'limit' the motor status will change to ERROR
}MotorTimeoutStruct;

/**
 * Dynaically changing struct of motor based on the actual desired movement
 */
typedef struct {
	uint32_t					sumPulseCounter;		// number of taken pulses during movement
	uint32_t    				pulseCounter;			// taken pulses during actual state of acceleration (hint: this counter is reseted when acceleration is done)
	uint32_t    				pulseDifference;		// desired pulse to take
	uint32_t					pulsesAtConstSpeed;		// counted number of pulses during accelerated constant speed
	uint16_t					arrayIndex;				// counting index variable during acceleration
	uint16_t					maxIndex;				// maximum speed index during a movement. It can be vary based on desired pulses to take
	MotorDrivingModeEnum    	drivingMode;			// desired driving mode:  FREERUN/STEP & BLOCKING / NON_BLOCKING
	MotorDirectionEnum      	direction;				// desired direction of movement
}MotorMovementStruct;

/**
 * Status struct of motor
 */
typedef struct {
	MotorStatusEnum 			runningStatus;			// actual status of motor (standing | moving | error)
	uint32_t					position;				// [pulses]
	GPIO_PinState            	LimitSwitchState_fwd;	// forward limit switch status
	GPIO_PinState            	LimitSwitchState_bwd;	// backward limit switch status
}MotorStatusStruct;

/**
 * HAL Properties of motor
 * Pins, PWM timer
 */
typedef struct {
	uint32_t				 	timerSourceFreq;		// Timer source frequency. Can be checked in CubeMX config generation
	TIM_HandleTypeDef        	*Tim;
	uint32_t                 	TimChannel;
	GPIO_TypeDef             	*directionPort;
	uint16_t                 	directionPin;
	MotorHoldingTorqueEnum		holdingTorque;
	MotorDriverLogicEnum		driverInputLogic;
	GPIO_TypeDef             	*enablePort;
	uint16_t                 	enablePin;
	MotorLimitSwitchModeEnum 	limitSwitchMode;
	GPIO_TypeDef             	*LimitSwitchPort_fwd;
	uint16_t                 	LimitSwitchPin_fwd;
	GPIO_TypeDef             	*LimitSwitchPort_bwd;
	uint16_t                 	LimitSwitchPin_bwd;
}MotorPinsStruct;

/**
 *	Motor handling struct
 *	Contains HAL properties (pins, timer, etc.), timeout properties, motor status,
 *	static acceleration and decceleration profile, and dynamic parameters for actual movement.
 */
typedef struct {
    MotorPinsStruct         	mPins;		// static for pins and timer
	MotorTimeoutStruct      	mTimeout;	// timeout properties for error detection
	MotorStatusStruct       	mStatus;	// actual status
	MotorMovingProfileStruct 	mProfile;	// acceleration and deceleration profile
	MotorMovementStruct			mMovement;	// dynamic parameters for actual movements
}MotorHandlerStruct, *MotorHandlerStructPtr;

/**
 * Struct for giving command to motor. Have to define the Driving mode, the Direction.
 * If Driving mode is is FREERUN cUserUnitDifference is ignored!
 */
typedef struct {
   MotorDrivingModeEnum 		cMode;					// driving mode: FREERUN/STEP & BLOCKING / NON_BLOCKING
   MotorDirectionEnum   		cDirection;				// direction
   uint32_t             		cUserUnitDifference;	// desired movement difference [UU]
}MotorCommandMove;

/************************/
/*** PUBLIC FUNCTIONS ***/
/************************/

void MotorDriverInit			(MotorHandlerStruct *hMotor, MotorPinsStruct *pins, MotorUserParametersStruct *input);
void MotorSetTimeout			(MotorHandlerStruct *hMotor, uint16_t timeOutLimit);
void MotorCheckTimeout			(MotorHandlerStruct *hMotor);
void MotorStart					(MotorHandlerStruct *hMotor, MotorCommandMove command);
void MotorStop					(MotorHandlerStruct *hMotor);
void MotorWaitUntilFinish		(MotorHandlerStruct *hMotor, uint16_t waitDelay);
void MotorPulseCallback			(TIM_HandleTypeDef 	*htim);
void MotorLimitSwitchCallback	(uint16_t			GPIO_pin);

#endif /* INC_STEPMOTORDRIVER_H_ */
















































