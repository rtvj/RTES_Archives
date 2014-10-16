


/* --------------------------------------------------------------------------
 * The First part of the code is video driver for bt878 based camera system
 * ------------------------------------------------------------------------ 
 * The Second part of the code is the Image Processing and position value
 *                         computation segment
 * ------------------------------------------------------------------------*/


/* ------------------------------------------------------------------------------------------------ 
 * * ----------------------------------------------------------------------------------------------
 *              
 *                                           PART 1
 *                                                               
 * ------------------------------------------------------------------------------------------------
 * * ------------------------------------------------------------------------------------------------ */





/****************************************************************
*
* MODULE:		Bt878 Decoder Interace
*
* DESCRITPION:		Connexant Bt878 NTSC Video Decoder VxWorks
*			5.4 PCI Driver and Video Processing Utilities
*
* ORIGINAL AUTHOR:		Sam Siewert, University of Colorado, Boulder
* UPDATED BY:			Zach Pfeffer, University of Colorado, Boulder
*				Dan Walkes, University of Colorado, Boulder
* 
* CREATED:		May 7, 2000
* MODIFIED:		June 17, 2004
* 			Feb 22, 2005
* 
* NOTES:
*
* This code is intended to be used as driver bottom-half functions
* rather than an exported top-half video driver interface.
*
* Note that this was coded for VxWorks 5.4 for x86/PCI architecture
* and assumes the pcPentium BSP PCI API.
*
* As of Feb 22, the Pentium 3 BSP and Tornado 2.2 development tool is also
* supported.
*
* CODE USAGE:
*
* To test the bottom half functions, start video acquisition with:
* ---------------------------------------------------------------------
*
* 1) call to start_video_report(void (*pNewFrameReportFn) (unsigned char *pNewBuffer))
* 	with a pointer to the function to call when a new frame is received 
* 	(or to simply call start_video() to test functionality)
*
* 2) call to activate(1) for 320x240 RGB 32 bit 30 fps decoding
*
* 3) call to set_mux(3) for external NTSC camera S-video input
* 	Note: the mux value depends on which card you are using and which video
* 	input you are using.  See btvid.h #defines for more information.
*
* 4) call to reset_status() to ensure proper sync of code and hardware
*
* 5) repeated calls to report() to ensure frame acauisition is working
*    and that errors in decoding are being handled properly.
*
* The frame_acq_cnt (value returned by GetFrameAcqCount()) 
* should increase and that the DMA PC should advance as well as the decoder count.
*
* Finally try dumping a frame to the host with
* the write_save_buffer(0) function call.
*
*
* 6) call to full_reset() to shut the bottom-half down completely.
*
*
* REVISION HISTORY AND NOTES:
*
* Date			Update
* ---------------------------------------------------------------------
* May 7, 2000		Created.
* March 23, 2001	Updated for VxWorks 5.4 - changed PCI interrupt
*                       vector registration code.
*
* July 2004		Built a struct array to store info about each PCI
*			card of interest.
*
* July 2004		Wrote:
*				decodePIRQxRouteControlRegister
*				findAndDecodePCIInterruptRoutingTable
*				dumpPCIFunctionsConfigurationHeader
*				dumpPCIStatusAndCommand
*
* July 2004		Changed the way the Interrupt is specified. 
*			Instead of statically setting it in a global
*			define it is now read from the BT878's interrupt 
*			line register in start_video().
*
* July 2004		Expanded btvid_controller_config to output more
*			debug information about the base address registers
*			in the BT878.
*
* July 2004		Due to incompatibilities in versions of
*			Northbridge and Southbridge Intel chips
*			the current code looks for and explicit 
*			vendor,device value in intel_pci_config. 
*			If it fails to find such a value it 
*			defaults to the old settings.
*				
* Feb 2005		Split files into debug and .h files for readability
* 			Added #defines for typical SET_MUX values.
* 			Made some modifications to PCI_DEV operation and
* 			added functionality specific to P3 targets.
* 			Added start_video_report(void (*pNewFrameReportFn) (unsigned char *pNewBuffer))
*			to decrease coupling between btvid file and application file.
*			Moved intel_pci_config(),btvid_controller_config(),test_status() to start_video
*			from btvid_drvr so that errors do not occur when driver functions are spawned at
*			shell priority (so activate() does not occur until after these functions have run.)
*
* WARNINGS:
*
* The only mode(s) fully supported and test to date are:
* ---------------------------------------------------------------------
* NTSC_320_X_240
*
* Other modes have not been tested and may not work without modification.
*
*
* REFERENCES:
*
* 1) Bt878 chipset documentation - available on the class WWW site: 
*    http://www-sgc.colorado.edu/~siewerts/ecen/hardware/pdf/
*
* 2) Ph.D. dissertation on RT Video pipelines - available on the CU
*    research WWW site: 
*    http://www-sgc.colorado.edu/~siewerts/research/rt.html
*
* 3) PCI vendor and device IDs - available on class WWW site:
*    http://www.datashopper.dk/~finth/pci.html
*
****************************************************************/



/* VxWorks API includes */
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
#include "frametx.h"
#include "netdb.h"

/* VxWorks 5.4 PCI driver interface includes */
#include "drv/pci/pciConfigLib.h"
#include "drv/pci/pciConfigShow.h"
#include "drv/pci/pciHeaderDefs.h"
#include "drv/pci/pciLocalBus.h"
#include "drv/pci/pciIntLib.h"


/* pcPentium BSP includes */
#include "sysLib.h"

#include "btvid.h"
int frame_to_net(unsigned char *buffer);

STATUS  pciLibInitStatus = NONE;    /* initialization done */
int     pciConfigMech = NONE;       /* 1=mechanism-1, 2=mechanism-2 */
int     pciMaxBus = PCI_MAX_BUS;    /* Max number of sub-busses */


/* Bottom-half status variables */

UINT last_dstatus = 0x0;
UINT last_isr_status = 0x0;
UINT total_dma_disabled_errs = 0x0;
UINT total_sync_errs = 0x0;
UINT total_abort_errs = 0x0;
UINT total_dma_errs = 0x0;
UINT total_parity_errs = 0x0;
UINT total_bus_parity_errs = 0x0;
UINT total_fifo_overrun_errs = 0x0;
UINT total_fifo_resync_errs = 0x0;
UINT total_bus_latency_errs = 0x0;
UINT total_risc_ints = 0x0;
UINT total_ints = 0x0;
UINT total_write_tags = 0x0;
UINT total_skip_tags = 0x0;
UINT total_jump_tags = 0x0;
UINT total_sync_tags = 0x0;




unsigned char R[76800];
unsigned char B[76800];
unsigned char G[76800];
unsigned char X[76800];
unsigned char avgX[76800];


unsigned char header[22];
unsigned char maxR = 255;
unsigned char maxG = 255;
unsigned char maxB = 255;
unsigned char minR = 0;
unsigned char minG = 0;
unsigned char minB = 0;
int xdef=850, ydef=200, zdef=512;

int xval, yval, xvalprev, yvalprev;
int dis = 0;



/* Bottom-half control-monitor variables */
int BT878INT; 	/* this will be set with the IRQ of the BT878 */

UINT	sysVectorIRQ0		= INT_NUM_IRQ0;
UINT	ir_table_addr		= NULL;
UINT	frame_rdy_cnt 		= 0;
int	replace_write_with_skip	= FALSE;
int	acq_type		= NTSC_320_X_240;


UINT	int_errors_to_check = (
				SCERR_INT	|
				OCERR_INT	|
                                PABORT_INT	|
                                RIPERR_INT	|
                                PPERR_INT	|
                                FDSR_INT	|
                                FTRGT_INT	|
                                FBUS_INT	|
                                MYSTERY_INT
);

int	btvid_tid;
int	frame_acq_cnt = 0;
int	current_frame = 1;

int logfd;

/*			Socket Declarations				*/
struct addrinfo hints, *res, *p;

extern int millis;

static int sockfd, numbytes;
	
	
SEM_ID frameRdy;
SEM_ID sync1;
SEM_ID sync2;
SEM_ID sync3;
SEM_ID sync4;
/* a global pointer to the frame received function */
void (*pglobFnNewFrameRcvd) (unsigned char *pNewFrameBuffer);


PCI_DEV_BUS_NAME PCI_DEV[NUMDEVICES];

#define MAX_DEVICE_TYPES	9	
const DEVTYPE_S asDefDevtype[MAX_DEVICE_TYPES] = 
{
	{ NORTH_BRIDGE, INTEL_VENDOR_ID, PCI_DEVICE_ID_INTEL_82441FX, "Northbridge: Intel 82441FX", FALSE },
	{ NORTH_BRIDGE, INTEL_VENDOR_ID, PCI_DEVICE_ID_INTEL_82439TX, "Northbridge: Intel 82439TX", FALSE },
	{ NORTH_BRIDGE, INTEL_VENDOR_ID, PCI_DEVICE_ID_INTEL_820820,	"Northbridge: Intel 820820", FALSE },
	{ SOUTH_BRIDGE, INTEL_VENDOR_ID, PCI_DEVICE_ID_INTEL_82371SB_0,	"Southbridge: Intel 82371SB", FALSE },
	{ SOUTH_BRIDGE, INTEL_VENDOR_ID, PCI_DEVICE_ID_INTEL_82371AB_0,	"Southbridge: Intel 82371AB", FALSE },
	{ SOUTH_BRIDGE, INTEL_VENDOR_ID, PCI_DEVICE_ID_INTEL_82801AA,	"Southbridge: Intel 82801AA", FALSE },
	{ CAPTURE_VIDEO_CARD1, PCI_VENDOR_ID_BROOKTREE, PCI_DEVICE_ID_BT878, "Capture Card 1: BT878", FALSE},
	{ CAPTURE_VIDEO_CARD2, PCI_VENDOR_ID_BROOKTREE, PCI_DEVICE_ID_BT878, "Capture Card 2: BT878", FALSE},
	{ 0,0,0,0},	/* null terminate list*/
};

DEVTYPE_S asDevtype[MAX_DEVICE_TYPES];	/* a RAM copy of the same list */

/* Returns the first instance of the specified device in the device list 
 * Returns a pointer to the device type entry if success, 0 if fail */
DEVTYPE_S *FindNextDevTypeInList(unsigned int uiVendorID, unsigned int uiDeviceID)
{
	DEVTYPE_S *psDevType;

	psDevType = &asDevtype[0];

	while(psDevType->enDevType || psDevType->uiVenId || psDevType->uiDevId || psDevType->pDevDescString)
	{
		if(	psDevType->uiVenId == uiVendorID &&
			psDevType->uiDevId == uiDeviceID &&
		 	!psDevType->bFound )
		{
			psDevType->bFound = TRUE;
			/*printf("Found Device Type: %s\n",psDevType->pDevDescString);*/
			return psDevType;
		}
		psDevType++;
	};

	return FALSE;
	
}

