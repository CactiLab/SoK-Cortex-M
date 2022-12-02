/*----------------------------------------------------------------------------
 * Name:    Blinky.c
 * Purpose: LED Flasher for MPS2
 * Note(s): possible defines set in "options for target - C/C++ - Define"
 *            __USE_LCD    - enable Output on GLCD
 *            __USE_TIMER0 - use Timer0  to generate timer interrupt
 *                         - use SysTick to generate timer interrupt (default)
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2015 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include "test.h"

//extern GLCD_FONT GLCD_Font_16x24;

extern int stdout_init(void);

/*----------------------------------------------------------------------------
  SysTick IRQ Handler
 *----------------------------------------------------------------------------*/
void SysTick_Handler(void)
{

  static uint32_t ticks;
  ticks++;
}

void GLCD_display()
{
	GLCD_Initialize();  /* Initialize the GLCD            */
	 /* display initial screen */
  GLCD_SetFont(&GLCD_Font_16x24);
  GLCD_SetBackgroundColor(GLCD_COLOR_WHITE);
  GLCD_ClearScreen();
  GLCD_SetBackgroundColor(GLCD_COLOR_BLUE);
  GLCD_SetForegroundColor(GLCD_COLOR_RED);
  GLCD_DrawString(0 * 16, 0 * 24, "   xxxxxxxx Test    ");
  GLCD_DrawString(0 * 16, 1 * 24, "      Blinky        ");

  GLCD_SetBackgroundColor(GLCD_COLOR_WHITE);
  GLCD_SetForegroundColor(GLCD_COLOR_BLACK);
	
	GLCD_SetFont(&GLCD_Font_6x8);
}

/*----------------------------------------------------------------------------
  Main function
 *----------------------------------------------------------------------------*/
int main(void)
{
  uint32_t ledMax = LED_GetCount() - 1;
  uint32_t ledNum = 0;
  uint32_t keyMax = Buttons_GetCount();
  uint32_t keyNum = 0;
  int32_t dir = 1;
  int32_t keyboard = 0, keyboard_ = -1;
  char text[] = "Hello World";
#ifdef __USE_LCD
  TOUCH_STATE tsc_state;
  int32_t tsc = -1;
  int32_t i;
#endif

  SystemCoreClockUpdate();

  LED_Initialize();     /* Initializ LEDs                 */
  Buttons_Initialize(); /* Initializ Push Buttons         */
#ifdef RTE_Compiler_IO_STDOUT_User
  stdout_init(); /* Initializ Serial interface     */
#endif
  Touch_Initialize(); /* Initialize touchscreen         */
  
	GLCD_display();

	test_cases();
  
}
