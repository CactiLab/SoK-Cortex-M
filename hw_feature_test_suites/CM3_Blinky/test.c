/*
test cases for cortex-m3 processsor
dwt, and mpu
*/

#include "test.h" /* test header */
static char msg[MAXMSG];

/*---------------------------------------------------------------------------
	Other configuration
 *---------------------------------------------------------------------------*/
uint8_t check_first_non_zero_bit(uint32_t value)
{
   int i = 0;
   for (i = 0; i < 32; i++)
   {
      if (CHECK_BIT(value, i))
         return i;
   }
   return 0xff;
}

float increase_lcd_pos_s(float *pos)
{
   *pos += 0.5;
   if (*pos < 20)
      return *pos - 0.5;
   return 0;
}

void reset_lcd_pos(float *pos)
{
   *pos = 0;
   GLCD_ClearScreen();
}

void lcd_print_dynamic(float *pos, char *msg)
{
   GLCD_DrawString(0.5 * 16, (*pos) * 24, msg);
   increase_lcd_pos_s(pos);
}

void print_case_info(float *pos, char *case_name)
{
   reset_lcd_pos(pos);
   sprintf(msg, "Test in secure state: %s", case_name);
   lcd_print_dynamic(pos, msg);
}

/*---------------------------------------------------------------------------
	Fault_Handler configuration
 *---------------------------------------------------------------------------*/

void stackDump(sContextStateFrame *stack)
{
   sprintf(msg, "r0  = 0x%08x.", stack->r0);
   lcd_print(3.5, msg);
   sprintf(msg, "r1  = 0x%08x.", stack->r1);
   lcd_print(4, msg);
   sprintf(msg, "r2  = 0x%08x.", stack->r2);
   lcd_print(4.5, msg);
   sprintf(msg, "r3  = 0x%08x.", stack->r3);
   lcd_print(5, msg);
   sprintf(msg, "r12 = 0x%08x.", stack->r12);
   lcd_print(5.5, msg);
   sprintf(msg, "lr  = 0x%08x.", stack->lr);
   lcd_print(6, msg);
   sprintf(msg, "pc  = 0x%08x.", stack->pc);
   lcd_print(6.5, msg);
   sprintf(msg, "psr = 0x%08x.", stack->psr);
   lcd_print(7, msg);
}

void dummy_handler(sContextStateFrame *stack)
{
   __ASM volatile("BKPT #02");

   while (1)
      ;
}

/*---------------------------------------------------------------------------
Recover the CPU state only when usage fault, others will reboot the system
 *---------------------------------------------------------------------------*/
void sys_recover(sContextStateFrame *stack)
{
   __ASM volatile("BKPT #02");

   // clear any logged faults from CFSR
   SCB->CFSR |= SCB->CFSR;
   // return to where exit from the exception
   stack->pc = stack->lr;
   // set the lr a pattern
   stack->lr = 0xdeadbeef;

   // reset the psr state and only leave the "thumb instruction interworking" bit set
   stack->psr = (1 << 24);
}

/*---------------------------------------------------------------------------
Reboot the system for unrecoverable fault: Application interrupt and reset control register (AIRCR-0xE000ED0C)
 *---------------------------------------------------------------------------*/
void sys_reset()
{
   __ASM volatile("BKPT #02");

   SCB->AIRCR = (0x05FA << 16) | 0x1 << 2;
   while (1)
      ;
}

void clear_stack(sContextStateFrame *stack)
{
   __asm volatile(
       "MOV    R0, #0;      "
       "MOV    R1, R0;       "
       "MOV    R2, R0;       "
       "MOV    R3, R0;       "
       "MOV    R4, R0;       "
       "MOV    R5, R0;       "
       "MOV    R6, R0;       "
       "MOV    R7, R0;       "
       "MOV    R12, R0;       ");
}

void printUsageErrorMsg(uint32_t CFSRValue)
{
   GLCD_ClearScreen();
   lcd_print(1.5, "Usage fault: ");
   uint8_t pos = 0xff;
   pos = check_first_non_zero_bit(CFSRValue);

   switch (pos)
   {
   case DIVBYZERO:
      lcd_print(2, "*Divide by 0.");
      break;
   case UNALIGNED:
      lcd_print(2, "*Unaligned memory access.");
      break;
   case NOCP:
      lcd_print(2, "*Attempted to access a coprocessor.");
      break;
   case INVPC:
      lcd_print(2, "*Illegal attempt to load of EXC_RETURN to the PC.");
      break;
   case INVSTATE:
      lcd_print(2, "*Instruction that makes illegal use of the EPSR.");
      break;
   case UNDEFINSTR:
      lcd_print(2, "*Execute an undefined instruction.");
      break;
   default:
      break;
   }
}