STATUS findBusDevFunNo(void) {
	
	int i = 0;
	STATUS temp;
	for(i = 0; i < NUMDEVICES; i++)
		PCI_DEV[i].Found = pciFindDevice(	
				PCI_DEV[i].venId, 
				PCI_DEV[i].devId,
				PCI_DEV[i].index,
				&(PCI_DEV[i].busNo),
				&(PCI_DEV[i].deviceNo),
				&(PCI_DEV[i].funcNo)
			     );

	return temp;
}


/* -----------------------------------------------------------
 	Input:	none
	Return: STATUS OK if all specified devices were found
			ERROR if any device was not found

	This function prints any errors that occured during
	PCI_DEV structure init. 
*/
STATUS reportDevFoundStatus(void)
{
	STATUS stReturn = OK;

	if(PCI_DEV[NORTH_BRIDGE].Found != OK)
	{
		/*printf("Warning: Northbridge not found!!!\n");*/
		stReturn = ERROR;
	}
	
	if(PCI_DEV[SOUTH_BRIDGE].Found != OK)
	{
		/*printf("Warning: Southbridge not found!!!\n");*/
		
		stReturn = ERROR;
	}
	
	if(PCI_DEV[CAPTURE_VIDEO_CARD1].Found != OK)
	{
		/*printf("Error: Capture card not found!!!\n");*/
		stReturn = ERROR;
	}

	if(PCI_DEV[CAPTURE_VIDEO_CARD2].Found != OK && stReturn != ERROR)
	{
		/*printf("Note: Only one capture card found\n");*/
	}
	
	return stReturn;
}
	
/* Fills in the PCI dev structure with the vendor ID, bus number, and device number for
 * All devices found.
 * */	
unsigned char FillPCIDev(void)
{
	int deviceNo;
	int devices;
	int busNo=0;
	ushort_t vendorId;
	ushort_t deviceId;
	DEVTYPE_S *psDevType;

	if (pciLibInitStatus != OK)         /* sanity check */
	{
		if (pciConfigLibInit (PCI_MECHANISM_1, 0xCF8, 0xCFC, 0) != OK)
		{
			/*printf("PCI lib config error\n");*/
			      return (ERROR);
		}
	}
  
	if (pciConfigMech == PCI_MECHANISM_1)
		devices = 0x1f;
	else
		devices = 0x1f;
    
	memcpy(asDevtype,asDefDevtype,sizeof(asDefDevtype));	/* copy the default device type list */
 	for(busNo = 0; busNo < 10; busNo++)
	{
		for (deviceNo=0; deviceNo <= devices; deviceNo++)
		{
			pciConfigInWord (busNo, deviceNo, 0, PCI_CFG_VENDOR_ID, &vendorId);
			pciConfigInWord (busNo, deviceNo, 0, PCI_CFG_DEVICE_ID, &deviceId);
	
			if (vendorId != 0xffff)
	  		{
				psDevType = FindNextDevTypeInList(vendorId,deviceId);	/* is this device found in the list? */
				if(psDevType)	/* the device was found */
				{	/* init respective portion of PCIDEV structure */
					PCI_DEV[psDevType->enDevType].venId = vendorId;
					PCI_DEV[psDevType->enDevType].devId = deviceId;
					PCI_DEV[psDevType->enDevType].busNo = busNo;
					PCI_DEV[psDevType->enDevType].deviceNo = deviceNo;
				}
			}
		}
	}
	
		
	return (OK);
  
}


/* -----------------------------------------------------------
 	Input:	none
	Return: STATUS OK if all specified devices were found
			ERROR if any device was not found


	This function is used to initialize PCI devices.  Set
	the vendor ID and device ID for each of the respective
	devices you assume you will find on the PCI bus for your 
	particular target.
*/
STATUS initializePCI_DEV(void) {

	int i = 0;
	

	BT878INT = 10; /* as a default in ECEN5623, BT878 cards will use IRQ11 */

	for(i = 0; i < NUMDEVICES; i++) {
		
		PCI_DEV[i].venId = 0;
		PCI_DEV[i].devId = 0;
		PCI_DEV[i].index = 0;
		PCI_DEV[i].busNo = 0;
		PCI_DEV[i].deviceNo = 0;
		PCI_DEV[i].funcNo = 0;
	}

	FillPCIDev();	/* fill in the PCIDEV structure with all device info */
	
	findBusDevFunNo();
	
	return reportDevFoundStatus();
}


/* For Base Address Register Checks */


#define BAR_IS_IMPLEMENTED(readback) 		readback & 0xFFFFFFFF
#define BAR_IS_MEM_ADDRESS_DECODER(readback) 	!(readback & 0x1)
#define BAR_IS_IO_ADDRESS_DECODER(readback) 	(readback & 0x0)
#define BAR_IS_32_BIT_DECODER(readback)		((readback & 0x6) >> 1) == 0x0
#define BAR_IS_64_BIT_DECODER(readback)		((readback & 0x6) >> 1) == 0x2
#define BAR_IS_PREFETCHABLE(readback)		(readback & 0x8)

#define PCI_ROUTING_TABLE_SIGNATURE		0x52495024
#define PCI_ROUTING_TABLE_SIG_BYTE0		0x24
#define PCI_ROUTING_TABLE_SIG_BYTE1		0x50
#define PCI_ROUTING_TABLE_SIG_BYTE2		0x49
#define PCI_ROUTING_TABLE_SIG_BYTE3		0x52



/*

 Design Notes on Microcode

 PACKED RGB32 320x240 NTSC MODE MICROCODE
 ----------------------------------------

 BYTE3	BYTE2	BYTE1	BYTE0
 alpha	RED	GREEN	BLUE

 ODD LINES first (120 x 320)

 00320 ... 00639
 00960 ... 01179
 01500 ... 01819
 ...
 70480 ... 70799

 
 EVEN LINES second (120 x 320)
 
 00000 ... 00319
 00640 ... 00959
 01180 ... 01499
 ... 
 70160 ... 70479  ---- GENERATES FRAME INTA


 490 microcode instructions per frame

 X

 NUMFRAMES
 -----------
 490 * NUMFRAMES microcode instructions


 2
 ------------------------------------------------------- FIELD SYNC
 001 DMA_MC_SYNC_FM1_WORD_0,
 002 DMA_MC_SYNC_WORD_1,

 120 x 2 = 240
 ------------------------------------------------------- BEG ODD LINE FIELD
 001 DMA_MC_WRITE_1280_LINE,
 002 &(frame_buffer[0][320]),
 ------------------------------------------------------- END ODD LINE FIELD

 4
 ------------------------------------------------------- VRE & FIELD SYNC
 001 DMA_MC_SYNC_VRE_WORD_0,
 002 DMA_MC_SYNC_WORD_1,
 003 DMA_MC_SYNC_FM1_WORD_0,
 004 DMA_MC_SYNC_WORD_1,

 120 x 2 = 240
 ----------------------------/--------------------------- BEG EVEN LINE FIELD
 001 DMA_MC_WRITE_1280_LINE,
 002 &(frame_buffer[0][0]),
 ------------------------------------------------------- END EVEN LINE FIELD
 
 2
 ------------------------------------------------------- VRO SYNC
 001 DMA_MC_SYNC_VRO_WORD_0,
 002 DMA_MC_SYNC_WORD_1,

 001 JUMP
 002 ADDRESS
*/


/******************************** BEG DMA MC ****************/

/**** MC TAGS FOR DEBUGGING ****/
#define DMA_MC_SYNC	0x00080000      
#define DMA_MC_WRITE	0x00010000
#define DMA_MC_SKIP	0x00020000
#define DMA_MC_JUMP	0x00B40000 /* Clears all for next frame */

#define DMA_MC_SYNC_FM1_WORD_0		(0x80008006 | DMA_MC_SYNC)
#define DMA_MC_SYNC_FM1_WORD_0_IRQ	(0x81008006 | DMA_MC_SYNC)
#define DMA_MC_SYNC_WORD_1		(0x00000000)

#define DMA_MC_SKIP_1280_LINE		(0x2C000500 | DMA_MC_SKIP)
#define DMA_MC_SKIP_640_SOL		(0x28000280 | DMA_MC_SKIP)
#define DMA_MC_SKIP_640_EOL		(0x24000280 | DMA_MC_SKIP)
#define DMA_MC_SKIP_320_SOL		(0x28000140 | DMA_MC_SKIP)
#define DMA_MC_SKIP_320			(0x20000140 | DMA_MC_SKIP)
#define DMA_MC_SKIP_320_EOL		(0x24000140 | DMA_MC_SKIP)
#define DMA_MC_SKIP_1280_LINE_IRQ	(0x2D000500 | DMA_MC_SKIP)
#define DMA_MC_SKIP_40_SOL		(0x28000028 | DMA_MC_SKIP)
#define DMA_MC_SKIP_40_EOL        	(0x24000028 | DMA_MC_SKIP)

#define DMA_MC_SKIP_2560_LINE     	(0x2C000A00 | DMA_MC_SKIP)
#define DMA_MC_SKIP_1280_SOL      	(0x28000500 | DMA_MC_SKIP)
#define DMA_MC_SKIP_1280_EOL      	(0x24000500 | DMA_MC_SKIP)
#define DMA_MC_SKIP_2560_LINE_IRQ 	(0x2D000A00 | DMA_MC_SKIP)

#define DMA_MC_WRITE_2560_LINE     	(0x1C000A00 | DMA_MC_WRITE)
#define DMA_MC_WRITE_1280_SOL      	(0x18000500 | DMA_MC_WRITE)
#define DMA_MC_WRITE_1280_EOL      	(0x14000500 | DMA_MC_WRITE)
#define DMA_MC_WRITE_2560_LINE_IRQ 	(0x1D000A00 | DMA_MC_WRITE)

#define DMA_MC_WRITE_1280_LINE     	(0x1C000500 | DMA_MC_WRITE)
#define DMA_MC_WRITE_80_LINE       	(0x1C000050 | DMA_MC_WRITE)
#define DMA_MC_WRITE_640_SOL       	(0x18000280 | DMA_MC_WRITE)
#define DMA_MC_WRITE_640_EOL       	(0x14000280 | DMA_MC_WRITE)
#define DMA_MC_WRITE_320_SOL       	(0x18000140 | DMA_MC_WRITE)
#define DMA_MC_WRITE_320           	(0x10000140 | DMA_MC_WRITE)
#define DMA_MC_WRITE_320_EOL       	(0x14000140 | DMA_MC_WRITE)
#define DMA_MC_WRITE_1280_LINE_IRQ 	(0x1D000500 | DMA_MC_WRITE)

