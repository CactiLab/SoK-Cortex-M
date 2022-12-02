#include "test_ns.h"

float increase_lcd_pos_ns(float *pos)
{
  *pos += 0.5;
  if (*pos < 20)
    return *pos - 0.5;
  return 0;
}

void SysTick_Handler(void)
{
  static uint32_t ticks;
  ticks++;
}

void mpu_config_mpsssram3_ns(void)
{
#ifdef TEST_IGNORE_ISB
#else
  __asm volatile("dmb\n"); /* Force any outstanding transfers to complete before disabling MPU */
#endif
  MPU->CTRL = 0;                                                                     /* Disable MPU */
  uint32_t MPU_regions = (MPU->TYPE & MPU_TYPE_DREGION_Msk) >> MPU_TYPE_DREGION_Pos; /* Check number of supported MPU regions */

  /* Configure MPU Memory Attribution Indirection Registers */
  uint32_t non_cacheable_attr = NORMAL_O_NC | NORMAL_I_NC;
  uint32_t device_attr = DEVICE_NG_NR_NE;
  MPU->MAIR0 = MPU->MAIR0 | ((device_attr << MPU_MAIR0_Attr1_Pos) | (non_cacheable_attr));
  /* MPU_MAIR0 index 0: SRAM1, SRAM2&3, System ROM MPU_MAIR0 index 1: Device */

  /* configure unsecure data area MPSSSRAM3 (0x28200000 - 0x283FFFFF) */
  /* Configure region 7: NS SRAM1 (Normal, Non-Shareable, RW, Privilege only)*/
  MPU->RNR = 7;
  MPU->RBAR = (0x28200000 & MPU_RBAR_ADDR_Msk) | NON_SHAREABLE | RW_P_ONLY;
  MPU->RLAR = (0x283FFFFF & MPU_RLAR_LIMIT_Msk) | ((0 << MPU_RLAR_AttrIndx_Pos) & MPU_RLAR_AttrIndx_Msk) | REGION_ENABLE;

  /* Disable all other regions */
  for (uint32_t i = 0; (i < MPU_regions) && (i != 7); i++)
  {
    MPU->RNR = i;
    MPU->RLAR &= 0x0;
  }
  /* Enable MPU and the default memory map as a background region (acts as region number -1) for privileged access only (MPU_CTRL.PRIVDEFENA). */
  MPU->CTRL = 5;
  /* Insert DSB followed by ISB */
#ifdef TEST_IGNORE_ISB
#else
  __asm volatile("dsb\n"
                 "isb\n");
#endif
}

/*---------------------------------------------------------------------------
	MPU configuration case2: protect the memory
 *---------------------------------------------------------------------------*/
