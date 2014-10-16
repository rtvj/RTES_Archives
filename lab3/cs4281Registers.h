/****************************************************************************
 *
 *  CS4281/AC97 Hardware Register Definitions
 *
 ***************************************************************************/

/****************************************************************************
/
/ The following define the offsets of the registers accessed via base address
/ register zero on the CS4281 part.
/
****************************************************************************/
#define BA0_HISR                                0x00000000L
#define BA0_HICR                                0x00000008L
#define BA0_HIMR                                0x0000000CL
#define BA0_IIER                                0x00000010L
#define BA0_HDSR0                               0x000000F0L
#define BA0_HDSR1                               0x000000F4L
#define BA0_HDSR2                               0x000000F8L
#define BA0_HDSR3                               0x000000FCL

#define BA0_DCA0                                0x00000110L
#define BA0_DCC0                                0x00000114L
#define BA0_DBA0                                0x00000118L
#define BA0_DBC0                                0x0000011CL
#define BA0_DCA1                                0x00000120L
#define BA0_DCC1                                0x00000124L
#define BA0_DBA1                                0x00000128L
#define BA0_DBC1                                0x0000012CL

#define BA0_DMR0                                0x00000150L
#define BA0_DCR0                                0x00000154L
#define BA0_DMR1                                0x00000158L
#define BA0_DCR1                                0x0000015CL

#define BA0_DLMR                                0x00000170L
#define BA0_DLSR                                0x00000174L

#define BA0_FCR0                                0x00000180L
#define BA0_FCR1                                0x00000184L

#define BA0_FCHS                                0x0000020CL

#define BA0_PMCS                                0x00000344L
#define BA0_CWPR                                0x000003E0L
#define BA0_EPPMC                               0x000003E4L
#define BA0_GPIOR                               0x000003E8L
#define BA0_SPMC                                0x000003ECL
#define BA0_CFLR                                0x000003F0L
#define BA0_IISR                                0x000003F4L
#define BA0_TMS                                 0x000003F8L
#define BA0_SSVID                               0x000003FCL
#define BA0_CLKCR1                              0x00000400L
#define BA0_FRR                                 0x00000410L
#define BA0_SLT12O                              0x0000041CL
#define BA0_SERMC                               0x00000420L
#define BA0_SERC1                               0x00000428L
#define BA0_SERC2                               0x0000042CL
#define BA0_SLT12M                              0x0000045CL
#define BA0_ACCTL                               0x00000460L
#define BA0_ACSTS                               0x00000464L
#define BA0_ACOSV                               0x00000468L
#define BA0_ACCAD                               0x0000046CL
#define BA0_ACCDA                               0x00000470L
#define BA0_ACISV                               0x00000474L
#define BA0_ACSAD                               0x00000478L
#define BA0_ACSDA                               0x0000047CL



#define BA0_FMSR                                0x00000730L
#define BA0_B0AP                                0x00000730L
#define BA0_FMDP                                0x00000734L
#define BA0_B1AP                                0x00000738L
#define BA0_B1DP                                0x0000073CL
#define BA0_SSPM                                0x00000740L
#define BA0_DACSR                               0x00000744L
#define BA0_ADCSR                               0x00000748L
#define BA0_SSCR                                0x0000074CL
#define BA0_FMLVC                               0x00000754L
#define BA0_FMRVC                               0x00000758L
#define BA0_SRCSA                               0x0000075CL
#define BA0_PPLVC                               0x00000760L
#define BA0_PPRVC                               0x00000764L
#define BA0_PASR                                0x00000768L
#define BA0_CASR                                0x0000076CL

/****************************************************************************
/
/ The following define the offsets of the AC97 shadow registers, which appear
/ as a virtual extension to the base address register zero memory range.
/
****************************************************************************/
#define AC97_REG_OFFSET_MASK                    0x0000007EL
#define AC97_CODEC_NUMBER_MASK                  0x00003000L

#define BA0_AC97_RESET                          0x00001000L
#define BA0_AC97_MASTER_VOLUME                  0x00001002L
#define BA0_AC97_HEADPHONE_VOLUME               0x00001004L
#define BA0_AC97_MASTER_VOLUME_MONO             0x00001006L
#define BA0_AC97_MASTER_TONE                    0x00001008L
#define BA0_AC97_PC_BEEP_VOLUME                 0x0000100AL
#define BA0_AC97_PHONE_VOLUME                   0x0000100CL
#define BA0_AC97_MIC_VOLUME                     0x0000100EL
#define BA0_AC97_LINE_IN_VOLUME                 0x00001010L
#define BA0_AC97_CD_VOLUME                      0x00001012L
#define BA0_AC97_VIDEO_VOLUME                   0x00001014L
#define BA0_AC97_AUX_VOLUME                     0x00001016L
#define BA0_AC97_PCM_OUT_VOLUME                 0x00001018L
#define BA0_AC97_RECORD_SELECT                  0x0000101AL
#define BA0_AC97_RECORD_GAIN                    0x0000101CL
#define BA0_AC97_RECORD_GAIN_MIC                0x0000101EL
#define BA0_AC97_GENERAL_PURPOSE                0x00001020L
#define BA0_AC97_3D_CONTROL                     0x00001022L
#define BA0_AC97_MODEM_RATE                     0x00001024L
#define BA0_AC97_POWERDOWN                      0x00001026L