#define DMA_MC_SYNC_VRE_WORD_0     	(0x80008004 | DMA_MC_SYNC)
#define DMA_MC_SYNC_VRE_WORD_0_IRQ 	(0x81008004)

#define DMA_MC_SYNC_VRO_WORD_0     	(0x8000800C | DMA_MC_SYNC)
#define DMA_MC_SYNC_VRO_WORD_0_IRQ 	(0x8100800C)

#define DMA_MC_JUMP_TO_BEG     		(0x70000000 | DMA_MC_JUMP)
#define DMA_MC_JUMP_TO_BEG_IRQ 		(0x71000000 | DMA_MC_JUMP)

#define DMA_MC_HALT 			(0xFFFFFFFF)


/**** TEST SCANLINE BUFFER ****/
unsigned int test_buffer[320];


/**** TEST MICROCODE 1 ****/

/* For one 640 pixel by 480 line NTSC frame

  2
  ---------------------
  DMA_MC_SYNC_FM1_WORD_0,
  DMA_MC_SYNC_WORD_1,

  240 x 1 = 240
  -------------
  DMA_MC_SKIP_1280_LINE,

  4
  ---------------------
  DMA_MC_SYNC_VRE_WORD_0,
  DMA_MC_SYNC_WORD_1,
  DMA_MC_SYNC_FM1_WORD_0,
  DMA_MC_SYNC_WORD_1,

  240 x 1 = 240
  -------------
  DMA_MC_SKIP_1280_LINE,

  4
  ---------------------
  DMA_MC_SYNC_VRO_WORD_0,
  DMA_MC_SYNC_WORD_1,

  DMA_MC_JUMP_TO_BEG, 
  (unsigned int)&(dma_test_microcode[0])

*/
unsigned int dma_test_microcode[970];


/**** RGB32 MICROCODE BUFFER ****

 488 * NUMFRAMES microcode instructions + 2 for jump

****/
#define MCSIZE 488
unsigned int dma_microcode[(NUMFRAMES*MCSIZE)+2];

/*
 126 * NUMFRAMES
*/
#define PUNYMCSIZE 126
unsigned int dma_puny_microcode[(NUMFRAMES*PUNYMCSIZE)+2];


/**** RGB32 MICROCODE BUFFER ****

 968 * NUMFRAMES microcode instructions

****/
#define LGMCSIZE 968
unsigned int dma_large_microcode[(NUMFRAMES*LGMCSIZE)+2];


/******************************** END DMA MC ****************/



void print_mc(int mc)
{
  int i;

  if(mc == RGB32_640x480_MICROCODE)
  {
    for(i=0;i<(sizeof(dma_large_microcode)/(sizeof(unsigned int)));i++)
    {
      /*logMsg("%4d 0x%8x @ 0x%8x\n", i, dma_large_microcode[i], &(dma_large_microcode[i]),0,0,0);*/
    }
  }
  if(mc == TEST_MICROCODE)
  {
    for(i=0;i<(sizeof(dma_test_microcode)/(sizeof(unsigned int)));i++)
    {
      /*logMsg("%4d 0x%8x @ 0x%8x\n", i, dma_test_microcode[i], &(dma_test_microcode[i]),0,0,0);*/
    }
  }

  else if(mc == RGB32_320x240_MICROCODE)
  {
    for(i=0;i<(sizeof(dma_microcode)/(sizeof(unsigned int)));i++)
    {
      /*logMsg("%4d 0x%8x @ 0x%8x\n", i, dma_microcode[i], &(dma_microcode[i]),0,0,0);*/
    }
  }

}



/**** RGB32 FRAME BUFFER for 640x480 frame 

 LGFRAME 307200

 ****/
UINT large_frame_buffer[NUMFRAMES][307200];


/**** RGB32 FRAME BUFFER for 320x240 frame ****/
UINT frame_buffer[NUMFRAMES][76800];
UINT y8_frame_buffer[NUMFRAMES][19200];
UINT save_buffer[76800];
UINT y8_save_buffer[19200];

/**** Y8 FRAME BUFFER for 80x60 frame ****/
UINT puny_frame_buffer[NUMFRAMES][1200];



void write_y8_save_buffer(void)
{
  int fd, i;
  char *savebuffbyteptr = (char *)&(y8_save_buffer[0]);

  fd = open("/tgtsvr/testy8frame.out", O_RDWR|O_CREAT, 0644);

  write(fd,"P5\n",3);
  write(fd,"#test\n",6);
  write(fd,"320 240\n",8);
  write(fd,"255\n",4);

  /* Write out RED channel */
  for(i=0;i<76800;i++)
  {
    write(fd,(savebuffbyteptr+1),1);
    savebuffbyteptr+=4;
  }
  
  close(fd);

}

char SaveBuffer[320*240*3];

void SetSockStruct()
{
	
	
	/*int status;
	
	memset(&hints, 0, sizeof(hints) );
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_STREAM;
	
	if ((status = getaddrinfo("172.21.74.100", "50000", &hints, &res)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 1;
	}
	
	
	int sockaddrlen;
	sockaddrlen = sizeof(struct sockaddr_in);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = inet_addr("172.21.74.100");
	my_addr.sin_port = htons(50000);
	
	/*
	for(p = res; p != NULL; p = p->ai_next) {
	if ((sockfd = socket(PF_INET, p->ai_socktype,
	p->ai_protocol)) == -1) {
	perror("client: socket");
	continue;
	}
	if (connect(sockfd, (struct sockaddr *)(&my_addr), sockaddrlen) == ERROR) {
	close(sockfd);
	perror("client: connect");
	while(1){
	printf("Inside connect while 1\n");
	}
	}
	
	
	if (p == NULL) {
	fprintf(stderr, "client: failed to connect\n");
	return 2;
	}
	
	freeaddrinfo(res); 
	*/
	 struct sockaddr_in my_addr;
	  int sockAddrSize;
	  struct linger opt;
	  int sockarg;

	  /* create client's socket */
	  if ( (sockfd = socket( AF_INET, SOCK_STREAM, 0 )) == ERROR )
	  {
	    perror("socket() failed in SetSockStruct");
	    close( sockfd );
	    return ERROR;
	  }

	  /* turn on zero linger time so that undelivered data is discarded when
	     socket is closed
	   */
	  opt.l_onoff = 1;
	  opt.l_linger = 0;

	  sockarg = 1;

	  setsockopt(sockfd, SOL_SOCKET, SO_LINGER, (char*) &opt, sizeof(opt));
	  setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&sockarg, sizeof(int));

	  /* build server socket address */
	  /* build server socket address */
	  sockAddrSize = sizeof (struct sockaddr_in);
	  bzero( (char *)&my_addr, sockAddrSize );
	  my_addr.sin_family = AF_INET;
	  my_addr.sin_port = htons(50000);
	  my_addr.sin_addr.s_addr = inet_addr("172.21.74.100");

	  /* connect to server */
	  if (connect(sockfd, (struct sockaddr *)(&my_addr), sockAddrSize) == ERROR)
	  {
	    perror("connect() failed in framegrab_init");
	    close( sockfd );
	    return ERROR;
	  }

	  return OK;

}

void send_command(int b[10]) 
{
	
	if(send(sockfd, (char *)b, (sizeof(int)*10), 0) == -1)
		fprintf(stderr, "client: failed to send command\n");
	
}


UINT check_buffers(int fsize)
{
  int i,j;
  unsigned int test = 0;

  for(i=0;i<NUMFRAMES;i++)
  {
    /*if(fsize==1)*/
	  if(fsize == 0)
    {
      for(j=0;j<307199;j++)
        test |= large_frame_buffer[i][j];
    }
   /* else if(fsize==0)*/
	  else if(fsize == 1)
    {
      for(j=0;j<7679;j++)
        test |= frame_buffer[i][j];
    }
  }


  return test;

}


void clear_buffers(int fsize)
{
  int i,j;

  for(i=0;i<NUMFRAMES;i++)
  {
    bzero(&(large_frame_buffer[i]),(307200*4));
    bzero(&(frame_buffer[i]), (76800*4));
  }

  bzero(&(test_buffer[0]),(320*4));

}


void initialize_test_mc(void)
{
  int j;

  dma_test_microcode[0] = DMA_MC_SYNC_FM1_WORD_0;
  dma_test_microcode[1] = DMA_MC_SYNC_WORD_1;

  /* Initialize the 240 lines of ODD and EVEN microcode */

  /* odd lines */
  for(j=2;j<482;j+=2)
  {
    dma_test_microcode[j] = DMA_MC_SKIP_1280_SOL;
    dma_test_microcode[j+1] = DMA_MC_SKIP_1280_EOL;
  }

  j=482;
  dma_test_microcode[j] = DMA_MC_SYNC_VRE_WORD_0; j++;
  dma_test_microcode[j] = DMA_MC_SYNC_WORD_1; j++;

  dma_test_microcode[j] = DMA_MC_SYNC_FM1_WORD_0; j++;
  dma_test_microcode[j] = DMA_MC_SYNC_WORD_1; j++;
  
  /* even line */
  for(j=486;j<966;j+=2)
  {
    dma_test_microcode[j] = DMA_MC_SKIP_1280_SOL;
    dma_test_microcode[j+1] = DMA_MC_SKIP_1280_EOL;
  }

  j=966;
  dma_test_microcode[j] = DMA_MC_SYNC_VRO_WORD_0_IRQ; j++;
  dma_test_microcode[j] = DMA_MC_SYNC_WORD_1; j++;

  dma_test_microcode[j] = DMA_MC_JUMP_TO_BEG; j++;
  dma_test_microcode[j] = (unsigned int)&(dma_test_microcode[0]); j++;

}