void printBusFaultErrorMsg(uint32_t CFSRValue)
{
   // lcd_print(1, "-------------------------------");
   lcd_print(1.5, "Bus fault: ");

   uint8_t pos = 0xff;
   pos = check_first_non_zero_bit(CFSRValue);

   switch (pos)
   {
   case BFARVALID:
      lcd_print(2, "*BFAR holds a valid address.");
      break;
   case LSPERR:
      lcd_print(2, "*Floating-point lazy state preservation.");
      break;
   case STKERR:
      lcd_print(2, "*Stacking has caused a Bus fault.");
      break;
   case UNSTKERR:
      lcd_print(2, "*Unstacking has caused a Bus fault.");
      break;
   case IMPRECISERR:
      lcd_print(2, "*Data bus error - ");
      lcd_print(2.5, "Ret in the stack is not related to the fault.");
      break;
   case PRECISERR:
      lcd_print(2, "*Data bus error - ");
      lcd_print(2.5, "Ret points to the instruction that caused the fault.");
      break;
   case IBUSERR:
      lcd_print(2, "*Instruction bus error.");
      break;
   default:
      lcd_print(2, "*BFAR holds an invalid address.");
      break;
   }
}

void printMemoryManagementErrorMsg(uint32_t CFSRValue)
{
   // lcd_print(1, "-------------------------------");
   lcd_print(1.5, "Memory Management fault: ");

   uint8_t pos = 0xff;
   pos = check_first_non_zero_bit(CFSRValue);

   switch (pos)
   {
   case MMARVALID:
      lcd_print(2, "*MMAR holds a valid address.");
      break;
   case MLSPERR:
      lcd_print(2, "*Floating-point lazy state preservation.");
      break;
   case MSTKERR:
      lcd_print(2, "*Stacking has caused an access violation.");
      break;
   case MUNSTKERR:
      lcd_print(2, "*Unstacking has caused an access violation.");
      break;
   case DACCVIOL:
      lcd_print(2, "*Not allowed to load/store at a location.");
      break;
   case IACCVIOL:
      lcd_print(2, "*Execution prevented.");
      break;
   default:
      lcd_print(2, "*MMAR holds an invalid address.");
      break;
   }
}

void Hard_Fault_Handler(sContextStateFrame *stack)
{
   GLCD_ClearScreen();
   lcd_print(0.5, "In Hard Fault Handler");
   sprintf(msg, "SCB->HFSR = 0x%08x, SCB->CFSR = 0x%08x.", SCB->HFSR, SCB->CFSR);
   lcd_print(1, msg);

   if ((SCB->HFSR & (1 << 30)) != 0)
   {
      if ((SCB->CFSR & 0xFFFF0000) != 0)
      {
         printUsageErrorMsg(SCB->CFSR & 0xFFFF0000);
      }
      if ((SCB->CFSR & 0xFF00) != 0)
      {
         printBusFaultErrorMsg(SCB->CFSR & 0xFF00);
      }
      if ((SCB->CFSR & 0xFF) != 0)
      {
         printMemoryManagementErrorMsg(SCB->CFSR & 0xFF);
      }
      else
         lcd_print(1.5, "Unidentified fault.");
   }
   stackDump(stack);
   //dummy_handler(stack);
   sys_reset();
}

void Bus_Fault_Handler(sContextStateFrame *stack)
{
   GLCD_ClearScreen();
   sprintf(msg, "SCB->CFSR = 0x%08x.", SCB->CFSR);
   lcd_print(0.5, msg);
   printBusFaultErrorMsg(SCB->CFSR & 0xFF00);
   stackDump(stack);
   //dummy_handler(stack);
   sys_reset();
}
void Mem_Fault_Handler(sContextStateFrame *stack)
{
   GLCD_ClearScreen();
   sprintf(msg, "SCB->CFSR = 0x%08x.", SCB->CFSR);
   lcd_print(0.5, msg);
   printMemoryManagementErrorMsg(SCB->CFSR & 0xFF);
   stackDump(stack);
   //dummy_handler(stack);
   sys_reset();
}

