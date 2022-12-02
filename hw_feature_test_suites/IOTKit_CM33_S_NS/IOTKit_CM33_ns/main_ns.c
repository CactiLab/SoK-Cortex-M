/*----------------------------------------------------------------------------
 * Name:    main_ns.c
 * Purpose: Main function non-secure mode
 *----------------------------------------------------------------------------*/

#include "test_ns.h"

/*----------------------------------------------------------------------------
  NonSecure functions used for callbacks
 *----------------------------------------------------------------------------*/
int32_t test_add_ns(uint32_t a, uint32_t b)
{
  uint32_t c;
  c = a + b;
  return c;
}

static uint32_t x;
/*----------------------------------------------------------------------------
  Main function
 *----------------------------------------------------------------------------*/
int main(void)
{
  uint32_t i;

  /* exercise some core register from Non Secure Mode */
  x = __get_MSP();
  x = __get_PSP();

  /* register NonSecure callbacks in Secure application */
  Secure_test_add_ns_callback(test_add_ns);

#if 0
  LED_Initialize ();                      /* already done in Secure part */
#endif

  SystemCoreClockUpdate();
  SysTick_Config(SystemCoreClock / 1000); /* Generate interrupt each 1 ms */

  elapsed_time_init();

  //test_cases_from_ns(TEST_CASE_S);
  test_cases();
}
