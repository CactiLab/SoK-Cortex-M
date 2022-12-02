#ifndef TEST_S_H
#define TEST_S_H
#endif

#include <inttypes.h>
#include <arm_cmse.h>
#include <stdio.h>
#include "IOTKit_CM33_FP.h" /* Device header */
#include "Board_LED.h"		/* ::Board Support:LED */
#include "Board_GLCD.h"		/* ::Board Support:Graphic LCD */
#include "GLCD_Config.h"	/* Keil.SAM4E-EK::Board Support:Graphic LCD */
#include "SMM_MPS2.h"		/* Keil::Board Support:V2M-MPS2:Common */

/* typedef for NonSecure callback functions */
typedef int32_t (*NonSecure_fpParam)(uint32_t) __attribute__((cmse_nonsecure_call));
typedef void (*NonSecure_fpVoid)(void) __attribute__((cmse_nonsecure_call));

/*----------------------------------------------------------------------------
  Constants
 *----------------------------------------------------------------------------*/
#define MAXMSG 80
#define MAXCOUNT 500
#define MAX_ALIAS 10
// Types
#undef FALSE
#undef TRUE
#define FALSE 0
#define TRUE 1
#define MSP_TOP 0x38000000

/*----------------------------------------------------------------------------
  Define test cases
 *----------------------------------------------------------------------------*/
/*-- Those tests run in the secure state --*/
enum test_s
{
	TEST_USER_BUTTON_S,		   /* arm express vasatile board provides userpb0 and userpb1 two buttons */
	TEST_SYSTICK_S,			   /* test the systick interrupt to collect running time */
	TEST_LCD_S,				   /* set message to the LCD */
	TEST_MPU_MPC_ROM_CONFIG_S, /* set MPU region 0 to protect MPC (ROM) */
	TEST_MOD_PRIVILEGES,	   /* change the privilege */

	TEST_SAU,	/* configure SAU at run-time */
	TEST_MPC,	/* configure MPC at run-time */
	TEST_PPC,	/* configure PPC at run-time */
	TEST_MPU_S, /* set 6 regions to protect by MPU */

	TEST_SWITCH_S,	   /* observe the state switches */
	TEST_MOD_IPSR,	   /* try to modify the IPSR directly */
	TEST_UNSTACKING_S, /* put value EXC_RETURN in LR to trigger the unstacking */
	TEST_DWT_S,

	TEST_MTB,  /* observe the value in memory 0x24000000 */
	TEST_BB_S, /* unintended branch */
	TEST_SVC_S,
	TEST_TT_S,
	TEST_SG_S,
	TEST_SG_2_NS_S
};

/*-- Those tests run in the secure world after non-secure initialized --*/
//#define DROP_P_S
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

/*---------------------------------------------------------------------------
	Exception stack frame
 *---------------------------------------------------------------------------*/
typedef struct __attribute__((packed)) ContextStateFrame
{
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r12;
	uint32_t lr;
	uint32_t pc;  // return_address
	uint32_t psr; //xpsr
} sContextStateFrame;

/*---------------------------------------------------------------------------
	LCD_PRINT macro
 *---------------------------------------------------------------------------*/
extern GLCD_FONT GLCD_Font_16x24;
extern GLCD_FONT GLCD_Font_6x8;
#define lcd_print(s_, ...) GLCD_DrawString(0.5 * 16, (s_)*24, __VA_ARGS__);

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
#define MPU_RBAR_ADDR_Pos 5U								 /*!< MPU RBAR: ADDR Position */
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
#define RW_P_ONLY 0x00	/* Read/Write, Privileged code only */
#define RW_P_U 0x02		/* Read/Write, Any Privilege Level */
#define RO_P_ONLY 0x04	/* Read-Only, Privileged code only */
#define RO_P_U 0x06		/* Read-Only, Any Privilege Level */

/* Read/Write Allocation Configurations for Cacheable Memory Attr<n>[7:4] and Attr<n>[3:0] are of the format: 0bXXRW */
#define R_NON_W_NON 0x0		/* Do not allocate Read/Write */
#define R_NON_W_ALLOC 0x1	/* Do not allocate Read, Allocate Write */
#define R_ALLOC_W_NON 0x2	/* Allocate Read, Do not allocate Write */
#define R_ALLOC_W_ALLOC 0x3 /* Allocate Read/Write */

/* Memory Attribute Masks */
#define DEVICE 0x0F
#define NORMAL_OUTER 0xF0
#define NORMAL_INNER 0x0F

