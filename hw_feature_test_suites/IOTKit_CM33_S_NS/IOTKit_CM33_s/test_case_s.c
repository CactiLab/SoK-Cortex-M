/*----------------------------------------------------------------------------
 * Name:    test_case_s.c
 * Purpose: test cases in the secure state
 *----------------------------------------------------------------------------*/

#include "test_s.h" /* test helper and configuration functions*/

static char msg[MAXMSG];  /* used for showing message on the mps2+ lcd screen */
static float lcd_pos = 0; /* reset the lcd message position */

/* used for dwt measurement */
volatile uint32_t s_time = 0;
volatile uint32_t e_time = 0;
volatile uint32_t r_time = 0;
volatile uint32_t msTicks = 0;

my_struct_t m;

/*----------------------------------------------------------------------------
  NonSecure callback functions
 *----------------------------------------------------------------------------*/
extern NonSecure_fpParam pfNonSecure_test_add_ns;
NonSecure_fpParam pfNonSecure_test_add_ns = (NonSecure_fpParam)NULL;

/*----------------------------------------------------------------------------
  Secure function for NonSecure callbacks exported to NonSecure application
 *----------------------------------------------------------------------------*/
int32_t Secure_test_add_ns_callback(NonSecure_fpParam callback) __attribute__((cmse_nonsecure_entry));
int32_t Secure_test_add_ns_callback(NonSecure_fpParam callback)
{
	pfNonSecure_test_add_ns = callback;
	return 0;
}

/*----------------------------------------------------------------------------
  Secure functions exported to NonSecure application
 *----------------------------------------------------------------------------*/
void Secure_lcd_print(uint32_t pos, char *pString) __attribute__((cmse_nonsecure_entry));
void Secure_lcd_print(uint32_t pos, char *pString)
{
	//printf("%s", pString);
	lcd_print(pos, pString);
}

/*----------------------------------------------------------------------------
SysTick IRQ Handler: count the time cycle
 *----------------------------------------------------------------------------*/
void __inline SysTick_Handler(void)
{
	msTicks++;
}

/*----------------------------------------------------------------------------
test_systick_s: test dwt cycle counting 
 *----------------------------------------------------------------------------*/
void test_systick_s()
{
	s_time = msTicks;

	for (int i = 0; i < 1000000; i++)
	{
	}
	e_time = msTicks;
	r_time = e_time - s_time;
	GLCD_ClearScreen();
	sprintf(msg, "test_systick_s: execution time %d(ms)", r_time);
	lcd_print(2, msg);
}

/* set message to the LCD */
void test_lcd_s()
{
	GLCD_ClearScreen();
	lcd_print(5, "   xxxxxxxx Tests    ");
	lcd_print(6, " Secure/Non-Secure  ");
}

/*----------------------------------------------------------------------------
 IOTKIT_SECURE_GPIO1: 50101000
 IOTKIT_GPIO1: 40101000 GPIO1[5]->userbutton0 GPIO[6]->userbotton1
 refer to Cortex-M33_IoT_Kit_2.0/software/selftest/apleds
 *----------------------------------------------------------------------------*/
void test_user_botton_s()
{
	int timer, failtest;

	/* The two USER buttons that are connected directly to the FPGA */
	lcd_print(1, "Please press userbutton to farward");

	/* Test the buttons are released */
	if ((MPS2_FPGAIO->BUTTON & 0x03) != 0x00)
	{
		sprintf("Error: USER buttons are pressed (0x%02X).", MPS2_FPGAIO->BUTTON & 0x03);
		lcd_print(2, msg);
		failtest = TRUE;
	}

	/* Test push buttons 0 */
	lcd_print(2, "Please press USER push button 0 (S4).");
	timer = 100;
	while (((MPS2_FPGAIO->BUTTON & 0x03) != 0x01) && timer)
	{
		lcd_print(3, "You pressed the USER button 0.");
		apSleep(100);
		timer--;
	}
	if (!timer)
	{
		lcd_print(3, "Error: Timeout waiting for USER push button 0.");
		failtest = TRUE;
	}

	// Test push buttons 1
	lcd_print(4, "Please press USER push button 1 (S5)\n");
	timer = 100;
	while (((MPS2_FPGAIO->BUTTON & 0x03) != 0x02) && timer)
	{
		lcd_print(3, "You pressed the USER button 1\n");
		apSleep(100);
		timer--;
	}
	if (!timer)
	{
		lcd_print(5, "Error: Timeout waiting for USER push button 1.");
		failtest = TRUE;
	}
}