void initialize_frame_mc(int fsize)
{
  int mcidx, i,j, k;

  if(fsize==NTSC_80_X_60)
  {
    /* For NUMFRAMES Frames */
    for(i=0;i<NUMFRAMES;i++)
    {

      mcidx = PUNYMCSIZE * i; j = 0;

      /* FIELD SYNC */  
      dma_puny_microcode[mcidx+j] = DMA_MC_SYNC_FM1_WORD_0; j++;
      dma_puny_microcode[mcidx+j] = DMA_MC_SYNC_WORD_1; j++;

      /* Initialize 30 lines of ODD microcode */
      for(j=2,k=0;j<32;j+=2,k++)
      {
  
        /* odd line */
        if(replace_write_with_skip)
        {
          dma_puny_microcode[mcidx+j] = DMA_MC_SKIP_40_SOL;
          dma_puny_microcode[mcidx+j+1] = DMA_MC_SKIP_40_EOL;
        }
        else
        {
          dma_puny_microcode[mcidx+j] = DMA_MC_WRITE_80_LINE;
          dma_puny_microcode[mcidx+j+1] = (unsigned int)&(puny_frame_buffer[i][(20+(k*40))]);
        }
 
      }

      j=32;

      /* VRE FIELD SYNC */ 
      dma_puny_microcode[mcidx+j] = DMA_MC_SYNC_VRE_WORD_0; j++;
      dma_puny_microcode[mcidx+j] = DMA_MC_SYNC_WORD_1; j++;
      dma_puny_microcode[mcidx+j] = DMA_MC_SYNC_FM1_WORD_0; j++;
      dma_puny_microcode[mcidx+j] = DMA_MC_SYNC_WORD_1; j++;

      /* Initialize 30 lines of EVEN microcode */
      for(j=36,k=0;j<66;j+=2,k++)
      {

        /* even line */
        if(replace_write_with_skip)
        {
          dma_puny_microcode[mcidx+j] = DMA_MC_SKIP_40_SOL;
          dma_puny_microcode[mcidx+j+1] = DMA_MC_SKIP_40_EOL;
        }
        else
        {
          dma_microcode[mcidx+j] = DMA_MC_WRITE_80_LINE;
          dma_microcode[mcidx+j+1] = (unsigned int)&(puny_frame_buffer[i][0+(k*40)]);
        }
      }
 
      j = 66;
   
      dma_puny_microcode[mcidx+j] = DMA_MC_SYNC_VRO_WORD_0_IRQ; j++;
      dma_puny_microcode[mcidx+j] = DMA_MC_SYNC_WORD_1; j++;

    }

    dma_puny_microcode[(NUMFRAMES*PUNYMCSIZE)] = DMA_MC_JUMP_TO_BEG;
    dma_puny_microcode[(NUMFRAMES*PUNYMCSIZE)+1] = (unsigned int)&(dma_microcode[0]);

  }

  else if(fsize==NTSC_320_X_240_GS)
  {
    /* For NUMFRAMES Frames */
    for(i=0;i<NUMFRAMES;i++)
    {

      mcidx = MCSIZE * i; j = 0;

      /* FIELD SYNC */  
      dma_microcode[mcidx+j] = DMA_MC_SYNC_FM1_WORD_0; j++;
      dma_microcode[mcidx+j] = DMA_MC_SYNC_WORD_1; j++;

      /* Initialize 120 lines of ODD microcode */
      for(j=2,k=0;j<242;j+=2,k++)
      {
  
        /* odd line */
        if(replace_write_with_skip)
        {
          dma_microcode[mcidx+j] = DMA_MC_SKIP_40_SOL;
          dma_microcode[mcidx+j+1] = DMA_MC_SKIP_40_EOL;
        }
        else
        {
          dma_microcode[mcidx+j] = DMA_MC_WRITE_80_LINE;
          dma_microcode[mcidx+j+1] = (unsigned int)&(y8_frame_buffer[i][(80+(k*160))]);
        }
 
      }

      j=242;

      /* VRE FIELD SYNC */ 
      dma_microcode[mcidx+j] = DMA_MC_SYNC_VRE_WORD_0; j++;
      dma_microcode[mcidx+j] = DMA_MC_SYNC_WORD_1; j++;
      dma_microcode[mcidx+j] = DMA_MC_SYNC_FM1_WORD_0; j++;
      dma_microcode[mcidx+j] = DMA_MC_SYNC_WORD_1; j++;

      /* Initialize 120 lines of EVEN microcode */
      for(j=246,k=0;j<486;j+=2,k++)
      {

        /* even line */
        if(replace_write_with_skip)
        {
          dma_microcode[mcidx+j] = DMA_MC_SKIP_40_SOL;
          dma_microcode[mcidx+j+1] = DMA_MC_SKIP_40_EOL;
        }
        else
        {
          dma_microcode[mcidx+j] = DMA_MC_WRITE_80_LINE;
          dma_microcode[mcidx+j+1] = (unsigned int)&(y8_frame_buffer[i][0+(k*160)]);
        }
      }
 
      j = 486;
   
      dma_microcode[mcidx+j] = DMA_MC_SYNC_VRO_WORD_0_IRQ; j++;
      dma_microcode[mcidx+j] = DMA_MC_SYNC_WORD_1; j++;

    }

    dma_microcode[(NUMFRAMES*MCSIZE)] = DMA_MC_JUMP_TO_BEG;
    dma_microcode[(NUMFRAMES*MCSIZE)+1] = (unsigned int)&(dma_microcode[0]);

  }

    /* For NUMFRAMES Frames */
  else if(fsize==NTSC_320_X_240)
  {
    /* For NUMFRAMES Frames */
    for(i=0;i<NUMFRAMES;i++)
    {

      mcidx = MCSIZE * i; j = 0;

      /* FIELD SYNC */  
      dma_microcode[mcidx+j] = DMA_MC_SYNC_FM1_WORD_0; j++;
      dma_microcode[mcidx+j] = DMA_MC_SYNC_WORD_1; j++;

      /* Initialize 120 lines of ODD microcode */
      for(j=2,k=0;j<242;j+=2,k++)
      {
  
        /* odd line */
        if(replace_write_with_skip)
        {
          dma_microcode[mcidx+j] = DMA_MC_SKIP_640_SOL;
          dma_microcode[mcidx+j+1] = DMA_MC_SKIP_640_EOL;
        }
        else
        {
          dma_microcode[mcidx+j] = DMA_MC_WRITE_1280_LINE;
          dma_microcode[mcidx+j+1] = (unsigned int)&(frame_buffer[i][(320+(k*640))]);
        }
 
      }

      j=242;

      /* VRE FIELD SYNC */ 
      dma_microcode[mcidx+j] = DMA_MC_SYNC_VRE_WORD_0; j++;
      dma_microcode[mcidx+j] = DMA_MC_SYNC_WORD_1; j++;
      dma_microcode[mcidx+j] = DMA_MC_SYNC_FM1_WORD_0; j++;
      dma_microcode[mcidx+j] = DMA_MC_SYNC_WORD_1; j++;

      /* Initialize 120 lines of EVEN microcode */
      for(j=246,k=0;j<486;j+=2,k++)
      {

        /* even line */
        if(replace_write_with_skip)
        {
          dma_microcode[mcidx+j] = DMA_MC_SKIP_640_SOL;
          dma_microcode[mcidx+j+1] = DMA_MC_SKIP_640_EOL;
        }
        else
        {
          dma_microcode[mcidx+j] = DMA_MC_WRITE_1280_LINE;
          dma_microcode[mcidx+j+1] = (unsigned int)&(frame_buffer[i][0+(k*640)]);
        }
      }
 
      j = 486;
   
      dma_microcode[mcidx+j] = DMA_MC_SYNC_VRO_WORD_0_IRQ; j++;
      dma_microcode[mcidx+j] = DMA_MC_SYNC_WORD_1; j++;

    }

    dma_microcode[(NUMFRAMES*MCSIZE)] = DMA_MC_JUMP_TO_BEG;
    dma_microcode[(NUMFRAMES*MCSIZE)+1] = (unsigned int)&(dma_microcode[0]);

  }

  else if(fsize==NTSC_640_X_480)
  {
    /* For NUMFRAMES Frames */
    for(i=0;i<NUMFRAMES;i++)
    {

      mcidx = LGMCSIZE * i; j = 0;

      /* FIELD SYNC */  
      dma_large_microcode[mcidx+j] = DMA_MC_SYNC_FM1_WORD_0; j++;
      dma_large_microcode[mcidx+j] = DMA_MC_SYNC_WORD_1; j++;

      /* Initialize 240 lines of ODD microcode */
      for(j=2,k=0;j<482;j+=2,k++)
      {

        /* odd line */
        dma_microcode[mcidx+j] = DMA_MC_SKIP_1280_SOL;
        dma_microcode[mcidx+j+1] = DMA_MC_SKIP_1280_EOL;
      }

      j=482;

      /* VRE FIELD SYNC */ 
      dma_large_microcode[mcidx+j] = DMA_MC_SYNC_VRE_WORD_0; j++;
      dma_large_microcode[mcidx+j] = DMA_MC_SYNC_WORD_1; j++;
      dma_large_microcode[mcidx+j] = DMA_MC_SYNC_FM1_WORD_0; j++;
      dma_large_microcode[mcidx+j] = DMA_MC_SYNC_WORD_1; j++;

      /* Initialize 240 lines of EVEN microcode */
      for(j=486;j<966;j+=2)
      {

        /* even line */
        dma_microcode[mcidx+j] = DMA_MC_SKIP_1280_SOL;
        dma_microcode[mcidx+j+1] = DMA_MC_SKIP_1280_EOL;
      }
 
      j = 966;
   
      dma_large_microcode[mcidx+j] = DMA_MC_SYNC_VRO_WORD_0_IRQ; j++;
      dma_large_microcode[mcidx+j] = DMA_MC_SYNC_WORD_1; j++;

    } /* end for NUMFRAMES */

    dma_large_microcode[(NUMFRAMES*LGMCSIZE)] = DMA_MC_JUMP_TO_BEG;
    dma_large_microcode[(NUMFRAMES*LGMCSIZE)+1] = (unsigned int)&(dma_microcode[0]);

  } 

}



