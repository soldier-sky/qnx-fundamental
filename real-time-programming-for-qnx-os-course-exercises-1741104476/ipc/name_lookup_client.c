////////////////////////////////////////////////////////////////////////////////
// client.c
//
// A QNX msg passing client.  It's purpose is to send a string of text to a
// server.  The server calculates a checksum and replies back with it.  The client
// expects the reply to come back as an int
//
// This program program must be started with commandline args.  
// See  if(argc != 4) below
//
// To complete the exercise, put in the code, as explained in the comments below
// Look up function arguments in the course book or the QNX documentation.
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/neutrino.h>
#include "msg_def.h"
#include <sys/dispatch.h>  // for name_attach_t

int main(int argc, char* argv[])
{
	int coid; //Connection ID to server
	cksum_msg_t msg;
	int incoming_checksum; //space for server's reply
	int status; //status return value used for MsgSend

	/* No longer needed as using name_attach() method in server and client
	int server_pid; //server's process ID
	int server_chid; //server's channel ID

	if (argc != 4)
	{
		printf("ERROR: This program must be started with commandline arguments, for example:\n\n");
		printf("   client 482834 1 abcdefghi    \n\n");
		printf(" 1st arg(482834): server's pid\n");
		printf(" 2nd arg(1): server's chid\n");
		printf(" 3rd arg(abcdefghi): string to send to server\n"); //to make it 
		//easy, let's not bother handling spaces
		exit(EXIT_FAILURE);
	}
	*/
	if (argc != 2)
	{
		printf("ERROR: Provide a string to send\n");
		exit(EXIT_FAILURE);
	}
	//server_pid = atoi(argv[1]);
	//server_chid = atoi(argv[2]);

	//printf("attempting to establish connection with server pid: %d, chid %d\n", server_pid, server_chid);

	//PUT CODE HERE to establish a connection to the server's channel, store the
	//connection id in the variable 'coid'
	//coid = ConnectAttach(0, server_pid, server_chid, _NTO_SIDE_CHANNEL, 0);

	// look up name, rather than get it	from command line
	coid = name_open(SERVER_NAME, 0);
	if (coid == -1)
	{ //was there an error attaching to server?
		perror("name_open"); //look up error code and print
		exit(EXIT_FAILURE);
	}

	msg.msg_type = CKSUM_MSG_TYPE;
	strlcpy(msg.string_to_cksum, argv[3], sizeof(msg.string_to_cksum));
	printf("Sending string: %s\n", msg.string_to_cksum);

	status = MsgSendPulse(coid, -1, 3, 0xdeadc0de);
	if (status == -1)
	{
		perror("MsgSendPulse");
	}
	
	//PUT CODE HERE to send message to server and get the reply
	status = MsgSend(coid, &msg, sizeof(msg.msg_type) + strlen(msg.string_to_cksum) + 1, &incoming_checksum,
			sizeof(incoming_checksum));
	if (status == -1)
	{ //was there an error sending to server?
		perror("MsgSend");
		exit(EXIT_FAILURE);
	}

	printf("received checksum=%d from server\n", incoming_checksum);
	printf("MsgSend return status: %d\n", status);

	return EXIT_SUCCESS;
}

