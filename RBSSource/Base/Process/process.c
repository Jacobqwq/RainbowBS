/*
----------------------------------------------------------------------
                    RainbowBS File : process.c
----------------------------------------------------------------------
*/

/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 * Implementation of the process kernel.
 * \author
 *         Adam Dunkels <adam@sics.se>\n
 *         QWQ <jacobqwq@icloud.com> modified for RainbowBS as following\n
 *         1. modify comment format.
 *         2. redefine some data type.
 *         3. add 'BS_DEBUG_' for checking.
 *         4. make PROCESS_CURRENT() not NULL only on the condition that a process is calling,
 *            otherwise NULL.
 *         5. rename process_post_synch() to process_send().
 *         6. add return value for some functions.
 *         7. modify exit_process() handle PROCESS_EVENT_EXIT before PROCESS_EVENT_EXITED,
 *            for being in keeping with call_process() handling PROCESS_EVENT_EXIT.
 */

#include "../../RainbowBS.h"

#if (RBS_CFG_APP_MODEL == RBS_APP_PTP)

/*
 * Pointer to the currently running process structure.
 */
tPROCESS *process_list;
tPROCESS *process_current;

static process_event_t lastevent;

/*
 * Structure used for keeping the queue of active events.
 */
typedef struct event_struct {
  process_event_t ev;
  process_data_t data;
  tPROCESS *p;
} tEVENT;

static U16 nevents,fevent;
static tEVENT events[RBS_CFG_PTP_NUMEVENTS];

#if RBS_CFG_PTP_PROCESS_STATS
  U16 process_maxevents;
#endif

static volatile BOOL poll_requested;

/* process state */
#define PROCESS_STATE_NONE        0
#define PROCESS_STATE_RUNNING     1
#define PROCESS_STATE_CALLED      2

static void call_process(tPROCESS *p,process_event_t ev,process_data_t data);