void Usage_Fault_Handler(sContextStateFrame *stack)
{
   GLCD_ClearScreen();
   sprintf(msg, "SCB->CFSR = 0x%08x.", SCB->CFSR);
   lcd_print(0.5, msg);
   printUsageErrorMsg(SCB->CFSR & 0xFFFF0000);
   stackDump(stack);
   //dummy_handler(stack);
   sys_recover(stack);
}

void Pend_SV_Handler(sContextStateFrame *stack)
{
   GLCD_ClearScreen();
   __asm("bkpt 1");
   uint32_t ICSRValue = SCB->ICSR;
   uint32_t act_num = ICSRValue >> 11 & 0x1;
   uint32_t excp_num = ICSRValue >> 12 & 0xff;
   int IRQ_num = ICSRValue & 0xff;
   sprintf(msg, "SCB->ICSR = 0x%08x.", ICSRValue);
   lcd_print(0.5, msg);
   sprintf(msg, "Active exception: #%d; remain exception: #%d.", act_num, excp_num);
   lcd_print(1, msg);
   lcd_print(1.5, "Pend_SV_Handler");
   switch (IRQ_num)
   {
   case HardFault_IRQn + 16:
      lcd_print(2, "HardFault");
      break;
   case MemoryManagement_IRQn + 16:
      lcd_print(2, "MemoryManagement Fault");
      break;
   case BusFault_IRQn + 16:
      lcd_print(2, "Bus Fault");
      break;
   case UsageFault_IRQn + 16:
      lcd_print(2, "Usage Fault");
      break;
   case SysTick_IRQn + 16:
      lcd_print(2, "Syatick");
      break;
   case PendSV_IRQn + 16:
      lcd_print(2, "PendSV Exception");
      break;
   default:
      break;
   }
   sys_recover(stack);
}

/* change privileged mode */
void SVC_Handler_Main(unsigned int *svc_args)
{
   //	DISABLE_IRQ;
   //	#ifdef MISSED_CYCLES
   //		elapsed_time_start(2);
   //	#endif
   uint32_t svc_number;
   svc_number = ((char *)svc_args[6])[-2];
   switch (svc_number)
   {
   case 0: /* EnablePrivilegeMode*/
      __set_CONTROL(__get_CONTROL() & ~CONTROL_nPRIV_Msk);
      break;
   case 10: /* call NSC function, 10, 11, 13, -5 */
      break;
   case 20:
      DROP_S_PRIVILEGES;
      break;
   case 28: /* 2, 8, 13, 14. xx <--> non-secure privilege handler: exception */
      break;
   case 66: /* drop privilege in handler mode */
      DROP_S_PRIVILEGES;
      __asm("SVC #77");
      break;
   case 77: /* response the svc 66*/
      break;
   default:
      break;
   }
}

/*---------------------------------------------------------------------------
	MPU configuration case2: protect the memory

FLASH:0x00000000 0x0007FFFF RAM 		Replaces subsystem FLASH RAM
Code: 0x00400000 0x007FFFFF ZBT1 		User Memory
Data: 0x20400000 0x207FFFFF ZBT2&3 	User Memory
0x40010000 0x40010FFF GPIO0
0x40011000 0x40011FFF GPIO1
0x40030000 0x40030FFF GPIO4
0x40031000 0x40031FFF GPIO5

#define ARM_MPU_AP_NONE 0U ///!< MPU Access Permission no access
#define ARM_MPU_AP_PRIV 1U ///!< MPU Access Permission privileged access only
#define ARM_MPU_AP_URO  2U ///!< MPU Access Permission unprivileged access read-only
#define ARM_MPU_AP_FULL 3U ///!< MPU Access Permission full access
#define ARM_MPU_AP_PRO  5U ///!< MPU Access Permission privileged access read-only
#define ARM_MPU_AP_RO   6U ///!< MPU Access Permission read-only access
 *---------------------------------------------------------------------------*/
