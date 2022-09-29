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
#include "common.h"
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
  HAL_GPIO_WritePin(GPIOB, GreenLed_Pin|RedLed_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, MotorB_Pin|MotorA_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PBPin PBPin PBPin PBPin */
  GPIO_InitStruct.Pin = GreenLed_Pin|RedLed_Pin|MotorB_Pin|MotorA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PBPin PBPin PBPin */
  GPIO_InitStruct.Pin = LockStateDetect1_Pin|LockStateDetect2_Pin|GunStateDetect_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

}

/* USER CODE BEGIN 2 */
void lock_stop_detect(void)
{
	static uint8_t lastState = 0;
	uint8_t stateChange = 0;
	if(lock.lockDetectState1  && !lock.lockDetectState2){
		lock.lockState = LOCK_STATE_UNLOCK;//unlock state

	}else if(!lock.lockDetectState1 && lock.lockDetectState2){
		lock.lockState = LOCK_STATE_LOCK;//lock state
	}
	
	if(lastState != lock.lockState){
		lastState = lock.lockState;
		stateChange = 1;
	}

	if(LOCK_TASK_STATE_BACKWARD == lock.lockTaskState && lock.lockState){
		/* totally lock and stop motor */
		appSetMotorState(MOTOR_STOP);
		lock.lockTaskState = LOCK_TASK_STATE_IDLE;
		if(lock.autoLockEnable && lock.isReport){
			lock.cmdControl.reportAutoLockAlarm.sendCmdEnable = 1;
			lock.cmdControl.reportAutoLockAlarm.sendCmdDelay = 0;
		}
		lock.autoLockEnable = 0;
		lock.faultControl.faultDectEnable = 0;
	}else if(LOCK_TASK_STATE_FORWARD == lock.lockTaskState && !lock.lockState){
		/* totally unlock and stop motor */
		appSetMotorState(MOTOR_STOP);
		lock.lockTaskState = LOCK_TASK_STATE_IDLE;
		lock.autoLockEnable = 0;
		lock.faultControl.faultDectEnable = 0;
	}else if(LOCK_TASK_STATE_IDLE == lock.lockTaskState && stateChange){
		if(lock.isReport){
			lock.cmdControl.reportOperateStatus.sendCmdEnable = 1;
			lock.cmdControl.reportOperateStatus.sendCmdDelay = 0;
		}

		lock.autoLockEnable = 0;
		
		/* manual operate alarm */
		if(lock.lockState == LOCK_STATE_LOCK){
			lock.alarmStatus = LOCK_ALARM_LOCK;
		}else{
			lock.alarmStatus = LOCK_ALARM_UNLOCK;
		}
		/* save database */
		user_database_save();
	}
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(LockStateDetect1_Pin == GPIO_Pin){
		lock.lockDetectState1 = HAL_GPIO_ReadPin(GPIOB, LockStateDetect1_Pin);
		lock_stop_detect();
	}else if(LockStateDetect2_Pin == GPIO_Pin){
		lock.lockDetectState2 = HAL_GPIO_ReadPin(GPIOB, LockStateDetect2_Pin);
		lock_stop_detect();
	}else if(GunStateDetect_Pin == GPIO_Pin){
		lock.gunState = HAL_GPIO_ReadPin(GPIOB, GunStateDetect_Pin);
		static uint8_t lastGunState = 0;
		if(lastGunState != lock.gunState){
			lastGunState = lock.gunState;
			lock.ledTask.state = LED_TASK_STATE_IDLE;
		}
	}
}

