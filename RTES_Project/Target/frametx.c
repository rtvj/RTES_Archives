#include "inetLib.h"
#include "sockLib.h"
#include "netinet/tcp.h"
#include "ioLib.h"
#include "wvLib.h"
#include "vxWorks.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "ioLib.h"
#include "semLib.h"
#include "intLib.h"
#include "vmLib.h"   /* added to support */
#include "iv.h"
#include "inetLib.h"
#include "sockLib.h"
#include "netinet/tcp.h"
#include "wvLib.h"
#include "netdb.h"
#include "frametx.h"

/* VxWorks 5.4 PCI driver interface includes */
#include "drv/pci/pciConfigLib.h"
#include "drv/pci/pciConfigShow.h"
#include "drv/pci/pciHeaderDefs.h"
#include "drv/pci/pciLocalBus.h"
#include "drv/pci/pciIntLib.h"


/* pcPentium BSP includes */
#include "sysLib.h"

#include "btvid.h"

/* GLOBALS */

unsigned char frame_seq_cnt = 0;
unsigned char state_seq_cnt = 0;

static int FrameSockFd;
static int StateSockFd;

#define DISPLAY_IP "172.21.74.80"
#define DISPLAY_PORT 50007
#define STATE_PORT 8386

/*extern unsigned int frameProcCnt;*/

int init_statetx(void) 
{
  struct sockaddr_in serverAddr;
  int sockAddrSize;
  struct linger opt;
  int sockarg;


  /* create client's socket */
  if ( (StateSockFd = socket( AF_INET, SOCK_STREAM, 0 )) == ERROR )
  {
    perror("connect() failed in frametx_init");
    close( StateSockFd );
    return ERROR;
  }

  /* turn on zero linger time so that undelivered data is discarded when
     socket is closed
   */
  opt.l_onoff = 1;
  opt.l_linger = 0;

  sockarg = 1;

  setsockopt(StateSockFd, SOL_SOCKET, SO_LINGER, (char*) &opt, sizeof(opt));
  setsockopt(StateSockFd, IPPROTO_TCP, TCP_NODELAY, (char *)&sockarg, sizeof(int));

  /* build server socket address */
  sockAddrSize = sizeof (struct sockaddr_in);
  bzero( (char *)&serverAddr, sockAddrSize );
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(STATE_PORT);
  serverAddr.sin_addr.s_addr = inet_addr(DISPLAY_IP);

  /* connect to server */
  if (connect(StateSockFd, (struct sockaddr *)(&serverAddr), sockAddrSize) == ERROR)
  {
    perror("connect() failed in framegrab_init");
    close( StateSockFd );
    return ERROR;
  }

  return OK;

}


void shutdown_statetx(void) 
{
  close( StateSockFd );
}


int start_frame_tx = 0;

int init_frametx(void) 
{
  struct sockaddr_in serverAddr;
  int sockAddrSize;
  struct linger opt;
  int sockarg;

  /* create client's socket */
  if ( (FrameSockFd = socket( AF_INET, SOCK_STREAM, 0 )) == ERROR )
  {
    perror("connect() failed in frametx_init");
    close( FrameSockFd );
    return ERROR;
  }

  /* turn on zero linger time so that undelivered data is discarded when
     socket is closed
   */
  opt.l_onoff = 1;
  opt.l_linger = 0;

  sockarg = 1;

  setsockopt(FrameSockFd, SOL_SOCKET, SO_LINGER, (char*) &opt, sizeof(opt));
  setsockopt(FrameSockFd, IPPROTO_TCP, TCP_NODELAY, (char *)&sockarg, sizeof(int));

  /* build server socket address */
  /* build server socket address */
  sockAddrSize = sizeof (struct sockaddr_in);
  bzero( (char *)&serverAddr, sockAddrSize );
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(DISPLAY_PORT);
  serverAddr.sin_addr.s_addr = inet_addr(DISPLAY_IP);

  /* connect to server */
  if (connect(FrameSockFd, (struct sockaddr *)(&serverAddr), sockAddrSize) == ERROR)
  {
    perror("connect() failed in framegrab_init");
    close( FrameSockFd );
    return ERROR;
  }

  return OK;

}


void shutdown_frametx(void) 
{
  close( FrameSockFd );
}


int xga_frame_to_net(unsigned short *frameptr, int length)
{
  int bytes_written=0;
  int total_bytes_written=0;
  int bytes=0;
  unsigned char c = FRAME_ID;
  unsigned short pixel;
 
  c = frame_seq_cnt;
  bytes = write(FrameSockFd, &c, 1);
  total_bytes_written += bytes;

  frame_seq_cnt = (frame_seq_cnt+1) % 255;
  
  c = FRAME_ID;
  bytes = write(FrameSockFd, &c, 1);
  total_bytes_written += bytes;

  while(bytes_written < length)
  {
    pixel = htons(*frameptr);
    bytes = write(FrameSockFd, (char *)&pixel, 2);
    bytes_written += bytes;
    frameptr++;
  }

  return total_bytes_written;
 
}


