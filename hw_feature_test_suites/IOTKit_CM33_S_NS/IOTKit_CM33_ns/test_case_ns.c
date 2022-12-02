#include "test_ns.h"
static char msg[MAXMSG]; // macro constant

/*----------------------------------------------------------------------------
stack sealing senario1:
  	
 *----------------------------------------------------------------------------*/
void test_unstacking_ns()
{
	__asm("SVC #66");
}

/*----------------------------------------------------------------------------
  	Try to access secure region from no-secure code
		test1: Call NSC function to read non-secure data section from secure world
		test2: Secure code memory: 0x1000_0000 - 0x1DFF_FFFF, trigger secure fault: tribution unit violation
 *----------------------------------------------------------------------------*/
void test_access_ns()
{
	/*  access secure region from non-secure world: secure fault */
	char *test1 = (char *)0x10000000;
	*test1 = 1; /* write to secure region */
}

/*----------------------------------------------------------------------------
	access non-secure region from secure world
 *----------------------------------------------------------------------------*/
void test_access_s()
{
	volatile char *p_ns = (char *)0x28200000;
	for (int i = 0; i < 9; i++)
	{
		p_ns[i] = '$';
	}
	p_ns[9] = '\n';
	test_cases_from_ns(TEST_ACCESS_NS_S);
}

volatile ALIAS_REGION alias_region_tbl[MAX_ALIAS];

/*----------------------------------------------------------------------------
	test MPC alias
 *----------------------------------------------------------------------------*/
void test_mpc_alias_ns()
{
	/* ns ssram1 alias with s ssram1 alias*/
	uint32_t data_ns[4];
	//data_ns[0] = *((uint32_t *)0x28300000); // idau ns ssram3
	alias_region_tbl[3].data = *((uint32_t *)0x28300000); //ns ssram3
	alias_region_tbl[3].data_alias = 0;					  //s ssram3 can not access, RAZ

	test_cases_from_ns(TEST_ALIAS_MPC_S);
	//data_ns[1] = *((uint32_t *)0x28300000); // idau ns ssram3
}

/*----------------------------------------------------------------------------
	non-secure can not access MPCSSRAM1, will trigger secure fault: attribution unit volation
 *----------------------------------------------------------------------------*/
void test_mpc_ns()
{
	IOTKIT_MPCSSRAM1->CTRL &= ~(1UL << 8U);	  /* clear auto increment */
	IOTKIT_MPCSSRAM1->BLK_IDX = 0;			  /* write LUT index */
	IOTKIT_MPCSSRAM1->BLK_LUT = 0xFFFF0000UL; /* configure blocks.  0x00200000-0x003fffff: non-secure; 0x00000-0x1ffff: secure, alias to 0x10000000-0x101fffff*/
}

void test_reconfig_mpc_ns()
{
	/* by default, SSRAM2 (0x28000000 - 0x281FFFFF) is configured as secure, so non-secure world can not access this region */
	int data_ns = 0;
	data_ns = *((uint32_t *)0x28000000);
	// test_cases_from_ns(TEST_RECONFIG_MPC_S);
}

/*----------------------------------------------------------------------------
TT tests: target test instructions
 *----------------------------------------------------------------------------*/
void test_tt_ns(void) //0x00202806 test_tt_ns
{
	__asm volatile(
		"MOVW R0, #0x0000;"
		"MOVT R0, #0x1010;"
		"TT r0, r0;"
		"MOVW R0, #0x0000;"
		"MOVT R0, #0x1010;"
		"TTT r0, r0;"

		"MOVW R0, #0x0000;"
		"MOVT R0, #0x0030;"
		"TT r0, r0;"
		"MOVW R0, #0x0000;"
		"MOVT R0, #0x0030;"
		"TTT r0, r0;");
	DROP_NS_PRIVILEGES;
	__asm volatile(
		"MOVW R0, #0x0000;"
		"MOVT R0, #0x1010;"
		"TT r0, r0;"
		"MOVW R0, #0x0000;"
		"MOVT R0, #0x1010;"
		"TTT r0, r0;"

		"MOVW R0, #0x0000;"
		"MOVT R0, #0x0030;"
		"TT r0, r0;"
		"MOVW R0, #0x0000;"
		"MOVT R0, #0x0030;"
		"TTT r0, r0;");
	__asm volatile("SVC #0");
}

