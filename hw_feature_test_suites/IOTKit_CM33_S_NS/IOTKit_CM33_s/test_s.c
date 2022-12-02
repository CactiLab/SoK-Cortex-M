#include "test_s.h" /* test header */
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

enum
{
   r0,
   r1,
   r2,
   r3,
   r12,
   lr,
   pc,
   psr
};
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
   //GLCD_ClearScreen();
   lcd_print(0.5, "In Hard Fault Handler");
   sprintf(msg, "SCB->HFSR = 0x%08x, SCB->CFSR = 0x%08x.", SCB->HFSR, SCB->CFSR);
   lcd_print(1, msg);

   mtb_disable();

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
   //dummy_handler(stack);
   //sys_recover(stack);
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
   //sys_recover(stack);
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
   //sys_recover(stack);
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

void Secure_Fault_Handler(sContextStateFrame *stack)
{
   GLCD_ClearScreen();
   uint32_t SFSRValue = SAU->SFSR;
   static uint32_t x;
   sprintf(msg, "SAU->SFSR = 0x%08x.", SFSRValue);
   lcd_print(0.5, msg);
   lcd_print(1.5, "Secure fault: ");

   if (SFSRValue & SAU_SFSR_INVEP_Msk)
   {
      lcd_print(2, "*Invalid entry point.");
   }
   else if (SFSRValue & SAU_SFSR_INVIS_Msk)
   {
      lcd_print(2, "*Invalid integrity signature.");
   }
   else if (SFSRValue & SAU_SFSR_INVER_Msk)
   {
      lcd_print(2, "*Invalid exception return.");
   }
   else if (SFSRValue & SAU_SFSR_AUVIOL_Msk)
   {
      lcd_print(2, "*Attribution unit violation.");
   }
   else if (SFSRValue & SAU_SFSR_INVTRAN_Msk)
   {
      lcd_print(2, "*Invalid transition.");
   }
   else if (SFSRValue & SAU_SFSR_LSPERR_Msk)
   {
      lcd_print(2, "*Lazy state preservation.");
   }
   else if (SFSRValue & SAU_SFSR_LSERR_Msk)
   {
      lcd_print(2, "*Lazy state error.");
   }

   if (SCB_NS->ICSR & SCB_ICSR_RETTOBASE_Msk)
   {
      x = __TZ_get_PSP_NS();
   }
   else
   {
      x = __TZ_get_MSP_NS();
   }
   sprintf(msg, " NS instruction address:  0x%x.", x);
   lcd_print(2.5, msg);
   /* clear SFSR sticky bits */
   SAU->SFSR |= 0xFF;
   stackDump(stack);
   //dummy_handler(stack);
   //sys_recover(stack);
   sys_reset();
}

