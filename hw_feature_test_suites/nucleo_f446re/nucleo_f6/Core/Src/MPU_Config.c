#include "MPU_Config.h"

void MPU_Config(void) {
    // Disable MPU
    MPU->CTRL &= ~MPU_CTRL_ENABLE_Msk;

    // -----------------------------------
    // Region 0: SRAM with full access (Read/Write for both modes)
    // -----------------------------------
    MPU->RNR = 0;                        // Select Region 0
    MPU->RBAR = 0x20000000;              // Base address
    MPU->RASR = (0b011 << MPU_RASR_AP_Pos) |  // Full access for both modes
                (0x8 << MPU_RASR_SIZE_Pos) |  // 32 KB region size
                (0b1 << MPU_RASR_ENABLE_Pos); // Enable region

    // -----------------------------------
    // Region 1: Privileged Read/Write, Unprivileged No Access
    // -----------------------------------
    MPU->RNR = 1;                        // Select Region 1
    MPU->RBAR = 0x20004000;              // Base address
    MPU->RASR = (0b001 << MPU_RASR_AP_Pos) |  // Privileged R/W, Unprivileged No Access
                (0x8 << MPU_RASR_SIZE_Pos) |  // 32 KB region size
                (0b1 << MPU_RASR_ENABLE_Pos); // Enable region

    // -----------------------------------
    // Region 2: Read-only Flash memory (unchanged)
    // -----------------------------------
    MPU->RNR = 2;                        // Select Region 2
    MPU->RBAR = 0x08000000;              // Base address
    MPU->RASR = (0b110 << MPU_RASR_AP_Pos) |  // Read-only for both modes
                (0x15 << MPU_RASR_SIZE_Pos) | // 1 MB region size
                (0b1 << MPU_RASR_ENABLE_Pos); // Enable region

    // Enable MPU with default memory map for undefined regions
    MPU->CTRL |= MPU_CTRL_ENABLE_Msk | MPU_CTRL_PRIVDEFENA_Msk;

    // Memory barriers to ensure MPU settings take effect
    __DSB();
    __ISB();
}


