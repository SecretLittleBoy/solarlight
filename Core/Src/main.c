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
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

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
#define SUM_TIMES 100
#define LITTLE_SUM_TIMES 100
struct cycle_queue {
    uint16_t data[SUM_TIMES];
    uint8_t next_store_index;
    uint32_t data_num;
    uint16_t data_average;
    uint8_t data_count;  // data_count<=SUM_TIMES, data_count=SUM_TIMES means the queue is full
};
struct cycle_queue_little_sum_times {
    uint16_t data[LITTLE_SUM_TIMES];
    uint8_t next_store_index;
    uint32_t data_num;
    uint16_t data_average;
    uint8_t data_count;  // data_count<=SUM_TIMES, data_count=SUM_TIMES means the queue is full
};
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t which_key_pressed = 0;  // which key is pressed.0: no key pressed
const char sun[8] = {
    0x91, 0x52, 0x3c, 0xbe,
    0x7d, 0x3c, 0x4a, 0x89};
const char moon[8] = {
    0x60, 0xc0, 0xc0, 0xe0,
    0xf0, 0x78, 0x3f, 0x1e};
uint16_t battary_voltage = 0;                              // real-time battary voltage
struct cycle_queue battary_voltage_queue;                  // battary voltage queue
uint8_t battary_percentage = 0;                            //[0,14] real-time battary percentage
uint8_t battary_charging_showing_percentage = 14;          // for dynamically show charging cartoon
const uint16_t battary_max_voltage = 2700;                // stop charge:8.2V ,8.19V max ADC value <2720<8.2V ADC max value
const uint16_t battary_stop_charge_voltage = 2537;         // stop charge:7.8V ,7.80V ADC min value <2537<7.81V ADC min value
const uint16_t battary_start_power_supply_voltage = 2067;  // start power supply:6.4V ,6.39V ADC max value:2105 <  <6.40V ADC max value:2110
const uint16_t battary_min_voltage = 1990;                 // stop power supply:6V,  6.00 min ADC value:1955 <  <6.01 min ADC value: 1955
// const uint16_t battary_min_voltage_when_LED_OFF = 1955;                // stop power supply:6V,  6.00 min ADC value:1955 <  <6.01 min ADC value: 1955
//  0:7.8, 1:82 battery last voltage is 7.8V or 8.2V
//  if 7.8V, when battary_voltage (7.8,8.2)V,should charge
//  if 8.2V, when battary_voltage (7.8,8.2)V,should NOT charge
uint8_t battary_last_78_or_82 = 0;
// 0:60, 1:64 battery last voltage is 6.0V or 6.4V
// if 6.0V, when battary_voltage (6.0,6.4)V,should NOT power supply
// if 6.4V, when battary_voltage (6.0,6.4)V,should power supply
uint8_t battary_last_60_or_64 = 0;

uint16_t solar_voltage = 0;                    // real-time solar voltage
struct cycle_queue solar_voltage_queue;        // solar voltage queue
const uint16_t solar_boundary_voltage = 1000;  // if the voltage is higher than this value, it's daytime, otherwise it's night

uint16_t LED_taget_current = 1525;  // LED_current get by ADC when 300mA
uint16_t LED_current = 0;

uint16_t battary_charge_current = 0;  // real-time battary charge current
struct cycle_queue_little_sum_times battary_charge_current_queue;
uint16_t battary_charge_target_current = 1500;
const uint16_t battary_charge_100mA_current = 1350;  // 100mA //604
uint16_t battary_charge_current_boundary = 510;     // if the current is higher than this value, the battary is charging

uint8_t battary_charge_PWM = 0;                         // real-time battary charge PWM duty
uint8_t battary_charge_ON_OFF = 0;                      // real-time battary charge ON/OFF; 0:OFF, 1:ON
uint8_t const_voltage_limited_current_charge_mode = 0;  // 0:off, 1:on