/*----------------------------------------------------------------------------
	Test MPU
	1. arm_mpu_config_s() configured 7 MPU regions. 
		region 0 @ 0x00200000-0x003FFFFF: NS SRAM1 for code (Normal, Non-Shareable, RO, Any Privilege Level)
		region 1 @ 0x10000000-0x101FFFFF: S SRAM1 code (Normal, Non-Shareable, RO, Any Privilege Level)
		region 2 @ 0x38000000-0x381FFFFF: NS SRAM3 data (Normal, Non-Shareable, RW, Any Privilege Level, XN)
		region 3 @ 0x40000000-0x4000FFFF: NS CMSDK APB (Device-nGnRnE, Non-Shareable, RW, Any Privilege Level, XN)
		region 4 @ 0x40010000-0x40013FFF: NS GPIO (Device-nGnRnE, Non-Shareable, RW, Any Privilege Level, XN)
		region 5 @ 0x40014000-0x40017FFF: NS Default AHB Slave (Device-nGnRnE, Non-Shareable, RO, Any Privilege Level, XN)
		region 6 @ 0xF0000000-0xF0000FFF: System ROM (Normal, Non-Shareable, RO, Any Privilege Level)
	2. Before and after MPU congiguration, test1, 2, 3, 4
	3. change the level to unprivileged
 *----------------------------------------------------------------------------*/
void test_mpu_s()
{
	uint32_t x = 0;
	//uint16_t func[] = {0x4601, 0x1c48, 0x4770};
	/* test1-1: reading reserved region, will generate bus fault -- data buss error--ret point to the instruction that caused the fault */
	//int *test11 = (int *)0x0E000000; // reserved location
	//x = *test11;					 // try to read from reserved location

	/* test2-1: writing to flash, ignored by the processor */
	char *test21 = (char *)0x00070000;
	*test21 = 1;

	/* test3-1: reading undefined region (where memory exists), 
			will generate bus fault -- imprecise bus error */
	//int* test31 = (int*)0x1fff0000 ;   // undefined region
	//x = *test31 ;                      // try to read from undefined region

	/* test4-1: Executing SRAM
		will generate usage fault if trying to excute an undefined instruction */
	funcPtr test41 = (funcPtr)0x38000001; //setup RAM function (+1 for thumb)
	x = test41(x);						  //call RAM function

	/* configure the MPU */
	arm_mpu_config_s();

	/* test1-2: reading reserved region, will generate a preciser bus error */
	//int* test12 = (int*)0x0E000000;   // reserved location
	//x= *test12;                        // try to read from reserved location

	/* test2-2: writing to flash (RO region), will generate a DACCVIOL MemManage Fault */
	//char *test22 = (char *)0x00070000;
	//*test22 = 1;

	/* test3-2: reading undefined region (where memory exists), will generate a impreciser bus error */
	//int* test32 = (int*)0x1fff0000 ;   // undefined region
	//x = *test32 ;                      // try to read from undefined region

	/* test4-2: Executing code in XN marked region, will generate IACCVIOL memmanage fault */
	funcPtr test42 = (funcPtr)0x38000001; //setup RAM function (+1 for thumb)
	x = test42(x);
}

/*----------------------------------------------------------------------------
	Configure MPU region 0 to protect MPC-ROM
 *----------------------------------------------------------------------------*/
void test_mpu_mpc_rom_config_s()
{
	mpu_mpc_rom_config();
	IOTKIT_MPCSSRAM1->CTRL &= ~(1UL << 8U);	  /* clear auto increment */
	IOTKIT_MPCSSRAM1->BLK_IDX = 0;			  /* write LUT index */
	IOTKIT_MPCSSRAM1->BLK_LUT = 0xFFFF000FUL; /* configure blocks */
}

/*----------------------------------------------------------------------------
  	Change privileged mode to non-privileged mode
 *----------------------------------------------------------------------------*/
void test_drop_privileges()
{
	DROP_S_PRIVILEGES;
}

/*----------------------------------------------------------------------------
 	Modify IPSR directly
 *----------------------------------------------------------------------------*/
void test_ipsr()
{
	uint32_t ipsr = 0, xpsr = 0;
	GLCD_ClearScreen();
	ipsr = __get_IPSR();
	xpsr = __get_xPSR();
	sprintf(msg, "default ipsr = 0x%08x, xpsr = 0x%08x.", ipsr, xpsr);
	lcd_print(2, msg);

	__ASM volatile("MSR ipsr, %[reg] " ::[reg] "r"(6)
				   : "memory");
	/* the same as */
	//__ASM volatile("MSR xpsr, %[reg] " ::[reg] "r"(6) : "memory");
	ipsr = __get_IPSR();
	xpsr = __get_xPSR();
	sprintf(msg, "ipsr = 0x%08x, xpsr = 0x%08x.", ipsr, xpsr);
	lcd_print(3, msg);
}