/****************************************************************************
/
/ The following defines are for the flags in the host interrupt status
/ register.
/
****************************************************************************/

#define HISR_DMA0                                0x00000100L
#define HISR_DMA1                                0x00000200L

/****************************************************************************
/
/ The following defines are for the flags in the host interrupt control
/ register.
/
****************************************************************************/
#define HICR_IEV                                 0x00000001L
#define HICR_CHGM                                0x00000002L

/****************************************************************************
/
/ The following defines are for the flags in the DMA Mode Register n
/ (DMRn)
/
****************************************************************************/
#define DMRn_MONO                                0x00020000L
#define DMRn_AUTO                                0x00000010L
#define DMRn_DMA                                 0x20000000L
#define DMRn_TR_READ                             0x00000008L
#define DMRn_TR_WRITE                            0x00000004L
#define DMRn_SIZE8                               0x00010000L
#define DMRn_USIGN                               0x00080000L

/****************************************************************************
/
/ The following defines are for the flags in the DMA Command Register n
/ (DCRn)
/
****************************************************************************/
#define DCRn_HTCIE                               0x00020000L
#define DCRn_TCIE                                0x00010000L
#define DCRn_MSK                                 0x00000001L

/****************************************************************************
/
/ The following defines are for the flags in the FIFO Control 
/ register n.(FCRn)
/
****************************************************************************/
#define FCRn_FEN                                0x80000000L
#define FCRn_PSH                                0x20000000L

/****************************************************************************
/
/ The following defines are for the flags in the serial port Power Management
/ control register.(SPMC)
/
****************************************************************************/
#define SPMC_RSTN                               0x00000001L



/****************************************************************************
/
/ The following defines are for the flags in the Clock Control Register 1. 
/ (CLKCR1)
/
****************************************************************************/
#define CLKCR1_DLLP                             0x00000010L
#define CLKCR1_SWCE                             0x00000020L
#define CLKCR1_DLLRDY                           0x01000000L

/****************************************************************************
/
/ The following defines are for the flags in the serial port master control
/ register.(SERMC)
/
****************************************************************************/
#define SERMC_PTC_AC97                          0x00000002L



/****************************************************************************
/
/ The following defines are for the flags in the clock control register 1.
/
****************************************************************************/
#define CLKCR1_PLLP                             0x00000010L
#define CLKCR1_SWCE                             0x00000020L
#define CLKCR1_PLLOS                            0x00000040L


/****************************************************************************
/
/ The following defines are for the flags in the AC97 control register.
/
****************************************************************************/
#define ACCTL_ESYN                              0x00000002L
#define ACCTL_VFRM                              0x00000004L
#define ACCTL_DCV                               0x00000008L
#define ACCTL_CRW                               0x00000010L
#define ACCTL_TC                                0x00000040L

/****************************************************************************
/
/ The following defines are for the flags in the AC97 status register.
/
****************************************************************************/
#define ACSTS_CRDY                              0x00000001L
#define ACSTS_VSTS                              0x00000002L

/****************************************************************************
/
/ The following defines are for the flags in the AC97 output slot valid
/ register.
/
****************************************************************************/
#define ACOSV_SLV3                              0x00000001L
#define ACOSV_SLV4                              0x00000002L
#define ACOSV_SLV5                              0x00000004L
#define ACOSV_SLV6                              0x00000008L
#define ACOSV_SLV7                              0x00000010L
#define ACOSV_SLV8                              0x00000020L
#define ACOSV_SLV9                              0x00000040L
#define ACOSV_SLV10                             0x00000080L
#define ACOSV_SLV11                             0x00000100L
#define ACOSV_SLV12                             0x00000200L


/****************************************************************************
/
/ The following defines are for the flags in the AC97 input slot valid
/ register.
/
****************************************************************************/
#define ACISV_ISV3                              0x00000001L
#define ACISV_ISV4                              0x00000002L
#define ACISV_ISV5                              0x00000004L
#define ACISV_ISV6                              0x00000008L
#define ACISV_ISV7                              0x00000010L
#define ACISV_ISV8                              0x00000020L
#define ACISV_ISV9                              0x00000040L
#define ACISV_ISV10                             0x00000080L
#define ACISV_ISV11                             0x00000100L
#define ACISV_ISV12                             0x00000200L

