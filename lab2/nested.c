#include <stdio.h>
#include "vxWorks.h"
#include "drv/timer/timerDev.h"

double tick_diff, jiffy_diff, micro_diff;
double jiffies_per_tick;
double clock_frequency;
double microseconds_per_tick = 0;
double microseconds_per_jiffy = 0;
double last_jiffies = 0;
double last_ticks = 0;
double current_jiffies = 0;
double current_ticks = 0;
double tick_diff1, jiffy_diff1, micro_diff1;
double last_jiffies1 = 0;
double last_ticks1 = 0;
double current_jiffies1 = 0;
double current_ticks1 = 0;

void start_profiling(void){
	
	
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

void output_profiling(char *func){
	
	printf("\nThe time taken for execution in the function %s is: %f\n", func, micro_diff);
}
void start_profiling_in(void){
	
	
	last_jiffies1 = sysTimestampLock();
	last_ticks1 = tickGet();
	
}

void stop_profiling_in(void){
	current_jiffies1 = sysTimestampLock();
	current_ticks1 = tickGet();
		 
	tick_diff1 = (current_ticks1 - last_ticks1)*microseconds_per_tick;
	jiffy_diff1 = (current_jiffies1 - last_jiffies1)*microseconds_per_jiffy;
	micro_diff1 = tick_diff1 + jiffy_diff1;	
}

void output_profiling_in(char *func){
	
	printf("\nThe time taken for execution in the function %s is: %f\n", func, micro_diff1);
}

void nested()
{
   /* local variable definition */
	sysClkRateSet(1000);
	sysTimestampEnable();
	jiffies_per_tick = sysTimestampPeriod();
	clock_frequency = sysTimestampFreq();

	microseconds_per_tick = (jiffies_per_tick / clock_frequency)*1000000.0;
	microseconds_per_jiffy = microseconds_per_tick / jiffies_per_tick;
   int i, j, p;
   start_profiling();
   for(i=2; i<100; i++) {
	   start_profiling_in();
      for(j=2; j <= (i/j); j++)
        if(!(i%j)) break; /* if factor found, not prime*/
      stop_profiling_in();
      if(j > (i/j)) p = i;
   }
   stop_profiling();
   output_profiling("nested for outer loop including inner loop profiling");
   
   start_profiling();
   for(i=2; i<100; i++) {
         for(j=2; j <= (i/j); j++)
           if(!(i%j)) break; /* if factor found, not prime*/
         if(j > (i/j)) p = i;
      }
   stop_profiling();
      output_profiling("nested for outer loop without inner loop profiling");
}