/*----------------------------------------------------------------------------
	TestUnstacking: puts EXC_RETURN in LR
	EXC_RETURN has three valid values when without FPU: 0xFFFFFFF1, 0xFFFFFFF9, 0xFFFFFFFD
	Three valid values with FPU: 0xFFFFFFE1, 0xFFFFFFE9, 0xFFFFFFED
	Result: did not trigger the unstacking, pc changed to [lr] - 1, then run instuctions locatted at 0xffffffe0, then trigger IACCVIOL MemMangeFault
 *----------------------------------------------------------------------------*/
void test_unstacking_s()
{
	__asm volatile(
		"MOV LR, #0xFFFFFFE1; "
		"BX LR; ");
}

void test_dwt_cyccount()
{
	DROP_S_PRIVILEGES;
	elapsed_time_start(0);
	elapsed_time_stop(0);
	elapsed_time_clr(0);
}

/*----------------------------------------------------------------------------
 	Configure SAU at run-time
 *----------------------------------------------------------------------------*/
void test_sau(void)
{
	/* Disable SAU */
	SAU->CTRL = 0U;

	/* Configure SAU region 1 - CODE_NS */
	//test_config_sau_s(1, 0x9DE00, 0x9FDFF, 0);

	/* Configure SAU region 4 - CODE_S to NS */
	test_config_sau_s(4, 0x10200000, 0x103FFFFF, 0);

	/* Configure SAU region 5 - Data_NS to NSC */
	test_config_sau_s(5, 0x00000000, 0x001FFFFF, 1);

	/* Configure SAU region 6 -  */
	/* Configure SAU region 7 -  */

	/* Force memory writes before continuing */
	__DSB();
	/* Flush and refill pipeline with updated permissions */
	__ISB();
	/* Enable SAU */
	SAU->CTRL = 1U;

	/* Interrupt configuration */
	NVIC->ITNS[0] = 0x2040000C; // RTC, ADC, GINT1, GINT0
	NVIC->ITNS[1] = 0x00018000; // USB1_NEEDCLK, USB1
}

/*----------------------------------------------------------------------------
 	Configure MPC at run-time, check if unprivileged can configure MPC or not
 *----------------------------------------------------------------------------*/
void test_mpc_s(void)
{
	test_config_mpc_s(MPC1, 0, 0xFFFF0000);
	DROP_S_PRIVILEGES;
	test_config_mpc_s(MPC1, 0, 0xFFFF0000);
}

/*----------------------------------------------------------------------------
 	read the value of mpc alias memory
 *----------------------------------------------------------------------------*/
void print_mpc_alias_s()
{
	GLCD_ClearScreen();
	sprintf(msg, "In Secure state\n");
	lcd_print(0.5, msg);
	sprintf(msg, "Before MPC\n");
	lcd_print(1, msg);
	sprintf(msg, "NS-SSRAM3: 0x%x; S-SSRAM3: 0x%x", alias_region_tbl[0].data, alias_region_tbl[0].data_alias);
	lcd_print(1.5, msg);

	sprintf(msg, "After MPC, before SAU. SSRAM3 is non-secure\n");
	lcd_print(2, msg);
	sprintf(msg, "NS-SSRAM3: 0x%x; S-SSRAM3: 0x%x", alias_region_tbl[1].data, alias_region_tbl[1].data_alias);
	lcd_print(2.5, msg);

	sprintf(msg, "After SAU, SSRAM3 is non-secure \n");
	lcd_print(3, msg);
	sprintf(msg, "NS-SSRAM3: 0x%x; S-SSRAM3: 0x%x", alias_region_tbl[2].data, alias_region_tbl[2].data_alias);
	lcd_print(3.5, msg);
}

uint32_t add_s()
{
	return 0;
}

/*----------------------------------------------------------------------------
 	check the compiled assembly code to find the difference between the nsc code and s code
 *----------------------------------------------------------------------------*/
uint32_t test_add_s(uint32_t a, uint32_t b) __attribute__((cmse_nonsecure_entry));
uint32_t test_add_s(uint32_t a, uint32_t b)
{
	uint32_t sum;
	sum = a + b;
	return sum;
}
uint32_t test_add_ss(uint32_t a, uint32_t b)
{
	uint32_t sum;
	sum = a + b;
	return sum;
}
/*----------------------------------------------------------------------------
 	Collect the time for state switching
 *----------------------------------------------------------------------------*/
