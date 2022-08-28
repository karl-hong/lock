/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */
#include <stdio.h>
/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3|GPIO_PIN_4, GPIO_PIN_RESET);

  /*Configure GPIO pins : PB0 PB1 PB3 PB4 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_3|GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB6 PB7 PB8 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

}

/* USER CODE BEGIN 2 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(LockStateDetect1_Pin == GPIO_Pin){
		//Sensor.smoke_sensor_state = HAL_GPIO_ReadPin(GPIOB, SMOKE_DET_Pin);
	}else if(LockStateDetect2_Pin == GPIO_Pin){
		//Sensor.displacement_sensor_state = HAL_GPIO_ReadPin(GPIOB, MOVE_DET_Pin);
	}else if(GunStateDetect_Pin == GPIO_Pin){
		//Sensor.water_sensor_state = HAL_GPIO_ReadPin(GPIOB, WATER_DET_Pin);
	}
}

void appSetLedState(uint8_t led, uint8_t state)
{
	switch(led){
		case GREEN_LED:{
			if(state) HAL_GPIO_WritePin(GPIOB, GreenLed_Pin, GPIO_PIN_SET);
			else			HAL_GPIO_WritePin(GPIOB, GreenLed_Pin, GPIO_PIN_RESET);
			break;
		}
		
		case RED_LED:{
			if(state) HAL_GPIO_WritePin(GPIOB, RedLed_Pin, GPIO_PIN_SET);
			else			HAL_GPIO_WritePin(GPIOB, RedLed_Pin, GPIO_PIN_RESET);
			break;
		}
		
		default:{
			printf("Error: led %d not found!\r\n", led);
			break;
		}
	}
}

void appSetMotorState(uint8_t state)
{
	switch(state){
		case MOTOR_FORWARD:{
			HAL_GPIO_WritePin(GPIOB, MotorA_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOB, MotorB_Pin, GPIO_PIN_SET);
			break;
		}
		
		case MOTOR_BACK:{
			HAL_GPIO_WritePin(GPIOB, MotorA_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOB, MotorB_Pin, GPIO_PIN_RESET);
			break;
		}
		
		case MOTOR_STOP:
		default:{
			HAL_GPIO_WritePin(GPIOB, MotorA_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOB, MotorB_Pin, GPIO_PIN_RESET);
			break;
		}
	}
}
/* USER CODE END 2 */