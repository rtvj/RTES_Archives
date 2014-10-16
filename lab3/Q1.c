#include "vxWorks.h"
#include "stdio.h"
#include "stdlib.h"
#include "ioLib.h"
#include "semLib.h"
#include "intLib.h"
#include "iv.h"

/* VxWorks 5.4 PCI driver interface includes */
#include "drv/pci/pciConfigLib.h"
#include "drv/pci/pciConfigShow.h"
#include "drv/pci/pciHeaderDefs.h"
#include "drv/pci/pciLocalBus.h"
#include "drv/pci/pciIntLib.h"
/*#include "VxTypes.h"	/* This file includes typedefs required for porting linux functions
#include "VxPCI.h"
*/

/* pcPentium BSP includes */
#include "sysLib.h"

#define PCI_SYSTEM_START_MEM_ALLOC_SPACE 0x04000000
#define PCI_SYSTEM_START_IO_ALLOC_SPACE 0xE000
/*
unsigned long  	pci_memAllocRegionStart = PCI_SYSTEM_START_MEM_ALLOC_SPACE;
unsigned long 	pci_IOAllocRegionStart = PCI_SYSTEM_START_IO_ALLOC_SPACE;
*/
STATUS  pciLibInitStatus = NONE;    /* initialization done */
int     pciConfigMech =NONE;
int probe_PCI_bus(void) {
	
	
	int deviceNo;
	int devices;
	ushort_t vendorId;
	ushort_t deviceId;
	union {
		int classCode;
		unsigned char array[4];
	}u;
	int busNo = 0;
	/*struct pci_dev *pDev;*/
	if(pciLibInitStatus != OK){
		if(pciConfigLibInit(PCI_MECHANISM_1, 0xCF8, 0xCFC, 0) != OK) {
			printf("\nPCI config error\n");
			return (ERROR);
			}
	}
			devices = 0x1f;
			for(busNo = 0; busNo<10; busNo++){
			printf("\nvendorID, 	deviceID, 	Bus #, 	Device#, 	Func #, 	u.classCode\n");
			for(deviceNo = 0; deviceNo < devices; deviceNo++) {
				pciConfigInWord(busNo, deviceNo, 0, PCI_CFG_VENDOR_ID, &vendorId);
				pciConfigInWord(busNo, deviceNo, 0, PCI_CFG_DEVICE_ID, &deviceId);
				pciConfigInWord(busNo, deviceNo, 0, PCI_CFG_PROGRAMMING_IF, &u.array[3]);
				pciConfigInWord(busNo, deviceNo, 0, PCI_CFG_SUBCLASS, &u.array[2]);
				pciConfigInWord(busNo, deviceNo, 0, PCI_CFG_CLASS, &u.array[1]);
				u.array[0] = 0;
				if(vendorId != 0xFFFF) {
					printf("0x%.8x  0x%.8x  0x%.8x  0x%.8x  0x%.8x  0x%.8x\n", vendorId, deviceId,  busNo, deviceNo, 0, u.classCode);
				}
/*				
				if((pDev = vx_pci_find_pci_dev(busNo, deviceNo, 0)) != NULL) {
					printf("Device already found on this bus, device # %.8x\n", deviceNo);
				}
				else if((pDev = vx_pci_alloc_new_pci_dev(busNo)) == NULL) {
					return ERROR;
				}
				
				pDev->bus->number = (unsigned char)busNo;
				pDev->device = deviceId;
				pDev->devfn = PCI_DEFN(deviceNo, 0);
				pDev->vendor = vendorId;
			}
*/
			}
		}
		return(OK);
}