void test_state_switch_s() __attribute__((cmse_nonsecure_entry));
void test_state_switch_s()
{
	uint32_t tmp;
	elapsed_time_stop(0);
	tmp = 0;
	elapsed_time_start(1);
}

/*----------------------------------------------------------------------------
	Access non-secure data sections from secure world, no errors.
 *----------------------------------------------------------------------------*/
void test_access_ns_data()
{
	char s = NULL;
	char *p_s = (char *)0x28200000;
	s = p_s[0];
	p_s[0] = 0xff;
}

/*----------------------------------------------------------------------------
	Access alias memories
declaration: volatile ALIAS_REGION alias_region_tbl[MAX_ALIAS];
 *----------------------------------------------------------------------------*/
void test_alias_s()
{
	uint32_t data_s[4];

	sprintf(msg, "In Non-secure state\n");
	lcd_print(4.5, msg);
	sprintf(msg, "NS-SSRAM3: 0x%x; S-SSRAM3: cannot access", alias_region_tbl[3].data);
	lcd_print(5, msg);

	// data_s[0] = *((uint32_t *)0x28300000);		// idau ns ssram3
	// data_s[1] = *((uint32_t *)0x38300000);		// idau ns ssram3
	alias_region_tbl[4].data = *((uint32_t *)0x28300000);		//ns ssram3
	alias_region_tbl[4].data_alias = *((uint32_t *)0x38300000); //s ssram3 can not access, RAZ

	sprintf(msg, "Back to Secure state\n");
	lcd_print(6, msg);
	sprintf(msg, "NS-SSRAM3: 0x%x; S-SSRAM3: 0x%x", alias_region_tbl[4].data, alias_region_tbl[4].data_alias);
	lcd_print(6.5, msg);

	IOTKIT_MPCSSRAM3->CTRL &= ~(1UL << 8U);	  /* clear auto increment */
	IOTKIT_MPCSSRAM3->BLK_IDX = 1;			  /* write LUT index */
	IOTKIT_MPCSSRAM3->BLK_LUT = 0x00000000UL; /* configure blocks */
	// data_s[2] = *((uint32_t *)0x28300000);	  // idau ns ssram3
	// data_s[3] = *((uint32_t *)0x38300000);	  // idau ns ssram3
	alias_region_tbl[5].data = *((uint32_t *)0x28300000);		//ns ssram3
	alias_region_tbl[5].data_alias = *((uint32_t *)0x38300000); //s ssram3 can not access, RAZ

	//sprintf(msg, "Reconfigure the SSRAM3 as secure\n");
	sprintf(msg, "NS-SSRAM3: 0x%x; S-SSRAM3: 0x%x", alias_region_tbl[5].data, alias_region_tbl[5].data_alias);
	sprintf(msg, "lcd_print will trigger secure fault: invalid entry point");
	//lcd_print(7.5, msg); /* secure fualt: invalid entry point */
}

/*----------------------------------------------------------------------------
	test unitended branch transition
 *----------------------------------------------------------------------------*/
void test_bb_s()
{
	// unintended branches
	// S-P BL to NSC 0x1000_0000 0x101F_FFFF
	// S-P BL to NSC
	__asm volatile("BL #0x100056cc;");

	// S-P BL to NS: secure fault  0x2820_0000
	__asm volatile("BL #0x28200000;");

	// S-UP BL to NSC
	DROP_S_PRIVILEGES;
	__asm volatile("BL #0x100056cc;");

	// S-UP BX to NSC
	__asm volatile(
		"MOVW r12, #0x56cc;" // test_add_s: 0x10056a9: NSC
		"MOVT r12, #0x1000;"
		"BX r12;");

	// S-UP BL to NS
	__asm volatile("BL 0x28200000;");

	// S-P BX to NSC: no error
	__asm volatile("SVC 0"); // enable privilege
	__asm volatile(
		"MOVW r12, #0x56cc;" // test_add_s: 0x10056a9: NSC
		"MOVT r12, #0x1000;"
		"BX r12;");
}

/*---------------------------------------------------
	Test the state switch from secure state to non-secure state 
	----------------------------------------------------*/