int configure_ntsc(int fsize)
{
  unsigned int testval = 0;
  PCI_WRITE(RESET_REG,0x0,0x00000001);
  PCI_READ(TIMING_GEN_REG,0x0,&testval);
  PCI_WRITE(TIMING_GEN_REG,0x0,0x00000000);
  if((fsize==NTSC_320_X_240))
  {
    PCI_WRITE(MSB_CROP_EVEN_REG,0x0,0x00000011);
    PCI_WRITE(MSB_CROP_ODD_REG,0x0,0x00000011);

    PCI_WRITE(HACTIVE_LO_EVEN_REG,0x0,0x00000040);
    PCI_WRITE(HACTIVE_LO_ODD_REG,0x0,0x00000040);
    PCI_WRITE(VACTIVE_LO_EVEN_REG,0x0,0x000000E0);
    PCI_WRITE(VACTIVE_LO_ODD_REG,0x0,0x000000E0);
    PCI_WRITE(VDELAY_LO_EVEN_REG,0x0,0x00000016);
    PCI_WRITE(VDELAY_LO_ODD_REG,0x0,0x00000016);
    PCI_WRITE(HDELAY_LO_EVEN_REG,0x0,0x0000003C);
    PCI_WRITE(HDELAY_LO_ODD_REG,0x0,0x0000003C);

    /* Set HSCALE for 320 pixels per line */
    PCI_WRITE(HSCALE_EVEN_MSB_REG,0x0,0x00000015);
    PCI_WRITE(HSCALE_ODD_MSB_REG,0x0,0x00000015);
    PCI_WRITE(HSCALE_EVEN_LSB_REG,0x0,0x00000055);
    PCI_WRITE(HSCALE_ODD_LSB_REG,0x0,0x00000055);

    /* Set VSCALE for 240 lines per frame */
    PCI_WRITE(VSCALE_EVEN_MSB_REG,0x0,0x0000007E);
    PCI_WRITE(VSCALE_ODD_MSB_REG,0x0,0x0000007E);
    PCI_WRITE(VSCALE_EVEN_LSB_REG,0x0,0x00000000);
    PCI_WRITE(VSCALE_ODD_LSB_REG,0x0,0x00000000);

    PCI_WRITE(COLOR_FORMAT_REG,0x0,0x00000000);
    acq_type = NTSC_320_X_240;

  }

  if((fsize==NTSC_320_X_240_GS))
  {

    PCI_WRITE(MSB_CROP_EVEN_REG,0x0,0x00000011);
    PCI_WRITE(MSB_CROP_ODD_REG,0x0,0x00000011);

    PCI_WRITE(HACTIVE_LO_EVEN_REG,0x0,0x0000002C);
    PCI_WRITE(HACTIVE_LO_ODD_REG,0x0,0x0000002C);
    PCI_WRITE(VACTIVE_LO_EVEN_REG,0x0,0x000000E0);
    PCI_WRITE(VACTIVE_LO_ODD_REG,0x0,0x000000E0);
    PCI_WRITE(VDELAY_LO_EVEN_REG,0x0,0x00000016);
    PCI_WRITE(VDELAY_LO_ODD_REG,0x0,0x00000016);
    PCI_WRITE(HDELAY_LO_EVEN_REG,0x0,0x00000046);
    PCI_WRITE(HDELAY_LO_ODD_REG,0x0,0x00000046);

    /* Set HSCALE for 320 pixels per line */
    PCI_WRITE(HSCALE_EVEN_MSB_REG,0x0,0x00000015);
    PCI_WRITE(HSCALE_ODD_MSB_REG,0x0,0x00000015);
    PCI_WRITE(HSCALE_EVEN_LSB_REG,0x0,0x00000055);
    PCI_WRITE(HSCALE_ODD_LSB_REG,0x0,0x00000055);

    /* Set VSCALE for 240 lines per frame */
    PCI_WRITE(VSCALE_EVEN_MSB_REG,0x0,0x0000007E);
    PCI_WRITE(VSCALE_ODD_MSB_REG,0x0,0x0000007E);
    PCI_WRITE(VSCALE_EVEN_LSB_REG,0x0,0x00000000);
    PCI_WRITE(VSCALE_ODD_LSB_REG,0x0,0x00000000);

    PCI_WRITE(COLOR_FORMAT_REG,0x0,0x00000066);
    acq_type = NTSC_320_X_240_GS;
  }

  else if(fsize==NTSC_80_X_60)
  {

    PCI_WRITE(MSB_CROP_EVEN_REG,0x0,0x00000010);
    PCI_WRITE(MSB_CROP_ODD_REG,0x0, 0x00000010);

    PCI_WRITE(HACTIVE_LO_EVEN_REG,0x0,0x00000050);
    PCI_WRITE(HACTIVE_LO_ODD_REG,0x0,0x00000050);
    PCI_WRITE(VACTIVE_LO_EVEN_REG,0x0,0x000000E0);
    PCI_WRITE(VACTIVE_LO_ODD_REG,0x0,0x000000E0);
    PCI_WRITE(VDELAY_LO_EVEN_REG,0x0,0x00000016);
    PCI_WRITE(VDELAY_LO_ODD_REG,0x0,0x00000016);
    PCI_WRITE(HDELAY_LO_EVEN_REG,0x0,0x00000010);
    PCI_WRITE(HDELAY_LO_ODD_REG,0x0,0x00000010);

    /* Set HSCALE for 320 pixels per line */
    PCI_WRITE(HSCALE_EVEN_MSB_REG,0x0,0x00000086);
    PCI_WRITE(HSCALE_ODD_MSB_REG,0x0,0x00000086);
    PCI_WRITE(HSCALE_EVEN_LSB_REG,0x0,0x0000001A);
    PCI_WRITE(HSCALE_ODD_LSB_REG,0x0,0x0000001A);

    /* Set VSCALE for 240 lines per frame */
    PCI_WRITE(VSCALE_EVEN_MSB_REG,0x0,0x00000072);
    PCI_WRITE(VSCALE_ODD_MSB_REG,0x0,0x00000072);
    PCI_WRITE(VSCALE_EVEN_LSB_REG,0x0,0x00000000);
    PCI_WRITE(VSCALE_ODD_LSB_REG,0x0,0x00000000);

    /* Set color format for Y8 GRAYSCALE on ODD and EVEN */
    PCI_WRITE(COLOR_FORMAT_REG,0x0,0x00000066);

  }

  else if(fsize==NTSC_640_X_480)
  {

    PCI_WRITE(MSB_CROP_EVEN_REG,0x0,0x00000012);
    PCI_WRITE(MSB_CROP_ODD_REG,0x0,0x00000012);

    PCI_WRITE(HACTIVE_LO_EVEN_REG,0x0,0x00000080);
    PCI_WRITE(HACTIVE_LO_ODD_REG,0x0,0x00000080);
    PCI_WRITE(VACTIVE_LO_EVEN_REG,0x0,0x000000E0);
    PCI_WRITE(VACTIVE_LO_ODD_REG,0x0,0x000000E0);
    PCI_WRITE(VDELAY_LO_EVEN_REG,0x0,0x00000016);
    PCI_WRITE(VDELAY_LO_ODD_REG,0x0,0x00000016);
    PCI_WRITE(HDELAY_LO_EVEN_REG,0x0,0x00000078);
    PCI_WRITE(HDELAY_LO_ODD_REG,0x0,0x00000078);

    /* Set HSCALE for 640 pixels per line */
    PCI_WRITE(HSCALE_EVEN_MSB_REG,0x0,0x00000002);
    PCI_WRITE(HSCALE_ODD_MSB_REG,0x0,0x00000002);
    PCI_WRITE(HSCALE_EVEN_LSB_REG,0x0,0x000000AA);
    PCI_WRITE(HSCALE_ODD_LSB_REG,0x0,0x000000AA);

    /* Set VSCALE for 480 lines per frame */
    PCI_WRITE(VSCALE_EVEN_MSB_REG,0x0,0x00000060);
    PCI_WRITE(VSCALE_ODD_MSB_REG,0x0,0x00000060);
    PCI_WRITE(VSCALE_EVEN_LSB_REG,0x0,0x00000000);
    PCI_WRITE(VSCALE_ODD_LSB_REG,0x0,0x00000000);


    /* Set color format for RGB32 on ODD and EVEN */
    PCI_WRITE(COLOR_FORMAT_REG,0x0,0x00000000);
  }

  /* Enable the DMA RISC instruction IRQ */ 
  PCI_WRITE(INT_ENABLE_REG,0x0,(int_errors_to_check|RISCI_INT|VPRES_INT));

  /* Reduce frame rate from max of 30 frames/sec or 60 fields/sec */
  PCI_WRITE(TEMP_DECIMATION_REG,0x0,0x00000000);  /* reset */
  PCI_WRITE(TEMP_DECIMATION_REG,0x0,0x00000000);  /* set */

}


int decimate_frames(int count)
{
  unsigned int fcnt = count;

  if(count > 60 || count < 0)
    return -1;

  else
  {
    PCI_WRITE(TEMP_DECIMATION_REG,0x0,0x00000000);  /* reset */
    PCI_WRITE(TEMP_DECIMATION_REG,0x0,fcnt);  /* set */

    return count;
  }

}


void disable_capture(void)
{
  PCI_WRITE(CAPTURE_CTL_REG,0x0,0x00000000);
}


void enable_capture(void)
{
  vdfc_capture();
}


void set_brightness(int b)
{
  int bright;
  unsigned int hb;

  PCI_READ(BRIGHTNESS_REG,0x0,&hb);

  if(hb < 0x7f)
    bright = hb + 0x80;
  else
    bright = hb - 0x7f;
  if(b > 255)
    b = 255;
  else if(b < 0)
    b = 0;
  bright = b + 0x80;
  if(bright > 0xFF)
    bright = bright - 0xFF;
  hb = bright;
  PCI_WRITE(BRIGHTNESS_REG,0x0,hb);
}


void set_contrast(int c)
{
  int contrast;
  unsigned int hc;
  PCI_READ(CONTRAST_REG,0x0,&hc);
  if(hc < 0x7f)
    contrast = hc + 0x80;
  else
    contrast = hc - 0x7f;
  if(c > 255)
    c = 255;
  else if(c < 0)
    c = 0;
  contrast = c + 0x80;
  if(contrast > 0xFF)
    contrast = contrast - 0xFF;
  hc = contrast;
  PCI_WRITE(CONTRAST_REG,0x0,hc);

}


void vdfc_capture(void)
{
  PCI_WRITE(CAPTURE_CTL_REG,0x0,0x00000013);
}


void vbi_capture(void)
{
  PCI_WRITE(CAPTURE_CTL_REG,0x0,0x0000001C);
  PCI_WRITE(VBI_PACKET_SIZE_LO_REG,0x0,0x0000000B);
  PCI_WRITE(VBI_PACKET_SIZE_HI_REG,0x0,0x00000000);

}


void load_test_mc(int mc)
{
  if(mc == TEST_MICROCODE)
  {
    initialize_test_mc();
    PCI_WRITE(DMA_RISC_START_ADDR_REG,0x0,(unsigned int)&(dma_test_microcode[0]));
  }
}


