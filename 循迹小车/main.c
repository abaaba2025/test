/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
int16_t left_enc = 0, right_enc = 0;       // 编码器10ms计数
float left_pwm = 0, right_pwm = 0;         // PID输出PWM
float left_err = 0, left_last_err = 0, left_sum_err = 0;
float right_err = 0, right_last_err = 0, right_sum_err = 0;
uint8_t speed_mode = 0;                   // 0=停(0m/s)，1=行(0.5m/s)
uint8_t turn_flag = 0;                    // 0=直走，1=左转，2=右转

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
// 全局变量（确保在中断中可访问）
uint8_t track_dat = 0x00;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// 电机初始化
void Motor_Init(void) {
    // 方向脚初始低电平
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11|GPIO_PIN_12, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10|GPIO_PIN_11, GPIO_PIN_RESET);
    // 启动外设
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
}

// 读取编码器（10ms清零）
void Encoder_Read(void) {
    left_enc = (int16_t)__HAL_TIM_GET_COUNTER(&htim2);
    right_enc = (int16_t)__HAL_TIM_GET_COUNTER(&htim3);
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    __HAL_TIM_SET_COUNTER(&htim3, 0);
}

// PID计算（转弯切换减速参数）
void PID_Calc(uint8_t is_turn) {
    float p = P_NORM, i = I_NORM, d = D_NORM;
    int16_t target = speed_mode == 0 ? (int16_t)TARGET_CNT : STOP_CNT;


    if (is_turn != 0) { // 转弯时切换减速参数
        p = P_TURN;
        i = I_TURN;
        d = D_TURN;
    }

    // 左PID计算
    left_err = target - left_enc;
    // 积分限幅
    if (left_sum_err + left_err > SUM_ERR_LIM) {
        left_sum_err = SUM_ERR_LIM;
    } else if (left_sum_err + left_err < -SUM_ERR_LIM) {
        left_sum_err = -SUM_ERR_LIM;
    } else {
        left_sum_err += left_err;
    }
    left_pwm = p * left_err + i * left_sum_err + d * (left_err - left_last_err);
    left_last_err = left_err;
    // PWM限幅
    left_pwm = (left_pwm > PWM_MAX) ? PWM_MAX : (left_pwm < PWM_MIN) ? PWM_MIN : left_pwm;

    // 右PID计算
    right_err = target - right_enc;
    // 积分限幅
    if (right_sum_err + right_err > SUM_ERR_LIM) {
        right_sum_err = SUM_ERR_LIM;
    } else if (right_sum_err + right_err < -SUM_ERR_LIM) {
        right_sum_err = -SUM_ERR_LIM;
    } else {
        right_sum_err += right_err;
    }
    right_pwm = p * right_err + i * right_sum_err + d * (right_err - right_last_err);
    right_last_err = right_err;
    // PWM限幅
    right_pwm = (right_pwm > PWM_MAX) ? PWM_MAX : (right_pwm < PWM_MIN) ? PWM_MIN : right_pwm;

    // 转向调节（适配弧形/直角）
    if (speed_mode == 0) {
    	if (is_turn == 0){
    		left_pwm *= 0.45;
    		right_pwm *= 0.45;
    	}else if (is_turn == 1) { // 右转：左快右慢
             left_pwm *= 0.7;
            right_pwm *= 0.15;
        } else if (is_turn == 2) { // 左转：左慢右快
           left_pwm *= 0.15;
            right_pwm *= 0.7;
        }else if( is_turn == 3){
        	left_pwm = 0;
        	right_pwm *= 0.7;
        }else if (is_turn == 4){
        	left_pwm *= 0.7;
        	right_pwm = 0;
        }
    }

    // 电机正转控制（前进循迹）
    HAL_GPIO_WritePin(LEFT_DIR_PORT, LEFT_DIR_FWD, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LEFT_DIR_PORT, LEFT_DIR_REV, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RIGHT_DIR_PORT, RIGHT_DIR_FWD, GPIO_PIN_SET);
    HAL_GPIO_WritePin(RIGHT_DIR_PORT, RIGHT_DIR_REV, GPIO_PIN_RESET);
    // PWM输出
//    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, (uint16_t)left_pwm*10);
//    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, (uint16_t)right_pwm*10);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, (uint16_t)left_pwm);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, (uint16_t)right_pwm);
}