void test_switch_s()
{
	/* test_add_ns: 0x00201EB0*/

	/* 21: secure privilege thread --> secure privilege handler: exception */
	__asm("SVC #28");

	/* 22: secure privilege thread --> non-secure privilege thread */
	if (pfNonSecure_test_add_ns != NULL)
	{
		pfNonSecure_test_add_ns(1);
	}

	DROP_NS_PRIVILEGES;
	/* 23: secure privilege thread --> non-secure non-privilege thread */
	if (pfNonSecure_test_add_ns != NULL)
	{
		pfNonSecure_test_add_ns(1);
	}

	/* 24: secure privilege thread --> secure non-privilege thread: set CONTROL[0] */
	DROP_S_PRIVILEGES;

	/* 25: secure non-privilege thread --> non-secure non-privilege thread */
	if (pfNonSecure_test_add_ns != NULL)
	{
		pfNonSecure_test_add_ns(1);
	}

	/* 27:  secure non-privilege thread --> secure privilege handler: SVC 0 */
	__asm("SVC #0");

	SET_NS_PRIVILEGES; // secure non-privileged can not change the privielge level in non-secure state to privileged
	DROP_S_PRIVILEGES;
	/* 26: secure non-privilege thread --> non-secrure privilege thread*/
	if (pfNonSecure_test_add_ns != NULL)
	{
		pfNonSecure_test_add_ns(1);
	}

	__asm("SVC #0");
}

void test_nsc_ns_s()
{
	test_add_s(2, 3); // address: 0x10004B48 SG: 0x10004BC8
}

void test_scb_ns_s()
{
#ifdef DROP_P_S
	DROP_S_PRIVILEGES;
#endif
	/* Read from SCB */
	uint32_t CFSRValue;
	CFSRValue = SCB->CFSR;
	/* Write to SCB, BFSR register setting to enable precise errors (borrow from system_IOTKit_CM33.c) */
	SCB->CFSR = 7;
}

/*----------------------------------------------------------------------------
test read and write from control register
 *----------------------------------------------------------------------------*/
void test_ctrl_ns_s()
{
#ifdef DROP_P_S
	DROP_S_PRIVILEGES;
#endif
	/* read from control */
	__ASM volatile("MRS r12, CONTROL");

	/* write to control */
	__ASM volatile("MSR CONTROL, %[reg] " ::[reg] "r"(1)
				   : "memory");
	__ASM volatile("MSR CONTROL, %[reg] " ::[reg] "r"(0)
				   : "memory");
}

/*----------------------------------------------------------------------------
	Directly put code in the NSC region and try to run it
 *----------------------------------------------------------------------------*/
void test_sg_call_ns_s()
{
	int ins_sz = 8;
	char ins[] = {
		0x7f, 0xe9, 0x7f, 0xe9, // SG
		0x00, 0x00, 0x00, 0x00, // MOVW R0, #0x0000;,
		0x00, 0x00, 0x00, 0x00, // MOVT R0, #0x1010;,
		0x10, 0x00, 0x00, 0x00	// BXNS r0
	};
	char *dst = (char *)0x10008C4C;
	for (int i = 0; i < ins_sz; i++)
		dst[i] = ins[i];

	__asm(
		"ldr r12, =0x10008C4D;"
		"bx r12;");
}

/*----------------------------------------------------------------------------
	access non-secure R/W privilege region
 *----------------------------------------------------------------------------*/
int32_t test_lp_access_from_ns()
{
	char *p_s = (char *)0x28200000;
	p_s[0] = 'D';
	return p_s[0];
}

/*----------------------------------------------------------------------------
SVC tests: svc instruction in privilege level handler mode
 *----------------------------------------------------------------------------*/
void test_svc_s()
{
	__asm("SVC #66");
}

/*----------------------------------------------------------------------------
TT tests: target test instructions
 *----------------------------------------------------------------------------*/