void load_frame_mc(int fsize)
{
  initialize_frame_mc(fsize);

  if((fsize==NTSC_320_X_240) || (fsize==NTSC_320_X_240_GS))
    PCI_WRITE(DMA_RISC_START_ADDR_REG,0x0,(unsigned int)&(dma_microcode[0]));
  else if(fsize==NTSC_80_X_60)
    PCI_WRITE(DMA_RISC_START_ADDR_REG,0x0,(unsigned int)&(dma_puny_microcode[0]));
  else if(fsize==NTSC_640_X_480)
    PCI_WRITE(DMA_RISC_START_ADDR_REG,0x0,(unsigned int)&(dma_large_microcode[0]));
}


void sw_reset(void)
{
  PCI_WRITE(RESET_REG,0x0,0x00000001);
}


void toggle_fifo(int t)
{
  unsigned int testval;

  PCI_READ(DMA_CTL_REG,0x0,&testval);

  if(t)
  {
    /* Enable FIFO */
    testval = testval | 0x00000001;
  }
  else
  {
   
    testval = testval & 0xFFFFFFFE;
  }

  PCI_WRITE(DMA_CTL_REG,0x0,testval);
}


void toggle_dma(int t)
{
  unsigned int testval;

  PCI_READ(DMA_CTL_REG,0x0,&testval);

  if(t)
  {
    
    testval = testval | 0x00000002;
  }
  else
  {
   
    testval = testval & 0xFFFFFFFD;
  }

  PCI_WRITE(DMA_CTL_REG,0x0,testval);

}


void start_acq(void)
{
  PCI_WRITE(DMA_CTL_REG,0x0,0x0000000F);
}


void halt_acq(void)
{
  PCI_WRITE(DMA_CTL_REG,0x0,0x00000000);
}



void pci_inta_isr(int param)
{


  total_ints++;

  PCI_READ(INT_STATUS_REG,0x0,&last_isr_status);

  PCI_WRITE(INT_STATUS_REG,0x0,last_isr_status);


  if(last_isr_status & int_errors_to_check)
  {

    if(last_isr_status & 0x08000000)
      total_dma_disabled_errs++;

    halt_acq();

    start_acq();


    if(last_isr_status & SCERR_INT)
    {
      total_sync_errs++;
      halt_acq();
      start_acq();
    }

    if(last_isr_status & OCERR_INT)
    {
      total_dma_errs++;
      halt_acq();
      start_acq();
    }

    if(last_isr_status & PABORT_INT)
      total_abort_errs++;

    if(last_isr_status & RIPERR_INT)
      total_parity_errs++;

    if(last_isr_status & PPERR_INT)
      total_bus_parity_errs++;

    if(last_isr_status & FDSR_INT)
      total_fifo_resync_errs++;

    if(last_isr_status & FTRGT_INT)
      total_fifo_overrun_errs++;

    if(last_isr_status & FBUS_INT)
      total_bus_latency_errs++;

  }

  
  else if(last_isr_status & RISCI_INT)
  {
    total_risc_ints++;
    if(last_isr_status & (DMA_MC_WRITE<<12))
      total_write_tags++;
    if(last_isr_status & (DMA_MC_SKIP<<12))
      total_skip_tags++;
    if(last_isr_status & (DMA_MC_JUMP<<12))
      total_jump_tags++;
    if(last_isr_status & (DMA_MC_SYNC<<12))
      total_sync_tags++;
    frame_acq_cnt++;
    current_frame = !current_frame;
    semGive(frameRdy);
    PCI_READ(BTVID_MMIO_ADDR,0x0,&last_dstatus);
  }


}


void full_reset(void)
{

  halt_acq();
  disable_capture();
  taskDelete(btvid_tid);
  frame_acq_cnt = 0;
  frame_rdy_cnt = 0;
  last_dstatus = 0x0;
  last_isr_status = 0x0;
  total_dma_disabled_errs = 0x0;
  total_sync_errs = 0x0;
  total_abort_errs = 0x0;
  total_dma_errs = 0x0;
  total_parity_errs = 0x0;
  total_bus_parity_errs = 0x0;
  total_fifo_overrun_errs = 0x0;
  total_fifo_resync_errs = 0x0;
  total_bus_latency_errs = 0x0;
  total_risc_ints = 0x0;
  total_write_tags = 0x0;
  total_skip_tags = 0x0;
  total_jump_tags = 0x0;
  total_sync_tags = 0x0;
  semDelete(frameRdy);
  sw_reset();
  intel_pci_clear_status();
  reset_status();

}



void activate(int fsize)
{
  test_status();
  connect_pci_int(BT878INT);
  sysIntEnablePIC(BT878INT);
  configure_ntsc(fsize);
  set_mux(3);
  load_frame_mc(fsize);
  clear_buffers(fsize);
  enable_capture();
  start_acq();
  test_status();
}


void activate_test(int mc)
{
  test_status();

  connect_pci_int(BT878INT);
  sysIntEnablePIC(BT878INT);

  configure_ntsc(1);

  set_mux(1);

  load_test_mc(mc);
  clear_buffers(2);
  enable_capture();
  start_acq();


  test_status();
}


void connect_pci_int(int inum)
{

  pciIntConnect((INUM_TO_IVEC (inum+INT_NUM_IRQ0)), (VOIDFUNCPTR)pci_inta_isr, 0);

}

void btvid_drvr(void)
{ 
  while(1)
  {
    semTake(frameRdy, WAIT_FOREVER);  
    frame_rdy_cnt++;
    if(acq_type == NTSC_320_X_240)
      bcopy(&(frame_buffer[current_frame][0]), save_buffer, (76800*4));
    if(acq_type == NTSC_320_X_240_GS)
      bcopy(&(y8_frame_buffer[current_frame][0]), y8_save_buffer, (19200*4));
	if(pglobFnNewFrameRcvd)
	{ 
		(*pglobFnNewFrameRcvd)(&frame_buffer[current_frame][0]);
	}
	semGive(sync4);
	semGive(sync3);
	semGive(sync1);
	semGive(sync2);
		
  }

}

void start_video_report(void (*pFnNewFrameRcvd)(unsigned char *))
{
	start_video();
	if(pFnNewFrameRcvd)	/* is a new frame receive function specified? */
	{
		pglobFnNewFrameRcvd = pFnNewFrameRcvd; /* save the value */
	}
	else
	{
		pglobFnNewFrameRcvd = 0; /* zero out the new frame received function */
	}
}
	

void start_video(void)
{

	char temp;
	int pciBusNo;
  	int pciDevNo;
  	int pciFuncNo;
  	
  	
  	logfd = open("/tgtsvr/distance.txt", O_CREAT | O_RDWR, 0777);
	pglobFnNewFrameRcvd = 0;
	logFdSet(logfd);
	initializePCI_DEV();

	frameRdy = semBCreate(SEM_Q_FIFO, SEM_EMPTY);
	sync1=semBCreate(SEM_Q_FIFO, SEM_FULL);
	sync2=semBCreate(SEM_Q_FIFO, SEM_FULL);
	sync3=semBCreate(SEM_Q_FIFO, SEM_FULL);
	sync4=semBCreate(SEM_Q_FIFO, SEM_FULL);
	intel_pci_config();

	btvid_controller_config();

	test_status();

	btvid_tid = taskSpawn("tBtvid", 9, 0, (1024*8), (FUNCPTR) btvid_drvr, 0,0,0,0,0,0,0,0,0,0);

}


int reset_status(void)
{
  unsigned int testval = 0;
  PCI_READ(INT_STATUS_REG,0x0,&testval);
  PCI_WRITE(INT_STATUS_REG,0x0,0xFFFFFFFF);
  test_status();
}


int test_status(void)
{
  unsigned int testval = 0;
  PCI_READ(INT_STATUS_REG,0x0,&testval);
  PCI_READ(CAPTURE_CNT_REG,0x0,&testval);
  PCI_READ(DMA_RISC_PC_REG,0x0,&testval);
}




void set_mux(int mux)
{
  unsigned int testval = 0x00000019; /* NTSC source */

  if(mux==0)
    testval |= 0x00000040;
  else if(mux==1)
    testval |= 0x00000060;
  else if(mux==2)
    testval |= 0x00000020;
  else if(mux==3)
    testval |= 0x00000000;
    
  /* Select NTSC source */
  PCI_WRITE(INPUT_REG,0x0,testval);

 /* printf("Setting INPUT_REG = 0x%x\n", testval);*/

}




