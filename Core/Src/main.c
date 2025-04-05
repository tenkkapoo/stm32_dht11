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
#include <stdio.h>
#include <string.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void delay_us(uint8_t us) {
	__HAL_TIM_SET_COUNTER(&htim2, 0);
	while ( __HAL_TIM_GET_COUNTER(&htim2) < us) {

	}
	char delay_ms2[50];
	snprintf(delay_ms2, sizeof(delay_ms2), "delay = %lu\n", __HAL_TIM_GET_COUNTER(&htim2));
	HAL_UART_Transmit(&huart2, (uint8_t*)delay_ms2, strlen(delay_ms2), HAL_MAX_DELAY);
}

void start (void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOA_CLK_ENABLE();
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, 0);
  HAL_Delay(25);

  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8) == 1) {

  }

  char start_message[50];
  snprintf(start_message, sizeof(start_message), "Start function complete\n");
  HAL_UART_Transmit(&huart2, (uint8_t*)start_message, strlen(start_message), HAL_MAX_DELAY);

}

int check_dht (void) {

	__HAL_TIM_SET_COUNTER(&htim2, 0);
	while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8) == 0) {
		if (__HAL_TIM_GET_COUNTER(&htim2) > 85) {
						return 0;
		}

	}
	char delay_ms[50];
	snprintf(delay_ms, sizeof(delay_ms), "low signal for = %lu\n", __HAL_TIM_GET_COUNTER(&htim2));
	HAL_UART_Transmit(&huart2, (uint8_t*)delay_ms, strlen(delay_ms), HAL_MAX_DELAY);
	__HAL_TIM_SET_COUNTER(&htim2, 0);
	while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8) == 1) {
		if (__HAL_TIM_GET_COUNTER(&htim2) > 85) {
						return 2;
		}
	}


	char check_message[50];
    snprintf(check_message, sizeof(check_message), "DHT11 check complete\n");
    HAL_UART_Transmit(&huart2, (uint8_t*)check_message, strlen(check_message), HAL_MAX_DELAY);
	return 1;
}

uint8_t read_dht (void) {
	uint8_t data_byte = 0;

	for (int i = 0; i < 8; i++) { /* Read 8 bits */
		__HAL_TIM_SET_COUNTER(&htim2, 0);
		while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8) == 0) { //start transmit 50us
			if (__HAL_TIM_GET_COUNTER(&htim2) > 75) {
				break;
			}
		}

		__HAL_TIM_SET_COUNTER(&htim2, 0);
		while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8) == 1) { //bit value
			if (__HAL_TIM_GET_COUNTER(&htim2) > 75) {
				break;
			}
		}

		if ((__HAL_TIM_GET_COUNTER(&htim2) > 65) && (__HAL_TIM_GET_COUNTER(&htim2) < 75)) {
			data_byte = data_byte | (1 << (7 - i)); // set bit if 1
		}


	}


	return data_byte;
}

void dht_data (void) {
	char dht_msg[50];
	snprintf(dht_msg, sizeof(dht_msg), "Starting data reading \n");
	HAL_UART_Transmit(&huart2, (uint8_t*)dht_msg, strlen(dht_msg), HAL_MAX_DELAY);
	uint8_t humid_int = read_dht();
	uint8_t humid_dec = read_dht();
	uint8_t temp_int = read_dht();
	uint8_t temp_dec = read_dht();
	uint8_t check_sum = read_dht();
	char humid_msg[50];
	char temp_msg[50];

	if (check_sum == ((humid_int + humid_dec + temp_int + temp_dec) & (0xff))) {
		// Works
		snprintf(humid_msg, sizeof(humid_msg), "Humidity = %d.%d \n", humid_int, humid_dec/10);
		HAL_UART_Transmit(&huart2, (uint8_t*)humid_msg, strlen(humid_msg), HAL_MAX_DELAY);

		snprintf(temp_msg, sizeof(temp_msg), "Temperature = %d.%d \n", temp_int, temp_dec/10);
		HAL_UART_Transmit(&huart2, (uint8_t*)temp_msg, strlen(temp_msg), HAL_MAX_DELAY);
	}
	else {
		//Doesnt work
		char message_err[] = "Check sum is wrong \n";
		HAL_UART_Transmit(&huart2, (uint8_t*)message_err, strlen(message_err), HAL_MAX_DELAY);

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
  MX_TIM2_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  // change prescaler to 15 after generating code
  HAL_Delay(1000);
  char message_start[50];
  snprintf(message_start, sizeof(message_start), "New run\n");
  HAL_UART_Transmit(&huart2, (uint8_t*)message_start, strlen(message_start), HAL_MAX_DELAY);
  HAL_TIM_Base_Start(&htim2);
  char message[50];
  snprintf(message, sizeof(message), "Initialzation done\n");
  HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
  HAL_Delay(2000);

  start();

  int check = check_dht();
  if (check == 1) {
	  dht_data();
  }
  else if (check == 0) {
	  char message_err2[] = "80us low fails\n";
	  HAL_UART_Transmit(&huart2, (uint8_t*)message_err2, strlen(message_err2), HAL_MAX_DELAY);
  }
  else {
	  char message_err3[] = "80us up fails\n";
	  HAL_UART_Transmit(&huart2, (uint8_t*)message_err3, strlen(message_err3), HAL_MAX_DELAY);
  }




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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

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
  htim2.Init.Prescaler = 15;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
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

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
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
