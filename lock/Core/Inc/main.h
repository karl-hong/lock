/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define GreenLed_Pin GPIO_PIN_0
#define GreenLed_GPIO_Port GPIOB
#define RedLed_Pin GPIO_PIN_1
#define RedLed_GPIO_Port GPIOB
#define MotorB_Pin GPIO_PIN_3
#define MotorB_GPIO_Port GPIOB
#define MotorA_Pin GPIO_PIN_4
#define MotorA_GPIO_Port GPIOB
#define LockStateDetect1_Pin GPIO_PIN_6
#define LockStateDetect1_GPIO_Port GPIOB
#define LockStateDetect1_EXTI_IRQn EXTI4_15_IRQn
#define LockStateDetect2_Pin GPIO_PIN_7
#define LockStateDetect2_GPIO_Port GPIOB
#define LockStateDetect2_EXTI_IRQn EXTI4_15_IRQn
#define GunStateDetect_Pin GPIO_PIN_8
#define GunStateDetect_GPIO_Port GPIOB
#define GunStateDetect_EXTI_IRQn EXTI4_15_IRQn
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