int intel_pci_config(void)
{
	unsigned short int testval;
	unsigned int longword;
	unsigned char byte;

	int pciBusNo = PCI_DEV[NORTH_BRIDGE].busNo;
	int pciDevNo = PCI_DEV[NORTH_BRIDGE].deviceNo;
	int pciFuncNo = PCI_DEV[NORTH_BRIDGE].funcNo;
	int pciVenId = PCI_DEV[NORTH_BRIDGE].venId;
    	int pciDevId = PCI_DEV[NORTH_BRIDGE].devId;


	switch(pciDevId)
	{
		case PCI_DEVICE_ID_INTEL_820820:
			/* I'm going to assume nothing needs to be set for this northbridge until
			 * proven othewhise */
			break;
			
		case PCI_DEVICE_ID_INTEL_82441FX:
			
  			/* Concurrency enable setting does not exist in 82441FX */
			
			/* Set latency timer value to 0.. no guaranteed time slice */
			pciConfigInByte(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_LATENCY_TIMER, &byte);
			/*printf("Intel NB controller PCI latency timer = 0x%x\n", byte);*/

			pciConfigOutByte(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_LATENCY_TIMER, 0x00);
			/*printf("Modified Intel NB controller PCI latency timer = 0x%x\n", 0x00);*/

 			/* MAE Bit in PCICFG register - see P22.. this bit hardwired 1 in 82441FX */
			
			/* XPLDE.. this bit does not exist in 82441 */
			
			break;
		
		case 	PCI_DEVICE_ID_INTEL_82439TX:
			

			pciConfigInByte(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_PCI_CTL, &byte);
  			/*printf("Intel NB controller PCI concurrency enable = 0x%x\n", byte);*/

			pciConfigOutByte(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_PCI_CTL, 0x00);
			/*printf("Modified Intel NB controller PCI concurrency enable = 0x%x\n", byte);*/

			pciConfigInByte(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_LATENCY_TIMER, &byte);
			/*printf("Intel NB controller PCI latency timer = 0x%x\n", byte);*/

			pciConfigOutByte(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_LATENCY_TIMER, 0x00);
			/*printf("Modified Intel NB controller PCI latency timer = 0x%x\n", byte);*/

			/* Enable the North Bridge memory controller to allow memory access
			by another bus master.

			MAE=1
	
			*/

			pciConfigInWord(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_COMMAND, &testval);
			/*printf("Intel NB controller PCI Cmd Reg = 0x%x\n", testval);*/

			pciConfigOutWord(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_COMMAND, 0x0002);

			pciConfigInWord(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_COMMAND, &testval);
			/*printf("Modified Intel NB controller PCI Cmd Reg = 0x%x\n", testval);*/
	
			/* See P26.. Set Extended CPU-toPIIX4 PHLDA# Signalling enable */	
			
			pciConfigInByte(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_ARB_CTL, &byte);
			/*printf("Intel NB controller PCI ARB CTL = 0x%x\n", byte);*/
			pciConfigOutByte(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_ARB_CTL, 0x80);
			/*printf("PCI 2.1 Compliant Intel NB controller PCI ARB CTL = 0x%x\n", byte);*/		
			break;

		default:
			/*printf("!!!Error!!! Northbridge undefined!!!!\n");*/
			break;
	}


	pciBusNo = PCI_DEV[SOUTH_BRIDGE].busNo;
	pciDevNo = PCI_DEV[SOUTH_BRIDGE].deviceNo;
	pciFuncNo = PCI_DEV[SOUTH_BRIDGE].funcNo;
	pciVenId = PCI_DEV[SOUTH_BRIDGE].venId;
	pciDevId = PCI_DEV[SOUTH_BRIDGE].devId;

	switch(pciDevId)
	{
		case PCI_DEVICE_ID_INTEL_82801AA:

			/*printf("Detected Southbridge 82801AA\n");*/
			

/*				
			pciConfigInLong(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_IRQ_ROUTING, &longword);
			printf("Intel SB controller IRQ Routing Reg = 0x%x\n", longword);

			longword = (((longword & 0x70FFFFFF) | ((BT878INT)<<24)) ); // NOTE: IRQEN SET TO ZERO TO ROUTE TO ISA INT
			pciConfigOutLong(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_IRQ_ROUTING, longword);
			pciConfigInLong(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_IRQ_ROUTING, &longword);
			printf("Modified Intel SB controller IRQ Routing Reg = 0x%x\n", longword);
*/
			pciConfigInLong(pciBusNo, pciDevNo, pciFuncNo,PCI_CFG_GEN_CNTL, &longword);

			if(longword & PCI_CFG_GEN_CNTL_APIC_EN_BIT)
			{
				/*printf("APIC_EN bit already set\n");*/
			}
			else
			{
				/*printf("APIC_EN bit not set\n");*/
				longword |= PCI_CFG_GEN_CNTL_APIC_EN_BIT;
				pciConfigOutLong(pciBusNo, pciDevNo, pciFuncNo,PCI_CFG_GEN_CNTL, longword);
				pciConfigInLong(pciBusNo, pciDevNo, pciFuncNo,PCI_CFG_GEN_CNTL, &longword);
				if(longword & PCI_CFG_GEN_CNTL_APIC_EN_BIT)
				{
					/*printf("APIC_EN bit set successfully\n");*/
				}
				else
				{
					/*printf("ERROR!!! Could not set APIC_EN!!!\n");*/		
				}

				
			}

/*			pciConfigInByte(pciBusNo,pciD */
	
			
			break;
				
		case PCI_DEVICE_ID_INTEL_82371SB_0:
			/* Deterministic Latency Conrol Register, DLC (See p 42) */
			/* Mask in 0x03 to enable Passive Release and Delayed Transaction.
			 * This will make the sb function the same way the 82371FB responded
			 * * Most likely not necessary, according to Sam 12-1-04  */
			/*printf("Detected Southbridge 82371SB\n");*/
			pciConfigInByte(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_LATENCY_CTL, &byte);
			/*printf("Intel SB controller latency control  = 0x%x\n", byte);*/
			byte |= 0x03;
			pciConfigOutByte(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_LATENCY_CTL, byte);
			/*printf("PCI 2.1 Compliant Intel SB controller latency control  = 0x%x\n", byte);*/
			pciConfigInByte(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_LATENCY_CTL, &byte);
			/*printf("Intel SB controller latency control  = 0x%x\n\n", byte);*/

			/* IRQ Routing Setup */

			
			/* Note: Zach had commented PIRQRC settig out, and I'm not sure why.  It appears that it 
			* should be included - DW 12/1/04*/
			
			/* Set PIRQRC register to map INTA to The IRQ specified by BT878INT and enable INTA
			 * See P36 */
			pciConfigInLong(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_IRQ_ROUTING, &longword);
			/*printf("Intel SB controller IRQ Routing Reg = 0x%x\n", longword);*/

			longword = (((longword & 0x70FFFFFF) | ((BT878INT)<<24)) | 0x80000000);
			pciConfigOutLong(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_IRQ_ROUTING, longword);
			pciConfigInLong(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_IRQ_ROUTING, &longword);
			/*printf("Modified Intel SB controller IRQ Routing Reg = 0x%x\n", longword);*/
			
			break;
		
		case PCI_DEVICE_ID_INTEL_82371AB_0:

			/* Deterministic Latency Conrol Register, DLC (See p 42) */
			/* Mask in 0x03 to enable Passive Release and Delayed Transaction.
			 * This will make the sb function the same way the 82371FB responded
			 * Most likely not necessary, according to Sam 12-1-04 */
			/*printf("Detected Southbridge 82371AB\n");*/
			pciConfigInByte(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_LATENCY_CTL, &byte);
			/*printf("Intel SB controller latency control  = 0x%x\n", byte);*/
			byte |= 0x03;
			pciConfigOutByte(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_LATENCY_CTL, byte);
			/*printf("PCI 2.1 Compliant Intel SB controller latency control  = 0x%x\n", byte);*/

			/* Set PIRQRC register to map INTA to The IRQ specified by BT878INT and enable INTA
			 * See P36 */

			pciConfigInLong(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_IRQ_ROUTING, &longword);
			/*printf("Intel SB controller IRQ Routing Reg = 0x%x\n", longword);*/

			longword = (0x00808080 | ((BT878INT)<<24));

			pciConfigOutLong(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_IRQ_ROUTING, longword);
  
			pciConfigInLong(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_IRQ_ROUTING, &longword);
			/*printf("Modified Intel SB controller IRQ Routing Reg = 0x%x\n", longword);*/

			pciConfigInByte(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_APIC_ADDR, &byte);
			/*printf("Intel SB controller APIC Addr Reg = 0x%x\n", byte);*/	
			
			break;

		default:
			/*printf("!!!Error!!!Southbridge Not defined!!!!");*/
			break;

	}

}


void intel_pci_clear_status(void)
{
  int pciBusNo = PCI_DEV[NORTH_BRIDGE].busNo;
  int pciDevNo = PCI_DEV[NORTH_BRIDGE].deviceNo;
  int pciFuncNo = PCI_DEV[NORTH_BRIDGE].funcNo;

  pciConfigOutWord(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_STATUS, 0x3000);

}


int btvid_controller_config(void)
{
  int pciBusNo = PCI_DEV[CAPTURE_VIDEO_CARD1].busNo;
  int pciDevNo = PCI_DEV[CAPTURE_VIDEO_CARD1].deviceNo;
  int pciFuncNo = PCI_DEV[CAPTURE_VIDEO_CARD1].funcNo;

  int ix;
  unsigned int testval;
  unsigned short command = (BUS_MASTER_MMIO);
  unsigned char irq;
  unsigned char byte;

  /* Disable btvid */
  pciConfigOutWord (pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_COMMAND, 0);


  for (ix = PCI_CFG_BASE_ADDRESS_0; ix <= PCI_CFG_BASE_ADDRESS_5; ix+=4)
  {
      pciConfigInLong(pciBusNo, pciDevNo, pciFuncNo, ix, &testval);
      
      /*printf("BAR %d testval=0x%x before writing 0xffffffff's \n", ((ix/4)-4), testval);*/

      /* Write all f's and read back value */

      pciConfigOutLong(pciBusNo, pciDevNo, pciFuncNo, ix, 0xffffffff);
      pciConfigInLong(pciBusNo, pciDevNo, pciFuncNo, ix, &testval);
      
      	if(!BAR_IS_IMPLEMENTED(testval)) {
        	/*printf("BAR %d not implemented\n", ((ix/4)-4));*/
	} else {
		if(BAR_IS_MEM_ADDRESS_DECODER(testval)) {
			/*printf("BAR %d is a Memory Address Decoder\n", ((ix/4)-4));
			printf("Configuring BAR %d for address 0x%x\n", ix, BTVID_MMIO_ADDR);*/
        		pciConfigOutLong(pciBusNo, pciDevNo, pciFuncNo, ix, BTVID_MMIO_ADDR);
			/*printf("BAR configured \n");*/
			
			if(BAR_IS_32_BIT_DECODER(testval)){
				/*printf("BAR %d is a 32-Bit Decoder \n", ((ix/4)-4));*/
			}
			else if(BAR_IS_64_BIT_DECODER(testval)){	
				/*printf("BAR %d is a 64-Bit Decoder \n", ((ix/4)-4));*/
			}
			else{
				/*printf("BAR %d memory width is undefined \n", ((ix/4)-4));*/
			}
			if(BAR_IS_PREFETCHABLE(testval)){
				/*printf("BAR %d address space is prefetachable \n", ((ix/4)-4));*/
			}
		} 
		else if(BAR_IS_IO_ADDRESS_DECODER(testval)) {
			/*printf("BAR %d is an IO Address Decoder \n", ((ix/4)-4));*/
		}
		else {
			/*printf("BAR %d is niether an IO Address Decoder or an Memory Address Decoder (error probably) \n", ((ix/4)-4));*/	
		}
		baseAddressRegisterSizeOfDecodeSpace(testval);
	/*printf("BAR %d decodes a space 2^%i big\n", ((ix/4)-4), 
		baseAddressRegisterSizeOfDecodeSpace(testval));*/
	}

	printf("\n\n");
  }

  /* Set the INTA vector */
  pciConfigInByte(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_DEV_INT_LINE, &irq);
  /*printf("Found Bt878 configured for IRQ line: %d\n", irq);*/
  irq = BT878INT;
  pciConfigOutByte(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_DEV_INT_LINE, irq);
  pciConfigInByte(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_DEV_INT_LINE, &irq);
  /*printf("Updated Bt878 IRQ line to: 0x%x\n", irq);*/

  /* Configure Cache Line Size Register -- Write-Command 

     Do not use cache line.

   */
  pciConfigOutByte(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_CACHE_LINE_SIZE, 0x0);

  /* Configure Latency Timer */
  pciConfigInByte(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_LATENCY_TIMER, &byte);
  /*printf("Bt878 Allowable PCI bus latency = 0x%x\n", byte);*/
  pciConfigInByte(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_MIN_GRANT, &byte);
 /* printf("Bt878 PCI bus min grant = 0x%x\n", byte);*/
  pciConfigInByte(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_MAX_LATENCY, &byte);
  /*printf("Bt878 PCI bus max latency = 0x%x\n", byte);*/

  pciConfigOutByte(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_LATENCY_TIMER, 0xFF);

  pciConfigInByte(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_LATENCY_TIMER, &byte);
  /*printf("Modified Bt878 Allowable PCI bus latency = 0x%x\n", byte);*/


  /* Enable the device's capabilities as specified */
  pciConfigOutWord(pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_COMMAND, (unsigned short) command);
  
}


