#include "race_ctl.h"

#define FRAME_ID 0xA
#define STATE_ID 0xD

int init_frametx(void);
int init_statetx(void);
void shutdown_frametx(void);
void shutdown_statetx(void);
int xga_frame_to_net(unsigned short *frameptr, int length);
int grayscale_frame_to_net(unsigned char *grayscale_buffer);
int state_to_net(struct race_state *stateptr);
