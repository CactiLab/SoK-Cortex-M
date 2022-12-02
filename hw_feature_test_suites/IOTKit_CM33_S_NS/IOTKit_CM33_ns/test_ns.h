#ifndef TEST_NS_H
#define TEST_NS_H

#include <arm_cmse.h>
#include "IOTKit_CM33_FP.h"                    /* Device header */
#include "Board_LED.h"                         /* ::Board Support:LED */
#include "..\IOTKit_CM33_s\Secure_Functions.h" /* Secure Code Entry Points */

/*----------------------------------------------------------------------------
  Constants
 *----------------------------------------------------------------------------*/
#define MAXMSG 80
#define MAXCOUNT 500
#define MAX_ALIAS 10
#define MTB_BUFF 8192
#define MISSED_CYCLES

/*----------------------------------------------------------------------------
  Define test cases
 *----------------------------------------------------------------------------*/
#define TEST_CASE_NS /* enable test cases */

#ifdef TEST_CASE_NS
enum test_ns
{
  /* those cases will trigger faults */
  TEST_UNSTACKING_NS,
  TEST_ACCESS_NS,
  TEST_ALIAS_MPC_NS,
  TEST_MPC_NS, /* Only secure state can configure the MPC */

  /* benign tests */
  TEST_TT_NS,
  TEST_SWITCH_NS,
  TEST_NSC_NS,
  TEST_DWT_NS, /* !! SET *(uint32_t*)0xE000EDF0 to value a05f0000 (0xA05F<<16) via the debugger memory view */

  TEST_SCB_NS,
  TEST_CTRL_NS,

  TEST_LP_ACCESS, /* low-privileged region call NSC funtion to read the privileged region */
  TEST_RUNNING_TIME,
};

/*--	Those tests run in the secure world after non-secure initialized --*/
enum test_from_ns
{
  TEST_CASE_S,
  TEST_ACCESS_NS_S, /* access non-secure data sections from the secure state program */
  TEST_ALIAS_MPC_S,
  TEST_SWITCH_NS_S,
  TEST_NSC_NS_S,
  TEST_SCB_NS_S,
  TEST_CRTL_NS_S,
  TEST_UP_ACCESS_NS_S,
  MPU_CONFIG_MPSSSRAM3_NS_S,
  DROP_P_NS_S,
  SET_P_NS_S,
};

#endif

/*---------------------------------------------------------------------------
	System MPU configuration
	Refer to Connect ARMv8-M Memory Protection Unit User Guide V2.0
 *---------------------------------------------------------------------------*/
/* MPU registers	*/
/*
#define MPU_CTRL_S *((volatile uint32_t *)0xE000ED94)
#define MPU_RNR_S *((volatile uint32_t *)0xE000ED98)
#define MPU_RBAR_S *((volatile uint32_t *)0xE000ED9C)
#define MPU_RLAR_S *((volatile uint32_t *)0xE000EDA0)
#define SHCSR_S *((volatile uint32_t *)0xE000ED24)
*/

/* MPU Region Base Address Register Definitions */
#define MPU_RBAR_ADDR_Pos 5U                                 /*!< MPU RBAR: ADDR Position */
#define MPU_RBAR_ADDR_Msk (0x7FFFFFFUL << MPU_RBAR_ADDR_Pos) /*!< MPU RBAR: ADDR Mask */
//#define SCB_SHCSR_MEMFAULTENA_Pos 16

/* General MPU Masks */
#define REGION_ENABLE 0x00000001
/* Shareability */
#define NON_SHAREABLE 0x00
#define RESERVED 0x08
#define OUTER_SHAREABLE 0x10
#define INNER_SHAREABLE 0x18

/* Access Permissions */
#define EXEC_NEVER 0x01 /* All instruction fetches abort */
#define RW_P_ONLY 0x00  /* Read/Write, Privileged code only */
#define RW_P_U 0x02     /* Read/Write, Any Privilege Level */
#define RO_P_ONLY 0x04  /* Read-Only, Privileged code only */
#define RO_P_U 0x06     /* Read-Only, Any Privilege Level */

