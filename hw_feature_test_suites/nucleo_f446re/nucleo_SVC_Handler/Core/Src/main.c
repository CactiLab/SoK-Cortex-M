#include "main.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_uart.h"

/* Global UART handler */
UART_HandleTypeDef huart2;

/* Function prototypes */
void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_USART2_UART_Init(void);
void MPU_Config(void);
void switch_to_unprivileged_mode(void);
void switch_to_privileged_mode(void);
void test_privileged_and_unprivileged_access(void);
void Error_Handler(void);

/**
 * @brief Send a single character over UART.
 */
void UART_SendChar(uint8_t ch)
{
    HAL_UART_Transmit(&huart2, &ch, 1, HAL_MAX_DELAY);
}

/**
 * @brief Send a string over UART.
 */
void UART_Print(const char* str)
{
    while (*str)
    {
        UART_SendChar(*str++);
    }
}

void MX_USART2_UART_Init(void)
{
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
}

int main(void)
{
    // Initialize the HAL Library
    HAL_Init();

    // Configure the system clock
    SystemClock_Config();

    // Initialize GPIO and UART
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    // Enable MemManage and BusFault exceptions
    SCB->SHCSR |= (SCB_SHCSR_MEMFAULTENA_Msk | SCB_SHCSR_BUSFAULTENA_Msk);

    // Configure MPU
    MPU_Config();

    // Send startup message via UART
    UART_Print("System Initialization Completed\n");

    // Test access in both privileged and unprivileged modes
    test_privileged_and_unprivileged_access();

    // Main loop
    while (1)
    {
        UART_Print("Main Loop Running...\n");
        HAL_Delay(1000);
    }
}

/**
 * @brief Test memory access in both privileged and unprivileged modes.
 */
void test_privileged_and_unprivileged_access(void)
{
    volatile uint32_t *sram_region0 = (uint32_t *)0x20000000;

    // -------- Test in Privileged Mode --------
    UART_Print("Testing privileged memory access...\n");
    *sram_region0 = 1;  // Write access in privileged mode
    if (*sram_region0 == 1)
    {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);  // Toggle LD2 (Pin A5 on STM32F4 Discovery)
    }

    // Switch to unprivileged mode
    switch_to_unprivileged_mode();
    UART_Print("Switched to unprivileged mode\n");

    // -------- Test in Unprivileged Mode --------
    *sram_region0 = 2;  // Attempt write access in unprivileged mode
    if (*sram_region0 == 2)
    {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);  // Toggle LED if successful
    }

    // Switch back to privileged mode
    switch_to_privileged_mode();
    UART_Print("Switched back to privileged mode\n");
}

/**
 * @brief Switch CPU to unprivileged mode.
 */
void switch_to_unprivileged_mode(void)
{
    __ASM volatile("MRS r0, CONTROL");        // Load CONTROL register into r0
    __ASM volatile("ORR r0, r0, #1");         // Set unprivileged mode (CONTROL[0] = 1)
    __ASM volatile("MSR CONTROL, r0");        // Store modified CONTROL register
    __ASM volatile("ISB");                    // Instruction Synchronization Barrier
}

/**
 * @brief Switch CPU back to privileged mode.
 */
void switch_to_privileged_mode(void)
{
    __ASM volatile("MRS r0, CONTROL");        // Load CONTROL register into r0
    __ASM volatile("BIC r0, r0, #1");         // Clear unprivileged mode (CONTROL[0] = 0)
    __ASM volatile("MSR CONTROL, r0");        // Store modified CONTROL register
    __ASM volatile("ISB");                    // Instruction Synchronization Barrier
}

/**
 * @brief MPU Configuration function.
 */
void MPU_Config(void)
{
    // Disable the MPU for configuration
    HAL_MPU_Disable();

    MPU_Region_InitTypeDef MPU_InitStruct = {0};

    // Configure Region 0 (SRAM1)
    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress = 0x20000000;   // SRAM1 base address
    MPU_InitStruct.Size = MPU_REGION_SIZE_64KB;  // Region size: 64 KB
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;  // Full access for privileged/unprivileged
    MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.Number = MPU_REGION_NUMBER0;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;

    // Configure this region (SRAM1)
    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    // Enable MPU with default access
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
 * @brief Error Handler.
 */
void Error_Handler(void)
{
    while (1)
    {
        // Stay in an infinite loop if an error occurs
    }
}

/**
 * @brief System Clock Configuration
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    // Configure the main internal regulator output voltage
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    // Initialize the RCC Oscillators
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 16;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
    RCC_OscInitStruct.PLL.PLLQ = 2;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief GPIO Initialization Function
 */
void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // GPIO Ports Clock Enable
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // Configure GPIO pin for LD2 (PA5)
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}
