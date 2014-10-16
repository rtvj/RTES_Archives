//##########################################################
//##                      R O B O T I S                   ##
//##         SyncWrite Example code for Dynamixel.        ##
//##                                           2009.11.10 ##
//##########################################################

#include <unistd.h>
#include <math.h>
#include <termio.h>

#include <dynamixel.h>

// *** tcpip includes***
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
// *** end tcpip includes ***

#define PI	3.141592f
//#define NUM_ACTUATOR	7

// Control table address
#define P_GOAL_POSITION_L	30
#define P_GOAL_POSITION_H	31
#define P_GOAL_SPEED_L		32
#define P_GOAL_SPEED_H		33

// Defulat setting
#define DEFAULT_BAUDNUM		1 // 1Mbps
#define NUM_ACTUATOR		7 // Number of actuator
#define STEP_THETA			(PI / 100.0f) // Large value is more fast
#define CONTROL_PERIOD		(10000) // usec (Large value is more slow)


// *** tcpip global variables ***
#define NSTRS 3

char *strs[NSTRS] = {
	"This is the first server string.\n",
	"This is the second server string.\n",
	"This is the third server string.\n"
};

extern int errno;
extern void int_handler();
extern void broken_pipe_handler();
extern void serve_clients();

static int server_sock, client_sock;
static int fromlen, i, j, num_sets_x, num_sets_y;
static char c;
static FILE *fp;
static struct sockaddr_in server_sockaddr, client_sockaddr;
int arr[10];
int new_fd;
struct sockaddr_storage their_addr;
socklen_t addr_size;
void PrintCommStatus(int CommStatus);
void PrintErrorCode(void);
// *** end tcpip global variables ***

void tcpip(void);