/* Memory Attributes */
#define DEVICE_NG_NR_NE 0x00 /* Device, Non-Gathering, Non-Reordering, NonEarly-Write-Acknowledgement */
#define DEVICE_NG_NR_E 0x04	 /* Device, Non-Gathering, Non-Reordering, EarlyWrite-Acknowledgement */
#define DEVICE_NG_R_E 0x08	 /* Device, Non-Gathering, Reordering, Early-Write-Acknowledgement */
#define DEVICE_G_R_E 0x0C	 /* Device, Gathering, Reordering, Early-Write-Acknowledgement */
#define NORMAL_O_WT_T 0x00	 /* Normal, Outer Write-through transient (if RW not 00) */
#define NORMAL_O_NC 0x40	 /* Normal, Outer Non-cacheable (if RW is 00) */
#define NORMAL_O_WB_T 0x40	 /* Normal, Outer Write-back transient (if RW not 00) */
#define NORMAL_O_WT_NT 0x80	 /* Normal, Outer Write-through non-transient */
#define NORMAL_O_WB_NT 0xC0	 /* Normal, Outer Write-back non-transient */
#define NORMAL_I_WT_T 0x00	 /* Normal, Inner Write-through transient (if RW not 00) */
#define NORMAL_I_NC 0x04	 /* Normal, Inner Non-cacheable (if RW is 00) */
#define NORMAL_I_WB_T 0x04	 /* Normal, Inner Write-back transient (if RW not 00) */
#define NORMAL_I_WT_NT 0x08	 /* Normal, Inner Write-through non-transient */
#define NORMAL_I_WB_NT 0x0C	 /* Normal, Inner Write-back non-transient */

/*---------------------------------------------------------------------------
	System Privilege Modification
 *---------------------------------------------------------------------------*/
#define DROP_S_PRIVILEGES __asm("msr CONTROL, %[reg] " ::[reg] "r"(1) \
								: "memory")
#define DROP_NS_PRIVILEGES __asm("msr CONTROL_NS, %[reg] " ::[reg] "r"(1) \
								 : "memory")
#define SET_S_PRIVILEGES __asm("msr CONTROL, %[reg] " ::[reg] "r"(0) \
							   : "memory")
#define SET_NS_PRIVILEGES __asm("msr CONTROL_NS, %[reg] " ::[reg] "r"(0) \
								: "memory");

#define EnablePrivilegedMode __asm("SVC #0")

/*---------------------------------------------------------------------------
	Hard Fault Handler
 *---------------------------------------------------------------------------*/
/* Bit masking. */
#define CHECK_BIT(REG, POS) ((REG) & (1u << (POS)))

/* Hard Fault Status Register. */
#define FORCED ((uint8_t)30u)
#define VECTTBL ((uint8_t)1u)

/* MemManage Fault Status Register (MMFSR; 0-7 bits in CFSR). */
#define MMARVALID ((uint8_t)7u)
#define MLSPERR ((uint8_t)5u) /**< Only on ARM Cortex-M4F. */
#define MSTKERR ((uint8_t)4u)
#define MUNSTKERR ((uint8_t)3u)
#define DACCVIOL ((uint8_t)1u)
#define IACCVIOL ((uint8_t)0u)

/* Bus Fault Status Register (BFSR; 8-15 bits in CFSR). */
#define BFARVALID ((uint8_t)15u)
#define LSPERR ((uint8_t)13u) /**< Only on ARM Cortex-M4F. */
#define STKERR ((uint8_t)12u)
#define UNSTKERR ((uint8_t)11u)
#define IMPRECISERR ((uint8_t)10u)
#define PRECISERR ((uint8_t)9u)
#define IBUSERR ((uint8_t)8u)

/* Usage Fault Status Register (BFSR; 16-25 bits in CFSR). */
#define DIVBYZERO ((uint8_t)25u) /**< Has to be enabled in CCR. */
#define UNALIGNED ((uint8_t)24u) /**< Has to be enabled in CCR. */
#define NOCP ((uint8_t)19u)
#define INVPC ((uint8_t)18u)
#define INVSTATE ((uint8_t)17u)
#define UNDEFINSTR ((uint8_t)16u)

/*---------------------------------------------------------------------------
	Hard Fault Handler Functions
 *---------------------------------------------------------------------------*/
void HardFault_Handler(void);
void BusFault_Handler(void);
void MemManage_Handler(void);
void UsageFault_Handler(void);
void SysTick_Handler(void);
void SVC_Handler(void);

/*---------------------------------------------------------------------------
	Setup the system timer
 *---------------------------------------------------------------------------*/
/* Systick Register address, refer datasheet for more info */
#define STCTRL (*((volatile unsigned long *)0xE000E010))
#define STRELOAD (*((volatile unsigned long *)0xE000E014))
#define STCURR (*((volatile unsigned long *)0xE000E018))

/*******STCTRL bits*******/
#define SBIT_ENABLE 0
#define SBIT_TICKINT 1
#define SBIT_CLKSOURCE 2

/* 100000000Mhz * 1ms = 1000000 - 1 */
#define RELOAD_VALUE 99999
#define LED 0 //P2_0

/*---------------------------------------------------------------------------
	Cortex-M DWT TIMER 
	Refer to https://www.embedded-computing.com/articles/measuring-code-execution-time-on-arm-cortex-m-mcus
 *---------------------------------------------------------------------------*/
