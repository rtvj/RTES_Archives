#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include <logLib.h>
#include <ioLib.h>
#include <fcntl.h>
#include "drv/timer/timerDev.h"

/*static unsigned long tick_count1 = 0;
static unsigned long tick_count2 = 0;*/
double tick_diff, jiffy_diff, micro_diff;
double jiffies_per_tick;
double clock_frequency;
double microseconds_per_tick = 0;
double microseconds_per_jiffy = 0;
double last_jiffies = 0;
double last_ticks = 0;
double current_jiffies = 0;
double current_ticks = 0;

SEM_ID synch_sem;                                                                   
int abort_test = FALSE;
int take_cnt = 0;
int give_cnt = 0;
int a = 0, b = 0, c = 0;
char fstr[50];
char str[10];

void start_profiling(void){
	/*sysClkRateSet(1000);
	sysTimestampEnable();
	
	jiffies_per_tick = sysTimestampPeriod();
	clock_frequency = sysTimestampFreq();
*/
	microseconds_per_tick = (jiffies_per_tick / clock_frequency)*1000000.0;
	microseconds_per_jiffy = microseconds_per_tick / jiffies_per_tick;
	
	last_jiffies = sysTimestampLock();
	last_ticks = tickGet();
	
}

void stop_profiling(void){
	current_jiffies = sysTimestampLock();
	current_ticks = tickGet();
		 
	tick_diff = (current_ticks - last_ticks)*microseconds_per_tick;
	jiffy_diff = (current_jiffies - last_jiffies)*microseconds_per_jiffy;
	micro_diff = tick_diff + jiffy_diff;	
}

void output_profiling(char *subsys, char *func){
	
	printf("\nThe time taken for execution in the function %s of sybsystem %s is: %f\n", func, subsys, micro_diff);
}
void my_log(char *subsys, int prio, char *msg, char *func, int *line) {
	
	
	
	
	if (prio == 1){
		sprintf(str, "HIGH");
	}
	if(prio == 2){
		sprintf(str, "MEDIUM");
		
	}
	if(prio == 3){
		sprintf(str, "LOW");
		
	}
	if(a==1){
	if(prio==1){
	
	logMsg("\n%s %s %s %s %d\n", (int)subsys, (int)str, (int)msg, (int)func, (int)line, 0);
	
	}
	}
	else if(a==2){
	if(prio==1 || prio==2){
	
	logMsg("\n%s %s %s %s %d\n", (int)subsys, (int)str, (int)msg, (int)func, (int)line, 0);
	
	}
	}
	else if(a==3){
		
	logMsg("\n%s %s %s %s %d\n", (int)subsys, (int)str, (int)msg, (int)func, (int)line, 0);
	
	}
	if(b==1){
	if(prio==1){
		
	logMsg("\n%s %s %s %s %d\n", (int)subsys, (int)str, (int)msg, (int)func, (int)line, 0);
	
	}
	}
	else if(b==2){
	if(prio==1 || prio==2){
		
	logMsg("\n%s %s %s %s %d\n", (int)subsys, (int)str, (int)msg, (int)func, (int)line, 0);
	
	}
	}
	else if(b==3){
		
	logMsg("\n%s %s %s %s %d\n", (int)subsys, (int)str, (int)msg, (int)func, (int)line, 0);
	
	}
	if(c==1){
	if(prio==1){
		
	logMsg("\n%s %s %s %s %d\n", (int)subsys, (int)str, (int)msg, (int)func, (int)line, 0);
	
	}
	}
	else if(c==2){
	if(prio==1 || prio==2){
		
	logMsg("\n%s %s %s %s %d\n", (int)subsys, (int)str, (int)msg, (int)func, (int)line, 0);
	
	}
	}
	else if(c==3){
		
	logMsg("\n%s %s %s %s %d\n", (int)subsys, (int)str, (int)msg, (int)func, (int)line, 0);
	
	}
}
void task_a(void)
{
	
  int cnt = 0;

  while(!abort_test)
  {
    taskDelay(1000);
    for(cnt=0;cnt < 10000000;cnt++);
    semGive(synch_sem); 
    my_log("Two Tasks", 1, "task_a semgive", __func__, __LINE__);
    give_cnt++;
  }
}

void task_b(void)
{
	
  int cnt = 0;

  while(!abort_test)
  {
    for(cnt=0;cnt < 10000000;cnt++);
    take_cnt++;
    semTake(synch_sem, WAIT_FOREVER);
    
    my_log("Two Tasks", 1, "task_b semtake", __func__, __LINE__);
  
    taskDelay(10);
  }
}

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