void arm_mpu_config_ns(void)
{
#ifdef TEST_IGNORE_ISB
#else
  __asm volatile("dmb\n"); /* Force any outstanding transfers to complete before disabling MPU */
#endif
  MPU->CTRL = 0;                                                                     /* Disable MPU */
  uint32_t MPU_regions = (MPU->TYPE & MPU_TYPE_DREGION_Msk) >> MPU_TYPE_DREGION_Pos; /* Check number of supported MPU regions */

  /* Configure MPU Memory Attribution Indirection Registers */
  uint32_t non_cacheable_attr = NORMAL_O_NC | NORMAL_I_NC;
  uint32_t device_attr = DEVICE_NG_NR_NE;
  MPU->MAIR0 = MPU->MAIR0 | ((device_attr << MPU_MAIR0_Attr1_Pos) | (non_cacheable_attr));
  /* MPU_MAIR0 index 0: SRAM1, SRAM2&3, System ROM MPU_MAIR0 index 1: Device */

  /* Configure region 0: NS SRAM1 code (Normal, Non-Shareable, RO, Any Privilege Level)*/
  MPU->RNR = 0;
  MPU->RBAR = (0x00200000 & MPU_RBAR_ADDR_Msk) | NON_SHAREABLE | RO_P_U;
  MPU->RLAR = (0x003FFFFF & MPU_RLAR_LIMIT_Msk) | ((0 << MPU_RLAR_AttrIndx_Pos) & MPU_RLAR_AttrIndx_Msk) | REGION_ENABLE;
  /* Configure region 1: S SRAM1 code (Normal, Non-Shareable, RO, Any Privilege Level) */ /* (Normal, Non-Shareable, RO, Any Privilege Level) (0x3 & MPU_RBAR_ADDR_Msk) | NON_SHAREABLE | RW_P_U | EXEC_NEVER */
  MPU->RNR = 1;
  MPU->RBAR = (0x10000000 & MPU_RBAR_ADDR_Msk) | NON_SHAREABLE | RO_P_U;
  MPU->RLAR = (0x101FFFFF & MPU_RLAR_LIMIT_Msk) | ((0 << MPU_RLAR_AttrIndx_Pos) & MPU_RLAR_AttrIndx_Msk) | REGION_ENABLE;
  /* Configure region 2: NS SRAM2 data (Normal, Non-Shareable, RW, Any Privilege Level) */
  MPU->RNR = 2;
  MPU->RBAR = (0x28200000 & MPU_RBAR_ADDR_Msk) | NON_SHAREABLE | RW_P_U;
  MPU->RLAR = (0x283FFFFF & MPU_RLAR_LIMIT_Msk) | ((0 << MPU_RLAR_AttrIndx_Pos) & MPU_RLAR_AttrIndx_Msk) | REGION_ENABLE;
  /* Configure region 3: NS CMSDK APB (Device-nGnRnE, Non-Shareable, RW, Any Privilege Level, XN) */
  MPU->RNR = 3;
  MPU->RBAR = (0x40000000 & MPU_RBAR_ADDR_Msk) | NON_SHAREABLE | RW_P_U | EXEC_NEVER;
  MPU->RLAR = (0x4000FFFF & MPU_RLAR_LIMIT_Msk) | ((1 << MPU_RLAR_AttrIndx_Pos) & MPU_RLAR_AttrIndx_Msk) | REGION_ENABLE;
  /* Configure region 3: NS GPIO (Device-nGnRnE, Non-Shareable, RW, Any Privilege Level, XN) */
  MPU->RNR = 4;
  MPU->RBAR = (0x40010000 & MPU_RBAR_ADDR_Msk) | NON_SHAREABLE | RW_P_U | EXEC_NEVER;
  MPU->RLAR = (0x40013FFF & MPU_RLAR_LIMIT_Msk) | ((1 << MPU_RLAR_AttrIndx_Pos) & MPU_RLAR_AttrIndx_Msk) | REGION_ENABLE;
  /* Configure region 4: NS Default AHB Slave (Device-nGnRnE, Non-Shareable, RO, Any Privilege Level, XN) */
  MPU->RNR = 5;
  MPU->RBAR = (0x40014000 & MPU_RBAR_ADDR_Msk) | NON_SHAREABLE | RO_P_U | EXEC_NEVER;
  MPU->RLAR = (0x40017FFF & MPU_RLAR_LIMIT_Msk) | ((1 << MPU_RLAR_AttrIndx_Pos) & MPU_RLAR_AttrIndx_Msk) | REGION_ENABLE;
  /* Configure region 5: System ROM (Normal, Non-Shareable, RO, Any Privilege Level) */
  MPU->RNR = 6;
  MPU->RBAR = (0xF0000000 & MPU_RBAR_ADDR_Msk) | NON_SHAREABLE | RO_P_U;
  MPU->RLAR = (0xF0000FFF & MPU_RLAR_LIMIT_Msk) | ((0 << MPU_RLAR_AttrIndx_Pos) & MPU_RLAR_AttrIndx_Msk) | REGION_ENABLE;
  /* Disable all other regions, because region 6 is used to protect MPCSSRAM1, it starts from 7, check later*/
  for (uint32_t i = 7; i < MPU_regions; i++)
  {
    MPU->RNR = i;
    MPU->RLAR &= 0x0;
  }
  /* Enable MPU and the default memory map as a background region (acts as region number -1) for privileged access only (MPU_CTRL.PRIVDEFENA). */
  MPU->CTRL = 5;
  /* Insert DSB followed by ISB */
#ifdef TEST_IGNORE_ISB
#else
  __asm volatile("dsb\n"
                 "isb\n");
#endif
}

