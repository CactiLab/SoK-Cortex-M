#include "MPU_Config.h"

#include "stm32f4xx.h"

#define P_UP_RW

#ifdef P_RO

void MPU_Config(void) {
    // Disable MPU
    MPU->CTRL &= ~MPU_CTRL_ENABLE_Msk;

    // Configure region 0
    MPU->RNR = 0;                        // Region number
    MPU->RBAR = 0x20000000;              // Base address (start of SRAM)
    MPU->RASR = (0b101 << MPU_RASR_AP_Pos) |  // Privileged read-only, no user access
                (0b001 << MPU_RASR_TEX_Pos) | // Normal memory
                (0b0 << MPU_RASR_C_Pos) |     // Not cacheable
                (0b0 << MPU_RASR_B_Pos) |     // Not bufferable
                (0x8 << MPU_RASR_SIZE_Pos) | // 32 KB size
                (0b1 << MPU_RASR_ENABLE_Pos); // Enable region

    // Enable MPU with default memory map for undefined regions
    MPU->CTRL |= MPU_CTRL_ENABLE_Msk | MPU_CTRL_PRIVDEFENA_Msk;
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;  // Enable MemManage exception

    // Memory barrier
    __DSB();
    __ISB();
}

#endif

#ifdef P_UP_RW
void MPU_Config(void) {
    // Disable MPU before configuration
    MPU->CTRL &= ~MPU_CTRL_ENABLE_Msk;

    // -----------------------------------
    // Region 0: Full access for both privileged and unprivileged modes
    // -----------------------------------
    MPU->RNR = 0;  // Select Region 0
    MPU->RBAR = 0x20000000;  // Base address of SRAM1
    MPU->RASR = (0b011 << MPU_RASR_AP_Pos) |  // full access
                (0b001 << MPU_RASR_TEX_Pos) | // Normal memory
                (0b0 << MPU_RASR_C_Pos) |     // Not cacheable
                (0b0 << MPU_RASR_B_Pos) |     // Not bufferable
                (0x8 << MPU_RASR_SIZE_Pos) |  // 32 KB size
                (0b1 << MPU_RASR_ENABLE_Pos); // Enable region

    // -----------------------------------
    // Region 1: Full access for both privileged and unprivileged modes
    // -----------------------------------
    MPU->RNR = 1;  // Select Region 0
    MPU->RBAR = 0x20010000;  // Base address of SRAM1
    MPU->RASR = (0b011 << MPU_RASR_AP_Pos) |  // full access
        		(0b001 << MPU_RASR_TEX_Pos) | // Normal memory
				(0b0 << MPU_RASR_C_Pos) |     // Not cacheable
				(0b0 << MPU_RASR_B_Pos) |     // Not bufferable
				(0xD << MPU_RASR_SIZE_Pos) |  // 128 KB size
				(0b1 << MPU_RASR_ENABLE_Pos); // Enable region

    // -----------------------------------
    // Configure other regions to allow unprivileged writes
    // -----------------------------------
    for (int region = 1; region < 8; region++) {
        MPU->RNR = region;  // Select the current region
        MPU->RBAR = 0x20000000 + (region * 0x8000);  // Example base addresses (modify as needed)
        MPU->RASR = (0b011 << MPU_RASR_AP_Pos) |  // full access
                	(0b001 << MPU_RASR_TEX_Pos) | // Normal memory
					(0b0 << MPU_RASR_C_Pos) |     // Not cacheable
					(0b0 << MPU_RASR_B_Pos) |     // Not bufferable
					(0x8 << MPU_RASR_SIZE_Pos) |  // 32 KB size
					(0b1 << MPU_RASR_ENABLE_Pos); // Enable region
    }

    // Enable MPU with default memory map for undefined regions
    MPU->CTRL |= MPU_CTRL_ENABLE_Msk | MPU_CTRL_PRIVDEFENA_Msk;
//    MPU->CTRL = 5;
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk | SCB_SHCSR_SVCALLACT_Msk;  // Enable MemManage exception

    // Memory barrier to ensure MPU configuration takes effect
    __DSB();
    __ISB();
}

#endif
