/* VxWorks API includes */
#include "vxWorks.h"
#include "stdio.h"
#include "stdlib.h"
#include <errnoLib.h>
#include "ioLib.h"
#include "sioLib.h"
#include "semLib.h"
#include "intLib.h"
#include "vmLib.h"   /* added to support */
#include "iv.h"


/* VxWorks 5.4 PCI driver interface includes */
#include "drv/pci/pciConfigLib.h"
#include "drv/pci/pciConfigShow.h"
#include "drv/pci/pciHeaderDefs.h"
#include "drv/pci/pciLocalBus.h"
#include "drv/pci/pciIntLib.h"

/* pcPentium BSP includes */
#include "sysLib.h"
#include"btvid.h"

void SetSockStruct();
void send_command(unsigned int *);
int init_frametx(void);
int arm_tid;
int m1, m2, m3, m4, m5,m6, pos, pos1,pos2, flag = 1 ;
int serial_fd, si = 0;
int duration = 0, millis = 0;
int status1, status2;
char output = 1;
int dur;
unsigned int arr[10];
extern SEM_ID sync1;
extern SEM_ID sync2;
extern SEM_ID sync3;
extern SEM_ID sync4;
extern int logfd;
void ImgProc();
int uart_rx_num(char a[]);
int microsecondsToMilimeters(int);

/* -------------------------------------------------------------------------------------------------------------------------------------------------
 * ************************************************************** Serial read task *****************************************************************
 * -------------------------------------------------------------------------------------------------------------------------------------------------*/

void serialread()
{
  while(1) {
  
	  semTake(sync4, WAIT_FOREVER);
	  
	  status1 = read(serial_fd, &dur, sizeof(unsigned int));
	  /*dur[7]='\0';
	  duration = atoi(dur);*/
	  /*millis = microsecondsToMilimeters(dur);*/
	  
          
}
}

/*  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  *************************************************************** Serial write and ArmMove tasks *****************************************************************
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------*/

void serialwrite()
{
	while(1)
	  {
		semTake(sync3, WAIT_FOREVER);
		status2 = write( serial_fd, &output, 1);
		
 	  }
}

/*ArmMove task body, which sends the data on socket to the robotic arm via the Atom Board*/

void armMove(void) 
{
	while(1) 
	{
		
		semTake(sync2,WAIT_FOREVER);
		arr[0] = m1;
		arr[1] = m2;
		arr[2] = m3;
		arr[3] = m4;
		arr[4] = m5;
		arr[5] = m6;
		arr[6] = pos;
		arr[7] = pos1;
		arr[8] = pos2;
		arr[9] = dur;
		if(flag == 1)
		{
		send_command(arr);
		flag = 0;
		}
		/*semGive(sync1);*/
		
		taskDelay(1);
	}
	
}

/* ------------------------------------------------------------------------------------------------------------------------------------------------------
 * ************************************************************** Serial initialization *****************************************************************
 * ------------------------------------------------------------------------------------------------------------------------------------------------------*/
int serial_init( void )
{

  int status;

  if((serial_fd = open("/tyCo/0", O_RDWR, 0644)) == ERROR) {
    printf("ERROR opening serial interface\r\n");
    return -1;
  }

  /* set the serial interface to 19200 baud */
  status = ioctl(serial_fd, SIO_BAUD_SET, 19200);
  if ( status == -1 ) {
    printf("ERROR: ioctl on SIO_BAUD_SET");
    return -1;
  }
  /* put the serial interface into "raw" mode */
  status = ioctl(serial_fd, FIOSETOPTIONS, OPT_RAW);
  if ( status == -1 ) {
    printf("ERROR: ioctl on FIOSETOPTIONS");
    return -1;
  }

  return 0;

}

/* ------------------------------------------------------------------------------------------------------------------------------------------------------
 * ************************************************************** Entry Point Function  *****************************************************************
 * ------------------------------------------------------------------------------------------------------------------------------------------------------*/

void start(void) 
{
	SetSockStruct();
	serial_init();
	start_video_report(NULL);
	activate(1);
	set_mux(3);
	reset_status();
	report();
	init_frametx();
	
	
	if(taskSpawn("armMove", 13, 0, (1024*8), (FUNCPTR) armMove, 0,0,0,0,0,0,0,0,0,0) == ERROR)
			perror("taskSpawn: armMove");
	
	if(taskSpawn("ImgProc", 12, 0, (1024*8), (FUNCPTR) ImgProc, 0,0,0,0,0,0,0,0,0,0) == ERROR)
				perror("taskSpawn: WriteSave");
	
	if(taskSpawn("uartread", 11, 0, (1024*8), (FUNCPTR) serialread, 0,0,0,0,0,0,0,0,0,0) == ERROR)
					perror("taskSpawn: Serial read");
	if(taskSpawn("uartwrite", 10, 0, (1024*8), (FUNCPTR) serialwrite, 0,0,0,0,0,0,0,0,0,0) == ERROR)
						perror("taskSpawn: Serial write");
	
	takeInput(0,0,3,4,1,2,600,350,512);
}

/* Function to pass the arm position values calculated by the centroid code to global variables */
void takeInput(int a, int b, int a1, int b1, int a2, int b2, int c, int c1, int c2) 
{
	flag = 1;
	m1 = a;
	m2 = b;
	m3 = a1;
	m4 = b1;
	m5 = a2;
	m6 = b2;
	pos = c;
	pos1 = c1;
        pos2 = c2;
}


int uart_rx_num(char t[10]) 
{
	
	int value, i, mul = 1;
	int *ptr, num[10];
	ptr = num;
	
	for(i = 0; i<10; i++) {
		*(ptr + i) = t[i];
		
		if(*(ptr + i) == '\0')
			break;
		
	}
	
	i--;
	value = *(ptr + i);
	i--;
	
	for(;i<0;i--){
		
		mul = mul*10;
		value = value + mul*(*(ptr + i));
	}

	return value;
	
}

/* code section to convert echo time to distance */
int microsecondsToMilimeters(int microseconds)

{

 return ((microseconds)/29/2);

}