void test_switch_ns()
{
	test_add_ns(1, 2); /* run this function to get the address: 0x00201AA0 */

	/* 1. non-secure privilege thread --> secure privilege thread */
	test_add_s(1, 1);
	/* 15. return: secure privilege thread --> non-secure privilege thread*/

	/* 2. non-secure privilege thread --> non-secure privilege handler: exception */
	__asm("SVC #28"); // svc 28, nothing, return
	/* 12. return: non-secure privilege handler --> non-secure privilege thread: exception exit*/

	/* 3. non-secure privilege thread --> secure privilege handler: secure fault */
	// test_access_ns();

	__asm("SVC #20"); // define svc to drop privilege in the handler mode.
	/* 4. non-secure privilege thread --> non-secure non-privilege thread: set CONTROL[0] */
	__asm("msr CONTROL, %[reg] " ::[reg] "r"(1)
		  : "memory");

	/* 5. non-secure non-privilege thread --> secure privilege thread */
	test_add_s(5, 5);
	/* 19. return: secure privilege thread --> non-secure non-privilege thread */

	/* 5 */
	test_cases_from_ns(DROP_P_NS_S); // secure privilege changes to secure non-privilege
	/* 18. return: secure non-privilege thread --> non-secure non-privilege thread */

	/* 6. non-secure non-privilege thread --> secure non-privilege thread */
	test_add_s(2, 2);
	/* 17. return: secure non-privilege handler --> non-secure non-privilege thread */

	/* 7. non-secure non-privilege thread --> secure privilege handler: secure fault */
	// test_access_ns();

	/* 8. non-secure non-privilege thread --> non-secure privilege handler: exception */
	__asm("SVC #28"); // svc, nothing, return
	/* 16. return: non-secure privilege handler --> non-secure non-privilege thread: exception exit*/

	/* 9. non-secure privilege thread --> secure non-privilege thread */
	test_add_s(3, 3);
	/* 15. return: secure non-privilege thread --> non-secure privilege thread */

	/* 10. non-secure privilege handler --> secure non-privilege thread */
	__asm("SVC #10"); // define svc 15 to call nsc function
	/* 14. return: secure privilege thread --> non-secure privilege handler */

	/* 9. */
	test_cases_from_ns(SET_P_NS_S); // enable secure privilege
	/* 13. return: secure privilege thread --> non-secure privilege thread */
	/* 2. */

	/* 10. non-secure privilege handler --> secure privilege thread */
	__asm("SVC #10"); // define svc to call nsc function
	/* 12. return: non-secure privilege handler --> non-secure privilege thread: exception exit*/

	/* 11. non-secure privilege handler --> secure privilege handler: secure fault */
	//__asm("SVC #12"); // define svc to access secure region

	test_cases_from_ns(TEST_SWITCH_NS_S);
}

/*----------------------------------------------------------------------------
test branches
 *----------------------------------------------------------------------------*/
void test_b_switch_ns()
{
	// NS-P BL/BLX/BXNS to NSC
	__asm volatile("BL #0x100056cc;");
	__ASM volatile(
		"MOVW R0, #0x56cc;"
		"MOVT R0, #0x1000;"
		"BLX R0;");
	__ASM volatile(
		"MOVW r12, #0x56cc;"
		"MOVT r12, #0x1000;"
		"BXNS R12;");
	// NS-P BL/BLX/BXNS to S
	__asm volatile("BL #0x10001000;");
	__ASM volatile(
		"MOVW R0, #0x1000;"
		"MOVT R0, #0x1000;"
		"BLX R0;");
	__ASM volatile(
		"MOVW r12, #0x1000;"
		"MOVT r12, #0x1000;"
		"BXNS R12;");

	DROP_NS_PRIVILEGES;
	// NS-UP BL/BLX/BXNS to NSC
	__asm volatile("BL #0x100056cc;");
	__ASM volatile(
		"MOVW R0, #0x56cc;"
		"MOVT R0, #0x1000;"
		"BLX R0;");
	__ASM volatile(
		"MOVW r12, #0x1000;"
		"MOVT r12, #0x1000;"
		"BXNS R12;");
	// BS-UP BL/BLX/BXNS to S
	__asm volatile("BL #0x10001000;");
	__ASM volatile(
		"MOVW R0, #0x1000;"
		"MOVT R0, #0x1000;"
		"BLX R0;");
	__ASM volatile(
		"MOVW r12, #0x1000;"
		"MOVT r12, #0x1000;"
		"BXNS R12;");
}

/*----------------------------------------------------------------------------
	Non-secure state calls NSC function V.S. Secure state calls NSC function
 *----------------------------------------------------------------------------*/

void test_nsc_ns()
{
	test_add_s(5, 6); // SG: 0x10004BC9, test_add_s: 0x1001A44
}