unsigned char syncstr[4];
char ppm_hdr[]="P6\n#test\n320 240\n255\n";
unsigned int frameToNetCnt = 0;
int total_bytes_written=0;
int hdr_size=0;

/* pass array of RGB bytes for 320x240 color frame */
int frame_to_net(char *buffer)
{
  int bytes_written=0;
  int bytes=0, i, hdr_bytes=0;
  unsigned char c;
  unsigned int word;
  unsigned int syncword;
  unsigned int frameNum = 0;

  wvEvent((frameNum % 999)+4000,NULL,0);

  total_bytes_written=0;

  hdr_size=sizeof(ppm_hdr);
  
  hdr_bytes = write(FrameSockFd, &(ppm_hdr[0]), (sizeof(ppm_hdr)-1));
  
  bytes_written=0;


  while(bytes_written < 230400)
  {
      bytes = write(FrameSockFd, &(buffer[total_bytes_written]), (230400-bytes_written));
      bytes_written += bytes;
      total_bytes_written += bytes;
  }


  frameToNetCnt++;

  wvEvent((frameNum % 999)+5000,NULL,0);

  total_bytes_written += hdr_bytes;

  return total_bytes_written;
 
}


int grayscale_frame_to_net(unsigned char *grayscale_buffer)
{
  int bytes_written=0;
  int total_bytes_written=0;
  int bytes=0, i;
  unsigned char c;
  unsigned int word;
  unsigned int syncword;

#if 0

  syncword = 0xAAA0 | frame_seq_cnt;
 
  word = HTONL(syncword);
  memcpy(&(syncstr[0]), &word, 4);

  frame_seq_cnt = (frame_seq_cnt+1) % 255;
  
  bytes = write(FrameSockFd, &(syncstr[0]), 4);
  total_bytes_written += bytes;
#endif

  bytes_written=0;


  for(i=0;i<240;i++)
  {

    bytes_written = 0;

    while(bytes_written < 320)
    {
      bytes = write(FrameSockFd, &(grayscale_buffer[total_bytes_written]), (320-bytes_written));
      bytes_written += bytes;
      total_bytes_written += bytes;
    }

    wvEvent(i,NULL,0);

  }

  return total_bytes_written;
 
}



unsigned char wordstr[256];
/*
int state_to_net(struct race_state *stateptr)
{
  int bytes_written=0;
  int total_bytes_written=0;
  int bytes=0, i;
  unsigned char c;
  unsigned int word;
  unsigned short hword;
 
  wordstr[0] = state_seq_cnt;
  state_seq_cnt = (state_seq_cnt+1) % 255;
  wordstr[1] = STATE_ID;

  /* 32 bytes 
  word = HTONL(stateptr->pos);
  memcpy(&(wordstr[2]), &word, 4);
  word = HTONL(stateptr->vel);
  memcpy(&(wordstr[6]), &word, 4);
  word = HTONL(stateptr->accel);
  memcpy(&(wordstr[10]), &word, 4);
  word = HTONL(stateptr->yaw);
  memcpy(&(wordstr[14]), &word, 4);
  word = HTONL(stateptr->yaw_rate);
  memcpy(&(wordstr[18]), &word, 4);
  word = HTONL(stateptr->yaw_accel);
  memcpy(&(wordstr[22]), &word, 4);
  word = HTONL(stateptr->target);
  memcpy(&(wordstr[26]), &word, 4);
  word = HTONL(stateptr->direction);
  memcpy(&(wordstr[30]), &word, 4);

  /* 14 bytes 
  hword = HTONS(stateptr->target_size);
  memcpy(&(wordstr[34]), &hword, 2);
  hword = HTONS(stateptr->right_servo_cmd);
  memcpy(&(wordstr[36]), &hword, 2);
  hword = HTONS(stateptr->left_servo_cmd);
  memcpy(&(wordstr[38]), &hword, 2);
  hword = HTONS(stateptr->target_x);
  memcpy(&(wordstr[40]), &hword, 2);
  hword = HTONS(stateptr->target_y);
  memcpy(&(wordstr[42]), &hword, 2);
  hword = HTONS(stateptr->tilt_servo_cmd);
  memcpy(&(wordstr[44]), &hword, 2);
  hword = HTONS(stateptr->pan_servo_cmd);
  memcpy(&(wordstr[46]), &hword, 2);


  bytes_written = 0;

  while(bytes_written < 48)
  {
    bytes = write(StateSockFd, &wordstr[bytes_written], (48-bytes_written));
    total_bytes_written += bytes;
    bytes_written += bytes;
  }

#if 0
  printf("Total bytes written = %d\n", total_bytes_written);
#endif

  return total_bytes_written;

}*/

void print_raw_sent(void)
{
  int i;

  printf("Raw data sent = \n");
  for(i=0;i<48;i++)
    printf("0x%x ", wordstr[i]);
  printf("\n");

}