void appSetLedState(uint8_t led, uint8_t state)
{
	switch(led){
		case GREEN_LED:{
			if(state) HAL_GPIO_WritePin(GPIOB, GreenLed_Pin, GPIO_PIN_SET);
			else	  HAL_GPIO_WritePin(GPIOB, GreenLed_Pin, GPIO_PIN_RESET);
			break;
		}
		
		case RED_LED:{
			if(state) HAL_GPIO_WritePin(GPIOB, RedLed_Pin, GPIO_PIN_SET);
			else	  HAL_GPIO_WritePin(GPIOB, RedLed_Pin, GPIO_PIN_RESET);
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

void MotorTask(void)
{
	static uint8_t oldState = LOCK_TASK_STATE_IDLE;
	switch(lock.lockTaskState){
		case LOCK_TASK_STATE_FORWARD:{
			if(oldState != LOCK_TASK_STATE_FORWARD){
				oldState = LOCK_TASK_STATE_FORWARD;
				appSetMotorState(MOTOR_FORWARD);
				lock.faultControl.faultState = LOCK_STATE_UNLOCK;
				lock.faultControl.faultDectEnable = 1;
				lock.faultControl.faultDectLatency = FAULT_DECT_TIME * DELAY_BASE;
			}
			break;
		}

		case LOCK_TASK_STATE_BACKWARD:{
			if(oldState != LOCK_TASK_STATE_BACKWARD){
				oldState = LOCK_TASK_STATE_BACKWARD;
				appSetMotorState(MOTOR_BACK);
				lock.faultControl.faultState = LOCK_STATE_LOCK;
				lock.faultControl.faultDectEnable = 1;
				lock.faultControl.faultDectLatency = FAULT_DECT_TIME * DELAY_BASE;
			}
			break;
		}

		case LOCK_TASK_STATE_STOP:{
			if(oldState != LOCK_TASK_STATE_STOP){
				oldState = LOCK_TASK_STATE_STOP;
				appSetMotorState(MOTOR_STOP);
			}
			break;
		}

		case LOCK_TASK_STATE_IDLE:
		default:
			oldState = LOCK_TASK_STATE_IDLE;
			break;
	}
}

void Led_Task(void)
{
	switch(lock.ledTask.state){
		case LED_TASK_STATE_FLASH:{
			if(lock.ledFlashStatus){
				if(lock.gunState){
					if(lock.ledTask.flashOn)	appSetLedState(GREEN_LED, LED_ON);
					else						appSetLedState(GREEN_LED, LED_OFF);
					appSetLedState(RED_LED, LED_OFF);
				}else{
					appSetLedState(GREEN_LED, LED_OFF);
					if(lock.ledTask.flashOn)	appSetLedState(RED_LED, LED_ON);
					else						appSetLedState(RED_LED, LED_OFF);
				}
			}else{
				if(lock.gunState){
					appSetLedState(GREEN_LED, LED_ON);
					appSetLedState(RED_LED, LED_OFF);
				}else{
					appSetLedState(GREEN_LED, LED_OFF);
					appSetLedState(RED_LED, LED_ON);
				}
			}
			break;
		}

		case LED_TASK_STATE_IDLE:{
			if(lock.gunState){
				appSetLedState(GREEN_LED, LED_ON);
				appSetLedState(RED_LED, LED_OFF);
			}else{
				appSetLedState(GREEN_LED, LED_OFF);
				appSetLedState(RED_LED, LED_ON);
			}
			break;
		}

		default:{
			break;
		}
	}
}

void Auto_Lock_Task(void)
{
	if(lock.lockDetectState1  && !lock.lockDetectState2){
		if(lock.HoldOnDetectEnable == 0){
			lock.HoldOnDetectEnable = 1;
			lock.HoldOnLatencyCnt = 0;
		}
	}else if(!lock.lockDetectState1 && lock.lockDetectState2){
		lock.HoldOnDetectEnable = 0;
		lock.HoldOnLatencyCnt = 0;
	}else{
		if(lock.HoldOnDetectEnable == 0){
			lock.HoldOnDetectEnable = 1;
			lock.HoldOnLatencyCnt = 0;
		}
	}
}

void lock_state_init(void)
{
	lock.lockDetectState1 = HAL_GPIO_ReadPin(GPIOB, LockStateDetect1_Pin);
	lock.lockDetectState2 = HAL_GPIO_ReadPin(GPIOB, LockStateDetect2_Pin);
	lock.gunState = HAL_GPIO_ReadPin(GPIOB, GunStateDetect_Pin);
	lock_stop_detect();
}
/* USER CODE END 2 */