/*----------------------------------------------------------------------------
		Call/Jump to an SG instruction that is not in the NSC
*----------------------------------------------------------------------------*/
void test_sg()
{
	/* 0x100014D8 is the address of test_cases_s, not NSC. 
	When running, 0x100014D8 is changed to 0x201d84 which is a non-secure address*/
	__ASM volatile(
		"SG;"
		"MOVW R0, #0x14D8;"
		"MOVT R0, #0x1000;"
		"BLX R0;");
}

/*----------------------------------------------------------------------------
		Read/Write to SCB
*----------------------------------------------------------------------------*/
void test_scb_ns()
{
#ifdef DROP_P_NS
	DROP_NS_PRIVILEGES;
#endif
	/* Read from SCB */
	uint32_t CFSRValue;
	CFSRValue = SCB->CFSR;
	/* Write to SCB, BFSR register setting to enable precise errors (borrow from system_IOTKit_CM33.c) */
	SCB->CFSR = 7;
}

void test_scb_ns_s()
{
	test_cases_from_ns(TEST_SCB_NS_S);
}
/*----------------------------------------------------------------------------
		Read/Write to CONTROL
*----------------------------------------------------------------------------*/
void test_ctrl_ns()
{
#ifdef DROP_P_NS
	DROP_NS_PRIVILEGES;
#endif
	/* read from control */
	__ASM volatile("MRS r12, CONTROL");

	/* write to control */
	__ASM volatile("MSR CONTROL, %[reg] " ::[reg] "r"(1)
				   : "memory");
	__ASM volatile("MSR CONTROL, %[reg] " ::[reg] "r"(0)
				   : "memory");
}

/* Both secure and non-secure world are non-privilege mode */
void test_lp_access_s_ns()
{
	test_cases_from_ns(DROP_P_NS_S);
	DROP_NS_PRIVILEGES;
	test_add_s(1, 2);
}

/* Only non-secure world is non-privilege mode */
void test_lp_access_ns()
{
	DROP_NS_PRIVILEGES;
	/* non-privilege mode non-secure world call NSC function */
	test_add_s(1, 2);
}

/* sram3 is configured non-secure r/w privilege only, but secuer world still can access this region */
void test_lp_access()
{
	volatile char *p_ns = (char *)0x28200000;
	char ns = NULL;
	/* read/write this region */
	ns = p_ns[0];
	p_ns[0] = 'A';
	// Configure this region as Privileged read only
	/* secure MPU can not control the access in non-secure world */
	// test_cases_from_ns(MPU_CONFIG_MPSSSRAM3_NS_S);
	mpu_config_mpsssram3_ns();
	/* read/write thie region, trigger memory management fault */
	ns = p_ns[0];
	p_ns[0] = 'B';
	// Drop privilege for both non-secure and secure world
	test_cases_from_ns(DROP_P_NS_S);
	DROP_NS_PRIVILEGES;
	// Call NSC function to access this region, low privilege can call NSC function
	ns = test_cases_from_ns(TEST_UP_ACCESS_NS_S);
}

/*----------------------------------------------------------------------------
measurement helper function
 *----------------------------------------------------------------------------*/
void test_state_switch_time_helper()
{
	uint32_t tmp0 = 0, tmp1 = 0, time_state_switch_min0 = 0, time_state_switch_max0 = 0, time_state_switch_min1 = 0, time_state_switch_max1 = 0;
	float time_state_switch0 = 0, time_state_switch1 = 0;
	uint32_t count = 0;
	DISABLE_IRQ;
	for (count = 0; count < MAXCOUNT; count++)
	{
		elapsed_time_start(0);
		test_state_switch_s();
		elapsed_time_stop(1);
		tmp0 = elapsed_time_result(0);
		tmp1 = elapsed_time_result(1);
		if (count == 0)
		{
			time_state_switch_min0 = tmp0;
			time_state_switch_max0 = tmp0;
			time_state_switch_min1 = tmp1;
			time_state_switch_max1 = tmp1;
			time_state_switch0 += tmp0;
			time_state_switch1 += tmp1;
		}
		else
		{
			if (tmp0 < time_state_switch_min0)
				time_state_switch_min0 = tmp0;
			if (tmp0 > time_state_switch_max0)
				time_state_switch_max0 = tmp0;
			if (tmp1 < time_state_switch_min1)
				time_state_switch_min1 = tmp1;
			if (tmp1 > time_state_switch_max1)
				time_state_switch_max1 = tmp1;
			time_state_switch0 += (float)tmp0;
			time_state_switch1 += (float)tmp1;
		}
		elapsed_time_clr(0);
		elapsed_time_clr(1);
	}
	ENABLE_IRQ;
	//Secure_lcd_print(1, "Non-secure to Secure (time cycles):");
	sprintf(msg, "ns->s: min %d, max %d, aver %0.4f\n", time_state_switch_min0, time_state_switch_max0, time_state_switch0 / MAXCOUNT);
	Secure_lcd_print(1, msg);
	//Secure_lcd_print(3, "Secure to Non-secure (time cycles):");
	sprintf(msg, "s->ns: min %d, max %d, aver %0.4f\n", time_state_switch_min1, time_state_switch_max1, time_state_switch1 / MAXCOUNT);
	Secure_lcd_print(2, msg);
}

