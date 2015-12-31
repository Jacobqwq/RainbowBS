/*
----------------------------------------------------------------------
                     RainbowBS example
----------------------------------------------------------------------
*/

#include "stdafx.h"
#include "../../../RBSSource/RainbowBS.h"

#if (RBS_CFG_APP_MODEL == RBS_APP_PTP)
PROCESS(ProcessA,"A");//define process A Object
PROCESS(ProcessB,"B");//define process B Object
//define process A thread
PROCESS_THREAD(ProcessA,ev,data) {
  static tETIME etA;
  PROCESS_BEGIN();
  while (1) {
    if (ev == PROCESS_EVENT_INIT)
      etimer_set(&etA,10);
    else if (ev == PROCESS_EVENT_TIMER) {
      etimer_reset(&etA);
      printf("Process A:Hello!\n");
    }
    PROCESS_YIELD();
  }
  PROCESS_END();
}
//define process B thread
PROCESS_THREAD(ProcessB,ev,data) {
  static tETIME etB;
  PROCESS_BEGIN();
  while (1) {
    if (ev == PROCESS_EVENT_INIT)
      etimer_set(&etB,20);
    else if (ev == PROCESS_EVENT_TIMER) {
      etimer_reset(&etB);
      printf("Process B:Hello!\n");
    }
    PROCESS_YIELD();
  }
  PROCESS_END();
}
#endif

int _tmain(int argc, _TCHAR* argv[])
{
  //RBS initialization
  if (!RBS_Init())
    return 1;
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  printf("\nRBS running on OS Model.\n");
#elif (RBS_CFG_APP_MODEL == RBS_APP_PTP)
  printf("\nRBS running on Protothread Model.\n");
  process_start(&ProcessA,NULL);//start process A
  process_start(&ProcessB,NULL);//start process B
  while (1) {
    MSG msg;
    //execute processes
    do {
      ;
    } while (process_run() > 0);
	//dispatch message for timer on MS Windows
	if (GetMessage(&msg,NULL,0,0))
      DispatchMessage(&msg);
  }
#else
  printf("\nRBS running on Normal Model.\n");
#endif
  return 0;
}