/* configure the mpu region
input: mpu region number, start_address, end_address, 
				security attributs, 
*/
void test_config_mpu_ns(uint32_t region_id, uint32_t start, uint32_t end, uint32_t attr)
{
#ifdef TEST_IGNORE_ISB
#else
  __asm volatile("dmb\n"); /* Force any outstanding transfers to complete before disabling MPU */
#endif
  MPU->CTRL = 0;                                                                     /* Disable MPU */
  uint32_t MPU_regions = (MPU->TYPE & MPU_TYPE_DREGION_Msk) >> MPU_TYPE_DREGION_Pos; /* Check number of supported MPU regions */

  /* Configure MPU Memory Attribution Indirection Registers */
  uint32_t non_cacheable_attr = NORMAL_O_NC | NORMAL_I_NC;
  uint32_t device_attr = DEVICE_NG_NR_NE;
  MPU->MAIR0 = MPU->MAIR0 | ((device_attr << MPU_MAIR0_Attr1_Pos) | (non_cacheable_attr));
  /* MPU_MAIR0 index 0: SRAM1, SRAM2&3, System ROM MPU_MAIR0 index 1: Device */

  /* Configure region*/
  if (region_id > MPU_regions)
  {
    MPU->CTRL = 5;
    __asm volatile("dsb\n"
                   "isb\n");
    return;
  }
  MPU->RNR = region_id;
  MPU->RBAR = (start & MPU_RBAR_ADDR_Msk) | attr;
  MPU->RLAR = (end & MPU_RLAR_LIMIT_Msk) | ((0 << MPU_RLAR_AttrIndx_Pos) & MPU_RLAR_AttrIndx_Msk) | REGION_ENABLE;

  /* Enable MPU and the default memory map as a background region (acts as region number -1) for privileged access only (MPU_CTRL.PRIVDEFENA). */
  MPU->CTRL = 5;
  /* Insert DSB followed by ISB */
#ifdef TEST_IGNORE_ISB
#else
  __asm volatile("dsb\n"
                 "isb\n");
#endif
}

/*---------------------------------------------------------------------------
	Cortex-M DWT TIMER 
	Refer to https://www.embedded-computing.com/articles/measuring-code-execution-time-on-arm-cortex-m-mcus
 *---------------------------------------------------------------------------*/
/* fix this address, non-secure IRAM starts with 0x28200000, we can fix our time table at very beginning of this address */
volatile ELAPSED_TIME elapsed_time_tbl[ELAPSED_TIME_MAX_SECTIONS] __attribute__((section(".ARM.__at_0x28200000")));

/* init function must be called at the very beginning of measurement */
void elapsed_time_init(void)
{
  uint32_t i;

  if (ARM_CM_DWT_CTRL != 0)
  {                          // See if DWT is available
    ARM_CM_DEMCR |= 1 << 24; // Set bit 24
    ARM_CM_DWT_CYCCNT = 0;
    ARM_CM_DWT_CTRL |= 1 << 0; // Set bit 0
  }
  for (i = 0; i < ELAPSED_TIME_MAX_SECTIONS; i++)
  {
    elapsed_time_clr(i);
  }
}

/* start the measurement */
void elapsed_time_start(uint32_t i)
{
  elapsed_time_tbl[i].start = ARM_CM_DWT_CYCCNT;
}

/* stop the measurement */
void elapsed_time_stop(uint32_t i)
{
  uint32_t stop;
  ELAPSED_TIME *p_tbl;

  stop = ARM_CM_DWT_CYCCNT;
  p_tbl = &elapsed_time_tbl[i];
  p_tbl->current = stop - p_tbl->start;
  if (p_tbl->max < p_tbl->current)
  {
    p_tbl->max = p_tbl->current;
  }
  if (p_tbl->min > p_tbl->current)
  {
    p_tbl->min = p_tbl->current;
  }
}

/* clear the measurement */
void elapsed_time_clr(uint32_t i)
{
  ELAPSED_TIME *p_tbl;

  p_tbl = &elapsed_time_tbl[i];
  p_tbl->start = 0;
  p_tbl->current = 0;
  p_tbl->min = 0xFFFFFFFF;
  p_tbl->max = 0;
}

/* Get the measurement result */
uint32_t elapsed_time_result(uint32_t i)
{
  ELAPSED_TIME *p_tbl;
  p_tbl = &elapsed_time_tbl[i];
  return p_tbl->current;
}

