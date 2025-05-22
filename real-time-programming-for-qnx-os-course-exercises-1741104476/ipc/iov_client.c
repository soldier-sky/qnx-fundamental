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
#include "iov_server.h"
#include <sys/dispatch.h>  // for name_attach_t

int main(int argc, char* argv[])
{
	int coid;                      //Connection ID to server
	cksum_header_t hdr;            // msg header will specify how many bytes of data will follow
	int incoming_checksum;         //space for server's reply
	int status;                    //status return value used for MsgSend
	iov_t siov[2];                 // create a 2 part iov

	if (argc != 2)
	{
		printf("ERROR: Provide a string to send\n");
		exit(EXIT_FAILURE);
	}

	// look up name, rather than get it	from command line
	coid = name_open(CKSUM_SERVER_NAME, 0);
	if (coid == -1)
	{ //was there an error attaching to server?
		perror("name_open"); //look up error code and print
		exit(EXIT_FAILURE);
	}

	printf("Sendingthe following text to checksum server: %s\n", argv[1]);

	//build the header
	hdr.msg_type = CKSUM_IOV_MSG_TYPE;
	hdr.data_size = strlen(argv[1]) + 1;

	// setup the message as a two part iov, first the header and then the data
	SETIOV(&siov[0], &hdr, sizeof(hdr));
	SETIOV(&siov[1], argv[1], hdr.data_size);

	// and send the message off the server
	status = MsgSendvs(coid, siov, 2, &incoming_checksum, sizeof(incoming_checksum));
	if (status == -1)
	{
		perror("MsgSendvs");
	}


	printf("received checksum=%d from server\n", incoming_checksum);
	printf("MsgSend return status: %d\n", status);

	return EXIT_SUCCESS;
}

