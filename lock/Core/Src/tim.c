/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.c
  * @brief   This file provides code for the configuration
  *          of the TIM instances.
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
#include "tim.h"

/* USER CODE BEGIN 0 */
#include "common.h"
#include "gpio.h"
/* USER CODE END 0 */

TIM_HandleTypeDef htim14;

/* TIM14 init function */
void MX_TIM14_Init(void)
{

  /* USER CODE BEGIN TIM14_Init 0 */

  /* USER CODE END TIM14_Init 0 */

  /* USER CODE BEGIN TIM14_Init 1 */

  /* USER CODE END TIM14_Init 1 */
  htim14.Instance = TIM14;
  htim14.Init.Prescaler = 1000;
  htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim14.Init.Period = 1201;
  htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim14) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM14_Init 2 */
  HAL_TIM_Base_Start_IT(&htim14);
  /* USER CODE END TIM14_Init 2 */

}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM14)
  {
  /* USER CODE BEGIN TIM14_MspInit 0 */

  /* USER CODE END TIM14_MspInit 0 */
    /* TIM14 clock enable */
    __HAL_RCC_TIM14_CLK_ENABLE();

    /* TIM14 interrupt Init */
    HAL_NVIC_SetPriority(TIM14_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(TIM14_IRQn);
  /* USER CODE BEGIN TIM14_MspInit 1 */

  /* USER CODE END TIM14_MspInit 1 */
  }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM14)
  {
  /* USER CODE BEGIN TIM14_MspDeInit 0 */

  /* USER CODE END TIM14_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM14_CLK_DISABLE();

    /* TIM14 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM14_IRQn);
  /* USER CODE BEGIN TIM14_MspDeInit 1 */

  /* USER CODE END TIM14_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
extern void appSetLedState(uint8_t led, uint8_t state);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM14){
			/* 100ms */
      if(lock.cmdControl.reportStatus.sendCmdDelay > 0) lock.cmdControl.reportStatus.sendCmdDelay --;

      if(lock.cmdControl.operateResult.sendCmdDelay > 0) lock.cmdControl.operateResult.sendCmdDelay --;

      if(lock.cmdControl.basicSetting.sendCmdDelay > 0) lock.cmdControl.basicSetting.sendCmdDelay --;

      if(lock.cmdControl.ledFlashSetting.sendCmdDelay > 0) lock.cmdControl.ledFlashSetting.sendCmdDelay --;

      if(lock.cmdControl.alarmSetting.sendCmdDelay > 0) lock.cmdControl.alarmSetting.sendCmdDelay --;

      if(lock.cmdControl.reportOperateStatus.sendCmdDelay > 0) lock.cmdControl.reportOperateStatus.sendCmdDelay --;

      if(lock.cmdControl.reportAutoLockAlarm.sendCmdDelay > 0) lock.cmdControl.reportAutoLockAlarm.sendCmdDelay --;

      if(lock.cmdControl.reportLockFaultAlarm.sendCmdDelay > 0) lock.cmdControl.reportLockFaultAlarm.sendCmdDelay --;

      if(lock.cmdControl.reportCheckSensorLockAlarm.sendCmdDelay > 0) lock.cmdControl.reportCheckSensorLockAlarm.sendCmdDelay --;

      if(lock.sensorLockCnt > 0)  lock.sensorLockCnt --;

      if(lock.HoldOnDetectEnable){
          lock.HoldOnLatencyCnt ++;
          if(lock.HoldOnLatencyCnt >= (lock.lockDelay * DELAY_BASE)){
              lock.HoldOnDetectEnable = 0;
              lock.HoldOnLatencyCnt = 0;
              if(lock.lockState == LOCK_STATE_UNLOCK){
                lock.lockTaskState = LOCK_TASK_STATE_BACKWARD;//lock device
                lock.autoLockEnable = 1;
              }
          }
      }

      if(lock.ledTask.state == LED_TASK_STATE_FLASH){
          lock.ledTask.flashCnt ++;
          if(FLASH_FREQ <= lock.ledTask.flashCnt){
              lock.ledTask.flashCnt = 0;
              lock.ledTask.flashOn = !lock.ledTask.flashOn;
          }
      }else{
        lock.ledTask.flashOn = 0;
        lock.ledTask.flashCnt = 0;
      }

      if(lock.faultControl.faultDectEnable && lock.faultControl.faultDectLatency > 0){
          lock.faultControl.faultDectLatency --;
          if(lock.faultControl.faultDectLatency == 0){
            /* operate fail */
            lock.faultControl.faultDectEnable = 0;
            appSetMotorState(MOTOR_STOP);
            lock.lockTaskState = LOCK_TASK_STATE_IDLE;
            lock.autoLockEnable = 0;
            lock.cmdControl.reportLockFaultAlarm.sendCmdEnable = CMD_ENABLE;
            lock.cmdControl.reportLockFaultAlarm.sendCmdDelay = 0;
          }
      }
			
    }
}
/* USER CODE END 1 */