// 8路循迹检测（修正语法错误）
void Track_Detect(void) {
    track_dat = 0x00;
//    uint8_t cnt = 0;
//    for(uint8_t i = 0;i < 5; i++){
//    // 读取8路循迹数据（高电平=黑线）
//    	if(HAL_GPIO_ReadPin(TRACK_PORT, TRACK1_PIN) == GPIO_PIN_SET){
//    		track_dat |= 0x01;
//    		cnt ++;
//    		HAL_Delay(10);
//    	}return cnt >= 3? 1:0;
//
//    	if(HAL_GPIO_ReadPin(TRACK_PORT, TRACK2_PIN) == GPIO_PIN_SET){
//        	track_dat |= 0x02;
//        	cnt ++;
//        	HAL_Delay(10);
//        }return cnt >= 3? 1:0;
//
//        if(HAL_GPIO_ReadPin(TRACK_PORT, TRACK3_PIN) == GPIO_PIN_SET){
//            track_dat |= 0x04;
//            cnt ++;
//            HAL_Delay(10);
//        }return cnt >= 3? 1:0;
//
//        if(HAL_GPIO_ReadPin(TRACK_PORT, TRACK4_PIN) == GPIO_PIN_SET){
//            track_dat |= 0x08;
//            cnt ++;
//            HAL_Delay(10);
//        }return cnt >= 3? 1:0;
//
//        if(HAL_GPIO_ReadPin(TRACK_PORT, TRACK5_PIN) == GPIO_PIN_SET){
//            track_dat |= 0x10;
//            cnt ++;
//            HAL_Delay(10);
//        }return cnt >= 3? 1:0;
//
//        if(HAL_GPIO_ReadPin(TRACK_PORT, TRACK6_PIN) == GPIO_PIN_SET){
//            track_dat |= 0x20;
//            cnt ++;
//            HAL_Delay(10);
//        }return cnt >= 3? 1:0;
//
//        if(HAL_GPIO_ReadPin(TRACK_PORT, TRACK7_PIN) == GPIO_PIN_SET){
//            track_dat |= 0x40;
//            cnt ++;
//            HAL_Delay(10);
//        }return cnt >= 3? 1:0;
//
//        if(HAL_GPIO_ReadPin(TRACK_PORT, TRACK8_PIN) == GPIO_PIN_SET){
//            track_dat |= 0x80;
//            cnt ++;
//            HAL_Delay(10);
//        }return cnt >= 3? 1:0;
    track_dat |= (HAL_GPIO_ReadPin(TRACK_PORT, TRACK1_PIN) == GPIO_PIN_RESET) ? 0x00 : 0x01;
    track_dat |= (HAL_GPIO_ReadPin(TRACK_PORT, TRACK2_PIN) == GPIO_PIN_RESET) ? 0x00 : 0x02;
    track_dat |= (HAL_GPIO_ReadPin(TRACK_PORT, TRACK3_PIN) == GPIO_PIN_RESET) ? 0x00 : 0x04;
    track_dat |= (HAL_GPIO_ReadPin(TRACK_PORT, TRACK4_PIN) == GPIO_PIN_RESET) ? 0x00 : 0x08;
    track_dat |= (HAL_GPIO_ReadPin(TRACK_PORT, TRACK5_PIN) == GPIO_PIN_RESET) ? 0x00 : 0x10;
    track_dat |= (HAL_GPIO_ReadPin(TRACK_PORT, TRACK6_PIN) == GPIO_PIN_RESET) ? 0x00 : 0x20;
    track_dat |= (HAL_GPIO_ReadPin(TRACK_PORT, TRACK7_PIN) == GPIO_PIN_RESET) ? 0x00 : 0x40;
    track_dat |= (HAL_GPIO_ReadPin(TRACK_PORT, TRACK8_PIN) == GPIO_PIN_RESET) ? 0x00 : 0x80;
    // 状态判断（修正case语法）
//    if (track_dat == 0x0C || track_dat == 0x18 || track_dat == 0x30) {
//        turn_flag = 0; // 直走
//    } else if (track_dat == 0x01 || track_dat == 0x03 || track_dat == 0x07 || track_dat == 0x0F) {
//        turn_flag = 1; // 左转
//    } else if (track_dat == 0x80 || track_dat == 0xC0 || track_dat == 0xE0 || track_dat == 0xF0) {
//        turn_flag = 2; // 右转
//    }
    if (track_dat == 0x18 || track_dat ==0x30 || track_dat == 0x0C) {
           turn_flag = 0; // 直走
       } else if (track_dat == 0x01 || track_dat == 0x03 || track_dat == 0x07 || track_dat == 0x0F) {
           turn_flag = 1; // 左转
       } else if (track_dat == 0x80 || track_dat == 0xC0 || track_dat == 0xE0 || track_dat == 0xF0) {
           turn_flag = 2; // 右转
       }else if (track_dat == 0xF8) {
           turn_flag = 3; // 左转
       }else if (track_dat == 0x1F) {
           turn_flag = 4; // 左转
       }
    // 其他情况维持当前状态
}

// 按键处理（上拉消抖）
void Key_Process(void) {
    static uint8_t key_cnt = 0;
    if (KEY_PRESSED) {
        key_cnt++;
        if (key_cnt >= 10) { // 100ms消抖
            speed_mode = !speed_mode;
            // 切换速度时清零PID积分，避免冲击
            left_sum_err = 0;
            right_sum_err = 0;
            left_last_err = 0;
            right_last_err = 0;
            key_cnt = 0;
        }
    } else {
        key_cnt = 0;
    }
}


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
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  Motor_Init();
  HAL_TIM_Base_Start_IT(&htim4);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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
  while (1)
  {
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
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

