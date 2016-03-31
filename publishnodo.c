#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>		    //      POSIX definitie
#include <fcntl.h>		    //      read() & write()
#include <string.h>
#include <signal.h>		    //      signal()
#include "jansson.h"
#include <hiredis.h>

// Include code for comms
#include "globals.h"
#include "comms.h"

#define IS_CTRL  (1 << 0)
#define IS_EXT	 (1 << 1)
#define IS_ALPHA (1 << 2)
#define IS_DIGIT (1 << 3) /* not used, just give you an idea */
 
unsigned int char_tbl[256] = {0};

char faulty[] = "?";		// used to be "Error in command."
 
/* could use ctypes, but then they pretty much do the same thing */
void init_table() {
	int i;
 
	for (i = 0; i < 32; i++) char_tbl[i] |= IS_CTRL;
	char_tbl[127] |= IS_CTRL;
 
	for (i = 'A'; i <= 'Z'; i++) {
		char_tbl[i] |= IS_ALPHA;
		char_tbl[i + 0x20] |= IS_ALPHA; /* lower case */
	}
 
	for (i = 128; i < 256; i++) char_tbl[i] |= IS_EXT;
}
 
/* depends on what "stripped" means; we do it in place.
 * "what" is a combination of the IS_* macros, meaning strip if
 * a char IS_ any of them
 */
void strip(char * str, int what) {
	unsigned char *ptr, *s = (void*)str;
	ptr = s;
	while (*s != '\0') {
		if ((char_tbl[(int)*s] & what) == 0)
			*(ptr++) = *s;
		s++;
	}
	*ptr = '\0';
}

void sig_handler(int sig)
{
// Handle signals
	fprintf(stderr, "Signal caught %d\n", sig);
	exit(sig);
}

int main(int argc, char **argv) {
    redisContext *c;
    redisReply *reply;
    json_t *root;
    // json_error_t error;
    char *msg;

	int ret;
	char buf[BUFSIZ];

	progname = argv[0];

	// Capture signals
	signal(SIGINT, sig_handler);
	signal(SIGKILL, sig_handler);

	// Sleep for a bit so everything settles down
	// pve: was 64
	sleep(2);

	fprintf(stderr, "%s started\n", progname);
   
    // This used to be portux.local, but portux is not great at spreading its address, so use IP
    // since it now runs on the portux, use localhost   
    const char *hostname = (argc > 1) ? argv[1] : "127.0.0.1";
    int port = (argc > 2) ? atoi(argv[2]) : 6379;

    // initialize the stripping table
	init_table();

    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
    c = redisConnectWithTimeout(hostname, port, timeout);
    if (c == NULL || c->err) {
        if (c) {
            fprintf(stderr, "Connection error: %s\n", c->errstr);
            redisFree(c);
        } else {
            fprintf(stderr, "Connection error: can't allocate redis context\n");
        }
        exit(1);
    }
    
	// open device for reading only
	if ((ret = initport(0)) == 0) {
		fprintf(stderr, "device opened ok\n");
	} else {
		fprintf(stderr, "failed to initialize device\n");
		close(fdDevice);
		fflush(stderr);
		exit(1);
	}

    // Create a JSON template
    root = json_object();
    json_object_set (root, "t", json_pack("s", "all")); // set the type of message
    json_object_set (root, "e", json_pack("s", "newMessage")); // the event is newMessage
    // and the received entry goes into the parameter field

    // Main loop
   
	while ((ret = readln_time(buf,100))) {
            if (ret > 0 && strlen(buf) > 0) {
                // remove extraneous characters
                strip(buf, IS_CTRL); 
                if (strlen(buf) > 0) {
                    if (bStderr) fprintf (stderr, "Received Buf %s\n", buf);
    
                    if (strncmp(buf, faulty, strlen(faulty)) != 0) {
                        // and the received entry goes into the parameter field
                        json_object_set (root, "p", json_pack("s", buf));
                        msg = json_dumps (root, JSON_COMPACT | JSON_ESCAPE_SLASH);
                        if (bStderr) fprintf (stderr, "Publish %s\n", msg); 
    
                        /* Publish this set */
                        reply = redisCommand(c,"PUBLISH ss:event %s", msg);
                        // printf("PUBLISH: %s\n", reply->str);
                        freeReplyObject(reply);
                    }
                } // if strlen > 0
            }	// if
            // Force the buffer to be empty
            buf[0] = (char) 0;
	}	// while


	close(fdDevice);
	fprintf(stderr, "%s finished\n", progname);
	fflush(stderr);

	/* Disconnects and frees the context */
	redisFree(c);

	return 0;
}
