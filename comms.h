/* comms.h - open serial port non-blocking and read messages */

// standard defines
#define	OK	0
#define TIMEOUT		-1

#define TRUE	1
#define FALSE	0

extern char	*progname;				// program name for error message

// Globals
extern int fdDevice;			//      Filedescriptor of serial port.
extern char bStderr;		    //      Boolean, show errors on the screen?

// Device
extern char *device;            // serial port on the rpi1

// Common functions
void m_hupcl(int fd, int on);                       // hangup on close
int readln_time(char *s, int time);                 // read line with time out
int readline(char *s);                             // read line with 10 sec timeout
int writestring(char *s);                           // write a line
int msleep(unsigned long millisec);                // millisecond sleep
int initport(int rw);                                // initialize serial port, if rw > 0, for writing