#define DISABLE_IRQ __disable_irq()			// Set PRIMASK
#define ENABLE_IRQ __enable_irq()			// Clear PRIMASK
#define DISABLE_I_CACHE SCB_DisableICache() // disable instruction cache
#define ENABLE_I_CACHE SCB_EnableICache()	// enbale instruction cache
#define DISABLE_D_CACHE SCB_DisableDCache() // disable data cache
#define ENABLE_D_CACHE SCB_EnableDCache()	// enable data cache
#define ARM_CM_DEMCR (*(uint32_t *)0xE000EDFC)
#define ARM_CM_DWT_CTRL (*(uint32_t *)0xE0001000)	// DWT->CTRL (core_cm33.h, DWT_Type)
#define ARM_CM_DWT_CYCCNT (*(uint32_t *)0xE0001004) // DWT->CYCCNT
#define ELAPSED_TIME_MAX_SECTIONS 10

typedef struct elapsed_time
{
	uint32_t start;
	uint32_t current;
	uint32_t max;
	uint32_t min;
} ELAPSED_TIME;

void elapsed_time_clr(uint32_t i);		  // Clear measured values
void elapsed_time_init(void);			  // Module initialization
void elapsed_time_start(uint32_t i);	  // Start measurement
void elapsed_time_stop(uint32_t i);		  // Stop  measurement
uint32_t elapsed_time_result(uint32_t i); // Get the measurement result

/*---------------------------------------------------------------------------
Cortex-M DWT data comparator: CMSIS_DWT
	Refer to https://interrupt.memfault.com/blog/cortex-m-watchpoints
 *---------------------------------------------------------------------------*/
//DWT_FUNCTION controls the type of memory to watch and what will happen when a watchpoint event occurs.
/*
ID						Evaluation performed		 bits[31:27]	
0b00000 			Reserved.
0b01000 			Data Address, and Data Address With Value.
0b01001 			Cycle Counter, Data Address, and Data Address With Value.
0b01010 			Instruction Address, Data Address, and Data Address With Value.
0b01011 			Cycle Counter, Instruction Address, Data Address and Data Address With Value.
0b11000 			Data Address, Data Address Limit, and Data Address With Value.
0b11010 			Instruction Address, Instruction Address Limit, Data Address, Data Address Limit, and Data Address With Value.
0b11100 			Data Address, Data Address Limit, Data Value, Linked Data Value, and Data Address With Value.
0b11110 			Instruction Address, Instruction Address Limit, Data Address, Data Address Limit, Data value, Linked Data Value, and Data Address With Value.

Action				Event										bits[5:4]
0b00 					Trigger only.
0b01 					Generate debug event.
0b10 					For a Cycle Counter, Instruction Address, Data Address, Data Value or Linked Data Value comparator, generate a Data Trace Match packet. For a Data Address With Value comparator, generate a Data Trace Data Value packet.
0b11 					For a Data Address Limit comparator, generate a Data Trace Data Address packet

Match					Type										bits[3:0]
0b0001				Cycle count
0b0010				Instruction address
0x0011				Instruction address limit
0b0100				Data address, both read and write
0b0101				data address, writes
0b0110				data address, reads
0b0111				data address limit
0b1000				data value
0b1001				data value, writes
0b1010				data value, reads
0b1011				linked data value
0b1100				data address with value
0b1101				data address with value, writes
0b1110				data address with value, reads
*/
typedef struct
{
	volatile uint32_t COMP;
	volatile uint32_t MASK;
	volatile uint32_t FUNCTION;
	volatile uint32_t RSVD;
} sDwtCompCfg;

typedef struct
{
	char string[4];
	uint32_t my_variable;
} my_struct_t;

/*---------------------------------------------------------------------------
	Cortex-M33 ALIAS REGIONS
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

// test1: MTB_SECURE program a secure SRAM as secure or non-secure
// test2: how to read MTB buffer as DWT access
// test3: test if MTB record exception address pairs
// test4 debug monitor: https://interrupt.memfault.com/blog/cortex-m-debug-monitor#enabling-the-debugmonitor-exception

extern volatile ALIAS_REGION alias_region_tbl[MAX_ALIAS] __attribute__((section(".ARM.__at_0x282000a0")));

/*---------------------------------------------------------------------------
	Security attributes configuration
 *---------------------------------------------------------------------------*/
enum
{
	MPC1,
	MPC2,
	MPC3
};
void Board_Config_TZ(void);
void test_config_sau_s(uint32_t, uint32_t, uint32_t, uint32_t);
void test_config_mpc_s(uint32_t, uint32_t, unsigned long);
void test_config_ppc_s(void);

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

#define MTB_BUFF 4096
int mtb_resume(size_t mtb_size, int pos);
int mtb_pause();
int mtb_enable(size_t size);
int mtb_disable(void);

/*---------------------------------------------------------------------------
	Functions declaration for test_caeses_s.c
 *---------------------------------------------------------------------------*/
typedef int (*funcPtr)(int);
uint32_t read_memory_s(uint32_t);
uint32_t write_memory_s(uint32_t, uint32_t);
float increase_lcd_pos_s(float *);
void print_case_info(float *pos, char *case_name);

void arm_mpu_config_s(void);
void mpu_config_mpsssram3_ns_s(void);
void mpu_mpc_rom_config(void);
void trigger_pendsv(void);
void trigger_nvic_int0(void);
void apSleep(unsigned int msec);
void memcpy(void *dest, void *src, int n);
void test_cases_s(uint32_t test_id);
void test_cases();