/* Read/Write Allocation Configurations for Cacheable Memory Attr<n>[7:4] and Attr<n>[3:0] are of the format: 0bXXRW */
#define R_NON_W_NON 0x0     /* Do not allocate Read/Write */
#define R_NON_W_ALLOC 0x1   /* Do not allocate Read, Allocate Write */
#define R_ALLOC_W_NON 0x2   /* Allocate Read, Do not allocate Write */
#define R_ALLOC_W_ALLOC 0x3 /* Allocate Read/Write */

/* Memory Attribute Masks */
#define DEVICE 0x0F
#define NORMAL_OUTER 0xF0
#define NORMAL_INNER 0x0F

/* Memory Attributes */
#define DEVICE_NG_NR_NE 0x00 /* Device, Non-Gathering, Non-Reordering, NonEarly-Write-Acknowledgement */
#define DEVICE_NG_NR_E 0x04  /* Device, Non-Gathering, Non-Reordering, EarlyWrite-Acknowledgement */
#define DEVICE_NG_R_E 0x08   /* Device, Non-Gathering, Reordering, Early-Write-Acknowledgement */
#define DEVICE_G_R_E 0x0C    /* Device, Gathering, Reordering, Early-Write-Acknowledgement */
#define NORMAL_O_WT_T 0x00   /* Normal, Outer Write-through transient (if RW not 00) */
#define NORMAL_O_NC 0x40     /* Normal, Outer Non-cacheable (if RW is 00) */
#define NORMAL_O_WB_T 0x40   /* Normal, Outer Write-back transient (if RW not 00) */
#define NORMAL_O_WT_NT 0x80  /* Normal, Outer Write-through non-transient */
#define NORMAL_O_WB_NT 0xC0  /* Normal, Outer Write-back non-transient */
#define NORMAL_I_WT_T 0x00   /* Normal, Inner Write-through transient (if RW not 00) */
#define NORMAL_I_NC 0x04     /* Normal, Inner Non-cacheable (if RW is 00) */
#define NORMAL_I_WB_T 0x04   /* Normal, Inner Write-back transient (if RW not 00) */
#define NORMAL_I_WT_NT 0x08  /* Normal, Inner Write-through non-transient */
#define NORMAL_I_WB_NT 0x0C  /* Normal, Inner Write-back non-transient */

/*---------------------------------------------------------------------------
	System Privilege Modification
 *---------------------------------------------------------------------------*/

#define DROP_NS_PRIVILEGES __asm("msr CONTROL, %[reg] " ::[reg] "r"(1) \
                                 : "memory")
#define SET_NS_PRIVILEGES __asm("msr CONTROL, %[reg] " ::[reg] "r"(0) \
                                : "memory")
// arm compiler 5
// void __svc( 0 ) EnablePrivilegedMode(void);
//arm compiler 6
#define EnablePrivilegedMode __asm("SVC #0")

/*---------------------------------------------------------------------------
	Cortex-M enable or disable cache
 *---------------------------------------------------------------------------*/
#define DISABLE_IRQ __disable_irq() // Set PRIMASK
#define ENABLE_IRQ __enable_irq()   // Clear PRIMASK

/* refer to https://github.com/ARM-software/CMSIS_5/blob/8ced160cc8362da0fdd794a3e87275b15416b287/CMSIS/Core/Include/cachel1_armv7.h */
/* Cache Size ID Register Macros */
#define CCSIDR_WAYS(x) (((x)&SCB_CCSIDR_ASSOCIATIVITY_Msk) >> SCB_CCSIDR_ASSOCIATIVITY_Pos)
#define CCSIDR_SETS(x) (((x)&SCB_CCSIDR_NUMSETS_Msk) >> SCB_CCSIDR_NUMSETS_Pos)
#define DISABLE_I_CACHE SCB_DisableICache() // disable instruction cache
#define ENABLE_I_CACHE SCB_EnableICache()   // enbale instruction cache
#define DISABLE_D_CACHE SCB_DisableDCache() // disable data cache
#define ENABLE_D_CACHE SCB_EnableDCache()   // enable data cache

