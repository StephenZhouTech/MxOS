#ifndef __LED_H__
#define __LED_H__

#include "stm32f4xx_conf.h"
#include <stdint.h>

#define LED_2       GPIO_Pin_8
#define LED_3       GPIO_Pin_9
#define LED_4       GPIO_Pin_10

void BspLedInit(void);

#define BspLEDOn(led)   GPIO_ResetBits(GPIOF, led);

#define BspLEDOff(led)  GPIO_SetBits(GPIOF, led);

#endif // __LED_H__