/*----------------------------------------------------------------------------
 measurement for security state switch
 *----------------------------------------------------------------------------*/
void test_state_switch_time()
{
	/* check if this board implements the cache */
	switch (check_cache_ns())
	{
	case NoCache: /* No cache, macro*/
		test_state_switch_time_helper();
		break;
	case ICache: /* Instruction cache only*/
		ENABLE_I_CACHE;
		test_state_switch_time_helper();
		DISABLE_I_CACHE;
		test_state_switch_time_helper();
		break;
	case DCache: /* Data cache only */
		ENABLE_D_CACHE;
		test_state_switch_time_helper();
		DISABLE_D_CACHE;
		test_state_switch_time_helper();
		break;
	case I_D_Cache: /* Seperate instruction and data caches */
		break;
	case IDCache: /* Unified cache */
		/* 1. measure state switching with instruction cache enabled and data cache enabled */
		ENABLE_I_CACHE;
		ENABLE_D_CACHE;
		test_state_switch_time_helper();
		/* 2. measure state switching with instruction cache disabled and data cache enabled */
		DISABLE_D_CACHE;
		test_state_switch_time_helper();
		/* 3. measure state switching with instruction cache enabled and data cache disabled */
		DISABLE_I_CACHE;
		ENABLE_D_CACHE;
		test_state_switch_time_helper();
		/* 4. measure state switching with instruction cache disabled and data cache disabled */
		DISABLE_D_CACHE;
		test_state_switch_time_helper();
	}
}
void test_privilege_switch_time_missed_cycles()
{
	uint32_t tmp = 0, time_privilege_switch_min = 0, time_privilege_switch_max = 0;
	float time_privilege_switch = 0;
	uint32_t count = 0;
	for (count = 0; count < MAXCOUNT; count++)
	{
		//elapsed_time_start(3);
		DROP_NS_PRIVILEGES;
		EnablePrivilegedMode;
		elapsed_time_stop(2);
		ENABLE_IRQ;
		tmp = elapsed_time_result(2);
		if (count == 0)
		{
			time_privilege_switch_min = tmp;
			time_privilege_switch_max = tmp;
			time_privilege_switch += tmp;
		}
		else
		{
			if (tmp < time_privilege_switch_min)
				time_privilege_switch_min = tmp;
			if (tmp > time_privilege_switch_max)
				time_privilege_switch_max = tmp;
			time_privilege_switch += (float)tmp;
		}
		elapsed_time_clr(2);
	}

	//Secure_lcd_print(1, "Non-privilege to Privilege (missed time cycles):");
	sprintf(msg, "np->p: min %d, max %d, aver %0.4f\n", time_privilege_switch_min, time_privilege_switch_max, time_privilege_switch / MAXCOUNT);
	Secure_lcd_print(3, msg);
}

void test_privilege_switch_time_added_cycles()
{
	uint32_t tmp = 0, time_privilege_switch_min = 0, time_privilege_switch_max = 0;
	float time_privilege_switch = 0;
	uint32_t count = 0;

	for (count = 0; count < MAXCOUNT; count++)
	{
		elapsed_time_start(3);
		DROP_NS_PRIVILEGES;
		EnablePrivilegedMode;
		elapsed_time_stop(3);
		ENABLE_IRQ;
		tmp = elapsed_time_result(3);
		if (count == 0)
		{
			time_privilege_switch_min = tmp;
			time_privilege_switch_max = tmp;
			time_privilege_switch += tmp;
		}
		else
		{
			if (tmp < time_privilege_switch_min)
				time_privilege_switch_min = tmp;
			if (tmp > time_privilege_switch_max)
				time_privilege_switch_max = tmp;
			time_privilege_switch += (float)tmp;
		}
		elapsed_time_clr(3);
	}

	//Secure_lcd_print(1, "Non-privilege to Privilege (added time cycles):");
	sprintf(msg, "lp->p: min %d, max %d, aver %0.4f\n", time_privilege_switch_min, time_privilege_switch_max, time_privilege_switch / MAXCOUNT);
	Secure_lcd_print(4, msg);
}

