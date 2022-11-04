/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define NO_OF_TIMERS 3
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */
static uint8_t seg7Hex[] = {0x01, 0x4F, 0x12, 0x06, 0x4C, 0x24, 0x20, 0x0F, 0x00, 0x04, 0xff};
static uint16_t seg7Port[7] = {SEG0, SEG1, SEG2, SEG3, SEG4, SEG5, SEG6};
static uint16_t seg7En[4] = {EN0, EN1, EN2, EN3};

const int MAX_LED = 4;
int index_led = 0;
int led_buffer [4] = {0, 0, 0, 0};

int hour = 15, minute = 8, second = 50;

const int MAX_LED_MATRIX = 8;
int index_led_matrix = 0;
//uint8_t matrix_buffer[8] = {0x01, 0x06, 0x38, 0xC8, 0xC8, 0x38, 0x06, 0x01};
uint8_t matrix_buffer[8] = {0xff, 0xff,0x18,0x18,0x18,0x18,0xff,0xff};
uint8_t charA[8] = {0x01, 0x06, 0x38, 0xC8, 0xC8, 0x38, 0x06, 0x01};
uint8_t charH[8] = {0xff, 0xff,0x18,0x18,0x18,0x18,0xff,0xff};

uint16_t row_pin[8] = {ROW0,ROW1,ROW2,ROW3,ROW4,ROW5,ROW6,ROW7};
uint16_t col_pin[8] = {ENM0,ENM1,ENM2,ENM3,ENM4,ENM5,ENM6,ENM7};



int TIMER_CYCLE;

int timer_counter[NO_OF_TIMERS];
int timer_flag[NO_OF_TIMERS];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_TIM2_Init(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */
void update7SEG(int);
void updateClockBuffer(void);
void display7SEG(int);
void updateLEDMatrixBuffer(uint8_t *);
void updateLEDMatrix(int);
void shiftLeftMatrix(uint8_t *);


void setTimer(int, int);
void timer_run(void);
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
  MX_TIM2_Init();
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  setTimer(0,TIMER_CYCLE);
  setTimer(1,TIMER_CYCLE);
  setTimer(2,TIMER_CYCLE);
  updateClockBuffer();

  HAL_GPIO_WritePin(ROW_PORT, ROW0|ROW1|ROW2|ROW3|ROW4|ROW5|ROW6|ROW7, 1);
  HAL_GPIO_WritePin(ENM_PORT, ENM0|ENM1|ENM2|ENM3|ENM4|ENM5|ENM6|ENM7, 1);

  HAL_GPIO_WritePin(ROW_PORT, ROW7, 0);
  HAL_GPIO_WritePin(ENM_PORT, ENM7, 0);
  while (1)
  {
	  if(timer_flag[0]) {
		  timer_flag[0] = 0;
		  setTimer(0,1000);

		  HAL_GPIO_TogglePin(DOT_PORT, DOT);
		  HAL_GPIO_TogglePin(RLED_PORT, RLED1);

		  second++;
		  if(second >= 60) {
			  second = 0;
			  minute++;
		  }
		  if(minute >= 60) {
			  minute = 0;
			  hour++;
		  }
		  if(hour >= 24) {
			  hour = 0;
		  }
		  updateClockBuffer();
	  }
	  if(timer_flag[1]) {
		  timer_flag[1] = 0;
		  setTimer(1,250);

		  shiftLeftMatrix(charH);
		  update7SEG(index_led++);
		  if(index_led == MAX_LED) index_led = 0;
	  }
	  if(timer_flag[2]) {
		  timer_flag[2] = 0;
		  setTimer(2,3);
//		  updateLEDMatrixBuffer(charA);
		  updateLEDMatrix(index_led_matrix++);
		  if(index_led_matrix == MAX_LED_MATRIX) index_led_matrix = 0;
	  }

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
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 799;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */
  	TIMER_CYCLE = ((htim2.Init.Prescaler+1) * (htim2.Init.Period+1) / 8000);
  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9
                          |GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13
                          |GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_15|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_6|GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA2 PA3 PA4 PA5
                           PA6 PA7 PA8 PA9
                           PA10 PA11 PA12 PA13
                           PA14 PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9
                          |GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13
                          |GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB2 PB10
                           PB11 PB12 PB13 PB14
                           PB15 PB3 PB4 PB5
                           PB6 PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_15|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_6|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void display7SEG(int num) {
	HAL_GPIO_WritePin(SEG_PORT, SEG0 | SEG1 | SEG2 | SEG3 | SEG4 | SEG5 | SEG6, 1);
	for(int i = 0; i < 7; i++) {
		HAL_GPIO_WritePin(SEG_PORT, seg7Port[i], seg7Hex[num] & (0x40 >> i));
	}
}

void update7SEG(int index) {
	HAL_GPIO_WritePin(EN_PORT, EN0 | EN1 | EN2 | EN3, 1);
	display7SEG(led_buffer[index]);
	HAL_GPIO_WritePin(EN_PORT, seg7En[index], 0);
}

void updateLEDMatrixBuffer(uint8_t * character){
	for(int i = 0 ; i< 8; i++){
		matrix_buffer[i] = character[i];
	}
}

void updateLEDMatrix(int index) {
	HAL_GPIO_WritePin(ROW_PORT, ROW0|ROW1|ROW2|ROW3|ROW4|ROW5|ROW6|ROW7, 1);
	HAL_GPIO_WritePin(ENM_PORT, ENM0|ENM1|ENM2|ENM3|ENM4|ENM5|ENM6|ENM7, 1);
	for(int i = 0; i < 8; i++) {
		if(matrix_buffer[index] & (0x80 >> i)) {
			HAL_GPIO_WritePin(ROW_PORT, row_pin[i], 0);
		}
	}
	HAL_GPIO_WritePin(ENM_PORT, col_pin[index], 0);
}
void shiftLeftMatrix(uint8_t * character) {
	static int shamt = 0;

	for(int i = 0; i < MAX_LED_MATRIX; i++) {
		if(i+shamt >= 0 && i+shamt < MAX_LED_MATRIX) {
			matrix_buffer[i] = character[i+shamt];
		}
		else {
			matrix_buffer[i] = 0x0;
		}
	}

	shamt++;
	if(shamt == MAX_LED_MATRIX) {
		shamt = -7;
	}
}

void updateClockBuffer() {
	led_buffer[0] = hour / 10;
	led_buffer[1] = hour % 10;
	led_buffer[2] = minute / 10;
	led_buffer[3] = minute % 10;
}

void setTimer(int timer, int duration) {
	timer_counter[timer] = duration / TIMER_CYCLE;
	timer_flag[timer] = 0;
}

void timer_run() {
	for(int i = 0; i < NO_OF_TIMERS; i++) {
		if(timer_counter[i] > 0) {
			timer_counter[i]--;
			if(timer_counter[i] == 0) {
				timer_flag[i] = 1;
			}
		}
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef*htim) {
	timer_run();
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
  while (1)
  {
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