void task1(void) {
	int my_tid, flags = 0;
	
	while(1) {
	my_tid = taskIdSelf();
	start_profiling();
	fibbonacci(2498162); 
	stop_profiling();
		
	output_profiling("Fibbonacci1 without logging", __func__);
	/*start_profiling();
	/*sprintf(fstr, "%f", micro_diff);*/
	my_log("Fibbonacci1", 1, "Calculated fibbonacci for task 2", __func__, __LINE__);
	taskDelay(100);
	}
}
void task2(void) {
	int my_tid, flags = 0;
	
	while(1) {
	my_tid = taskIdSelf();
	start_profiling();
	  
	  fibbonacci(1249081);
	  stop_profiling();
		
		output_profiling("Fibbonacci2 without logging", __func__);
	    /*start_profiling();
	  		 /*sprintf(fstr, "%f", micro_diff);*/
	  my_log("Fibbonacci2", 2, "Calculated fibbonacci for task 1", __func__, __LINE__);
	  taskDelay(100);
	 }
}
void run_tasks(void) {
	
	int taskId, taskId1, i, my_tid, flags = 0, taskId2, taskId3, fd, fd1;
	  int iMyPriority;
	  int tid;
	  
		 sysClkRateSet(1000);
	  	sysTimestampEnable();
	  	jiffies_per_tick = sysTimestampPeriod();
	  	clock_frequency = sysTimestampFreq();
	  	tid = taskNameToId("tLogTask");
	  	taskPrioritySet(tid, 140);
	  	
	 /*
	  fd = ioGlobalStdGet(1);
	  fd1 = open("/tgtsvr/logfile", O_CREAT | O_RDWR, 0777);
	  logFdSet(fd1);
	*/
	  printf("\nThe modules present are:\nfibbonacci1\nfibbonacci2\ntwotasks\n");
	  printf("Enter the level of message logging for the three modules as:\n1 for High\n2 for Medium\n3for Low\n");
	  printf("Fibbonacci1: ");
	  scanf("%d", &a);
	  printf("Fibbonacci2: ");
	  	  scanf("%d", &b);
	  printf("Two Tasks: ");
	  scanf("%d", &c);
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
		
	if((taskId1 = taskNameToId("task2")) != ERROR)
	  {
		if(taskDelete(taskId1) != ERROR)
			printf("Old task2 deleted\n");

	  }
	  else
	  {
		printf("Old task2 not found\n");
	  }
	  
	  if(taskSpawn("task2", 101, 0, 4000, (FUNCPTR)task2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == ERROR)
	  {
	    printf("task2 task spawn failed\n");
	  }
	  else
	    printf("task2 task spawned\n");
	  
	  if((taskId2 = taskNameToId("task_a")) != ERROR)
	    {
	  	if(taskDelete(taskId2) != ERROR)
	  		printf("Old task a deleted\n");

	    }
	    else
	    {
	  	printf("Old task a not found\n");
	    }
	    

	    if((taskId3 = taskNameToId("task_b")) != ERROR)
	    {
	  	if(taskDelete(taskId3)!= ERROR)
	  		printf("Old task b deleted\n");
	    }
	    else
	    {
	  	  printf("Old task b not found\n");
	    }
	    
	    synch_sem = semBCreate(SEM_Q_FIFO, SEM_FULL);

	    if(taskSpawn("task_a", 102, 0, 4000, (FUNCPTR)task_a, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == ERROR)
	    {
	      printf("Task A task spawn failed\n");
	    }
	    else
	      printf("Task A task spawned\n");


	    if(taskSpawn("task_b", 103, 0, 4000, (FUNCPTR)task_b, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == ERROR) 
	    {
	      printf("Task B task spawn failed\n");
	    }
	    else
	      printf("Task B task spawned\n");


}
void startstoplog(char ch) {
		int fd, fd1;
	fd1 = ioGlobalStdGet(1);
	/*printf("\nPress S to start logging to file logfile\nOr press N to stop logging to logfile\n");
	scanf("%s", &ch);
	*/
	if((ch == 'S') || (ch == 's')) {
		
		fd = open("/tgtsvr/logfile", O_CREAT | O_RDWR, 0777);
			logFdSet(fd);
		}
	if(ch == 'N' || ch == 'n'){
		logFdSet(fd1);
		
	}
}

