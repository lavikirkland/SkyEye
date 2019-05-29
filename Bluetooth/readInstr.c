#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

int main(int argc, char **argv)
{
      struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
      char buf[256] = { 0 };
      int client, bytes_read, b, s;
      socklen_t opt = sizeof(rem_addr);

      //allocate socket
    	if((s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM)) < 0)
    	{
       	 	perror ("cannot allocate socket\n");
        	return -1;
    	}

      // Bind socket to port 3  of the first available device
      loc_addr.rc_family = AF_BLUETOOTH;
      loc_addr.rc_bdaddr = *BDADDR_ANY;
      loc_addr.rc_channel = (uint8_t) 3;

      // Bind socket to local address
      if ((b = bind(s, (struct sockaddr *)&loc_addr, sizeof(loc_addr))) < 0)
      {
          perror("Error binding");
      }

      // put socket into listening mode
      if (listen(s, 3) < 0)
      {
          perror ("Error listening");
      }

      // accept one connection
      if ((client = accept(s, (struct sockaddr *)&rem_addr, &opt)) < 0)
      {
          perror("Error accepting");
      }

      // Continuously read the received data and write to the device file every 1 second
	    while(1)
      {
          memset(buf, 0, sizeof(buf));

          // Read instructions from received file
          bytes_read = read(client, buf, sizeof(buf));

        	if (bytes_read > 0)
        	{
        		FILE *pFile = fopen("/dev/motor", "r+");

        		// Write to device file
        		fputs(buf, pFile);

        		fclose(pFile);
        	}
        	sleep(1);
	     }

      close(client);
    	close(s);

    return 0;
}
