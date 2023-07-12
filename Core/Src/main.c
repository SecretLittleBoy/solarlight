/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

#include "adc.h"
#include "gpio.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "OLED12864.h"
#include "codetab.h"
//
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
const char sun[8] = {
    0x91, 0x52, 0x3c, 0xbe,
    0x7d, 0x3c, 0x4a, 0x89};
const char moon[8] = {
    0x60, 0xc0, 0xc0, 0xe0,
    0xf0, 0x78, 0x3f, 0x1e};
uint16_t battary_voltage = 0;    // real-time battary voltage
uint8_t battary_percentage = 0;  //[0,14] real-time battary percentage
uint8_t battary_charging_showing_percentage = 14;
const uint16_t battary_max_voltage = 3299;
const uint16_t battary_min_voltage = 2393;

uint16_t solar_voltage = 0;                    // real-time solar voltage
const uint16_t solar_boundary_voltage = 2000;  // if the voltage is higher than this value, it's daytime, otherwise it's night

uint16_t LED_taget_current = 1000;
uint16_t LED_current = 0;

uint16_t battary_charge_current = 0;  // real-time battary charge current
uint16_t battary_charge_target_current = 1000;
uint16_t battary_charge_current_boundary = 1000;  // if the current is higher than this value, the battary is charging

uint8_t battary_charge_PWM = 0;     // real-time battary charge PWM duty
uint8_t battary_charge_ON_OFF = 0;  // real-time battary charge ON/OFF; 0:OFF, 1:ON

// NOTO:LED_PWM bigger, LED darker
uint8_t LED_PWM = 0;                  // real-time LED PWM duty
uint8_t LED_PWM_low_boundary = 0;     // 范围下限
uint8_t LED_PWM_high_boundary = 100;  // 范围上限
uint8_t LED_ON_OFF = 0;               // real-time LED ON/OFF; 0:OFF, 1:ON
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_ADC1_Init();
    MX_I2C2_Init();
    MX_TIM1_Init();
    MX_USART1_UART_Init();
    MX_TIM2_Init();
    /* USER CODE BEGIN 2 */
    OLED_Init();
    OLED_draw_battary_icon(127 - 16, 0, 0);
    OLED_DisplayTurn(0);
    OLED_ColorTurn(0);

    HAL_TIM_Base_Start_IT(&htim2);
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1) {
        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, 50);
        battary_voltage = HAL_ADC_GetValue(&hadc1);
        if (battary_voltage > battary_max_voltage) {
            battary_percentage = 14;
        } else if (battary_voltage < battary_min_voltage) {
            battary_percentage = 0;
        } else {
            battary_percentage = round((battary_voltage - battary_min_voltage) * 14.0 / (battary_max_voltage - battary_min_voltage));
        }
        if (battary_charge_current <= battary_charge_current_boundary) {
            OLED_draw_battary_icon(127 - 16, 0, battary_percentage);
        }
        printf("battary_voltage:%d\r\n", battary_voltage);

        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, 50);
        solar_voltage = HAL_ADC_GetValue(&hadc1);
        if (solar_voltage > solar_boundary_voltage) {
            Draw_BMP(127 - 16 - 8 - 2, 0, 8, 8, sun);
        } else {
            Draw_BMP(127 - 16 - 8 - 2, 0, 8, 8, moon);
        }
        printf("solar_voltage:%d\r\n", solar_voltage);

        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, 50);
        battary_charge_current = HAL_ADC_GetValue(&hadc1);
        printf("battary_charge_current:%d\r\n", battary_charge_current);
        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, 50);
        LED_current = HAL_ADC_GetValue(&hadc1);
        printf("LED_current:%d\r\n", LED_current);
        my_P6x8Str(64, 2, "LED:", LED_current, "   ");

        if (solar_voltage > solar_boundary_voltage) {  // daytime,LED off
            LED_ON_OFF = 0;
        } else if (battary_voltage < battary_min_voltage) {  // nighttime,but battary lack power ,LED off
            LED_ON_OFF = 0;
        } else {
            LED_ON_OFF = 1;
        }
        if (LED_ON_OFF == 1) {
            if (LED_current > LED_taget_current) {
                if (LED_PWM < LED_PWM_high_boundary) LED_PWM_low_boundary = LED_PWM;
                LED_PWM = (LED_PWM_low_boundary + LED_PWM_high_boundary) / 2;
            } else {
                if (LED_PWM > LED_PWM_low_boundary) LED_PWM_high_boundary = LED_PWM;
                LED_PWM = (LED_PWM_low_boundary + LED_PWM_high_boundary) / 2;
            }
            HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, LED_PWM);
            printf("LED_PWM on :%d\r\n", LED_PWM);
        } else {
            LED_PWM_low_boundary = 0;
            LED_PWM_high_boundary = 100;  // reset
            HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 100);
            printf("LED_PWM on:%d\r\n", 100);
        }

        if (battary_voltage > battary_max_voltage) {  // battary is full,stop charging
            battary_charge_ON_OFF = 0;
        } else {
            battary_charge_ON_OFF = 1;
        }
        if (battary_charge_ON_OFF == 1) {
            battary_charge_PWM = 100;
            HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, battary_charge_PWM);
            printf("battary_charge_PWM on :%d\r\n", battary_charge_PWM);
        } else {
            HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
            printf("battary_charge_PWM off\r\n");
        }

        HAL_Delay(1000);
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
        Error_Handler();
    }
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */
int fputc(int ch, FILE *f) {
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 10);
    return ch;
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {
        if (battary_charge_current > battary_charge_current_boundary) {
            OLED_draw_battary_icon(127 - 16, 0, battary_charging_showing_percentage);
            battary_charging_showing_percentage++;
            if (battary_charging_showing_percentage > 14) {
                battary_charging_showing_percentage = battary_percentage;
            }
        }
    }
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