int baseAddressRegisterSizeOfDecodeSpace(int returnval) {

	int tmp = 0;
	int bitpos = 0;
	int i = 0;
	
	tmp = returnval;
       	tmp &= 0xFFFFFFF0;
	
	for(i = 0; i < 32; i++) {
		if(tmp & 0x1) {
			return bitpos;
		} else {
		
			bitpos++;
			tmp >>= 1;	
		}
	}
		
}

int GetFrameAcqCount(void)
{
	return frame_acq_cnt;
}






/* ------------------------------------------------------------------------------------------------ 
 * * ----------------------------------------------------------------------------------------------
 *              
 *                                           PART 2
 *                                                               
 * ------------------------------------------------------------------------------------------------
 * * ------------------------------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------------------------------ 
 * * ----------------------------------------------------------------------------------------------
 *              
 *                Image Processing and  position value computation segment
 *                                                               
 * ------------------------------------------------------------------------------------------------
 * * ------------------------------------------------------------------------------------------------ */ 


void ImgProc()       /* Image Processing task body*/
{
  
  while(1)
  {
  semTake(sync1,WAIT_FOREVER);
  int fd, i;
  int bo=0;
  char *savebuffbyteptr = (char *)&(save_buffer[0]);
  char *pWriteBuf;
 
  wvEvent(1,0,0);       
  
  pWriteBuf = SaveBuffer;
  if(bo == 0)
  {
    for(i=0;i<76800;i++)
    {
      *pWriteBuf =savebuffbyteptr[2];
      pWriteBuf++;
      *pWriteBuf =savebuffbyteptr[1];
      pWriteBuf++;
      *pWriteBuf =savebuffbyteptr[0];
      pWriteBuf++;

      savebuffbyteptr+=4;
    }
    
   centroid();                             /*call function to compute the centroid of the image*/
   takeInput(0,0,3,4,1,2,xdef,ydef,zdef);  /*call function to save the arm position values in a global variable to send to the robotic arm */
   frame_to_net(SaveBuffer);               /*call function to send the image data to vPipe*/
   
 
  }
  }
}

int select_color=2;
int ts=25;

void cc(int cc, int cc1)                  /*call function on the console to select color specifying the threshold value*/
{
	select_color=cc;
	ts=cc1;
}

void maintain(int val)                   /*call function on the console to maintain an approximate distance from the object */
{
	dis = val;
	
}

void centroid()	               /*core image processing component that computes centroid and translates it to arm position values*/
{
	int xCent=160, yCent=120, xCent1,yCent1;
	int i, j, inTarget=0;
	unsigned int xCnt=0, xCntPrev=0,  xEdgeL, xEdgeR;  
	unsigned int yCnt=0, yCntPrev=0,  yEdgeL, yEdgeR;
	unsigned int  xCntSum, xCntMax;
	
	int k =0;
	
	float x1;
	   	    
	  
	    for(i = 0; i< 76800; i++)
	    {
	    	
	    	R[i] = SaveBuffer[0+k];
	    	G[i] = SaveBuffer[1+k];		
	    	B[i] = SaveBuffer[2+k];
	    	k = k+3;
	    	 	
	    }
	   
	    
	    /* perform the grayscale-colorslice subtraction of the color image in the buffer */
	    for(i=0; i<240; i++)
	    	    {
	    	    	for(j=0; j<320; j++)
	    	    	{
	    	    		if(select_color == 1)
	    	    	    {
	    	    			if(R[(i*320)+j]>30+(0.2989*R[(i*320)+j]+0.5870*G[(i*320)+j]+0.1140*B[(i*320)+j]))
	    	    			X[(i*320)+j]=(R[(i*320)+j]-(0.2989*R[(i*320)+j]+0.5870*G[(i*320)+j]+0.1140*B[(i*320)+j]));
	    	    			else
	    	    			X[(i*320)+j]=0;
	    	    			
	    	    			x1+=X[(i*320)+j];
	    	    			x1=ts+(x1/(240*320));
	    	    		        avgX[(i*320)+j]=X[(i*320)+j];
	    	    		}
	    	    			            
	    	    		else if(select_color == 2)
	    	    		{
	    	    			if(G[(i*320)+j]>30+(0.2989*R[(i*320)+j]+0.5870*G[(i*320)+j]+0.1140*B[(i*320)+j]))
	    	    			X[(i*320)+j]=(G[(i*320)+j]-(0.2989*R[(i*320)+j]+0.5870*G[(i*320)+j]+0.1140*B[(i*320)+j]));
	    	    			else
	    	    			X[(i*320)+j]=0;
	    	    				    	    			
	    	    			x1+=X[(i*320)+j];
	    	    			x1=ts+(x1/(240*320));
	    	    		        avgX[(i*320)+j]=X[(i*320)+j];
	    	    			
	    	    		}
	    	    			            
	    	    		else
	    	    		{ 
	    	    			if(B[(i*320)+j]>30+(0.2989*R[(i*320)+j]+0.5870*G[(i*320)+j]+0.1140*B[(i*320)+j]))
	    	    			X[(i*320)+j]=(B[(i*320)+j]-(0.2989*R[(i*320)+j]+0.5870*G[(i*320)+j]+0.1140*B[(i*320)+j]));
	    	    			else
	    	    			X[(i*320)+j]=0;
	    	    				    	    			
	    	    			x1+=X[(i*320)+j];
	    	    			x1=ts+(x1/(240*320));
	    	    		        avgX[(i*320)+j]=X[(i*320)+j];
	    	    			
	    	    		}		    		    	
	    	    	}
	    	    }
	    	    
	    
	    /*Average filtering*/
	    for(i=0; i<240; i++)
	        {
	        for(j=0; j<315; j++)
	           {
	                  
	            avgX[(i*320)+j]=(X[(i*320)+j] + X[(i*320)+j+1] + X[(i*320)+j+2] + X[(i*320)+j+3] + X[(i*320)+j+4] + X[(i*320)+j+5]) / 6;
	          
	           }
	        }
	    
	   /*computing the centroid along the x-axis (xCent value)*/
          for(i=0; i<240; i++)
	    	    {

	    	        for(j=0; j<315; j++)
	    	        {
	    	       	       
	    	            if(!inTarget && (avgX[(i*320)+j]> x1 ))
	    	            {
	    	                inTarget=1;
	    	                xCnt=0;
	    	                xEdgeL=(unsigned int)j;
	    	               
	    	            } 
	    	            else if(inTarget && (avgX[(i*320)+j] < x1))
	    	            {
	    	                inTarget=0;
	    	                xEdgeR=(unsigned int)j;
     
	    	                if (xCnt > xCntPrev)
	    	                {
	    	                	xCent = xEdgeL + ((xEdgeR-xEdgeL)/2);
	    	                }
	    	                xCntPrev=xCnt;
	    	            }
	    	            else if(inTarget)
	    	            {
	    	                xCnt++;
	    	            }
	    	        }
	    	    }	    	
          
          /*computing the centroid along the y-axis (yCent value)*/
	  
	    	    for(j=0; j<320; j++)
	    	    {
	    	    	for(i=0; i<240; i++)
	    	   	    {
	    	   	    	            if(!inTarget && (avgX[(i*320)+j]> x1 ))
	    	   	    	            {
	    	   	    	                inTarget=1;
	    	   	    	                yCnt=0;
	    	   	    	                yEdgeL=(unsigned int)i;
	    	   	    	               
	    	   	    	            } 
	    	   	    	            else if(inTarget && (avgX[(i*320)+j] < x1))
	    	   	    	            {
	    	   	    	                inTarget=0;
	    	   	    	                yEdgeR=(unsigned int)i;

	    	   	    	                if (yCnt > yCntPrev)
	    	   	    	                {
	    	   	    	                	yCent = yEdgeL + ((yEdgeR-yEdgeL)/2);
	    	   	    	                }
	    	   	    	                yCntPrev=yCnt;
	    	   	    	            }
	    	   	    	            else if(inTarget)
	    	   	    	            {
	    	   	    	                yCnt++;
	    	   	    	            }
	    	   	    	        }
	    	   	    	    }
	    	    	    
	   /*Generation of crosshairs for the vPipe video display*/	    	    	    
	    for(i=0; i<76800; i++)
	        {
	            if((i % 320) == xCent)
	            {
	            	SaveBuffer[0+i*3]=maxR;
	            	SaveBuffer[1+i*3]=maxR;
	            	SaveBuffer[2+i*3]=maxR;
	            }
	            
	            else if((i/320) == yCent)
	             {
	            	SaveBuffer[0+i*3]=maxR;
	            	SaveBuffer[1+i*3]=maxR;
	            	SaveBuffer[2+i*3]=maxR;
	            	
	             }
	        }
	    
    /* Changing the origin of the image axis to (160,120), the center of frame*/     	    
	    xCent1=(xCent-160);
	    yCent1=(yCent-120);
	   
   /* Code section to avoid the issue of overshoot while the object is moved too fast*/	    
	    if (xCent1>100)
              xCent1=100;
	    
	    if (yCent1>80)
	     yCent1=80;
	    	        
   /* Translation to robotic arm position values from pixel distance of centroid from the origin */    	        
	    xval=0.1*xCent1;
	    yval=0.1*yCent1;
	    
	    
	   
	    xdef=(xdef+xval);
	    ydef=(ydef-yval);
	    
	    
	    
    /* Code section to move the arm forward or backwards based on the distance value */	    
	    dis=1000;
	    if(millis < dis)
	      {
	      if (zdef>512)
	  	zdef--;
	      }
	  	if(millis > dis)
	  	zdef++;
	    
    /* Providing non-jamming terminal values for the robotic arm positions */	  	
	    if (xdef>1023)
	    	xdef=1023;
	    if (ydef>900)
	    	ydef=900;
	    
	    if (xdef<0)
	    	xdef=0;
	    if (ydef<100)
	    	ydef=100;
	    
	    if (zdef>700)
	        zdef=700;
	       
}
