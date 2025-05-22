////////////////////////////////////////////////////////////////////////////////
// server.c
//
// A QNX msg passing server.  It should receive a string from a client,
// calculate a checksum on it, and reply back the client with the checksum.
//
// The server prints out its pid and chid so that the client can be made aware
// of them.
//
// Using the comments below, put code in to complete the program.  Look up
// function arguments in the course book or the QNX documentation.
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <process.h>

#include "msg_def.h"  //layout of msg's should be defined by a struct, here's its definition
#include <sys/dispatch.h>  // for name_attach_t

int
calculate_checksum(char *text);

/* added custom type for receive buffer to either accept a pulse or check sum kind message (server.c QNX message passing example).
 * Hence not added into common header which are included in both server as well as client side code.
 */
typedef union{
	uint16_t type;
	cksum_msg_t chsum_msg;
	struct _pulse pulse;
}recv_buf_t;

int main(void)
{
	//int chid;
	int pid;
	rcvid_t rcvid;
	//cksum_msg_t msg;
	recv_buf_t rbuf;
	int status;
	int checksum;
	name_attach_t *attr;

	/*Register our name*/
	attr = name_attach(NULL, SERVER_NAME, 0);

	/* commenting as we are using name_attach method to bypass cli based data
	 * to create connection
	//PUT CODE HERE to create a channel, store channel id in the chid variable
	chid = ChannelCreate(0);
	if (chid == -1)
	{ //was there an error creating the channel?
		perror("ChannelCreate()"); //look up the errno code and print
		exit(EXIT_FAILURE);
	}
    */
	pid = getpid(); //get our own pid
	printf("Server's pid: %d, chid: %d\n", pid, attr->chid); //print our pid/chid so
	//client can be told where to connect

	while (1)
	{
		//PUT CODE HERE to receive msg from client, store the receive id in rcvid
		rcvid = MsgReceive(attr->chid, &rbuf, sizeof(rbuf), NULL);
		if (rcvid == -1)
		{ //was there an error receiving msg?
			perror("MsgReceive"); //look up errno code and print
			exit(EXIT_FAILURE); //give up
		} else if (rcvid == 0) {
			printf("We got a pulse with code of %d, and the value of %#lx\n",rbuf.pulse.code,rbuf.pulse.value.sival_long );
			// Pulse code handling and cleanup if disconnect pulse code received.
			switch(rbuf.pulse.code)
			{
			case _PULSE_CODE_DISCONNECT:
				// a client went away, if we had any data clean up, then always release scoid
				ConnectDetach(rbuf.pulse.scoid);
				break;
			case CKSUM_PULSE_CODE:
				// This was our expected pulse code
				printf("This is our expected CKSUM_PULSE_CODE\n");
				break;
			default:
				// An unexpected pulse code, but we can't report an error as pulses are non-blocking
				printf("This was and unexpected pulse code\n");
				break;

			}
		}
		else // we got a message
		{

			//PUT CODE HERE to calculate the check sum by calling calculate_checksum()
			if (rbuf.type == CKSUM_MSG_TYPE)
			{
				printf("Got a checksum message\n");
				checksum = calculate_checksum(rbuf.chsum_msg.string_to_cksum);

				//PUT CODE HERE TO reply to client with checksum, store the return status in status
				status = MsgReply(rcvid, EOK, &checksum, sizeof(checksum));
				if (status == -1)
				{
					perror("MsgReply");
				}
			}
			else
			{
				printf("Got an unknown message type %d\n", rbuf.type);
				if (MsgError(rcvid, ENOSYS ) == -1)
				{
					perror("MsgError");

				}

			}
		}
	}
	return 0;
}

int calculate_checksum(char *text)
{
	char *c;
	int cksum = 0;

	for (c = text; *c != '\0'; c++)
		cksum += *c;
	return cksum;
}

