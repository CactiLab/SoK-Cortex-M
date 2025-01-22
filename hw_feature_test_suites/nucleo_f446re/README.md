**Nucleo F446RE**

This folder contains tests and documentation for the Nucleo F446RE
development board.

**Purpose**

-   To demonstrate Cortex-M features specific to the Nucleo F446RE
    board.

-   To organize test suites and relevant files for this board.

**Getting Started**

-   Load the appropriate default project files into STM32CubeIDE

**Nucleo F446RE**

**Nucleo_f\>Core\>Src\>MPU_Config.c**

**Configuration Steps and Initial Assignments**

1.  **Disable the MPU:**

    -   Disables the MPU to allow modifications.

MPU-\>CTRL &= \~MPU_CTRL_ENABLE_Msk;

2.  **Configure Region 0:**

    -   **Region Number:** Set to 0 (Region 0 selected).

MPU-\>RNR = 0;

-   **Base Address:** Set to the start of SRAM (0x20000000).

MPU-\>RBAR = 0x20000000;

-   **Region Attributes and Size:**

    -   **Access Rights:** Privileged Read-Only, No User Access.

    -   **Memory Type:** Normal Memory.

    -   **Region Size:** 32 KB (0b011).

    -   **Region Enable:** Enabled.

MPU-\>RASR = (0b01 \<\< MPU_RASR_AP_Pos) \| // Privileged read-only

(0b000 \<\< MPU_RASR_TEX_Pos) \| // Normal memory

(0b011 \<\< MPU_RASR_SIZE_Pos) \| // 32 KB size

(1 \<\< MPU_RASR_ENABLE_Pos); // Enable region

3.  **Enable MPU and Set Default Memory Map:**

    -   Enables the MPU and retains the default memory mapping for
        undefined regions.

MPU-\>CTRL \|= MPU_CTRL_ENABLE_Msk \| MPU_CTRL_PRIVDEFENA_Msk;

4.  **Enable Memory Management Faults:**

    -   Configures the system to generate a fault for invalid memory
        access.

SCB-\>SHCSR \|= SCB_SHCSR_MEMFAULTENA_Msk;

5.  **Memory Barriers:**

    -   Ensures the configuration takes effect using data and
        instruction synchronization barriers.

DSB();

ISB();

nucleo_UART_console_connection 1/21/2025

Open Console\>Command shell console

Note To Self: Default settings (do not open multiple console windows
without disconnecting the board)

Serial port:COM5

Baud Rate:115200

Data Size: 8

Parity: None

Stop Bits: 1
