/* AudioServer.c - TCP server example */

#include "sys/types.h"
#include "stdio.h"
#include "signal.h"
#include "vxWorks.h" 
#include "taskLib.h"
#include "tcpExample.h"
#include "stdlib.h"

#define CIRCULAR_SIZE 16384

VOID RecvTask();
VOID SendTask();



void CircularInit(CircularBuffer *cb, int size) {
	
	cb->size = size + 1;
	cb->start = 0;
	cb->end = 0;
	cb->d = (data *)calloc(cb->size, sizeof(data));
}

FreeBuffer(CircularBuffer *cb) {
	
	free(cb->d);
}

int FullBuffer(CircularBuffer *cb) {
	
	return (cb->end + 1) % cb->size == cb->start;
}

int EmptyBuffer(CircularBuffer *cb) {

	return (cb->end == cb->start);
}

void BufferWrite(CircularBuffer *cb, data *value) {

	cb->d[cb->end] = *value;
	cb->end = (cb->end + 1) % cb->size;
	if(cb->end == cb->start)
	cb->start = (cb->start + 1) % cb->size;
}

void BufferRead(CircularBuffer *cb, data *value) {
		
	*value = cb->d[cb->start];
	cb->start = (cb->start + 1) % cb->size;
	
}
/*void loadBuffer(unsigned char c[]);*/
void start(void);
void loadCircular(CircularBuffer *cb);
void passCircular(CircularBuffer *cb);
void sine_wave(unsigned int f);
void playSineWave(unsigned int f);
extern unsigned char saved_samples[MAX_LINE];
/*extern unsigned char a_data2[];
extern unsigned char a_data3[];
extern unsigned char a_data1[];
*/
extern unsigned char pbdata[];
extern unsigned char b_data1[];
extern unsigned char b_data2[];
extern unsigned char scc[];

FILE *fp;
long file_size_end, file_size_begin;
extern const long size_bdata1;
extern const long size_scc;
char sn;
/*STATUS tcpServer (int SERVER_PORT_NM)
{
  if (taskSpawn("tcpRecvTask", SERVER_WORK_PRIORITY, 0, SERVER_STACK_SIZE,(FUNCPTR) tcpRecvTask, 0, 0, 0,0,0, 0, 0, 0, 0, 0) == ERROR)
    {
      perror ("taskSpawn");
    }
}

STATUS tcpClient (int SERVER_PORT_NM)
{
  if (taskSpawn("tcpSendTask", CLIENT_WORK_PRIORITY, 0, SERVER_STACK_SIZE,(FUNCPTR) tcpSendTask, 0, 0, 0,0,0, 0, 0, 0, 0, 0) == ERROR)
    {
      perror ("taskSpawn");
    }
}*/
void playSound(char c) {
	/*dat = buf;*/
	sn = c;
	loadBuffer(c);
	
}

void start1()
{
	

cb1 = (CircularBuffer *)malloc(sizeof(CircularBuffer));
CircularInit(cb1, CIRCULAR_SIZE);
/*CircularInit(cb2, 8192);
CircularInit(cb3, 8192);
*/
loadCircular(cb1);




printf("Enter a choice of what you want to play:\n1. Houston message\n2. Splinter Cell music\n3. Sine wave\n4. 2 buffers in circular fashion\n5. Read from a file\n");
printf("choice: 1 or 2 or 3 or 4 or 5: ");
scanf("%d", &sn);

if(sn==3){
	playSineWave(440);
	start();
}
if((sn==1) || (sn==2)){
playSound(sn);
start();
}

if(sn==4){
	passCircular(cb1);
	start();
}
if(sn==5){
	read_file("/tgtsvr/SCC.txt");
	start();
}
/*
read_file("/tgtsvr/SCC1.txt");
playSineWave(440);
*/
/*playSound(cb1->d->ch);*/
/*passCircular(cb1);*/


if (taskSpawn("RecvTask", 5, 0, SERVER_STACK_SIZE,(FUNCPTR) RecvTask, 0, 0, 0,0 ,0, 0, 0, 0, 0, 0) == ERROR)
    {
      perror ("taskSpawn");
    }
/*
if (taskSpawn("SendTask", 4, 0, SERVER_STACK_SIZE,(FUNCPTR) SendTask, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == ERROR)
   {
     perror ("taskSpawn");
   }
*/
}

void loadCircular(CircularBuffer *cb) {
	
	int i;
	data *value;
	value = (data *)malloc(sizeof(data));
	/*
	for(i=0;i<MAX_LINE;i++) {
		
		value->ch = a_data1[i];
		
		BufferWrite(cb, value);
		
	}
	*/
	for(i=0;i<MAX_LINE;i++) {
			
		value->ch = b_data1[i];
			
		BufferWrite(cb, value);
			
	}
	for(i=0;i<MAX_LINE;i++) {
			
		value->ch = pbdata[i];
			
		BufferWrite(cb, value);
			
	}
}
void passCircular(CircularBuffer *cb) {
	sn=4;
	int i;
	
	for(i=0;i<MAX_LINE;i++) {
		
		BufferRead(cb, &saved_samples[i]);
		
	}
	
}

void read_file(char ch[10]){
	sn=5;
	fp = fopen(ch, "r");
	if(fp == NULL)
	    printf("file open error");
	    fseek(fp, 0, SEEK_END);
	    file_size_end = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		
}
playSineWave(unsigned int freq){
	sn=3;
	sine_wave(freq);
	
}