int main()
{
	int id[NUM_ACTUATOR];
	int baudnum = 1;
	int deviceIndex = 0;
	float phase[NUM_ACTUATOR];
	float theta = 0;
	int AmpPos = 300;
	//int AmpPos = 2048; // for EX series
	int GoalPos;
	int i;
	int CommStatus;
	int var=0;
	int temp;
	int var1=0;
	int temp1,temp2, temp3;
	int m1,m2,m3,m4,m5,m6;
	printf( "\n\nSyncWrite example for Linux\n\n" );

	// Initialize id and phase
	for( i=0; i<NUM_ACTUATOR; i++ )
	{
		id[i] = i+1;
		phase[i] = 2*PI * (float)i / (float)NUM_ACTUATOR;
	}

	///////// Open USB2Dynamixel ////////////
	if( dxl_initialize(deviceIndex, baudnum) == 0 )
	{
		printf( "Failed to open USB2Dynamixel!\n" );
		printf( "Press Enter key to terminate...\n" );
		getchar();
		return 0;
	}
	else
		printf( "Succeed to open USB2Dynamixel!\n" );

	// Set goal speed
	//dxl_write_word( BROADCAST_ID, P_GOAL_SPEED_L, 1000 ); //Was zero.***Changed***
	// Set goal position
	//dxl_write_word( BROADCAST_ID, P_GOAL_POSITION_L, AmpPos ); //Remember to uncomment **changed**

    tcpip();
	while(1)
	{
		//printf("start of while 1\n");
/*
		printf("\nEnter the motor id: ");
		//scanf("%d %d %d %d", &m1, &m2, &m3, &m4);
		scanf("%d %d", &m1, &m2);
		printf("\nEnter the AmpPos: ");
		//scanf("%d %d", &temp1,&temp2);
		scanf("%d", &temp1);

*/

		serve_clients();
		//printf("Received bytes from client...\n");
		m1 = arr[0];
		m2 = arr[1];
		m3 = arr[2];
		m4 = arr[3];
		m5 = arr[4];
		m6 = arr[5];
		temp1 = arr[6];
		temp2 = arr[7];
		temp3 = arr[8];
		
		//printf("Distance = %d\n", arr[9]);
		/*printf( "Press Enter key to continue motor\n" );
		if(getchar() == 0x1b)
			break; */


			// Make syncwrite packet
			dxl_set_txpacket_id(BROADCAST_ID);
			dxl_write_word( BROADCAST_ID, P_GOAL_SPEED_L, 0 );
			dxl_set_txpacket_instruction(INST_SYNC_WRITE);
			dxl_set_txpacket_parameter(0, P_GOAL_POSITION_L);
			dxl_set_txpacket_parameter(1, 2);

			for(i=m1; i<=m2; i++)
			{
				if (i==4 || i==2)
				temp1=1024-temp1;
				

				dxl_set_txpacket_parameter(2+3*i, id[i]);
				dxl_set_txpacket_parameter(2+3*i+1, dxl_get_lowbyte(temp1));
				dxl_set_txpacket_parameter(2+3*i+2, dxl_get_highbyte(temp1));
			}

			dxl_set_txpacket_length((2+1)*7+4);

			

			dxl_txrx_packet();
			CommStatus = dxl_get_result();
			if( CommStatus == COMM_RXSUCCESS )
			{
				PrintErrorCode();
			}
			else
			{
				PrintCommStatus(CommStatus);
				break;
			}


			// Make syncwrite packet
			dxl_set_txpacket_id(BROADCAST_ID);
			dxl_write_word( BROADCAST_ID, P_GOAL_SPEED_L, 0 );
			dxl_set_txpacket_instruction(INST_SYNC_WRITE);
			dxl_set_txpacket_parameter(0, P_GOAL_POSITION_L);
			dxl_set_txpacket_parameter(1, 2);

			for(i=m3; i<=m4; i++)
			{
				if (i==4 || i==2)
				temp2=1024-temp2;

				dxl_set_txpacket_parameter(2+3*i, id[i]);
				dxl_set_txpacket_parameter(2+3*i+1, dxl_get_lowbyte(temp2));
				dxl_set_txpacket_parameter(2+3*i+2, dxl_get_highbyte(temp2));
			}

			dxl_set_txpacket_length((2+1)*7+4);

			

			dxl_txrx_packet();
			CommStatus = dxl_get_result();
			if( CommStatus == COMM_RXSUCCESS )
			{
				PrintErrorCode();
			}
			else
			{
				PrintCommStatus(CommStatus);
				break;
			}


			
			// Make syncwrite packet
			dxl_set_txpacket_id(BROADCAST_ID);
			dxl_write_word( BROADCAST_ID, P_GOAL_SPEED_L, 0 );
			dxl_set_txpacket_instruction(INST_SYNC_WRITE);
			dxl_set_txpacket_parameter(0, P_GOAL_POSITION_L);
			dxl_set_txpacket_parameter(1, 2);

			for(i=m5; i<=m6; i++)
			{
				if (i==4 || i==2)
				temp3=1024-temp3;

				dxl_set_txpacket_parameter(2+3*i, id[i]);
				dxl_set_txpacket_parameter(2+3*i+1, dxl_get_lowbyte(temp3));
				dxl_set_txpacket_parameter(2+3*i+2, dxl_get_highbyte(temp3));
			}

			dxl_set_txpacket_length((2+1)*7+4);

			

			dxl_txrx_packet();
			CommStatus = dxl_get_result();
			if( CommStatus == COMM_RXSUCCESS )
			{
				PrintErrorCode();
			}
			else
			{
				PrintCommStatus(CommStatus);
				break;
			}

			
			
	}

	dxl_terminate();
	printf( "Press Enter key to terminate...\n" );
	getchar();

	return 0;
}

// Print communication result
void PrintCommStatus(int CommStatus)
{
	switch(CommStatus)
	{
	case COMM_TXFAIL:
		printf("COMM_TXFAIL: Failed transmit instruction packet!\n");
		break;

	case COMM_TXERROR:
		printf("COMM_TXERROR: Incorrect instruction packet!\n");
		break;

	case COMM_RXFAIL:
		printf("COMM_RXFAIL: Failed get status packet from device!\n");
		break;

	case COMM_RXWAITING:
		printf("COMM_RXWAITING: Now recieving status packet!\n");
		break;

	case COMM_RXTIMEOUT:
		printf("COMM_RXTIMEOUT: There is no status packet!\n");
		break;

	case COMM_RXCORRUPT:
		printf("COMM_RXCORRUPT: Incorrect status packet!\n");
		break;

	default:
		printf("This is unknown error code!\n");
		break;
	}
}

