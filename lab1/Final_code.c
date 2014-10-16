#include <stdlib.h>
#include <errnoLib.h>
#include <timers.h>
#include <time.h>
#include <signal.h>
#include "vxWorks.h"
#include "taskLib.h"
#include "semLib.h"
#include "sysLib.h"
#include "stdio.h"
#include "unistd.h"

#define TASK1PERIOD 20000000
#define TASK2PERIOD 50000000
#define NSEC_PER_MICROSEC 1000
#define NSEC_PER_MSEC 1000000
#define NSEC_PER_SEC 1000000000
#define MICROSEC_PER_MSEC 1000
#define MY_TICK_NSECS 10000000
#define TICKS_PER_SEC 100

static timer_t tt_timer;
static timer_t tt_timer1;
static struct itimerspec last_itime;
static struct itimerspec last_itime1;

static struct itimerspec itime1 = {{0,TASK2PERIOD}, {0,TASK2PERIOD}};
static unsigned long tick_count1 = 0;
static unsigned long tick_count2 = 0;
/*
 Q2). The section that follows is the solution for Question 2 of the Lab 1 Assignment
 */
void fibbonacci(int n) {

unsigned long first = 0, second = 1, next, c;
 
   
   for ( c = 0 ; c < n ; c++ )
   {
      if ( c <= 1 )
         next = c;
      else
      {
         next = first + second;
         first = second;
         second = next;
      }
   }
}
void timer1_shutdown(void)
{
  int status;

  /* disable and delete timer */

  if((status = timer_cancel(tt_timer)) == ERROR)
    perror("tt_timer");

  if((status = timer_delete(tt_timer)) == ERROR)
    perror("tt_timer");
  else {
    printf("\nTick/Tock interval timer deleted, status=%d\n", status);
    fflush(stdout);
  }                                 
  exit(0);
 
}
void timer2_shutdown(void)
{
  int status;

  /* disable and delete timer */

  if((status = timer_cancel(tt_timer1)) == ERROR)
    perror("tt_timer1");

  if((status = timer_delete(tt_timer1)) == ERROR)
    perror("tt_timer1");
  else {
    printf("\nTick/Tock interval timer deleted, status=%d\n", status);
    fflush(stdout);
  }                                 
  exit(0);
 
}

void monitor_task1(int signo, siginfo_t *info, void *ignored)
{
	int prio;
  int mon_tid = info->si_value.sival_int;
  taskPriorityGet(taskIdSelf(), &prio);
  
  /*Q3). The value of the argument for the function fibbonacci() was empirically found ot to correspond to 10ms of CPU computation time*/
  
  fibbonacci(2498162);
  tick_count1++;
  
  if(tick_count1 > (20))
    timer1_shutdown();
}
void monitor_task2(int signo, siginfo_t *info, void *ignored)
{
	/*Q3). The value of the argument for the function fibbonacci() was empirically found ot to correspond to 20ms of CPU computation time*/
	
	fibbonacci(4898459); 
  int prio;
  int mon_tid = info->si_value.sival_int;

  tick_count2++;
  
  if(tick_count2 > (50))
    timer2_shutdown();
}


void task1(void) {
	int my_tid, flags = 0;
	/*
	 * Q4). Time period for task1 is set to 20ms
	 * */
	static struct itimerspec itime = {{0,TASK1PERIOD}, {0,TASK1PERIOD}};
	struct sigevent ReleaseEvent;
	struct sigaction ReleaseAction;
	my_tid = taskIdSelf();
	ReleaseEvent.sigev_notify = SIGEV_SIGNAL;
	ReleaseEvent.sigev_signo = SIGRTMIN+1;
	ReleaseEvent.sigev_value.sival_int = my_tid;

	ReleaseAction.sa_sigaction = monitor_task1;
	ReleaseAction.sa_flags = SA_SIGINFO;
	sigaction(SIGRTMIN+1, &ReleaseAction, NULL);
	/* set up to signal if timer expires */
	 timer_create(CLOCK_REALTIME, NULL, &tt_timer);
	 /* initialize internal timer release wrapper */
	 timer_create(CLOCK_REALTIME, &ReleaseEvent, &tt_timer); 
	 /* The after the timer expires it points to structure ReleaseEvent where 
	  * sigev_notify value specifies to produce a signal specified in sigev_signo. When ReleaseEvent produces the required signal then the handler function pointed by the 
	  * sigaction function is executed
	  * */
	 timer_settime(tt_timer, flags, &itime, &last_itime);
	 while(1) {
	      pause();
	 }
}
void task2(void) {
	int my_tid, flags = 0;
	/*
	 * Q4). Time period for task1 is set to 50ms
	 * */
	static struct itimerspec itime = {{0,TASK2PERIOD}, {0,TASK2PERIOD}};
	struct sigevent ReleaseEvent1;
	struct sigaction ReleaseAction1;
	my_tid = taskIdSelf();
	ReleaseEvent1.sigev_notify = SIGEV_SIGNAL;
	ReleaseEvent1.sigev_signo = SIGRTMIN+2;
	ReleaseEvent1.sigev_value.sival_int = my_tid;

	ReleaseAction1.sa_sigaction = monitor_task2;
	ReleaseAction1.sa_flags = SA_SIGINFO;
	sigaction(SIGRTMIN+2, &ReleaseAction1, NULL);
	/* set up to signal if timer expires */
	 timer_create(CLOCK_REALTIME, NULL, &tt_timer1);
	 /* initialize internal timer release wrapper */
	 timer_create(CLOCK_REALTIME, &ReleaseEvent1, &tt_timer1);
	 timer_settime(tt_timer1, flags, &itime, &last_itime);
	 while(1) {
	      pause();
	 }
}

/*Q1). This section that follows is the solution to Question 1 of the Lab 1 Assignment. This task spawns two tasks with independently configurable timers with a resolution of 1ms */

void run_fibbonacci(void)
{
  int taskId, i, my_tid, flags = 0;
  int iMyPriority;

  sysClkRateSet(1000);

  if((taskId = taskNameToId("task1")) != ERROR)
  {
	if(taskDelete(taskId) != ERROR)
		printf("Old task1 deleted\n");

  }
  else
  {
	printf("Old task1 not found\n");
  }
  
  if(taskSpawn("task1", 100, 0, 4000, (FUNCPTR)task1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == ERROR)
  {
    printf("task1 task spawn failed\n");
  }
  else
    printf("task1 task spawned\n");
	
if((taskId = taskNameToId("task2")) != ERROR)
  {
	if(taskDelete(taskId) != ERROR)
		printf("Old task2 deleted\n");

  }
  else
  {
	printf("Old task2 not found\n");
  }
  
  if(taskSpawn("task2", 110, 0, 4000, (FUNCPTR)task2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == ERROR)
  {
    printf("task2 task spawn failed\n");
  }
  else
    printf("task2 task spawned\n");
 
 }
