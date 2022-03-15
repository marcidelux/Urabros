/*
 * LedDriver.c
 *
 *  Created on: Feb 8, 2021
 *      Author: NapoLion
 */
#include "LedDriver.h"
#include "UrabrosDriver.h"

#define DPRINT_LOCAL_ENABLE 1
#include "uDebugPrint.h"

Urabros_DriverPtrTypeDef ledDriverPtr = &Driver;

typedef struct
{
    GPIO_TypeDef    *port;
    uint16_t        pin;
    GPIO_PinState   state;
}LED_Typedef;


static LED_Typedef leds[3];

void ledDriver_Init(void)
{
    // If it was already inited.
    if(Driver.status == uDriverStatus_InitDone)
        return;

    // Create the mutex
    Driver.mutex        = xSemaphoreCreateMutex();
    Driver.mutexTimeout = LED_MUTEX_TIMEOUT;

    // PUT HERE THE DRIVER SPECIFIC INIT PARTS
    leds[LED_1].port    = LD1_GPIO_Port;
    leds[LED_1].pin     = LD1_Pin;
    leds[LED_1].state   = GPIO_PIN_RESET;
    leds[LED_2].port    = LD2_GPIO_Port;
    leds[LED_2].pin     = LD2_Pin;
    leds[LED_2].state   = GPIO_PIN_RESET;
    leds[LED_3].port    = LD3_GPIO_Port;
    leds[LED_3].pin     = LD3_Pin;
    leds[LED_3].state   = GPIO_PIN_RESET;

    // Set state to init Done.
    Driver.status       = uDriverStatus_InitDone;
}

Urabros_StatusTypeDef ledDriver_ON(ledDriver_LedIndexTypedef ledIdx)
{
    if(uDriverMutexTake() == uStatusOk) {
        Driver.status = uDriverStatus_Running;

        if(leds[ledIdx].state == GPIO_PIN_SET) {
            Driver.status = uDriverStatus_Finished;
            return uDriverMutexGive();
        }

        leds[ledIdx].state = GPIO_PIN_SET;
        HAL_GPIO_WritePin(leds[ledIdx].port, leds[ledIdx].pin, leds[ledIdx].state);
        Driver.status = uDriverStatus_Finished;
        return uDriverMutexGive();

    } else {
        return uStatusTimeOut;
    }
}

Urabros_StatusTypeDef ledDriver_OFF(ledDriver_LedIndexTypedef ledIdx)
{
    if(uDriverMutexTake() == uStatusOk) {
        Driver.status = uDriverStatus_Running;

        if(leds[ledIdx].state == GPIO_PIN_RESET) {
            Driver.status = uDriverStatus_Finished;
            return uDriverMutexGive();
        }

        leds[ledIdx].state = GPIO_PIN_RESET;
        HAL_GPIO_WritePin(leds[ledIdx].port, leds[ledIdx].pin, leds[ledIdx].state);
        Driver.status = uDriverStatus_Finished;
        return uDriverMutexGive();

    } else {
        return uStatusTimeOut;
    }
}

Urabros_StatusTypeDef ledDriver_TOGGLE(ledDriver_LedIndexTypedef ledIdx)
{
    if(uDriverMutexTake() == uStatusOk) {
        Driver.status = uDriverStatus_Running;
        leds[ledIdx].state = !leds[ledIdx].state;
        HAL_GPIO_WritePin(leds[ledIdx].port, leds[ledIdx].pin, leds[ledIdx].state);
        Driver.status = uDriverStatus_Finished;
        return uDriverMutexGive();
    } else {
        return uStatusTimeOut;
    }
}