void arm_mpu_config(void)
{
#ifdef TEST_IGNORE_ISB
#else
   __asm volatile("dmb\n"); /* Force any outstanding transfers to complete before disabling MPU */
#endif
   ARM_MPU_Disable();                                                                 /* Disable MPU */
   uint32_t MPU_regions = (MPU->TYPE & MPU_TYPE_DREGION_Msk) >> MPU_TYPE_DREGION_Pos; /* Check number of supported MPU regions */

   /* Configure MPU Memory Attribution Indirection Registers */
   //uint32_t non_cacheable_attr = NORMAL_O_NC | NORMAL_I_NC;
   //uint32_t device_attr = DEVICE_NG_NR_NE;
   //MPU->MAIR0 = MPU->MAIR0 | ((device_attr << MPU_MAIR0_Attr1_Pos) | (non_cacheable_attr));
   /* MPU_MAIR0 index 0: SRAM1, SRAM2&3, System ROM MPU_MAIR0 index 1: Device */

   /* Configure region 0: Flash (Normal, Non-Shareable, RO, Any Privilege Level)*/
   MPU->RBAR = 0x00000000 | REGION_VALID | 0;
   //ARM_MPU_RBAR(0, 0x00200000);
   MPU->RASR = REGION_ENABLE | NORMAL | ARM_MPU_REGION_SIZE_512KB | ARM_MPU_AP_PRO;
   //ARM_MPU_RASR(DisableExec, AccessPermission, TypeExtField, IsShareable, IsCacheable, IsBufferable, SubRegionDisable, Size)

   /* Configure region 1: S SRAM1 code (Normal, Non-Shareable, RO, Any Privilege Level) */ /* (Normal, Non-Shareable, RO, Any Privilege Level) (0x3 & MPU_RBAR_ADDR_Msk) | NON_SHAREABLE | RW_P_U | EXEC_NEVER */
                                                                                           /* Configure to RW for SG instruction testing*/

   /* Configure region 2: NS SRAM3 data (Normal, Non-Shareable, RW, Any Privilege Level) */

   /* Configure region 3: NS CMSDK APB (Device-nGnRnE, Non-Shareable, RW, Any Privilege Level, XN) */

   /* Configure region 3: NS GPIO (Device-nGnRnE, Non-Shareable, RW, Any Privilege Level, XN) */

   /* Configure region 4: NS Default AHB Slave (Device-nGnRnE, Non-Shareable, RO, Any Privilege Level, XN) */

   /* Configure region 5: System ROM (Normal, Non-Shareable, RO, Any Privilege Level) */

   /* Disable all other regions, because region 6 is used to protect MPCSSRAM1, it starts from 7, check later*/
   for (uint32_t i = 1; i < MPU_regions; i++)
   {
      ARM_MPU_ClrRegion(i);
   }
   /* Enable MPU and the default memory map as a background region (acts as region number -1) for privileged access only (MPU_CTRL.PRIVDEFENA). */
   MPU->CTRL = 5;
   //ARM_MPU_Enable(ARM_MPU_AP_PRIV);
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
void test_config_mpu(uint32_t region_id, uint32_t start, uint32_t size, uint32_t attr)
{
#ifdef TEST_IGNORE_ISB
#else
   __asm volatile("dmb\n"); /* Force any outstanding transfers to complete before disabling MPU */
#endif
   MPU->CTRL = 0;                                                                     /* Disable MPU */
   uint32_t MPU_regions = (MPU->TYPE & MPU_TYPE_DREGION_Msk) >> MPU_TYPE_DREGION_Pos; /* Check number of supported MPU regions */

   /* Configure MPU Memory Attribution Indirection Registers */
   //uint32_t non_cacheable_attr = NORMAL_O_NC | NORMAL_I_NC;
   //uint32_t device_attr = DEVICE_NG_NR_NE;
   //MPU->MAIR0 = MPU->MAIR0 | ((device_attr << MPU_MAIR0_Attr1_Pos) | (non_cacheable_attr));
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
   //MPU->RLAR = (end & MPU_RLAR_LIMIT_Msk) | ((0 << MPU_RLAR_AttrIndx_Pos) & MPU_RLAR_AttrIndx_Msk) | REGION_ENABLE;

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
   {                           // See if DWT is available
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

/*---------------------------------------------------------------------------
Cortex-M DWT data comparator: CMSIS_DWT
	Refer to https://interrupt.memfault.com/blog/cortex-m-watchpoints
 *---------------------------------------------------------------------------*/
void dwt_dump(void)
{
   sDwtCompCfg *COMP_CONFIG = &DWT->COMP0;
   const size_t num_comparators = (DWT->CTRL >> 28) & 0xF;
   for (size_t i = 0; i < num_comparators; i++)
   {
      const sDwtCompCfg *config = &COMP_CONFIG[i];
   }
}

void dwt_reset(void)
{
   sDwtCompCfg *COMP_CONFIG = &DWT->COMP0;
   const size_t num_comparators = (DWT->CTRL >> 28) & 0xF;
   for (size_t i = 0; i < num_comparators; i++)
   {
      sDwtCompCfg *config = &COMP_CONFIG[i];
      config->FUNCTION = config->COMP = config->MASK = 0;
   }
}

/*----------------------------------------------------------------------------
dwt_install_watchpoint: monitor the data access to an address. 
While using the debugger, it uses the halting debug with the DCB->DHCSR debug key enabled.
!! SET *(uint32_t*)0xE000EDF0 to value a05f0000 (0xA05F<<16) via the debugger memory to disable the halting debug, 
or load the image directly to the board !!!
then dwt will work. 

MASK
0x0						0 bit 2^0=1 bytes
0x1						1 bit 2^1=2 bytes
0x2						2 bit 2^2=4 bytes
0x4						4 bit 2^4=16 bytes

FUNCTION			Evaluation Performed			Event Generated
0b0000 (0x0)	None											Comparator is Disabled
0b0100(0x4)		Instruction 							Fetch	PC watchpoint debug event
0b0101 (0x5)	Read/Load Data Access			Watchpoint debug event
0b0110 (0x6)	Write/Store Data Access		Watchpoint debug event
0b0111 (0x7)	Read or Write Data Access	Watchpoint debug event
*----------------------------------------------------------------------------*/
void dwt_install_watchpoint(int comp_id, uint32_t addr, uint32_t mask, uint32_t func)
{
   // enable reacing | enable debug interrupt
   //CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk | CoreDebug_DEMCR_MON_EN_Msk;

   const size_t num_comparators = (DWT->CTRL >> 28) & 0xF;
   if (comp_id > num_comparators)
   {
      return;
   }
   switch (comp_id)
   {
   case 0:
      DWT->COMP0 = (uint32_t)addr;
			DWT->MASK0 = mask;
      DWT->FUNCTION0 = func;
      break;
   case 1:
      DWT->COMP1 = (uint32_t)addr;
			DWT->MASK1 = mask;
      DWT->FUNCTION1 = func;
      //(1 << 11) /*DATASIZE 1 - match whole word*/
      //| (1 << 1) | (1 << 2) /*generate a watchpoint event on write*/;
      break;
   case 2:
      DWT->COMP2 = (uint32_t)addr;
			DWT->MASK2 = mask;
      DWT->FUNCTION2 = func;
      break;
   case 3:
      DWT->COMP3 = (uint32_t)addr;
			DWT->MASK3 = mask;
      DWT->FUNCTION3 = func;
      break;
   }
}

// https://m0agx.eu/2018/08/25/cortex-m-debugging-runtime-memory-corruption/

void dwt_disable(uint8_t dwt_ind)
{
   switch (dwt_ind)
   {
   case 0:
      DWT->FUNCTION0 = 0;
      break;
   case 1:
      DWT->FUNCTION1 = 0;
      break;
   case 2:
      DWT->FUNCTION2 = 0;
      break;
   default:
      __BKPT(0);
   }
}

void DebugMon_Handler_Main(sContextStateFrame *stack)
{
   /* Attribute unused is to silence compiler warnings,
   * the variables are placed here, so they can be inspected
   * by the debugger.
   */
   //__asm("bkpt 2");
   //trigger_pendsv();

   // Configurable Fault Status Register
   // Consists of MMSR, BFSR and UFSR
   const bool is_dwt_dbg_evt = SCB->DFSR & SCB_DFSR_DWTTRAP_Msk;
   const bool is_bkpt_dbg_evt = SCB->DFSR & SCB_DFSR_BKPT_Msk;
   const bool is_halt_dbg_evt = SCB->DFSR & SCB_DFSR_HALTED_Msk;

   lcd_print(4, "Secure Debug monitor exception!");

   sprintf(msg, "DFSR: 0x%08x (bkpt=%d, halt=%d, dwt=%d)", (int)SCB->DFSR,
           (int)is_bkpt_dbg_evt, (int)is_halt_dbg_evt,
           (int)is_dwt_dbg_evt);
   lcd_print(5, msg);

   if (is_bkpt_dbg_evt)
   {
      return;
   }

   //volatile uint8_t __attribute__((unused)) watchpoint_number = 0;
   if (DWT->FUNCTION0 & DWT_FUNCTION_MATCHED_Msk)
   {
      //watchpoint_number = 0;
      lcd_print(7.5, "HIT dwt comparator0.");
      return;
   }
   else if (DWT->FUNCTION1 & DWT_FUNCTION_MATCHED_Msk)
   {
      //watchpoint_number = 1;
      lcd_print(8, "HIT dwt comparator1.");
      return;
   }
   else if (DWT->FUNCTION2 & DWT_FUNCTION_MATCHED_Msk)
   {
      //watchpoint_number = 2;
      lcd_print(8.5, "HIT dwt comparator2.");
      return;
   }

   else if (DWT->FUNCTION3 & DWT_FUNCTION_MATCHED_Msk)
   {
      //watchpoint_number = 3;
      lcd_print(9, "HIT dwt comparator3.");
      return;
   }
   //sys_recover(stack);
   //__BKPT(0); //data watchpoint!
   // TODO: save data to debug breadcrumbs and reboot
   // clear any logged faults from CFSR
   //SCB->CFSR |= SCB->CFSR;
   // return to where exit from the exception
   //stack->pc = stack->lr;
   // set the lr a pattern
   //stack->lr = 0xdeadbeef;

   // reset the psr state and only leave the "thumb instruction interworking" bit set
   //stack->psr = (1 << 24);
   //sys_recover(stack);
	 return;
}

/* print out all state, mode, level infroamtion */
void print_status_s()
{
   /* state: */
   /* mode: IPSR */
   /* CONTROL[0]*/
}

uint32_t read_memory_s(uint32_t addr)
{
   uint32_t *read_addr = (uint32_t *)addr; // reserved location
   return *read_addr;                      // try to read from reserved location
}

uint32_t write_memory_s(uint32_t addr, uint32_t value)
{
   char *write_addr = (char *)addr;
   *write_addr = value;
   return 0;
}

/* Sleep function to delay n*mS
 * Uses FPGA counter. Prescale is set to divide 25MHZ
 * to 1MHz
 */

void apSleep(unsigned int msec)
{
   unsigned int end;
   unsigned int start;

   start = MPS2_FPGAIO->COUNTER;
   end = start + (25 * msec * 1000);

   if (end >= start)
   {
      while (MPS2_FPGAIO->COUNTER >= start && MPS2_FPGAIO->COUNTER < end)
         ;
   }
   else
   {
      while (MPS2_FPGAIO->COUNTER >= start)
         ;
      while (MPS2_FPGAIO->COUNTER < end)
         ;
   }
}

void memcpy(void *dest, void *src, int n)
{
   char *csrc = (char *)src;
   char *cdest = (char *)dest;
   for (int i = 0; i < n; i++)
      cdest[i] = csrc[i];
}

// https://www.sciencedirect.com/topics/engineering/main-stack-pointer
void SwitchStackPointer_s()
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

void HardFault_Handler(void)
{
   __asm volatile(
       "TST    LR, #0b0100;      "
       "ITE    EQ;               "
       "MRSEQ  R0, MSP;          "
       "MRSNE  R0, PSP;          "
       "MOV    R1, LR;           "
       "B      Hard_Fault_Handler;  ");
}

void BusFault_Handler(void)
{
   __asm volatile(
       "TST    LR, #0b0100;      "
       "ITE    EQ;               "
       "MRSEQ  R0, MSP;          "
       "MRSNE  R0, PSP;          "
       "MOV    R1, LR;           "
       "B      Bus_Fault_Handler;  ");
}

void MemManage_Handler(void)
{
   __asm volatile(
       "TST    LR, #0b0100;      "
       "ITE    EQ;               "
       "MRSEQ  R0, MSP;          "
       "MRSNE  R0, PSP;          "
       "MOV    R1, LR;           "
       "B      Mem_Fault_Handler;  ");
}

void UsageFault_Handler(void)
{
   __asm volatile(
       "TST    LR, #0b0100;      "
       "ITE    EQ;               "
       "MRSEQ  R0, MSP;          "
       "MRSNE  R0, PSP;          "
       "MOV    R1, LR;           "
       "B      Usage_Fault_Handler;  ");
}

void PendSV_Handler(void)
{
   __asm volatile(
       "TST    LR, #0b0100;      "
       "ITE    EQ;               "
       "MRSEQ  R0, MSP;          "
       "MRSNE  R0, PSP;          "
       "MOV    R1, LR;           "
       "B      Pend_SV_Handler;  ");
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

//extern void DebugMon_Handler(void);
//__attribute__((naked))
void DebugMon_Handler(void)
{
   __asm volatile(
       "TST    LR, #0b0100;      "
       "ITE    EQ;               "
       "MRSEQ  R0, MSP;          "
       "MRSNE  R0, PSP;          "
       "MOV    R1, LR;           "
       "B      DebugMon_Handler_Main;  ");
}