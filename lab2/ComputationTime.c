/*#include "profile.h"*/
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
#define MAX_NUM_FACTORS 12

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
void quickSort1(int num[], int left, int right){
	
	int pivot, l_hold, r_hold, tmp;
	
	l_hold = left;
	r_hold = right;
	pivot = num[left];
	while(left < right){
		
		while((num[right]>=pivot)&&(left<right))
		--right;
		if(left!=right){
			num[left] = num[right];
			++left;
		}
		while((num[left]<=pivot)&&(left<right))
		++left;
		if(left!=right){
			num[right] = num[left];
			++right;
		}
	}
	num[left] = pivot;
	pivot = left;
	left = l_hold;
	right = r_hold;
	if(left<pivot)
		quickSort1(num, left, pivot-1);
	if(right>pivot)
		quickSort1(num, pivot+1, right);
}
void quickSort(int arr[], int left, int right) 
{
      int i = left, j = right;
      int tmp;
      int pivot = arr[(left + right) / 2];
  
      /* partition */
      while (i <= j) {
            while (arr[i] < pivot)
                  i++;
            while (arr[j] > pivot)
                  j--;
            if (i <= j) {
                  tmp = arr[i];
                  arr[i] = arr[j];
                  arr[j] = tmp;
                  i++;
                  j--;
            }
      };
  
      /* recursion */
      if (left < j)
            quickSort(arr, left, j);
      if (i < right)
            quickSort(arr, i, right);
}

int faster_calculate_function(int x, int y)
{
  int i, j, flag1 = 0, flag2 = 0;
  int x_factors[MAX_NUM_FACTORS];
  int x_index = 0;
  int y_factors[MAX_NUM_FACTORS];
  int y_index = 0;
  int common_factors[MAX_NUM_FACTORS*2];
  int common_index = 0;
  int max_x = 0;
  if(x%2 ==0){
  for (i=2, j=1; i < (x/2)+1; i++)
  {
	  if(x%i == 0)
	  {
		  x_factors[x_index++] = i;
		  if(j!=1){
				  if(x_factors[x_index-2]>=x_factors[x_index-1])
					  flag1 = 1;
		  }
		  j++;
		  /*printf("x_factors[%d]: %d, flag1: %d\n", (x_index-1), x_factors[x_index-1], flag1);
		  */
		  if (x_index == MAX_NUM_FACTORS)
			  exit(-1);
	  }
  }
  }
  else{
	  for (i=3, j=1; i < x; i+=2)
	   {
	 	  if(x%i == 0)
	 	  {
	 		  x_factors[x_index++] = i;
	 		  if(j!=1){
	 				  if(x_factors[x_index-2]>=x_factors[x_index-1])
	 					  flag1 = 1;
	 		  }
	 		  j++;
	 		  /*printf("x_factors[%d]: %d, flag1: %d\n", (x_index-1), x_factors[x_index-1], flag1);
	 		  */
	 		  if (x_index == MAX_NUM_FACTORS)
	 			  exit(-1);
	 	  }
	   } 
  }
  if(y%2 == 0){
  for (i=2, j=1; i < (y/2)+1; i++)
  {
	  if(y%i == 0)
	  {
		  y_factors[y_index++] = i;
		  if(j!=1){
		  			  if(y_factors[y_index-2]>=y_factors[y_index-1])
		  				  flag2 = 1;
		  		  }
		  j++;
		  if (y_index == MAX_NUM_FACTORS)
			  exit(-1);
	  }
  }
  }
  else {
	  for (i=3, j=1; i < y; i+=2)
	    {
	  	  if(y%i == 0)
	  	  {
	  		  y_factors[y_index++] = i;
	  		  if(j!=1){
	  		  			  if(y_factors[y_index-2]>=y_factors[y_index-1])
	  		  				  flag2 = 1;
	  		  		  }
	  		  j++;
	  		  if (y_index == MAX_NUM_FACTORS)
	  			  exit(-1);
	  	  }
	    }  
  }
  /*
  for(i=1;i<x_index;i++){
	  
	  if(x_factors[i-1]<=x_factors[i]){
	  continue;
	  }
	  else{
	  flag1 = 1;
	  }
  }
  for(i=1;i<y_index;i++){
 	  
 	  if(y_factors[i-1]<=y_factors[i]){
 	  continue;
 	  }
 	  else{
 	  flag2 = 1;
 	  }
   }
   
   
printf("\nflag1: %d\n", flag1);

printf("\nflag2: %d\n", flag2);
  */
if(flag1 == 1)
  quickSort1(x_factors, 0, x_index-1);

if(flag2 == 1)
  quickSort1(y_factors, 0, y_index-1);
  
  return 0;
}

int calculate_function(int x, int y)
{
  int i;
  int x_factors[MAX_NUM_FACTORS];
  int x_index = -1;
  int y_factors[MAX_NUM_FACTORS];
  int y_index = -1;
  int common_factors[MAX_NUM_FACTORS*2];
  int common_index = 0;
  int max_x = 0;
  
  for (i=2; i < x; i++)
  {
	  if(x%i == 0)
	  {
		  x_factors[++x_index] = i;
		  if (x_index == MAX_NUM_FACTORS)
			  exit(-1);
	  }
  }
  for (i=2; i < y; i++)
  {
	  if(y%i == 0)
	  {
		  y_factors[++y_index] = i;
		  if (y_index == MAX_NUM_FACTORS)
			  exit(-1);
	  }
  }
  quickSort(x_factors, 0, x_index-1);
  quickSort(y_factors, 0, y_index-1);
  return 0;
}

void time_fn()
{
	sysClkRateSet(1000);
		sysTimestampEnable();
		jiffies_per_tick = sysTimestampPeriod();
		clock_frequency = sysTimestampFreq();

		microseconds_per_tick = (jiffies_per_tick / clock_frequency)*1000000.0;
		microseconds_per_jiffy = microseconds_per_tick / jiffies_per_tick;
	/*int profile_index = profile_get_new_index();*/
	/*float profile_time;
	sysTimestampEnable();
	*/ start_profiling();
	 	  
	faster_calculate_function(97563,102755);
	
	 stop_profiling();
	 output_profiling("faster_calculate_function");
	  
	  
	 
	 /* printf("\nmicro_diff for normal calculate= %f\n", micro_diff);
	profile_time = profile_stop(profile_index);
	//printf("Base calculation took %f\n", profile_time);
	//profile_start(profile_index);*/
	 start_profiling();
	 	
	 	  calculate_function(97563,102755);
	 	  
	 	 stop_profiling();
	 	 output_profiling("calculate_function");
	 /*  printf("micro_diff for faster calculate= %f\n", micro_diff);
	profile_time = profile_stop(profile_index);
	//printf("Improved calculation took %f\n", profile_time);*/

}


