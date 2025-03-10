/* USER CODE BEGIN Header */  
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body with DWT watchpoint, cycle counting,
  *                   and event monitoring examples.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
// (Include any additional headers if needed, e.g., for debugging output)
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
// Global variables for data watchpoint demonstration.
// Any write access to watch_var or another_var will trigger the watchpoint if the debugger is attached.
volatile uint32_t watch_var = 0;
volatile uint32_t another_var = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// You can add additional user code here if needed.
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  uint32_t start_cycles, end_cycles, cycle_delta;
  uint32_t cpi_event_count;  // For event monitoring (if supported)
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
  /* USER CODE BEGIN 2 */
  
  /* --- LED Initialization --- */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* Configure PA5 as output (on-board LED for NUCLEO-F446RE) */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  /* --- End of LED Initialization --- */
  
  /* --- DWT Configuration for Cycle Counting, Event Monitoring, and Data Watchpoint --- */
  // Enable the trace unit
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  
  // Reset and enable the cycle counter
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  
  // (Optional) Reset the CPI counter if supported (event monitoring example)
  DWT->CPICNT = 0;
  
  // Set up data watchpoint for 'watch_var' using comparator 0.
  DWT->COMP0 = (uint32_t)&watch_var; // Set the watchpoint address to watch_var
  DWT->MASK0 = 0;                   // Exact address match (no range)
  DWT->FUNCTION0 = 0x6;             // 0x6: Watch for read/write accesses
  
  // Set up data watchpoint for 'another_var' using comparator 1.
  DWT->COMP1 = (uint32_t)&another_var; // Set the watchpoint address to another_var
  DWT->MASK1 = 0;                     // Exact address match (no range)
  DWT->FUNCTION1 = 0x6;               // 0x6: Watch for read/write accesses
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // Record cycle count before starting the operation
    start_cycles = DWT->CYCCNT;
    
    // Toggle the LED on PA5
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    watch_var=1;
    // Modify watch_var to trigger its watchpoint (if debugger is attached)
    watch_var++;
    
    // Modify another_var to also trigger its watchpoint (if debugger is attached)
    another_var++;
    
    // Insert 500 ms delay
    HAL_Delay(500);
    
    // Record cycle count after the operation
    end_cycles = DWT->CYCCNT;
    cycle_delta = end_cycles - start_cycles;  // Calculate cycles taken for the operations
    
    // (Optional) Read an event counter, e.g., the CPI counter
    cpi_event_count = DWT->CPICNT;
    
    // You can now inspect 'cycle_delta' and 'cpi_event_count' in your debugger
    // or send them over a debug interface (e.g., UART) if needed.
    
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

/* USER CODE BEGIN 4 */
// Additional user code can be added here
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
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
  // You can add your own implementation to report the file name and line number.
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