/* check device defines and use defaults */
#ifndef __ICACHE_PRESENT
#define __ICACHE_PRESENT 0U
#endif

#ifndef __DCACHE_PRESENT
#define __DCACHE_PRESENT 0U
#endif

/**
  \brief   Enable I-Cache
  \details Turns on I-Cache
  */
__STATIC_FORCEINLINE void
SCB_EnableICache(void)
{
#if defined(__ICACHE_PRESENT) && (__ICACHE_PRESENT == 1U)
  if (SCB->CCR & SCB_CCR_IC_Msk)
    return; /* return if ICache is already enabled */

  __DSB();
  __ISB();
  SCB->ICIALLU = 0UL; /* invalidate I-Cache */
  __DSB();
  __ISB();
  SCB->CCR |= (uint32_t)SCB_CCR_IC_Msk; /* enable I-Cache */
  __DSB();
  __ISB();
#endif
}
/**
  \brief   Disable I-Cache
  \details Turns off I-Cache
  */
__STATIC_FORCEINLINE void SCB_DisableICache(void)
{
#if defined(__ICACHE_PRESENT) && (__ICACHE_PRESENT == 1U)
  __DSB();
  __ISB();
  SCB->CCR &= ~(uint32_t)SCB_CCR_IC_Msk; /* disable I-Cache */
  SCB->ICIALLU = 0UL;                    /* invalidate I-Cache */
  __DSB();
  __ISB();
#endif
}

/**
  \brief   Enable D-Cache
  \details Turns on D-Cache
  */
__STATIC_FORCEINLINE void SCB_EnableDCache(void)
{
#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
  uint32_t ccsidr;
  uint32_t sets;
  uint32_t ways;

  if (SCB->CCR & SCB_CCR_DC_Msk)
    return; /* return if DCache is already enabled */

  SCB->CSSELR = 0U; /* select Level 1 data cache */
  __DSB();

  ccsidr = SCB->CCSIDR;

  /* invalidate D-Cache */
  sets = (uint32_t)(CCSIDR_SETS(ccsidr));
  do
  {
    ways = (uint32_t)(CCSIDR_WAYS(ccsidr));
    do
    {
      SCB->DCISW = (((sets << SCB_DCISW_SET_Pos) & SCB_DCISW_SET_Msk) |
                    ((ways << SCB_DCISW_WAY_Pos) & SCB_DCISW_WAY_Msk));
#if defined(__CC_ARM)
      __schedule_barrier();
#endif
    } while (ways-- != 0U);
  } while (sets-- != 0U);
  __DSB();

  SCB->CCR |= (uint32_t)SCB_CCR_DC_Msk; /* enable D-Cache */

  __DSB();
  __ISB();
#endif
}

/**
  \brief   Disable D-Cache
  \details Turns off D-Cache
  */
__STATIC_FORCEINLINE void SCB_DisableDCache(void)
{
#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
  uint32_t ccsidr;
  uint32_t sets;
  uint32_t ways;

  SCB->CSSELR = 0U; /* select Level 1 data cache */
  __DSB();

  SCB->CCR &= ~(uint32_t)SCB_CCR_DC_Msk; /* disable D-Cache */
  __DSB();

  ccsidr = SCB->CCSIDR;

  /* clean & invalidate D-Cache */
  sets = (uint32_t)(CCSIDR_SETS(ccsidr));
  do
  {
    ways = (uint32_t)(CCSIDR_WAYS(ccsidr));
    do
    {
      SCB->DCCISW = (((sets << SCB_DCCISW_SET_Pos) & SCB_DCCISW_SET_Msk) |
                     ((ways << SCB_DCCISW_WAY_Pos) & SCB_DCCISW_WAY_Msk));
#if defined(__CC_ARM)
      __schedule_barrier();
#endif
    } while (ways-- != 0U);
  } while (sets-- != 0U);

  __DSB();
  __ISB();
#endif
}

enum
{
  NoCache,   /* No cache */
  ICache,    /* Only instruction cache */
  DCache,    /* Only data cache */
  I_D_Cache, /* Seperate Instruction and Data cache */
  IDCache,   /* Unified Instruction and Data cache*/
};

