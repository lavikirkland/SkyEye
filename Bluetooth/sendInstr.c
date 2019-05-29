#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

int main(int argc, char **argv)
{
		// Address of Gumstix
		char dest[18] = "00:03:19:50:29:73";

		struct sockaddr_rc addr = { 0 };
    int status, s;

		// allocate socket
		if ((s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM)) < 0)
		{
			perror("Cannot allocate socket");
			return -1;
		}

		// Set up the connection parameters for target (i.e. Gumstix)
		addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = (uint8_t) 3;
    str2ba( dest, &addr.rc_bdaddr );

	 	// connect to server
    if ((status = connect(s, (struct sockaddr *)&addr, sizeof(addr))) < 0)
		{
          perror("Error connecting");
		}

		if (status == 0)
		{
				// Continuously read output from object detection alogrithm
				// and send instruction to Gumstix every 1 second
				while(1)
				{
						char buf[256];
						int bytes_read;

						FILE *file = fopen("instruct.txt", "rb");

						if(!file)
						{
							printf("Cannot open file for reading\n");
							return -1;
						}

						memset(buf, 0, sizeof(buf));
						bytes_read = fread(buf, 1, sizeof(buf), file);
						printf("%s\n", buf);

						if (bytes_read > 0)
							write(s, buf, bytes_read);

						fclose(file);
						sleep(1);
				}
		}

		close(s);

	return 0;
}
