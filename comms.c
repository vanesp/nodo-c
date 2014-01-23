/* comms.c - open serial port non-blocking and read messages */

#include <fcntl.h>		    //      read() & write()
#include <signal.h>		    //      signal()
#include <stdio.h>
#include <stdlib.h>		    //      exit()
#include <string.h>
#include <termios.h>		//      voor tty-verbinding
#include <errno.h>		    //      errno()
#include <unistd.h>		    //      POSIX definitie
#include <time.h>		    //      voor ErrorLogging, timespec


#include "comms.h"

void m_hupcl(int fd, int on)
{
// set hangup on close on/off
  {
    struct termios sgg;

    tcgetattr(fd, &sgg);
    if (on)
      sgg.c_cflag |= HUPCL;
    else
      sgg.c_cflag &= ~HUPCL;
    tcsetattr(fd, TCSANOW, &sgg);
  }
}

int readln_time(char *s, int time)
{
// string s should be long enough to capture a string.
// may return TIMEOUT if more than time sec no chars
	char *pC = s;
	char c;
	int len = 0;
	int j, ret;

	j = 0;
	//      Keep reading whilst we have chars, and not EOL.
	while ((ret = read(fdDevice, pC, 1)) >= 0) {
		if (ret == 1) {
			c = *pC;
			// debugging trace
			// fprintf (stderr, " %.2X", c);
			// fprintf (stderr, "%c", c);
			pC++;
			len++;
			if ((c == '\n') || (c == '\r')) {
				*pC = '\0';
				return len;
			}
		} else {
			j++;
			// timeout after time seconds of nothing
			if (j > time) {
				if (len == 0)
					return (TIMEOUT);
				else
					return len;
			}
			msleep (100);
		}
	}
	if (ret < 0) {
		fprintf(stderr, "readln_time: error %d\n", ret);
		if (len == 0)
			return ret;
		else
			return len;
	} else {
		return len;
	}
}


int readline(char *s)
{
// string s should be long enough to capture a string.
// may return TIMEOUT if more than 10 sec no chars
	return (readln_time (s, 10));
}

int writestring(char *s)
{
// write string s to the serial port
	char *pC = s;
	int ret;

	while (*pC != '\0') {
		if ((ret = write(fdDevice, pC, 1)) < 0) {
			fprintf(stderr, "writestring: error %d\n", ret);
			return ret;
		}
		if (ret == 1)
			pC++;
	}
	return strlen(s);
}

int msleep(unsigned long millisec)
// sleep for milliseconds
{
	struct timespec req;
	time_t sec = (int)(millisec / 1000);
	millisec = millisec - (sec * 1000);
	req.tv_sec = sec;
	req.tv_nsec = millisec * 1000000L;
	while (nanosleep(&req, &req) == -1)
		continue;
	return 1;
}


int initport(int rw)
{
// opens and initializes serial port, returns error if failed

	struct termios options;

    if (rw > 0) {
        // open for reading and writing
        fdDevice = open(device, O_WRONLY | O_NOCTTY);
    } else {
        fdDevice = open(device, O_RDONLY | O_NOCTTY);
    }
        
	// open for reading and writing, not as controlling terminal, and ignore DCD flag
	if (fdDevice < 0) {
		fprintf(stderr, "cannot open serial port on ttyAMA0\n");
		exit(EXIT_FAILURE);
	}

	// Set Hangup on Close if program crashes. (Hehe)
	m_hupcl(fdDevice, 1);

	// set read functions to return after VTIME (to return immediately, replace 0 with FNDELAY)
	fcntl(fdDevice, F_SETFL, 0);

	// now we'ĺl set-up the port parameters
	// get the current options for the port
	tcgetattr(fdDevice, &options);
	// set baud rates to 57600
	cfsetispeed(&options, B57600);
	cfsetospeed(&options, B57600);
	// select 1 second timeout
	options.c_cc[VMIN] = 0;
	options.c_cc[VTIME] = 10;

	// set the new options immediately
	tcsetattr(fdDevice, TCSANOW, &options);

	return (0);
}