/*---------------------------------------------------------------------------*/
process_event_t process_alloc_event(void)
{
  return lastevent++;
}
/*---------------------------------------------------------------------------*/
BOOL process_start(tPROCESS *p,process_data_t arg)
{
  tPROCESS *q;
  /* First make sure that we don't try to start a process that is
     already running. */
  for (q = process_list;q != p && q != NULL;q = q->next);
  /* If we found the process on the process list, we bail out. */
  if (q == p) {
    RBS_DEBUG_WARN_FORMAT(TRUE,"\ntry to start process '%s' that is already running!",
                         PROCESS_NAME_STRING(p));
    return FALSE;
  }
  /* Put on the process list.*/
  p->next = process_list;
  process_list = p;
  p->state = PROCESS_STATE_RUNNING;
  PT_INIT(&p->pt);
  RBS_DEBUG_LOG_FORMAT("\nstarting progress '%s'",PROCESS_NAME_STRING(p));
  /* Send a synchronous initialization event to the process. */
  process_send(p,PROCESS_EVENT_INIT,arg);
  return TRUE;
}
/*---------------------------------------------------------------------------*/
static void exit_process(tPROCESS *p,tPROCESS *fromprocess)
{
  register tPROCESS *q;
  tPROCESS *old_current = process_current;
  RBS_DEBUG_ASSERT((NULL != p) && (NULL != p->thread));
  /* Make sure the process is in the process list before we try to exit it. */
  for (q = process_list;q != p && q != NULL;q = q->next);
  if (q == NULL) {
    RBS_DEBUG_WARN_FORMAT(TRUE,"process '%s' not in the process list!",PROCESS_NAME_STRING(p));
    return;
  }
  if (process_is_running(p)) { /* Process was running */
    p->state = PROCESS_STATE_NONE;
    if (p != fromprocess) {
      /* Post the exit event to the process that is about to exit. */
      process_current = p;
      p->thread(&p->pt,PROCESS_EVENT_EXIT,NULL);
    }
    /*
     * Send a synchronous event to all processes to inform them that
     * this process is about to exit. This will allow services to
     * deallocate state associated with this process.
     */
    for (q = process_list;q != NULL;q = q->next) {
      if (p != q)
        call_process(q,PROCESS_EVENT_EXITED,(process_data_t)p);
    }
  }
  /* remove from process list */
  if (p == process_list) {
      process_list = process_list->next;
  } else {
    for (q = process_list;q != NULL;q = q->next) {
      if (q->next == p) {
        q->next = p->next;
        break;
      }
    }
  }
  RBS_DEBUG_LOG_FORMAT("exit process '%s'",PROCESS_NAME_STRING(p));
  process_current = old_current;
}
/*---------------------------------------------------------------------------*/
static void call_process(tPROCESS *p,process_event_t ev,process_data_t data)
{
  RBS_DEBUG_ASSERT((NULL != p) && (NULL != p->thread));
  RBS_DEBUG_WARN_FORMAT(PROCESS_STATE_NONE == p->state,
                       "\nprocess '%s' ignores event %d for unrunning!",PROCESS_NAME_STRING(p),ev);
  RBS_DEBUG_WARN_FORMAT(PROCESS_STATE_CALLED == p->state,
                       "\nprocess '%s' ignores event %d for reentrant!",PROCESS_NAME_STRING(p),ev);
  if (PROCESS_STATE_RUNNING == p->state) {
    int ret;
    RBS_DEBUG_LOG_FORMAT("\ncalling process '%s' with event %d",PROCESS_NAME_STRING(p),ev);
    process_current = p;
    p->state = PROCESS_STATE_CALLED;
    ret = p->thread(&p->pt,ev,data);
  if ((ret == PT_EXITED) || (ret == PT_ENDED) || (ev == PROCESS_EVENT_EXIT)) {
      exit_process(p,p);
    } else {
      p->state = PROCESS_STATE_RUNNING;
      process_current = NULL;
    }
  }
}
/*---------------------------------------------------------------------------*/
void process_exit(tPROCESS *p)
{
  RBS_DEBUG_ERROR(NULL == p,"NULL process",return);
  exit_process(p,PROCESS_CURRENT());
}
/*---------------------------------------------------------------------------*/
void process_init(void)
{
  lastevent = PROCESS_EVENT_MAX;
  nevents = fevent = 0;
#if RBS_CFG_PTP_PROCESS_STATS
  process_maxevents = 0;
#endif
  process_current = process_list = NULL;
  poll_requested = FALSE;
}
/*---------------------------------------------------------------------------*/
static void do_poll(void)
{
  tPROCESS *p;
  poll_requested = FALSE;
  /* Call the processes that needs to be polled. */
  for (p = process_list;p != NULL;p = p->next) {
    if (p->needspoll) {
      p->state = PROCESS_STATE_RUNNING;
      p->needspoll = FALSE;
      call_process(p,PROCESS_EVENT_POLL,NULL);
    }
  }
}
/*---------------------------------------------------------------------------*/
static void do_event(void)
{
  /*
   * If there are any events in the queue, take the first one and walk
   * through the list of processes to see if the event should be
   * delivered to any of them. If so, we call the event handler
   * function for the process. We only process one event at a time and
   * call the poll handlers in between.
   */
  if (nevents > 0) { /* There are events that we should deliver. */
    process_event_t ev = events[fevent].ev;
    process_data_t data = events[fevent].data;
    tPROCESS *receiver = events[fevent].p;
    /* Since we have seen the new event,we move pointer upwards
       and decrease the number of events. */
    fevent = (fevent + 1) % RBS_CFG_PTP_NUMEVENTS;
    --nevents;
    /* If this is a broadcast event,we deliver it to all events,in
       order of their priority. */
    if (receiver == PROCESS_BROADCAST) {
      tPROCESS *p;
      for (p = process_list;p != NULL;p = p->next) {
      /* If we have been requested to poll a process,we do this in
         between processing the broadcast event. */
        if (poll_requested)
          do_poll();
        call_process(p,ev,data);
      }
    } else {
      call_process(receiver,ev,data);
    }
  }
}
/*---------------------------------------------------------------------------*/
U16 process_run(void)
{
  /* Process all poll events. */
  if (poll_requested)
    do_poll();
  /* Process one event from the queue */
  do_event();
  return nevents + (poll_requested ? 1 : 0);
}
/*---------------------------------------------------------------------------*/
U16 process_nevents(void)
{
  return nevents + (poll_requested ? 1 : 0);
}
/*---------------------------------------------------------------------------*/
BOOL process_post(tPROCESS *p,process_event_t ev,process_data_t data)
{
  U16 snum;
  RBS_DEBUG_IF(NULL != PROCESS_CURRENT(),;);
  RBS_DEBUG_LOG_FORMAT("process '%s' posts event %d to process '%s'",PROCESS_NAME_STRING(PROCESS_CURRENT()),
                      ev,p == PROCESS_BROADCAST ? "<broadcast>": PROCESS_NAME_STRING(p));
  RBS_DEBUG_ELSIF(;,TRUE,;);
  RBS_DEBUG_LOG_FORMAT("post event %d to process '%s'",ev,p == PROCESS_BROADCAST ? "<broadcast>": PROCESS_NAME_STRING(p));
  RBS_DEBUG_ENDIF(;);
  if (nevents == RBS_CFG_PTP_NUMEVENTS) {
    RBS_DEBUG_LOG("can not post event as event queue is full!");
    return FALSE;
  }
  snum = (fevent + nevents) % RBS_CFG_PTP_NUMEVENTS;
  events[snum].ev = ev;
  events[snum].data = data;
  events[snum].p = p;
  ++nevents;
#if RBS_CFG_PTP_PROCESS_STATS
  if (nevents > process_maxevents)
    process_maxevents = nevents;
#endif
  return TRUE;
}
/*---------------------------------------------------------------------------*/
void process_send(tPROCESS *p,process_event_t ev,process_data_t data)
{
  tPROCESS *caller = process_current;
  RBS_DEBUG_ERROR(NULL == p,"NULL process.",return);
  RBS_DEBUG_IF(NULL != PROCESS_CURRENT(),;);
  RBS_DEBUG_LOG_FORMAT("process '%s' sends event %d to process '%s'",
                      PROCESS_NAME_STRING(PROCESS_CURRENT()),ev,PROCESS_NAME_STRING(p));
  RBS_DEBUG_ELSIF(;,TRUE,;);
  RBS_DEBUG_LOG_FORMAT("send event %d to process '%s'",ev,PROCESS_NAME_STRING(p));
  RBS_DEBUG_ENDIF(;);
  call_process(p,ev,data);
  process_current = caller;
}
/*---------------------------------------------------------------------------*/
BOOL process_poll(tPROCESS *p)
{
  if (process_is_running(p)) {
    p->needspoll = TRUE;
    poll_requested = TRUE;
  return TRUE;
  } else {
    return FALSE;
  }
}
/*---------------------------------------------------------------------------*/
BOOL process_is_running(tPROCESS *p)
{
  RBS_DEBUG_ERROR(NULL == p,"NULL process.",return FALSE);
  return p->state != PROCESS_STATE_NONE;
}
/*---------------------------------------------------------------------------*/
#endif

/*************************** End of file ****************************/
