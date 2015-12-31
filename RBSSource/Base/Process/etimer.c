/*
----------------------------------------------------------------------
                   RainbowBS File : etimer.c
----------------------------------------------------------------------
*/

/*
 * Copyright (c) 2004, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/**
 * Implementation of the etimer process.
 * \author
 *         Adam Dunkels <adam@sics.se>\n
 *         QWQ <jacobqwq@icloud.com> modified for RainbowBS as following\n
 *         1. add 'continue' to etimer_process for speed because there is no free event object.
 *         2. modify add_timer() not call etimer_request_poll() for speed
 *            because an timer interrupt handle should call etimer_request_poll().
 *         3. delete etimer_next_expiration_time() for not used frequently and speeding up.
 */

#include "../../RainbowBS.h"

#if (RBS_CFG_APP_MODEL == RBS_APP_PTP)

static tETIME *timerlist;
//static TICK next_expiration;

PROCESS(etimer_process,"Event timer");
/*---------------------------------------------------------------------------*/
//static void update_time(void) {
//  TICK tdist;
//  TICK now;
//  tETIME *t;
//  if (timerlist == NULL) {
//    next_expiration = 0;
//  } else {
//    now = RBS_GetTickCount();
//    t = timerlist;
//    /* Must calculate distance to next time into account due to wraps */
//    tdist = t->start + t->interval - now;
//    for (t = t->next;t != NULL;t = t->next) {
//      if (t->start + t->interval - now < tdist)
//        tdist = t->start + t->interval - now;
//    }
//    next_expiration = now + tdist;
//  }
//}
/*---------------------------------------------------------------------------*/
static BOOL timer_expired(tETIME *t) {
  /* Note: Can not return diff >= t->interval so we add 1 to diff and return
     t->interval < diff - required to avoid an internal error in mspgcc. */
  TICK diff = (RBS_GetTickCount() - t->start) + 1;
  return t->interval < diff;
}
/*---------------------------------------------------------------------------*/
void etimer_request_poll(void) {
  process_poll(&etimer_process);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(etimer_process,ev,data)
{
  tETIME *t,*u;
  PROCESS_BEGIN();
  timerlist = NULL;
  while (1) {
    PROCESS_YIELD();
    /*remove event timers to specific process*/
    if (ev == PROCESS_EVENT_EXITED) {
      tPROCESS *p = data;
      while (timerlist != NULL && timerlist->p == p)
        timerlist = timerlist->next;
      if (timerlist != NULL) {
        t = timerlist;
        while (t->next != NULL) {
          if (t->next->p == p)
            t->next = t->next->next;
          else
            t = t->next;
        }
      }
      continue;
    } else if (ev != PROCESS_EVENT_POLL) {
      continue;
    }
    /*check event timers*/
again:
    u = NULL;
    for (t = timerlist;t != NULL;t = t->next) {
      if (timer_expired(t)) {
        if (process_post(t->p,PROCESS_EVENT_TIMER,t)) {//post PROCESS_EVENT_TIMER successfully
          /* Reset the process ID of the event timer, to signal that the
             etimer has expired. This is later checked in the
             etimer_expired() function. */
          t->p = PROCESS_NONE;
          if (u != NULL)
            u->next = t->next;
          else
            timerlist = t->next;
          t->next = NULL;
          //update_time();
          goto again;
        } else {//post PROCESS_EVENT_TIMER failed because no free event object
          etimer_request_poll();//post PROCESS_EVENT_TIMER later
          continue;//added for RBS
        }
      }
      u = t;
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static void add_timer(tETIME *timer) {
  tETIME *t;
  //etimer_request_poll();//commented for RBS
  if (timer->p != PROCESS_NONE) {
    for (t = timerlist;t != NULL;t = t->next) {
      if (t == timer) {
        /* Timer already on list, bail out. */
        timer->p = PROCESS_CURRENT();
        //update_time();
        return;
      }
    }
  }
  /* Timer not on list. */
  timer->p = PROCESS_CURRENT();
  timer->next = timerlist;
  timerlist = timer;
  //update_time();
}
/*---------------------------------------------------------------------------*/
void etimer_set(tETIME *et,TICK interval) {
  et->interval = interval;
  et->start = RBS_GetTickCount();
  add_timer(et);
}
/*---------------------------------------------------------------------------*/
void etimer_reset(tETIME *et) {
  et->start += et->interval;
  add_timer(et);
}
/*---------------------------------------------------------------------------*/
void etimer_restart(tETIME *et) {
  et->start = RBS_GetTickCount();
  add_timer(et);
}
/*---------------------------------------------------------------------------*/
void etimer_adjust(tETIME *et,int timediff) {
  et->start += timediff;
  //update_time();
}
/*---------------------------------------------------------------------------*/
BOOL etimer_expired(tETIME *et) {
  return et->p == PROCESS_NONE;
}
/*---------------------------------------------------------------------------*/
TICK etimer_expiration_time(tETIME *et) {
  return et->start + et->interval;
}
/*---------------------------------------------------------------------------*/
TICK etimer_start_time(tETIME *et) {
  return et->start;
}
/*---------------------------------------------------------------------------*/
BOOL etimer_pending(void) {
  return timerlist != NULL;
}
/*---------------------------------------------------------------------------*/
//TICK etimer_next_expiration_time(void) {
//  return etimer_pending() ? next_expiration : 0;
//}
/*---------------------------------------------------------------------------*/
void etimer_stop(tETIME *et) {
  tETIME *t;
  /* First check if et is the first event timer on the list. */
  if (et == timerlist) {
    timerlist = timerlist->next;
    //update_time();
  } else {
    /* Else walk through the list and try to find the item before the
       et timer. */
    for (t = timerlist;t != NULL && t->next != et;t = t->next);
    if (t != NULL) {
      /* We've found the item before the event timer that we are about
         to remove. We point the items next pointer to the event after
       the removed item. */
      t->next = et->next;
      //update_time();
    }
  }
  /* Remove the next pointer from the item to be removed. */
  et->next = NULL;
  /* Set the timer as expired */
  et->p = PROCESS_NONE;
}
/*---------------------------------------------------------------------------*/
#endif

/*************************** End of file ****************************/
