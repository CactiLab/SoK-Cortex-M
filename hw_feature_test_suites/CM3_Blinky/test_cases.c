/*
test cases for cortex-m3 processsor
*/

#include "test.h"

/* check exception stacks */
void test_svc()
{
	DROP_S_PRIVILEGES;
	__ASM("SVC #0");
}

/*
TODO: DWT
void dwt_install_watchpoint(int comp_id, uint32_t addr, uint32_t mask, uint32_t func);
*/
void test_dwt()
{
	uint32_t data[4] = {0};
	CoreDebug->DEMCR |= CoreDebug_DEMCR_MON_EN_Msk | CoreDebug_DEMCR_TRCENA_Msk;
	/*
	DWT->COMP0 = (uint32_t)&data[0];
	DWT->FUNCTION0 = 0b01000000000000000000100000010100;
	*/
	dwt_install_watchpoint(0, (uint32_t)&data[0], 0x0, 0x6);
	dwt_install_watchpoint(1, (uint32_t)&data[1], 0x0, 0x6);
	dwt_install_watchpoint(2, (uint32_t)&data[2], 0x0, 0x6);
	dwt_install_watchpoint(3, (uint32_t)&data[3], 0x0, 0x6);

	lcd_print(3, "start to test dwt");
	for (uint32_t i = 0; i < 4; i++)
	{
		data[i] = i;
	}
}

void test_mpu()
{
	arm_mpu_config();
}

void test_cases_by_number(uint32_t index)
{
	switch (index)
	{
	case TEST_SVC:
		test_svc();
		break;
	case TEST_DWT:
		test_dwt();
		break;
	case TEST_MPU:
		test_mpu();
		break;
	default:
		break;
	}
}

void test_cases()
{
	test_cases_by_number(TEST_DWT);
}