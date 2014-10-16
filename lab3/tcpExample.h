/* tcpExample.h- header used by both TCP server and client examples */

/* defines */
#define PLAYBACK_FILE "/tgtsvr/Audio.out"
#define SERVER_PORT_NUM		6097	/* server's port number for bind() */
#define CLIENT_PORT_NUM		6087	/* server's port number for bind() */
#define SERVER_WORK_PRIORITY 4   /* priority of server's work task */
#define CLIENT_WORK_PRIORITY 5  /* priority of server's work task */
#define SERVER_STACK_SIZE 10000 /* stack size of server's work task */
#define SERVER_MAX_CONNECTIONS 4 /* max clients connected at a time */
/*#define MAX_LINE   4096 /* max size of data per block */
#define MAX_LINE 11969
#define MAX_REC_BLOCK 10

void loadBuffer(char c);
void set_acgaa();
void get_acgaa();

typedef struct {
	unsigned char ch;
	}data;

typedef struct {
	int size;
	int start;
	int end;
	data *d;
}CircularBuffer;

CircularBuffer *cb1;