void test_tt_s(void)
{
	//char *p_s = (char *)0x28200000;
	//char *p_ns = (char *)0x18200000;
	//char *p_s = (char *)0x10100000;  /* Configure region 1: S SRAM2&3 (Normal, Non-Shareable, RW, XN, Any Privilege Level) */
	//char *p_ns = (char *)0x00300000;  /* Configure region 0: NS SRAM1 (Normal, Non-Shareable, RO, Any Privilege Level)*/

	__asm volatile(
		"MOVW R0, #0x0000;"
		"MOVT R0, #0x1010;"
		"TT r0, r0;"
		"MOVW R0, #0x0000;"
		"MOVT R0, #0x1010;"
		"TTT r0, r0;"
		"MOVW R0, #0x0000;"
		"MOVT R0, #0x1010;"
		"TTA r0, r0;"
		"MOVW R0, #0x0000;"
		"MOVT R0, #0x1010;"
		"TTAT r0, r0;"

		"MOVW R0, #0x0000;"
		"MOVT R0, #0x0030;"
		"TT r0, r0;"
		"MOVW R0, #0x0000;"
		"MOVT R0, #0x0030;"
		"TTT r0, r0;"
		"MOVW R0, #0x0000;"
		"MOVT R0, #0x0030;"
		"TTA r0, r0;"
		"MOVW R0, #0x0000;"
		"MOVT R0, #0x0030;"
		"TTAT r0, r0;");
	DROP_S_PRIVILEGES;
	__asm volatile(
		"MOVW R0, #0x0000;"
		"MOVT R0, #0x1010;"
		"TT r0, r0;"
		"MOVW R0, #0x0000;"
		"MOVT R0, #0x1010;"
		"TTT r0, r0;"
		"MOVW R0, #0x0000;"
		"MOVT R0, #0x1010;"
		"TTA r0, r0;"
		"MOVW R0, #0x0000;"
		"MOVT R0, #0x1010;"
		"TTAT r0, r0;"

		"MOVW R0, #0x0000;"
		"MOVT R0, #0x0030;"
		"TT r0, r0;"
		"MOVW R0, #0x0000;"
		"MOVT R0, #0x0030;"
		"TTT r0, r0;"
		"MOVW R0, #0x0000;"
		"MOVT R0, #0x0030;"
		"TTA r0, r0;"
		"MOVW R0, #0x0000;"
		"MOVT R0, #0x0030;"
		"TTAT r0, r0;");

	__asm volatile("SVC #0");
}

/*----------------------------------------------------------------------------
MTB tests: ifinite loop. Check 0xE0043000 we can find the MTB buffer base address is 0x24000000
 *----------------------------------------------------------------------------*/
void test_mtb_s_infinite_loop(void)
{
	volatile int i;
	while (1)
	{
		i++;

		if ((i % 5) == 0)
		{
			__asm("bkpt 6");
		}
	}
}

/*----------------------------------------------------------------------------
MTB tests: stack overflow
 *----------------------------------------------------------------------------*/
int vulfoo_s(int i, char *p)
{
	__asm("bkpt 6"); // branch into this function
	int j = i;
	char buf[6];
	int pos = mtb_pause();
	strcpy(buf, p);
	mtb_resume(MTB_BUFF, pos);
	if (j)
		__asm("bkpt 6"); // stack overflow!

	return 0;
}

void test_mtb_s_stack_overflow(void)
{
	vulfoo_s(0, "deadbeef");
}

/*----------------------------------------------------------------------------
MTB tests: memory corruption, bad asm function-volfoo_s()
 *----------------------------------------------------------------------------*/
__attribute__((naked)) static void test_mtb_s_memory_corrup(void)
{
	__asm("mov r0, 0 \n"
		  "ldr r2, =0x100023D4 \n"
		  "mov lr, 0 \n"
		  "bx r2 \n");
}

/*----------------------------------------------------------------------------
insert code after SG instruction in the NSC region, BXNS to non-secure world
 *----------------------------------------------------------------------------*/
void test_insert_nsc_code_s()
{
	// write instruction to NSC region and execute: garbege memory: 0x10008c4c
	char ins[] = {0x7f, 0xe9, 0x7f, 0xe9, // SG
				  0x42, 0xf6, 0x00, 0x00, // MOVW r0, #0x2800
				  0xc0, 0xf2, 0x20, 0x00, // MOVT r0, #0x0020
				  0x04, 0x47};			  // BXNS R0
	char *dst = (char *)0x10008C4C;
	for (int i = 0; i < 14; i++)
		dst[i] = ins[i];
}

/*----------------------------------------------------------------------------
insert code after SG instruction in the NSC region, execute BL/BX instead of B, no error
write instruction to NSC region and execute: garbage memory: 0x10008c4c/0x10008C4D
 *----------------------------------------------------------------------------*/
void test_sg_s()
{
	/*
	E97FE97F 	SG												0x7f, 0xe9, 0xf7, 0xe9
	AD01	  	ADD r5, sp, #4						0x01, 0xad
	F7FCFCE4	BX #0x1000561e // secure, not nsc			0xfc, 0xf7, 0xe4, 0xfc
	F7FCBD26  B  test_add_s (0x100021FC)						0xfc, 0xf7, 0x26, 0xbd
	F002FFD5  BL test_sg_ns_s (0x100057C0)					0x02, 0xf0, 0xd5, 0xff
	FFF1FEEF	BLX #0x10200000												0xf1, 0xff, 0xef, 0xfe
	*/
	int ins_sz = 22;
	char ins[22] = {
		0x7f, 0xe9, 0x7f, 0xe9, // SG
		0x01, 0xad,				// ADD r5, sp, #4
		0xfc, 0xf7, 0xe4, 0xfc, // BX #0x1000561e
		0xfc, 0xf7, 0x26, 0xbd, // B  #0x100056a6
		0x02, 0xf0, 0xd5, 0xff, // BL #0x1000bc08
		0x88, 0x47				// BLX r1
	};
	char *dst = (char *)0x10008C4C;
	for (int i = 0; i < ins_sz; i++)
		dst[i] = ins[i];

	/* branch to the address to run SG instructions and other instructions */
	__asm(
		"ldr r12, =0x10008C4D;"
		"bx r12;");
}