uint32_t check_cache();

/*---------------------------------------------------------------------------
	MTB refer to https://interrupt.memfault.com/blog/instruction-tracing-mtb-m33
 *---------------------------------------------------------------------------*/
typedef struct __attribute__((packed)) MtbM33
{ // 0xE0043000
  __IOM uint32_t POSITION;
  __IOM uint32_t MASTER;
  __IOM uint32_t FLOW;
  __IM uint32_t BASE; // 0x24000000
  __IOM uint32_t TSTART;
  __IOM uint32_t TSTOP;
  __IOM uint32_t SECURE;
} sMtbM33;

static sMtbM33 *const MTB = ((sMtbM33 *)0xE0043000);

#define MTB_BUFF 8192
int mtb_resume_ns(size_t mtb_size, int pos);
int mtb_pause_ns();
int mtb_enable_ns(size_t size);
int mtb_disable_ns(void);

/*---------------------------------------------------------------------------
	Cortex-M DWT TIMER 
	Refer to https://www.embedded-computing.com/articles/measuring-code-execution-time-on-arm-cortex-m-mcus
 *---------------------------------------------------------------------------*/
#define ARM_CM_DEMCR (*(uint32_t *)0xE000EDFC)
#define ARM_CM_DWT_CTRL (*(uint32_t *)0xE0001000)
#define ARM_CM_DWT_CYCCNT (*(uint32_t *)0xE0001004)
#define ELAPSED_TIME_MAX_SECTIONS 10

typedef struct elapsed_time
{
  uint32_t start;
  uint32_t current;
  uint32_t max;
  uint32_t min;
} ELAPSED_TIME;

void elapsed_time_clr(uint32_t i);        // Clear measured values
void elapsed_time_init(void);             // Module initialization
void elapsed_time_start(uint32_t i);      // Start measurement
void elapsed_time_stop(uint32_t i);       // Stop  measurement
uint32_t elapsed_time_result(uint32_t i); // Get the measurement result

/*---------------------------------------------------------------------------
	Cortex-M ALIAS REGIONS
NS SSRAM1: 				1		0x0000_0000 0x003F_FFFF 4MB (alias with 5)
NS SSRAM1 alias:	2		0x0040_0000 0x007F_FFFF	4MB	(alias with 6)
S  SSRAM1:				5 	0x1000_0000 0x103F_FFFF 4MB (alias with 1)
S  SSRAM1 slias:  6 	0x1040_0000 0x107F_FFFF 4MB	(alias with 2)
----------------------------------------------------------------------
NS SSRAM2:				5		0x2800_0000 0x281F_FFFF 2MB	(alias with 10)
S	 SSRAM2:				10	0x3800_0000 0x381F_FFFF 2MB	(alias with 11)
NS SSRAM3:				6 	0x2820_0000 0x283F_FFFF 2MB	(alias with 5)
S  SSRAM3:				11	0x3820_0000 0x383F_FFFF 2MB	(alias with 6)
 *---------------------------------------------------------------------------*/
typedef struct alias_region
{
  uint32_t data;
  uint32_t data_alias;
} ALIAS_REGION;
/* 
tbl0: NS SSRAM1 with its alias
tbl1: S SSRAM1 with its alias
tbl2: NS SSRAM1 with S SSRAM1
tbl3:	NS SSRAM2 with S SSRAM2
tbls4: NS SSRAM3 with S SSRAM3
*/
extern volatile ALIAS_REGION alias_region_tbl[MAX_ALIAS] __attribute__((section(".ARM.__at_0x282000a0")));
/*---------------------------------------------------------------------------
	Global functions
 *---------------------------------------------------------------------------*/
void SysTick_Handler(void);
void SVC_Handler(void);
void test_cases_ns();
void mpu_config_mpsssram3_ns(void);
void arm_mpu_config_ns(void);

uint32_t read_memory_ns(uint32_t);
uint32_t write_memory_ns(uint32_t, uint32_t);
float increase_lcd_pos_ns(float *);

int32_t test_add_ns(uint32_t a, uint32_t b);

#endif