// NOTO:LED_PWM bigger, LED darker
uint8_t LED_PWM = 100;   // real-time LED PWM duty
uint8_t LED_ON_OFF = 0;  // real-time LED ON/OFF; 0:OFF, 1:ON
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void ADC_get_average_value(void);
void cycle_queue_add_data(struct cycle_queue *queue, uint16_t data);
void cycle_queue_little_sum_times_add_data(struct cycle_queue_little_sum_times *queue, uint16_t data);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
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
        ADC_get_average_value();

        if (solar_voltage > solar_boundary_voltage) {  // daytime,LED off
            LED_ON_OFF = 0;
        } else if (battary_voltage < battary_min_voltage) {  // nighttime,but battary lack power ,LED off
            LED_ON_OFF = 0;
            battary_last_60_or_64 = 0;
        } else if (battary_voltage > battary_start_power_supply_voltage) {
            LED_ON_OFF = 1;
            battary_last_60_or_64 = 1;
        } else {
            LED_ON_OFF = battary_last_60_or_64;
        }
        if (LED_ON_OFF == 1) {  // LED on
            if (LED_current - LED_taget_current > 500) {
                LED_PWM += 5;
                if (LED_PWM > 100) {
                    LED_PWM = 100;
                }
            } else if (LED_current > LED_taget_current) {
                LED_PWM += 1;
                if (LED_PWM > 100) {
                    LED_PWM = 100;
                }
            } else if (LED_taget_current - LED_current > 500) {
                if (LED_PWM > 5) {
                    LED_PWM -= 5;
                } else {
                    LED_PWM = 0;
                }
            } else {
                if (LED_PWM > 1) {
                    LED_PWM -= 1;
                } else {
                    LED_PWM = 0;
                }
            }
            HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, LED_PWM);
            printf("LED_PWM on :%d\r\n", LED_PWM);
        } else {  // LED off
            HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 100);
            printf("LED_PWM on:%d\r\n", 100);
        }

        if (battary_voltage >= battary_max_voltage) {  // battary is full, go into const_voltage_limited_current_charge_mode
            const_voltage_limited_current_charge_mode = 1;
            battary_last_78_or_82 = 1;
        } else if (battary_voltage < battary_stop_charge_voltage) {
            battary_charge_ON_OFF = 1;  // on ,should charge
            battary_last_78_or_82 = 0;
        } else {  // battary_voltage: (7.8,8.2]
            if (!const_voltage_limited_current_charge_mode) {
                battary_charge_ON_OFF = !battary_last_78_or_82;
            }
        }
        printf("charge ON OFF%d,charge_mode:%d\r\n", battary_charge_ON_OFF, const_voltage_limited_current_charge_mode);
        if (battary_charge_ON_OFF == 1 && const_voltage_limited_current_charge_mode == 0) {
            battary_charge_PWM = 100;
            HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, battary_charge_PWM);
            printf("battary_charge_PWM on :%d\r\n", battary_charge_PWM);
        } else if (battary_charge_ON_OFF == 1 && const_voltage_limited_current_charge_mode == 1) {
            if (battary_charge_current < battary_charge_100mA_current) {
                const_voltage_limited_current_charge_mode = 0;
                battary_charge_ON_OFF = 0;
            } else if (battary_voltage > battary_max_voltage) {
                if (battary_charge_PWM > 1) {
                    battary_charge_PWM--;
                } else {
                    battary_charge_PWM = 0;
                }
            } else {
                battary_charge_PWM++;
                if (battary_charge_PWM > 100) {
                    battary_charge_PWM = 100;
                }
            }
            if (battary_charge_ON_OFF) {
                HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, battary_charge_PWM);
                printf("battary_charge_PWM on :%d\r\n", battary_charge_PWM);
            } else {
                HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
                printf("line:249:battary_charge_PWM off\r\n");
            }
        } else {
            HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
            printf("line:253:battary_charge_PWM off\r\n");
        }

        printf("\r\n");

        which_key_pressed = KEY_Scan(0);
        switch (which_key_pressed) {
            case KEY0_PRES: {
                printf("KEY0_PRES\r\n");
                if (LED_taget_current > 200) {
                    LED_taget_current -= 200;
                }
                break;
            }
            case KEY1_PRES: {
                printf("KEY1_PRES\r\n");
                break;
            }
            case KEY2_PRES: {
                printf("KEY2_PRES\r\n");
                break;
            }
            case KEY3_PRES: {
                printf("KEY3_PRES\r\n");
                if (LED_taget_current < 3900) {
                    LED_taget_current += 200;
                }
                break;
            }
            default:
                break;
        }
        // HAL_Delay(1);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
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
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
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
            OLED_P6x8Str(32,1,"charging");
            OLED_draw_battary_icon(127 - 16, 0, battary_charging_showing_percentage);
            battary_charging_showing_percentage++;
            if (battary_charging_showing_percentage > 14) {
                battary_charging_showing_percentage = battary_percentage;
            }
        }
    }
}