void test_ctr_instruction_time()
{
	uint32_t tmp = 0, time_ins_min = 0, time_ins_max = 0;
	float time_ins = 0;
	uint32_t count = 0;

	for (count = 0; count < MAXCOUNT; count++)
	{
		elapsed_time_start(0);
		__ASM volatile("MSR control, %0"
					   :
					   : "r"(0b100)
					   : "memory"); // __set_CONTROL(0b10);
		elapsed_time_stop(0);
		ENABLE_IRQ;
		tmp = elapsed_time_result(0);
		if (count == 0)
		{
			time_ins_min = tmp;
			time_ins_max = tmp;
			time_ins += tmp;
		}
		else
		{
			if (tmp < time_ins_min)
				time_ins_min = tmp;
			if (tmp > time_ins_max)
				time_ins_max = tmp;
			time_ins += (float)tmp;
		}
		elapsed_time_clr(0);
	}

	//Secure_lcd_print(1, "Write to Control register (time cycles):");
	sprintf(msg, "w->ctr: min %d, max %d, aver %0.4f\n", time_ins_min, time_ins_max, time_ins / MAXCOUNT);
	Secure_lcd_print(5, msg);
}

void test_running_time()
{
	/* running time for calling NSC function */
	//for (uint32_t count = 0; count < 50; count++)
	//{
	test_state_switch_time();
	/* running time for privilege switch */
	test_privilege_switch_time_missed_cycles(); // uncomment #define MISSED_CYCLES and this line
	test_privilege_switch_time_added_cycles();
	test_ctr_instruction_time();
	//}
}

void test_dwt_ns()
{
	/* install the datawatchpoint in the secure state */
	test_dwt_ns_s();

	char *test = (char *)0x28203000;
	Secure_lcd_print(3, "start to test dwt in non-secure");
	test[0] = 8;
}

void test_cases_ns(uint32_t test_id)
{
	//int pos = mtb_pause();
	Secure_lcd_print(2, "-- Case testing from non-secure world --");
	//mtb_resume(MTB_BUFF, pos);
	switch (test_id)
	{
	case TEST_TT_NS:
		Secure_lcd_print(3, "-- Non-secure test TT instruction --");
		test_tt_ns();
		break;
	case TEST_UNSTACKING_NS:
		/* use PSP */
		Secure_lcd_print(3, "-- Non-secure test unstacking --");
		SwitchStackPointer_ns();
		test_unstacking_ns();
		break;
	case TEST_ACCESS_NS:
		Secure_lcd_print(3, "-- Non-secure test memory access --");
		//	test_access_ns();
		test_access_s();
		break;

	case TEST_ALIAS_MPC_NS:
		Secure_lcd_print(3, "-- Non-secure test access MPC alias --");
		test_mpc_alias_ns();
		break;

	case TEST_MPC_NS:
		Secure_lcd_print(3, "-- Non-secure test MPC configuration --");
		test_mpc_ns();
		break;

	case TEST_SWITCH_NS:
		Secure_lcd_print(3, "-- Non-secure test state switches --");
		test_switch_ns();
		break;

	case TEST_NSC_NS:
		Secure_lcd_print(3, "-- Non-secure test NSC function --");
		test_nsc_ns();
		break;

	case TEST_DWT_NS:
		Secure_lcd_print(3, "-- Non-secure test dwt --");
		test_dwt_ns();
		break;

	case TEST_SCB_NS:
		Secure_lcd_print(3, "-- Non-secure test SCB --");
		test_scb_ns();
		test_scb_ns_s();
		break;

	case TEST_CTRL_NS:
		Secure_lcd_print(3, "-- Non-secure test CTRL --");
		test_ctrl_ns();
		break;

	case TEST_LP_ACCESS:
		Secure_lcd_print(3, "-- Non-secure test Low privilege access --");
		test_lp_access_s_ns();
		test_lp_access_ns();
		test_lp_access();
		break;

	case TEST_RUNNING_TIME:
		Secure_lcd_print(3, "-- Non-secure test time measurement --");
		test_running_time();
		break;

	default:
		break;
	}
}

void test_cases()
{
	//arm_mpu_config_ns();
	test_cases_ns(TEST_SWITCH_NS);
}