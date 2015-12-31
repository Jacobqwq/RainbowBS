/*
----------------------------------------------------------------------
                     RainbowBS example
----------------------------------------------------------------------
*/

#include "stm32f4xx.h"
#include "RainbowBS.h"
#include <stdio.h>

#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
//define process A task
void TaskA(void const *arg) {
	while(1) {
		RBS_DEBUG_LOG("Task A:Hello!");
		osDelay(10);
	}
}
//define process B task
void TaskB(void const *arg) {
  while(1) {
    RBS_DEBUG_LOG("Task B:Hello!");
    osDelay(20);
  }
}
osThreadDef(TaskA,osPriorityNormal,1,0);//define task A Object
osThreadDef(TaskB,osPriorityNormal,1,0);//define task A Object
#elif (RBS_CFG_APP_MODEL == RBS_APP_PTP)
PROCESS(ProcessA,"A");//define process A Object
PROCESS(ProcessB,"B");//define process B Object
//define process A thread
PROCESS_THREAD(ProcessA,ev,data) {
	static tETIME etA;
	PROCESS_BEGIN();
	while(1) {
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
	while(1) {
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

int main(void) {
  //RBS initialization
  if (!RBS_Init())
    return 1;
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
	RBS_DEBUG_LOG("\nRBS running on OS Model.\n");
	osThreadCreate(osThread(TaskA),NULL);
	osThreadCreate(osThread(TaskB),NULL);
	if (osFeature_MainThread) {
    osThreadTerminate(osThreadGetId());
  } else {
    osKernelStart();
    while(1);
	}
	return 0;
#elif (RBS_CFG_APP_MODEL == RBS_APP_PTP)
	printf("\nRBS running on Protothread Model.\n");
	process_start(&ProcessA,NULL);//start process A
	process_start(&ProcessB,NULL);//start process B
	while(1) {
		//execute processes
	  do {
		  ;
	  } while (process_run() > 0);
		//go to sleep
		printf("Go to sleep at %dms!\n",(U32)RBS_GetRunTime());
		__WFI();//wait for interrupt
		printf("Wakeup at %dms!\n",(U32)RBS_GetRunTime());
  }
#else
	printf("\nRBS running on Normal Model.\n");
	while(1) {
		printf("Hello!\n");
	}
#endif
}

/*************************** End of file ****************************/
