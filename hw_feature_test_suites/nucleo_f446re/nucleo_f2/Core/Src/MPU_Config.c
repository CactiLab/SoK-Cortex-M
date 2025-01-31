#include "MPU_Config.h"

void MPU_Config(void) {
    // Disable MPU
    MPU->CTRL &= ~MPU_CTRL_ENABLE_Msk;

    // Configure region 1
    MPU->RNR = 1;                        // Region number changed to 1
    MPU->RBAR = 0x20004000;              // Base address (new base for region 1)
    MPU->RASR = (0b101 << MPU_RASR_AP_Pos) |  // Privileged read-only, no user access
                (0b001 << MPU_RASR_TEX_Pos) | // Normal memory
                (0b0 << MPU_RASR_C_Pos) |     // Not cacheable
                (0b0 << MPU_RASR_B_Pos) |     // Not bufferable
                (0x8 << MPU_RASR_SIZE_Pos) |  // 32 KB size
                (0b1 << MPU_RASR_ENABLE_Pos); // Enable region

    // Enable MPU with default memory map for undefined regions
    MPU->CTRL |= MPU_CTRL_ENABLE_Msk | MPU_CTRL_PRIVDEFENA_Msk;
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;  // Enable MemManage exception

    // Memory barrier
    __DSB();
    __ISB();
}
