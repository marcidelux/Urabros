/*
 * LedDriver.h
 *
 *  Created on: Feb 8, 2021
 *      Author: NapoLion
 */

#ifndef DRIVERS_LED_LEDDRIVER_H_
#define DRIVERS_LED_LEDDRIVER_H_

#include "UrabrosTypeDef.h"

extern Urabros_DriverPtrTypeDef ledDriverPtr;

#define LED_MUTEX_TIMEOUT (TickType_t)100

typedef enum {
    LED_1 = 0,
    LED_2 = 1,
    LED_3 = 2,
}ledDriver_LedIndexTypedef;

void ledDriver_Init(void);
Urabros_StatusTypeDef ledDriver_ON(ledDriver_LedIndexTypedef ledIdx);
Urabros_StatusTypeDef ledDriver_OFF(ledDriver_LedIndexTypedef ledIdx);
Urabros_StatusTypeDef ledDriver_TOGGLE(ledDriver_LedIndexTypedef ledIdx);

#endif /* DRIVERS_LED_LEDDRIVER_H_ */
