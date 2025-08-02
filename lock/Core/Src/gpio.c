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
static uint32_t s_MotorRunningCnt = 0;
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
			//lock.ledTask.state = LED_TASK_STATE_IDLE;
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
	static uint8_t auto_lock_state = 0;
	static uint8_t stLastGunState = 0xff;
	if(!lock.autoLockFlag){
		/* forbid auto lock */
		lock.HoldOnDetectEnable = 0;
		lock.HoldOnLatencyCnt = 0;
		goto auto_lock_by_gun_state;
	}

auto_lock_by_time:
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
auto_lock_by_gun_state:
	switch(auto_lock_state){
		case 1:{
			/* 等待延时时间 */
			if(lock.lockState != LOCK_STATE_UNLOCK || !lock.gunState){
				auto_lock_state = 0;
				break;
			}
			/* wait for delay time */
			if(lock.sensorLockCnt) break;

            lock.lockTaskState = LOCK_TASK_STATE_BACKWARD;//lock device

			lock.cmdControl.reportCheckSensorLockAlarm.sendCmdEnable = CMD_ENABLE;
            lock.cmdControl.reportCheckSensorLockAlarm.sendCmdDelay = 0;

			auto_lock_state = 0;
            
			break;
		}

		case 0:
		default:{
			if(stLastGunState == 0xff){
				stLastGunState = lock.gunState;//init value
			}else if(stLastGunState != lock.gunState){
					 /* state change */
					stLastGunState = lock.gunState;//change last state
					if(lock.gunState && lock.lockState == LOCK_STATE_UNLOCK){
						/* 枪在位 */
						auto_lock_state = 1;
						lock.sensorLockCnt = lock.sensorLockDelay * DELAY_BASE;
					}else{
						lock.sensorLockCnt = 0;
					}
			}			
			break;
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

void checkGunStateTask(void)
{
	static int lastGunState = -1;
	static int count = 0;

	if(lastGunState == -1){
		lastGunState = lock.gunState;
		return;
	}

	if(lastGunState != lock.gunState){
		count ++;
		if(count >= 1){
			count = 0;
			lastGunState = lock.gunState;
			lock.cmdControl.reportGunState.sendCmdEnable = CMD_ENABLE;
            lock.cmdControl.reportGunState.sendCmdDelay = 0;
		}
	}else{
		count = 0;
	}
}


/**s
 * 
 * 限制开锁时间
 * 1、下开锁指令后，等待150ms，停止马达转动
 * 2、马达停止转动后，等待250ms（400ms），霍尔检测到位。则停止。如果定时器检测到霍尔到位也会停止。
 * 如果250ms后霍尔没有到位，则继续转动马达。
 * 3、继续转动马达50ms后，停止转动。
 * 
 * 
 */
void lock_stop_wait_done(void)
{
	if(lock.lockTaskState == LOCK_TASK_STATE_FORWARD || lock.lockTaskState == LOCK_TASK_STATE_BACKWARD){
		uint32_t curTick = HAL_GetTick();
		if(s_MotorRunningCnt == 0){
			s_MotorRunningCnt = curTick;
		}

		if(curTick >= s_MotorRunningCnt + 450){// 400ms
			appSetMotorState(MOTOR_STOP);
		}else if(curTick >= s_MotorRunningCnt + 400){
			appSetMotorState(lock.lockTaskState == LOCK_TASK_STATE_FORWARD ? MOTOR_FORWARD : MOTOR_BACK);
		}else if(curTick >= s_MotorRunningCnt + 150){
			appSetMotorState(MOTOR_STOP);
		}
	}else{
		s_MotorRunningCnt = 0;
	}
}
/* USER CODE END 2 */