void ADC_get_average_value(void) {
    //OLED_Clear();
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 50);
    if (const_voltage_limited_current_charge_mode) {
        battary_voltage = HAL_ADC_GetValue(&hadc1);
    } else {
        cycle_queue_add_data(&battary_voltage_queue, HAL_ADC_GetValue(&hadc1));
        battary_voltage = battary_voltage_queue.data_average;
    }

    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 50);
    cycle_queue_add_data(&solar_voltage_queue, HAL_ADC_GetValue(&hadc1));
    solar_voltage = solar_voltage_queue.data_average;

    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 50);
    if (const_voltage_limited_current_charge_mode) {
        cycle_queue_little_sum_times_add_data(&battary_charge_current_queue, HAL_ADC_GetValue(&hadc1));
        battary_charge_current = battary_charge_current_queue.data_average;
    } else {
        battary_charge_current = HAL_ADC_GetValue(&hadc1);
    }

    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 50);
    LED_current = HAL_ADC_GetValue(&hadc1);

    if (battary_voltage > battary_max_voltage) {
        battary_percentage = 14;
    } else if (battary_voltage < battary_min_voltage) {
        battary_percentage = 0;
    } else {
        battary_percentage = round((battary_voltage - battary_min_voltage) * 14.0 / (battary_max_voltage - battary_min_voltage));
    }
    if (battary_charge_current <= battary_charge_current_boundary) {
        OLED_P6x8Str(32,1,"          ");
        OLED_draw_battary_icon(127 - 16, 0, battary_percentage);
    }
    printf("battary_voltage:%d\r\n", battary_voltage);

    if (solar_voltage > solar_boundary_voltage) {
        Draw_BMP(127 - 16 - 8 - 2, 0, 8, 8, sun);
    } else {
        Draw_BMP(127 - 16 - 8 - 2, 0, 8, 8, moon);
    }
    printf("solar_voltage:%d\r\n", solar_voltage);
    printf("battary_charge_current:%d\r\n", battary_charge_current);
    printf("LED_current:%d\r\n", LED_current);
    my_P6x8Str(0,2,"LED:",LED_current,"     ");
}

void cycle_queue_add_data(struct cycle_queue *queue, uint16_t data) {
    // Subtract the oldest value from the data_num
    queue->data_num -= queue->data[queue->next_store_index];

    // Add data to the queue
    queue->data[queue->next_store_index] = data;

    // Add the new value to the data_num
    queue->data_num += data;

    // Update the next store index
    queue->next_store_index = (queue->next_store_index + 1) % SUM_TIMES;

    // Update the data count
    if (queue->data_count < SUM_TIMES) {
        // Increment data count if the queue is not yet full
        queue->data_count++;
    }
    // Calculate the new data average
    queue->data_average = queue->data_num / queue->data_count;
}

void cycle_queue_little_sum_times_add_data(struct cycle_queue_little_sum_times *queue, uint16_t data) {
    // Subtract the oldest value from the data_num
    queue->data_num -= queue->data[queue->next_store_index];

    // Add data to the queue
    queue->data[queue->next_store_index] = data;

    // Add the new value to the data_num
    queue->data_num += data;

    // Update the next store index
    queue->next_store_index = (queue->next_store_index + 1) % LITTLE_SUM_TIMES;

    // Update the data count
    if (queue->data_count < LITTLE_SUM_TIMES) {
        // Increment data count if the queue is not yet full
        queue->data_count++;
    }
    // Calculate the new data average
    queue->data_average = queue->data_num / queue->data_count;
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {
    }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