// Print error bit of status packet
void PrintErrorCode()
{
	if(dxl_get_rxpacket_error(ERRBIT_VOLTAGE) == 1)
		printf("Input voltage error!\n");

	if(dxl_get_rxpacket_error(ERRBIT_ANGLE) == 1)
		printf("Angle limit error!\n");

	if(dxl_get_rxpacket_error(ERRBIT_OVERHEAT) == 1)
		printf("Overheat error!\n");

	if(dxl_get_rxpacket_error(ERRBIT_RANGE) == 1)
		printf("Out of range error!\n");

	if(dxl_get_rxpacket_error(ERRBIT_CHECKSUM) == 1)
		printf("Checksum error!\n");

	if(dxl_get_rxpacket_error(ERRBIT_OVERLOAD) == 1)
		printf("Overload error!\n");

	if(dxl_get_rxpacket_error(ERRBIT_INSTRUCTION) == 1)
		printf("Instruction code error!\n");
}



// *********************************************************************************************************************************
//***********************************************TCP IP ****************************************************************************
// *********************************************************************************************************************************

void tcpip()
{
	char hostname[64];
	struct hostent *hp;
    struct linger opt;
    int sockarg;

	//gethostname(hostname, sizeof(hostname));
/*
	if((hp = (struct hostent*) gethostbyname(argv[1])) == NULL) {
		fprintf(stderr, "%s: host unknown.\n", hostname);
		exit(1);
	}*/


	if((server_sock=socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("server: socket");
		exit(1);
	}
	printf("Formed the socket\n");
	bzero((char*) &server_sockaddr, sizeof(server_sockaddr));
	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_port = htons(50000);
	//bcopy (hp->h_addr, &server_sockaddr.sin_addr, hp->h_length);
	server_sockaddr.sin_addr.s_addr = inet_addr("172.21.74.100");

	/* Bind address to the socket */
	if(bind(server_sock, (struct sockaddr *) &server_sockaddr,
            sizeof(server_sockaddr)) < 0)
    {
		perror("server: bind");
		exit(1);
	}
	printf("Bound to the port\n");
	
    /* turn on zero linger time so that undelivered data is discarded when
       socket is closed
     */
    opt.l_onoff = 1;
    opt.l_linger = 0;

    sockarg = 1;

    setsockopt(server_sock, SOL_SOCKET, SO_LINGER, (char*) &opt, sizeof(opt));
    //setsockopt(client_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&sockarg, sizeof(int));

	if(listen(server_sock, 2) == -1)
	perror("Server: listen");
	
	printf("listening on the port\n");
	addr_size = sizeof their_addr;

	if((new_fd = accept(server_sock, (struct sockaddr *)&their_addr, &addr_size)) == -1)
	perror("Server: accept");

	printf("accepted the connection\n");

	//serve_clients();

return;
}

/* Listen and accept loop function */
void serve_clients()
{
	//for(;;) {                               // Commented out infinite for loop **** changed ****
		/* Listen on the socket */
	

	int zn;
	recv(new_fd, arr, (sizeof(int)*10), 0);
	//printf("received the data\n");
	for(zn = 0;zn<=9;zn++)
    	//printf("Data received is = %d\n", arr[zn]);

	//recv(client_sock, (char *)&num_sets_y, sizeof(int), 0);
    	//printf("number of sets = %d\n", num_sets_y);
	//printf("%s",&num_sets);




		//close(client_sock);

	//}
return;

}

/* Close sockets after a Ctrl-C interrupt */
void int_handler()
{
	char ch;

	printf("Enter y to close sockets or n to keep open:");
	scanf("%c", &ch);

	if(ch == 'y') {
		printf("\nsockets are being closed\n");
		close(client_sock);
		close(server_sock);
	}

	exit(0);

}

void broken_pipe_handler()
{
	char ch;

	printf("Enter y to continue serving clients or n to halt:");
	scanf("%c", &ch);

	if(ch == 'y') {
		printf("\nwill continue serving clients\n");
		serve_clients();
	}
	else
		exit(0);

}

