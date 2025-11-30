/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

extern uint8_t turn_flag;

// 按键（上拉输入：按下=低电平）
#define KEY_PIN GPIO_PIN_2
#define KEY_PORT GPIOA
#define KEY_PRESSED (HAL_GPIO_ReadPin(KEY_PORT, KEY_PIN) == GPIO_PIN_RESET)

// 8路循迹传感器
#define TRACK_PORT GPIOB
#define TRACK1_PIN GPIO_PIN_0
#define TRACK2_PIN GPIO_PIN_1
#define TRACK3_PIN GPIO_PIN_3
#define TRACK4_PIN GPIO_PIN_4
#define TRACK5_PIN GPIO_PIN_5
#define TRACK6_PIN GPIO_PIN_6
#define TRACK7_PIN GPIO_PIN_7
#define TRACK8_PIN GPIO_PIN_8

// 电机方向引脚
#define LEFT_DIR_FWD GPIO_PIN_11
#define LEFT_DIR_REV GPIO_PIN_12
#define LEFT_DIR_PORT GPIOA
#define RIGHT_DIR_FWD GPIO_PIN_10
#define RIGHT_DIR_REV GPIO_PIN_11
#define RIGHT_DIR_PORT GPIOB

// 核心参数
#define ENC_PER_ROUND 531          // 每圈编码器计数
#define WHEEL_DIAM 48.0f           // 轮子直径(mm)，按实际改
#define WHEEL_CIRC (WHEEL_DIAM * 3.1416f / 1000) // 周长(m)
#define TARGET_SPEED f          // 目标速度1.1m/s
#define TARGET_CNT 21 // 10ms目标计数
#define STOP_CNT 0

// PID参数（正常/转弯减速）
#define P_NORM 7.0f
#define I_NORM 2.0f
#define D_NORM 1.0f
#define P_TURN 4.5f
#define I_TURN 1.2f
#define D_TURN 0.6f
#define PWM_MAX 999
#define PWM_MIN 0
#define SUM_ERR_LIM 450 // 积分限幅

// 函数声明（必须包含所有被外部调用的函数）
void Motor_Init(void);
void Encoder_Read(void);
void PID_Calc(uint8_t is_turn);
void Track_Detect(void);
void Key_Process(void);



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

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
