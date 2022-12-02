#ifndef TEST_H
#define TEST_H
/*
test cases for cortex-m3 processsor
*/

#include <stdio.h>
#include "Device.h"			// Keil::Board Support:V2M-MPS2:Common
#include "RTE_Components.h" // Component selection
#include "Board_LED.h"		// ::Board Support:LED
#include "Board_Buttons.h"	// ::Board Support:Buttons
#include "Board_Touch.h"	// ::Board Support:Touchscreen
#include "Board_GLCD.h"		// ::Board Support:Graphic LCD
#include "GLCD_Config.h"	// Keil.SAM4E-EK::Board Support:Graphic LCD
#include "SMM_MPS2.h"

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
//#define TEST_SYSTICK

/* test cases */
enum test_s
{
	TEST_SVC,
	TEST_DWT,
	TEST_MPU
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
#define MPY_TYPE *((volatile uint32_t *)0xE000ED90)
#define MPU_CTRL *((volatile uint32_t *)0xE000ED94)
#define MPU_RNR *((volatile uint32_t *)0xE000ED98)
#define MPU_RBAR *((volatile uint32_t *)0xE000ED9C)
#define MPU_RASR *((volatile uint32_t *)0xE000EDA0)
*/

/* General MPU Masks */
#define REGION_VALID 0x10
#define NORMAL (8 << 16)		 // TEX:0b001 S:0b0 C:0b0 B:0b0
#define FULL_ACCESS (0x03 << 24) // Privileged Read Write, Unprivileged Read Write
#define NOT_EXEC (0x01 << 28)	 // All Instruction fetches abort

#define REGION_ENABLE 0x00000001
/* Shareability */
#define NON_SHAREABLE 0x00
#define RESERVED 0x08
#define OUTER_SHAREABLE 0x10
#define INNER_SHAREABLE 0x18

/* Access Permissions */
/* check mpu_armv7.h 
#define ARM_MPU_AP_NONE 0U ///!< MPU Access Permission no access
#define ARM_MPU_AP_PRIV 1U ///!< MPU Access Permission privileged access only
#define ARM_MPU_AP_URO  2U ///!< MPU Access Permission unprivileged access read-only
#define ARM_MPU_AP_FULL 3U ///!< MPU Access Permission full access
#define ARM_MPU_AP_PRO  5U ///!< MPU Access Permission privileged access read-only
#define ARM_MPU_AP_RO   6U ///!< MPU Access Permission read-only access
*/

/* Read/Write Allocation Configurations for Cacheable Memory Attr<n>[7:4] and Attr<n>[3:0] are of the format: 0bXXRW */
#define R_NON_W_NON 0x0		/* Do not allocate Read/Write */
#define R_NON_W_ALLOC 0x1	/* Do not allocate Read, Allocate Write */
#define R_ALLOC_W_NON 0x2	/* Allocate Read, Do not allocate Write */
#define R_ALLOC_W_ALLOC 0x3 /* Allocate Read/Write */

/*---------------------------------------------------------------------------
	System Privilege Modification
 *---------------------------------------------------------------------------*/
#define DROP_S_PRIVILEGES __asm("msr CONTROL, %[reg] " ::[reg] "r"(1) \
								: "memory")
#define SET_S_PRIVILEGES __asm("msr CONTROL, %[reg] " ::[reg] "r"(0) \
							   : "memory")

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
	Functions declaration for test_caeses_s.c
 *---------------------------------------------------------------------------*/
typedef int (*funcPtr)(int);
float increase_lcd_pos_s(float *);
void print_case_info(float *pos, char *case_name);

void apSleep(unsigned int msec);
void memcpy(void *dest, void *src, int n);
void test_cases();

void arm_mpu_config(void);

#endif