/**
  * @file     UrabrosDriver.h
  * @author   Marton.Lorinczi
  * @date     Feb 8, 2021
  *
  * @brief  This header file will give the uDriver it's functions and other important fields.
  *         Include this header only to an uDriver's source file you create.
  *         Similarly to UrabrosTask.h this file contains a static variable called Driver.
  *         From the uDriver you can reach and set the drivers state with this value.
  */

#ifndef DRIVERS_URABROSDRIVER_H_
#define DRIVERS_URABROSDRIVER_H_

#include "UrabrosTypeDef.h"
#include "UrabrosDriverSharedResources.h"
#include "main.h"

/** A static handler variable, the other source files make modifications on this varaible.
 */
static Urabros_DriverTypeDef Driver;

/** @brief  Takes the mutex driver, it is important, because multiple tasks can use thge same driver, and with this problems can be avoided.
 *      
 *      Example of usage. Write theese in the tempDriver.c / initTempDriver() function:
 *      Driver.mutex        = xSemaphoreCreateMutex();
 *      Driver.mutexTimeout = TEMP_DRIVER_TIMEOUT; // This value is defined in the tempDriver.h 
 *      Than in the logic of the tempDriver.c: 
 *      if(uDriverMutexTake() == uStatusOk) {//DoStuff} else {//DoStuff}
 *  @return #uStatusOk - if the Mutex is taken
 *          #uStatusTimeOut - if it could not take the driver in the given time. The timeout value can be set via the #Driver variable.
 */
static inline Urabros_StatusTypeDef uDriverMutexTake()
{
    if(xSemaphoreTake(Driver.mutex, Driver.mutexTimeout) == pdTRUE)
        return uStatusOk;
    else
        return uStatusTimeOut;
}

/** @brief  Gives the mutex driver, it is important, because multiple tasks can use thge same driver, and with this problems can be avoided.
 *      
 *      Example of usage:
 *      In the logic of the tempDriver.c: 
 *      if(uDriverMutexGive() == uStatusOk) {//DoStuff} else {//Error handling}
 *  @return #uStatusOk - if the Mutex is given or it wasnt taken before.
 *          #uStatusError - Indicating that the semaphore was not first obtained correctly.
 */
static inline Urabros_StatusTypeDef uDriverMutexGive()
{
    if(xSemaphoreGive(Driver.mutex) == pdTRUE)
        return uStatusOk;
    else
        return uStatusError;
}

/** @brief  Checks the given pin and port sampleCount times. If read sampleCount times the desired state returns 1 otherwise 0.
 *
 *  @param  port - The input port.
 *  @param  pin - The input pin
 *  @param  mode - To determine we are waiting for low or high voltage signal.
 *  @param  sampleDelay - The measuring time period given in miliseconds.
 *  @param  sampleCount -  How many times it measures
 *  @return 1 - if all measured values equal the desired one.
 *          2 - if at least one value doesn't match with the desired one.
 */
static inline uint8_t readPin(GPIO_TypeDef *port,
                                    uint16_t pin,
                                    Urabros_DriverSensorMode mode,
                                    uint32_t sampleDelay,
                                    uint8_t sampleCount)
{
    GPIO_PinState desiredState;
    uint8_t sensorCounter = 0;

    // Set desired sensor state.
    if(mode == uSensorMode_WaitHigh) {
        desiredState = GPIO_PIN_SET;
    } else if(mode == uSensorMode_WaitLow) {
        desiredState = GPIO_PIN_RESET;
    }

    for(uint8_t i = 0; i < sampleCount; i++) {
        // Get sensor value.
        if(HAL_GPIO_ReadPin(port, pin) == desiredState) {
            sensorCounter++;
        }
        osDelay(sampleDelay);
    }

    if(sensorCounter == sampleCount) {
        return 1;
    } else {
        return 0;
    }
}



/** @brief  This function is very usefull when the machine should be in a waiting state for an external signal.
 * 
 *      Forexample we are waiting for a light sensors trigger. Calling this function it will wait for the given signal for maximum #timeout miliseconds.
 *      The sample rate can be given with 1 milisecond precision. 
 *      Example settings:
 *          mode =  uSensorMode_WaitHigh    // We will be waiting for high level voltage.
 *          sampleDelay = 10                // It will make measurements in every 10ms --> 100 Hz sampling
 *          samplesLimit = 30               // We will be waitng for continous high level voltage for 30 periods meaning: 30 * 10 = 300ms = 0.3second
 *          timeout = 1956                  // The function will break the waiting loop when 1956 miliseconds passed, if good samples didn't reach the limit it will return timeout than.
 *  @param  port - The input port.
 *  @param  pin - The input pin
 *  @param  mode - To determine we are waiting for low or high voltage signal.
 *  @param  sampleDelay - The measuring time period given in miliseconds. 
 *  @param  sampleLimit -  How many samples needed to be measured.
 *  @param  timeout - The maximum time to be in waiting state, given in miliseconds.
 *  @return #uStatusOk - if it reached the desired amount of samples in time.
 *          #uStatusTimeOut - if could not collect enough samples in time.
 */
static inline Urabros_StatusTypeDef uDriverWaitUntil(GPIO_TypeDef *port,
                                                    uint16_t pin,
                                                    Urabros_DriverSensorMode mode,
                                                    uint32_t sampleDelay,
                                                    uint16_t samplesLimit,
                                                    uint32_t timeout)
{
    uint32_t startingTick =  HAL_GetTick();

    uint16_t sensorCounter = 0;
    GPIO_PinState desiredState;

    // Set desired sensor state.
    if(mode == uSensorMode_WaitHigh) {
        desiredState = GPIO_PIN_SET;
    } else if(mode == uSensorMode_WaitLow) {
        desiredState = GPIO_PIN_RESET;
    }

    while(1)
    {
        // Get sensor value.
        if(HAL_GPIO_ReadPin(port, pin) == desiredState) {
            sensorCounter++;
        } else {
            sensorCounter = 0;
        }

        // Check if reached the sample rate.
        if(sensorCounter >= samplesLimit) {
            return uStatusOk;
        }

        // Check if timeout happened.
        if((HAL_GetTick() - startingTick) > timeout) {
            return uStatusTimeOut;
        }

        osDelay(sampleDelay);
    }

    return uStatusOk;
}

#endif /* DRIVERS_URABROSDRIVER_H_ */