/*----------------------------------------------------------------------------
insert code after SG instruction in the NSC region, BX to non-secure world
 *----------------------------------------------------------------------------*/
void test_sg_2_ns_s()
{
	int ins_sz = 14;
	char ins[] = {
		0x7f, 0xe9, 0x7f, 0xe9, // SG
		0x40, 0xf2, 0x01, 0x00, // MOVW r0, #0
		0xc0, 0xf2, 0x30, 0x00, // MOVT r0, #0x30
		0x00, 0x47				// BX r0    branch to non-secure world
	};
	char *dst = (char *)0x10008C4C;
	for (int i = 0; i < ins_sz; i++)
		dst[i] = ins[i];

	__asm(
		"ldr r12, =0x10008C4D;"
		"bx r12;");
}

/*----------------------------------------------------------------------------
insert code after SG instruction in the NSC region, execute move instruction
 *----------------------------------------------------------------------------*/
void test_malicious_sg_s()
{
	int ins_sz = 8;
	char ins[] = {
		0x20, 0x20, 0x7f, 0xe9, // SG
		0x10, 0x00, 0x00, 0x00	// MOVW r0, #0
	};
	char *dst = (char *)0x10008C4C;
	for (int i = 0; i < ins_sz; i++)
		dst[i] = ins[i];

	__asm(
		"ldr r12, =0x10008C4F;"
		"bx r12;");
}

/*----------------------------------------------------------------------------
dwt_test_s: monitor the data access to an address. 
While using the debugger, it uses the halting debug with the DCB->DHCSR debug key enabled.
!! SET *(uint32_t*)0xE000EDF0 to value a05f0000 (0xA05F<<16) via the debugger memory to disable the halting debug, 
or load the image directly to the board !!!
then dwt will work. ()
ID[31:27]	bits[26:25]	Matched[24]	bits[23:12]		Datasize[11:10]	bits[9:6]	action[5:4]	match[3:0]
data addr																				4 bytes										debug event	data address, writes				
01000			00					0						00000000 0000	00							0000			01					0100 
*----------------------------------------------------------------------------*/
void test_dwt_s()
{
	uint32_t data[4] = {0};
	DCB->DEMCR |= DCB_DEMCR_MON_EN_Msk | DCB_DEMCR_TRCENA_Msk;
	/*
	DWT->COMP0 = (uint32_t)&data[0];
	DWT->FUNCTION0 = 0b01000000000000000000100000010100;
	*/
	dwt_install_watchpoint(0, (uint32_t)&data[0], 0b01000000000000000000100000010100);
	dwt_install_watchpoint(1, (uint32_t)&data[1], 0b01000000000000000000100000010100);
	dwt_install_watchpoint(2, (uint32_t)&data[2], 0b01000000000000000000100000010100);
	dwt_install_watchpoint(3, (uint32_t)&data[3], 0b01000000000000000000100000010100);

	lcd_print(3, "start to test dwt");
	for (uint32_t i = 0; i < 4; i++)
	{
		data[i] = i;
	}
}

/*----------------------------------------------------------------------------
test_dwt_ns_s: monitor the non-secure state code writes to 0x28203000
 *----------------------------------------------------------------------------*/
void test_dwt_ns_s() __attribute__((cmse_nonsecure_entry));
void test_dwt_ns_s()
{
	char *test = (char *)0x28203000;
	DCB->DEMCR |= DCB_DEMCR_MON_EN_Msk | DCB_DEMCR_TRCENA_Msk;
	DWT->COMP0 = (uint32_t)0x28203000;
	DWT->FUNCTION0 = 0b01000000000000000000100000010100;
	//dwt_install_watchpoint(0, 0x28203000, 0b01000000000000000000100000010100);

	lcd_print(3, "start to test dwt in secure");
	//test[0] = 8;
}

/* define a non-secure call */
void test_ns_function_s()
{
	NonSecure_fpVoid test_ns_function = 0x0000;
	test_ns_function();
}

/*----------------------------------------------------------------------------
 	Call test cases after initializing non-secure project
 *----------------------------------------------------------------------------*/
