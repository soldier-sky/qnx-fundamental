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
#include <sys/dispatch.h>  // for name_attach_t

#include "iov_server.h"


int
calculate_checksum(char *text);

/* added custom type for receive buffer to either accept a pulse or check sum kind message (server.c QNX message passing example).
 * Hence not added into common header which are included in both server as well as client side code.
 */
typedef union{
	uint16_t msg_type;
	struct _pulse pulse;
	cksum_header_t cksum_hdr;
}msg_buf_t;

int main(void)
{

	rcvid_t rcvid;
	name_attach_t* attach;
	msg_buf_t msg;
	int status;
	int checksum;
	char* data;
	struct _msg_info minfo;

	name_attach_t *attr;

	/*Register our name*/
	attach = name_attach(NULL, CKSUM_SERVER_NAME, 0);
	if(attach == NULL){
		perror("name_attach");           //look up errno code and print
		exit(EXIT_FAILURE);
	}


	while (1)
	{
		printf("Waiting for a message...\n");
		//PUT CODE HERE to receive msg from client, store the receive id in rcvid
		rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), &minfo);
		if (rcvid == -1)
		{ //was there an error receiving msg?
			perror("MsgReceive"); //look up errno code and print
			exit(EXIT_FAILURE); //give up
		} else if (rcvid > 0) { //msg

			// did we get enough bytes for a type?
			if (minfo.msglen < sizeof(msg.msg_type))
			{
				MsgError(rcvid, EBADMSG);
				continue;
			}
			switch (msg.msg_type)
			{
			case CKSUM_IOV_MSG_TYPE:
				// did we get a full header?
				if (minfo.msglen < sizeof(msg.cksum_hdr))
				{
					MsgError(rcvid, EBADMSG);
					continue;
				}
				printf("Received a checksum request msg, header says the data is %d bytes\n",
						msg.cksum_hdr.data_size);
				// Did the client send all the data (now check srcmsglen)?
				if(minfo.srcmsglen < sizeof(msg.cksum_hdr) + msg.cksum_hdr.data_size)
				{
					MsgError(rcvid, EBADMSG);
					continue;
				}
				data = malloc(msg.cksum_hdr.data_size);
				if (data == NULL)
				{
					if (MsgError(rcvid, ENOMEM ) == -1)
					{
						perror("MsgError");
						continue;
					}
				}
				else
				{
					status = MsgRead(rcvid, data, msg.cksum_hdr.data_size, sizeof(cksum_header_t));
					if (status == -1)
					{
						const int save_errno = errno;      // perror might change error no hence back up and send via MsgError to client
						perror("MsgRead");
						MsgError(rcvid, save_errno);
						free(data);
						continue;
					}
					checksum = calculate_checksum(data);
					free(data);
					status = MsgReply(rcvid, EOK, &checksum, sizeof(checksum));
					if (status == -1)
					{
						if (errno == ESRVRFAULT) {
							perror("MsgReply fatal");
							exit(EXIT_FAILURE);
						}
						perror("MsgReply");
					}
				}
				break;
			default:
				if (MsgError(rcvid, ENOSYS) == -1)
				{
					perror("MsgError");
				}
				break;
			}
		}
		else
		{ //pulse
			switch (msg.pulse.code)
			{
			case _PULSE_CODE_DISCONNECT:
				printf("Received disconnect pulse\n");
				if (ConnectDetach(msg.pulse.scoid) == -1)
				{
					perror("ConnectDetach");
				}
				break;
			case _PULSE_CODE_UNBLOCK:
				printf("Received unblock pulse\n");
				if (MsgError(msg.pulse.value.sival_long, -1) == -1)
				{
					perror("MsgError");
				}
				break;

			default:
				printf("unknown pulse received, code = %d\n", msg.pulse.code);
				break;
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
