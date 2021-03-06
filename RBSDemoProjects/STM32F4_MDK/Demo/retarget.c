/*
----------------------------------------------------------------------
                     RainbowBS example
----------------------------------------------------------------------
*/

#include "stm32f4xx.h"
#include <stdio.h>

#pragma import(__use_no_semihosting_swi)

struct __FILE { 
  int handle; 
  /* Add whatever you need here */ 
};
FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f) {
  return (ITM_SendChar(ch));
}

void _sys_exit(int return_code) {
  label: goto label; /* endless loop */
}