int32_t test_cases_from_ns(uint32_t index) __attribute__((cmse_nonsecure_entry));
int32_t test_cases_from_ns(uint32_t index)
{
	switch (index)
	{
	case TEST_CASE_S:
		test_cases();
		break;
	case TEST_ACCESS_NS_S:
		test_access_ns_data();
		break;
	case TEST_ALIAS_MPC_S:
		test_alias_s();
		break;
	case TEST_SWITCH_NS_S:
		test_switch_s();
		break;
	case TEST_NSC_NS_S:
		test_nsc_ns_s();
		break;
	case TEST_SCB_NS_S:
		test_scb_ns_s();
		break;
	case TEST_CRTL_NS_S:
		test_ctrl_ns_s();
		break;
	case TEST_UP_ACCESS_NS_S:
		return test_lp_access_from_ns();
		break;
	case MPU_CONFIG_MPSSSRAM3_NS_S:
		mpu_config_mpsssram3_ns_s();
		break;
	case DROP_P_NS_S:
		DROP_S_PRIVILEGES;
		break;
	case SET_P_NS_S:
		EnablePrivilegedMode;
		//test_sg_call_ns_s();
		break;
	default:
		break;
	}
	return 0;
}

/*----------------------------------------------------------------------------
 	Call test cases in the Secure project
 *----------------------------------------------------------------------------*/
void test_cases_s(uint32_t test_id)
{
	//  GLCD_DrawString(0*16, 3*24, "   CactLab Test Cases   ");
	//  GLCD_DrawString(0*16, 4*24, "    Secure/Non-Secure  ");

	switch (test_id)
	{
		/* test helper functions */
	case TEST_USER_BUTTON_S:
		lcd_print(3, "-- Secure test user bottons --");
		GLCD_ClearScreen();
		test_user_botton_s();
		break;
	case TEST_SYSTICK_S:
		lcd_print(3, "-- Secure test SysTick --");
		test_systick_s();
		break;
	case TEST_LCD_S:
		print_case_info(&lcd_pos, "test_lcd_s");
		test_lcd_s();
		break;
	case TEST_MPU_MPC_ROM_CONFIG_S:
		lcd_print(3, "-- Secure test mpu mpc rom configuration --");
		test_mpu_mpc_rom_config_s();
		break;
	case TEST_MOD_PRIVILEGES:
		lcd_print(3, "-- Secure test drop privilege --");
		test_drop_privileges();
		break;

	/* test sau, mpc, ppc, mpu configuration*/
	case TEST_SAU:
		lcd_print(3, "-- Secure test SAU configuration --");
		test_sau();
		break;
	case TEST_MPC:
		lcd_print(3, "-- Secure test MPC configuration --");
		test_mpc_s();
		break;
	case TEST_PPC:
		lcd_print(3, "-- Secure test PPC configuration --");
		test_config_ppc_s();
		break;
	case TEST_MPU_S:
		lcd_print(3, "-- Secure test MPU configuration --");
		test_mpu_s();
		break;

	/* test state transitions (benign) */
	case TEST_SWITCH_S:
		lcd_print(3, "-- Secure test state switches --");
		test_switch_s();
		break;

	case TEST_MOD_IPSR:
		lcd_print(3, "-- Secure test modify ipsr --");
		test_ipsr();
		break;
	case TEST_UNSTACKING_S:
		lcd_print(3, "-- Secure test unstacking --");
		test_unstacking_s();
		break;
	case TEST_DWT_S:
		lcd_print(3, "-- Secure test dwt --");
		test_dwt_s();
		break;
	case TEST_MTB:
		lcd_print(3, "-- Secure test MTB --");
		test_mtb_s_infinite_loop();
		//test_mtb_s_stack_overflow();
		//test_mtb_s_memory_corrup();
		break;
	case TEST_BB_S:
		lcd_print(3, "-- Secure test bad branches --");
		test_bb_s();
		break;
	case TEST_SVC_S:
		lcd_print(3, "-- Secure test SVC --");
		test_svc_s();
		break;
	case TEST_TT_S:
		lcd_print(3, "-- Secure test TT instruction --");
		test_tt_s();
		break;
	case TEST_SG_S:
		lcd_print(3, "-- Secure test SG instruction --");
		test_sg_s();
		//test_malicious_sg_s();
		//test_insert_nsc_code_s();
		break;
	case TEST_SG_2_NS_S:
		lcd_print(3, "-- Secure test SG instruction2 --");
		test_sg_2_ns_s();
		break;
	default:
		break;
	}
}

void test_cases()
{
	test_cases_s(TEST_SWITCH_S);
}