/* change privileged mode */
void SVC_Handler_Main(unsigned int *svc_args)
{
//DISABLE_IRQ;
#ifdef MISSED_CYCLES
  elapsed_time_start(2);
#endif
  uint32_t svc_number;
  /* stack contains: r0, r1, r2, r3, r12, r14, the return address and xPSR */
  svc_number = ((char *)svc_args[6])[-2];
  switch (svc_number)
  {
  case 0: /* EnablePrivilegeMode */
    __set_CONTROL(__get_CONTROL() & ~CONTROL_nPRIV_Msk);
    break;
  case 10: /* call NSC function, 10, 11, 13, -5 */
    test_add_s(13, 13);
    break;
  case 12: /* 12. access secure region to trigger secure fault */
    test_access_ns();
    break;
  case 20:
    DROP_NS_PRIVILEGES;
    break;
  case 28: /* 2, 8, 13, 14. xx <--> non-secure privilege handler: exception */
    break;
  case 66: /* stack manipulation: fake FNC_RETURN */
    __asm volatile(
        "MOV LR, #0xFEFFFFFF;"
        "BX LR;");
    break;
  default:
    break;
  }
}

void SVC_Handler(void)
{
  __asm volatile(
      "TST    LR, #0b0100;      "
      "ITE    EQ;               "
      "MRSEQ  R0, MSP;          "
      "MRSNE  R0, PSP;          "
      "MOV    R1, LR;           "
      "B      SVC_Handler_Main;  ");
}

uint32_t check_cache_ns()
{
  uint32_t ctr_value;
  ctr_value = ((SCB->CTR) << SCB_CTR_FORMAT_Pos) & SCB_CTR_FORMAT_Msk;
  return ctr_value;
}

uint32_t read_memory_ns(uint32_t addr)
{
  uint32_t *read_addr = (uint32_t *)addr; // reserved location
  return *read_addr;                      // try to read from reserved location
}

uint32_t write_memory_ns(uint32_t addr, uint32_t value)
{
  char *write_addr = (char *)addr;
  *write_addr = value;
  return 0;
}

void SwitchStackPointer_ns()
{
  // store MSP
  int stack = __get_MSP();
  // change MSP to new location
  __set_MSP(stack);
  // store current stack pointer is PSP
  __set_PSP(stack);
  // switch to PSP
  __set_CONTROL(0x2);
}

/* 
 * Function for MTB
 */

int mtb_enable_ns(size_t mtb_size)
{
  if ((mtb_size < 16) || (__builtin_popcount(mtb_size) != 1))
  {
    // MTB must be at least 16 bytes and be a power of 2
    return -1;
  }
  // configure the MTB buffer as S
  //test_config_sau_s(4, 0x24000000, 0x24003FFF, 1); // already configured 0-3
  // then separate the buffer into equally secure and non-secure: 00x2400_1FFFF
  // 100100000000000001111111111111
  //MTB->SECURE = 0x24001FFF;
  // scrub MTB SRAM so it's easy to see what has gotten written to
  memset((void *)MTB->BASE, 0x0, mtb_size);
  const int mask = __builtin_ctz(mtb_size) - 4;

  // we are about to reconfigure so turn off MTB
  mtb_disable_ns();

  // reset position
  MTB->POSITION = 0;

  // Start tracing!
  MTB->MASTER = (1 << 31) | (mask << 0);
  return 0;
}

int mtb_resume_ns(size_t mtb_size, int pos)
{
  if ((mtb_size < 16) || (__builtin_popcount(mtb_size) != 1))
  {
    // MTB must be at least 16 bytes and be a power of 2
    return -1;
  }

  // scrub MTB SRAM so it's easy to see what has gotten written to
  //memset((void *)MTB->BASE + pos, 0x0, mtb_size - pos);
  const int mask = __builtin_ctz(mtb_size) - 4;

  // we are about to reconfigure so turn off MTB
  mtb_disable_ns();

  // reset position
  //int pos = MTB->POSITION;
  //MTB->POSITION = pos;

  // Start tracing!
  MTB->MASTER = (1 << 31) | (mask << 0);
  return 0;
}

int mtb_pause_ns()
{
  // we are about to reconfigure so turn off MTB
  mtb_disable_ns();

  // read the pause position, should be - 4 because the mtb_disable() uses a pair os addresses
  int pos = MTB->POSITION;
  //MTB->POSITION = 0;

  // Start tracing!
  return pos;
}

int mtb_disable_ns(void)
{
  MTB->MASTER &= ~(1 << 31);
  return 0;
}