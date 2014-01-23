/* globals.h - definitions for programs */

char	*progname;				/* program name for error message */

// Globals
int fdDevice;			//      Filedescriptor of serial port.
char bStderr = 0;		//      Boolean, show commands on stderr?

// Device
char *device =  "/dev/ttyAMA0"; /* serial port on the rpi1 */