void Pend_SV_Handler(sContextStateFrame *stack)
{
   GLCD_ClearScreen();
   __asm("bkpt 1");
   uint32_t ICSRValue = SCB->ICSR;
   uint32_t act_num = ICSRValue >> 11 & 0x1;
   uint32_t excp_num = ICSRValue >> 12 & 0xff;
   int IRQ_num = ICSRValue & 0xff;
   sprintf(msg, "SCB->ICSR = 0x%08x\n", ICSRValue);
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
   case SecureFault_IRQn + 16:
      lcd_print(2, "Secure Fault");
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
   /* stack contains: r0, r1, r2, r3, r12, r14, the return address and xPSR */
   svc_number = ((char *)svc_args[6])[-2];
   switch (svc_number)
   {
   case 0: /* EnablePrivilegeMode*/
      __set_CONTROL(__get_CONTROL() & ~CONTROL_nPRIV_Msk);
      break;
   case 10: /* call NSC function, 10, 11, 13, -5 */
      test_add_s(13, 13);
      break;
   // case 12: /* 12. access secure region to trigger secure fault */
   //    test_access_ns();
   //    break;
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
	Triggering a Built In Exception (PendSV)
 *---------------------------------------------------------------------------*/
//__attribute__((optimize("O0")))
void trigger_pendsv(void)
{
   //volatile uint32_t *icsr = (void*)0xE000ED04;
   // Pend a PendSV exception using by writing 1 to PENDSVSET at bit 28
   //*icsr = 0x1 << 28;
   SCB->ICSR = 0x1 << 28;
   // flush pipeline to ensure exception takes effect before we return from this routine
   __ASM("isb");
}

void trigger_nvic_int0(void)
{
   // set the interrupt priority to be the lowest possible for the NRF52
   // default is 0
   NVIC_SetPriority(DebugMonitor_IRQn, 0xe0);
   // Enable UART0RX_IRQn (External Intterrupt 0)
   NVIC_EnableIRQ(DebugMonitor_IRQn);
   // Pend an interrupt
   NVIC_SetPendingIRQ(DebugMonitor_IRQn);
   __ASM("isb");
}
/*---------------------------------------------------------------------------
	MPU configuration case1: protect the MPC*ROM
 *---------------------------------------------------------------------------*/

void mpu_mpc_rom_config(void)
{
   __asm volatile("dmb\n");                                                           /* Force any outstanding transfers to complete before disabling MPU */
   MPU->CTRL = 0;                                                                     /* Disable MPU */
   SCB->SHCSR |= (1 << SCB_SHCSR_MEMFAULTENA_Pos);                                    /* Enable Memory Management Fault Handler using SHCSR */
   uint32_t MPU_regions = (MPU->TYPE & MPU_TYPE_DREGION_Msk) >> MPU_TYPE_DREGION_Pos; /* Check number of supported MPU regions */

   /* Configure region 0: NS Default AHB Slave (Device-nGnRnE, Non-Shareable, RO, Any Privilege Level, XN) */
   /* Configure SRAM0MPC (0x50083000*0x50083FFF), Non-Shareable, RO, Any Privilege Level, XN  */
   MPU->RNR = 6;
   MPU->RBAR = (0x50083000 & MPU_RBAR_ADDR_Msk) | NON_SHAREABLE | RO_P_ONLY | EXEC_NEVER;
   MPU->RLAR = (0x50083FFF & MPU_RLAR_LIMIT_Msk) | ((1 << MPU_RLAR_AttrIndx_Pos) & MPU_RLAR_AttrIndx_Msk) | REGION_ENABLE;

   /* Disable all other regions */
   for (uint32_t i = 0; (i < MPU_regions) && (i != 6); i++)
   {
      MPU->RNR = i;
      MPU->RLAR &= 0x0;
   } /*  */

   /* Enable MPU and the default memory map as a background region (acts as region number -1) for privileged access only (MPU_CTRL.PRIVDEFENA). */
   MPU->CTRL = 5; /* Insert DSB followed by ISB */
   __asm volatile("dsb\n"
                  "isb\n");
}

/*---------------------------------------------------------------------------
	MPU configuration case2: protect the memory
 *---------------------------------------------------------------------------*/
void arm_mpu_config_s(void)
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
   /* Configure region 1: S SRAM1 code (Normal, Non-Shareable, RO, Any Privilege Level) */ /* (Normal, Non-Shareable, RO, Any Privilege Level) (0x3 & MPU_RBAR_ADDR_Msk) | NON_SHAREABLE | RW_P_U | EXEC_NEVER */ /* Configure to RW for SG instruction testing*/
   MPU->RNR = 1;
   MPU->RBAR = (0x10000000 & MPU_RBAR_ADDR_Msk) | NON_SHAREABLE | RW_P_U;
   MPU->RLAR = (0x101FFFFF & MPU_RLAR_LIMIT_Msk) | ((0 << MPU_RLAR_AttrIndx_Pos) & MPU_RLAR_AttrIndx_Msk) | REGION_ENABLE;
   /* Configure region 2: NS SRAM3 data (Normal, Non-Shareable, RW, Any Privilege Level, XN) */
   MPU->RNR = 2;
   MPU->RBAR = (0x38000000 & MPU_RBAR_ADDR_Msk) | NON_SHAREABLE | RW_P_U | EXEC_NEVER;
   MPU->RLAR = (0x381FFFFF & MPU_RLAR_LIMIT_Msk) | ((0 << MPU_RLAR_AttrIndx_Pos) & MPU_RLAR_AttrIndx_Msk) | REGION_ENABLE;
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
void test_config_mpu_s(uint32_t region_id, uint32_t start, uint32_t end, uint32_t attr)
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

void mpu_config_mpsssram3_ns_s(void)
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

void dwt_install_watchpoint(int comp_id, uint32_t addr, uint32_t func)
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
      DWT->FUNCTION0 = func;
      break;
   case 1:
      DWT->COMP1 = (uint32_t)addr;
      DWT->FUNCTION1 = func;
      //(1 << 11) /*DATASIZE 1 - match whole word*/
      //| (1 << 1) | (1 << 2) /*generate a watchpoint event on write*/;
      break;
   case 2:
      DWT->COMP2 = (uint32_t)addr;
      DWT->FUNCTION2 = func;
      break;
   case 3:
      DWT->COMP3 = (uint32_t)addr;
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
   sys_recover(stack);
   //__BKPT(0); //data watchpoint!
   // TODO: save data to debug breadcrumbs and reboot
   // clear any logged faults from CFSR
   SCB->CFSR |= SCB->CFSR;
   // return to where exit from the exception
   stack->pc = stack->lr;
   // set the lr a pattern
   stack->lr = 0xdeadbeef;

   // reset the psr state and only leave the "thumb instruction interworking" bit set
   stack->psr = (1 << 24);
   //sys_recover(stack);
}

/* print out all state, mode, level infroamtion */
void print_status_s()
{
   /* state: */
   /* mode: IPSR */
   /* CONTROL[0]*/
}

/* configure the SAU region: starts from 4
input: SAU region number, start_address, end_address, 
				security attributs, the NSC bit, <0=>Non-Secure, <1=>Secure, Non-Secure Callable
*/
void test_config_sau_s(uint32_t region_id, uint32_t start, uint32_t end, uint32_t nsc_attr)
{
   /* region number */
   SAU->RNR = region_id & SAU_RNR_REGION_Msk;
   /* region start address*/
   SAU->RBAR = start & SAU_RBAR_BADDR_Msk;
   /* region end address */
   SAU->RLAR = (end & SAU_RLAR_LADDR_Msk) |
               /* Region memory attribute index*/
               ((nsc_attr << SAU_RLAR_NSC_Pos) & SAU_RLAR_NSC_Msk) |
               /* Enable region */
               ((1U << SAU_RLAR_ENABLE_Pos) & SAU_RLAR_ENABLE_Msk);
}

/* configure the MPC region: MPCSSRAM1, MPCSSRAM2, MPCSSRAM3
By default, MPCSSRAM1 code region 0x00200000 - 0x003FFFFF is unsecure
input: MPC number
LUT index
				block configuration
*/

void test_config_mpc_s(uint32_t n, uint32_t idx, unsigned long block)
{
   // uint32_t blk_max, blk_cfg, blk_size, blk_cnt;
   switch (n)
   {
   case MPC1:
      /* MPSSSRAM1 region (0x00000000 - 0x003FFFFF) */
      //  blk_max = IOTKIT_MPCSSRAM1->BLK_MAX;   /* = 0x1     */
      //  blk_cfg = IOTKIT_MPCSSRAM1->BLK_CFG;   /* = 0xC     */
      //  blk_size = 1UL << (blk_cfg + 5U);      /* = 0x20000 */
      //  blk_cnt  = 0x200000U / blk_size;       /* = 0x10    */
      IOTKIT_MPCSSRAM1->CTRL &= ~(1UL << 8U); /* clear auto increment */
      IOTKIT_MPCSSRAM1->BLK_IDX = idx;        /* write LUT index */
      IOTKIT_MPCSSRAM1->BLK_LUT = block;      /* configure blocks */
      break;
   case MPC2:
      /* configure unsecure data area MPSSSRAM2 (0x28000000 - 0x281FFFFF) */
      //  blk_max = IOTKIT_MPCSSRAM2->BLK_MAX;   /* = 0x1     */
      //  blk_cfg = IOTKIT_MPCSSRAM2->BLK_CFG;   /* = 0xB     */
      //  blk_size = 1UL << (blk_cfg + 5U);      /* = 0x10000 */
      //  blk_cnt  = 0x200000U / blk_size;       /* = 0x20    */
      IOTKIT_MPCSSRAM2->CTRL &= ~(1UL << 8U); /* clear auto increment */
      IOTKIT_MPCSSRAM2->BLK_IDX = idx;        /* write LUT index */
      IOTKIT_MPCSSRAM2->BLK_LUT = block;      /* configure blocks */
      break;
   case MPC3:
      /* configure unsecure data area MPSSSRAM3 (0x28200000 - 0x283FFFFF) */
      //  blk_max = IOTKIT_MPCSSRAM3->BLK_MAX;   /* = 0x1     */
      //  blk_cfg = IOTKIT_MPCSSRAM3->BLK_CFG;   /* = 0xB     */
      //  blk_size = 1UL << (blk_cfg + 5U);      /* = 0x10000 */
      //  blk_cnt  = 0x200000U / blk_size;       /* = 0x20    */
      IOTKIT_MPCSSRAM3->CTRL &= ~(1UL << 8U); /* clear auto increment */
      IOTKIT_MPCSSRAM3->BLK_IDX = idx;        /* write LUT index */
      IOTKIT_MPCSSRAM3->BLK_LUT = block;      /* configure blocks */
      break;
   default:
      break;
   }
}

void test_config_ppc_s(void)
{

   /* configure PPC --------------- */
   /* Allow Non-secure access for SCC/FPGAIO registers */
   IOTKIT_SPC->APBNSPPCEXP[2U] |= ((1UL << 0U) |
                                   (1UL << 2U));
   /* Allow Non-secure access for SPI1/UART0 registers */
   IOTKIT_SPC->APBNSPPCEXP[1U] |= ((1UL << 1U) |
                                   (1UL << 5U));
   /* Allow Secure unprivilege access for AHB DWT register???: 0xE0001000*/
   //IOTKIT_SPC->AHBSPPPCEXP[1U] |= ;
   //   /* Setup Peripheral Protection Controller (PPC) */
   // 	AHB_SECURE_CTRL->SEC_CTRL_AHB0_0_SLAVE_RULE = 0x03000000U;
   // 	AHB_SECURE_CTRL->SEC_CTRL_AHB0_1_SLAVE_RULE = 0x00010000U;
   // 	AHB_SECURE_CTRL->SEC_CTRL_AHB2[0].SEC_CTRL_AHB2_0_SLAVE_RULE = 0x00001001U;
   // 	AHB_SECURE_CTRL->SEC_CTRL_APB_BRIDGE[0].SEC_CTRL_APB_BRIDGE0_MEM_CTRL0 = 0x00001133U;
   // 	AHB_SECURE_CTRL->SEC_CTRL_APB_BRIDGE[0].SEC_CTRL_APB_BRIDGE1_MEM_CTRL1 = 0x00010000U;
   // 	AHB_SECURE_CTRL->SEC_CTRL_APB_BRIDGE[0].SEC_CTRL_APB_BRIDGE1_MEM_CTRL3 = 0x00000301U;
}

void Board_Config_TZ()
{
   //   test_config_sau_s();
   //   test_config_mpc_s();
   //   test_config_ppc_s();
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

/* 
 * Function for MTB
 */

int mtb_enable(size_t mtb_size)
{
   if ((mtb_size < 16) || (__builtin_popcount(mtb_size) != 1))
   {
      // MTB must be at least 16 bytes and be a power of 2
      return -1;
   }
   // configure the MTB buffer as S
   test_config_sau_s(4, 0x24000000, 0x24003FFF, 1); // already configured 0-3
                                                    // then separate the buffer into equally secure and non-secure: 00x2400_1FFFF
                                                    // 100100000000000001111111111111
                                                    //MTB->SECURE = 0x24001FFF;
   // scrub MTB SRAM so it's easy to see what has gotten written to
   memset((void *)MTB->BASE, 0x0, mtb_size);
   const int mask = __builtin_ctz(mtb_size) - 4;

   // we are about to reconfigure so turn off MTB
   mtb_disable();

   // reset position
   MTB->POSITION = 0;

   // Start tracing!
   MTB->MASTER = (1 << 31) | (mask << 0);
   return 0;
}

int mtb_resume(size_t mtb_size, int pos) __attribute__((cmse_nonsecure_entry));
int mtb_resume(size_t mtb_size, int pos)
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
   mtb_disable();

   // reset position
   //int pos = MTB->POSITION;
   //MTB->POSITION = pos;

   // Start tracing!
   MTB->MASTER = (1 << 31) | (mask << 0);
   return 0;
}

int mtb_pause() __attribute__((cmse_nonsecure_entry));
int mtb_pause()
{
   // we are about to reconfigure so turn off MTB
   mtb_disable();

   // read the pause position, should be - 4 because the mtb_disable() uses a pair os addresses
   int pos = MTB->POSITION;
   //MTB->POSITION = 0;

   // Start tracing!
   return pos;
}

int mtb_disable(void)
{
   MTB->MASTER &= ~(1 << 31);
   return 0;
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

void SecureFault_Handler(void)
{
   __asm volatile(
       "TST    LR, #0b0100;      "
       "ITE    EQ;               "
       "MRSEQ  R0, MSP;          "
       "MRSNE  R0, PSP;          "
       "MOV    R1, LR;           "
       "B      Secure_Fault_Handler;  ");
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