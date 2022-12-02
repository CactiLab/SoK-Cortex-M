/*----------------------------------------------------------------------------
 * Name:    main_s.c
 * Purpose: Main function secure mode
 *----------------------------------------------------------------------------*/

#include "test_s.h" /* declarations for tests and common header files */

/* Start address of non-secure application */
#define NONSECURE_START (0x00200000u)
#ifdef UNDEF_INS
__asm("msr CONTROL, %[reg] " ::[reg] "r"(1)
      : "memory");
#endif

extern int stdout_init(void);

/*----------------------------------------------------------------------------
LCD Initial: setup the screen and LED
 *----------------------------------------------------------------------------*/
void SystemLCDInit(void)
{

  LED_Initialize();
  GLCD_Initialize();

  /* display initial screen 16*24*/
  GLCD_SetFont(&GLCD_Font_16x24);
  GLCD_SetBackgroundColor(GLCD_COLOR_WHITE);
  GLCD_ClearScreen();
  GLCD_SetBackgroundColor(GLCD_COLOR_BLUE);
  GLCD_SetForegroundColor(GLCD_COLOR_RED);
  GLCD_DrawString(0 * 16, 0 * 24, "   xxxxxxxx Demo    ");
  GLCD_DrawString(0 * 16, 1 * 24, "      TZ Pro        ");

  /* display initial screen 6*8*/
  GLCD_SetFont(&GLCD_Font_6x8);
  GLCD_SetBackgroundColor(GLCD_COLOR_WHITE);
  GLCD_SetForegroundColor(GLCD_COLOR_BLACK);
}

void test_init()
{
  /* ret=0 means the systick interrupt initialization successfully */
  int ret = SysTick_Config(SystemCoreClock / 1000); /* Generate interrupt each 1 ms */
  //int ret = SysTick_Config(SystemCoreClock/1000000); /* Generate interrupt each 0.0001 ms */

  elapsed_time_init();

  /* configure the MPU */
  //arm_mpu_config_s();
}

static uint32_t x;
/*----------------------------------------------------------------------------
  Main function
 *----------------------------------------------------------------------------*/
int main(void)
{
  uint32_t NonSecure_StackPointer = (*((uint32_t *)(NONSECURE_START + 0u)));
  NonSecure_fpVoid NonSecure_ResetHandler = (NonSecure_fpVoid)(*((uint32_t *)(NONSECURE_START + 4u)));

  /* exercise some core register from Secure Mode */
  x = __get_MSP();
  x = __get_PSP();
  __TZ_set_MSP_NS(NonSecure_StackPointer);
  x = __TZ_get_MSP_NS();
  __TZ_set_PSP_NS(0x22000000u);
  x = __TZ_get_PSP_NS();

  SystemCoreClockUpdate();
  stdout_init(); /* Initializ Serial interface */
  SystemLCDInit();

  test_init();

  /* all test cases are in this function, called by main function */

  //test_cases();

  NonSecure_ResetHandler();
}
