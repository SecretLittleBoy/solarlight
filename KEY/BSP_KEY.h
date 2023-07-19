#ifndef _BSP_KEY_H
#define _BSP_KEY_H


#include "stm32f1xx_hal.h"

#define KEY0    HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4)  // PC8
#define KEY1    HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5)  // PC9
#define KEY2    HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6)  // PD2
#define KEY3   HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_7)  // PA0

#define KEY0_PRES   1 // KEY0
#define KEY1_PRES   2 // KEY1
#define KEY2_PRES   3 // KEY2
#define KEY3_PRES   4 // KEY3

uint8_t KEY_Scan(uint8_t mode);

#endif
