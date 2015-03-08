#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>		    //      POSIX definitie
#include <string.h>
#include <signal.h>
#include "hiredis.h"
#include "async.h"
#include "adapters/libev.h"
#include "jansson.h"

// Include code for comms
#include "globals.h"
#include "comms.h"

void onMessage(redisAsyncContext *c, void *reply, void *privdata) {
    redisReply *r = reply;
    json_t *root;
    json_error_t error;
    
    if (reply == NULL) return;

    if (r->type == REDIS_REPLY_ARRAY) {
        /*
        for (int j = 0; j < r->elements; j++) {
            printf("%u) %s\n", j, r->element[j]->str);
        }
        */
        // now analyse the message contents
        if (r->element[2]->str != NULL) {
            // a json message
            root = json_loads (r->element[2]->str, 0, &error);
            if (!root) {
                fprintf (stderr, "error: on line %d: %s\n", error.line, error.text);
                return;
            }
            if (!json_is_object(root)) {
                fprintf (stderr, "error: root is not an object\n");
                json_decref(root);
                return;
            }
            // get the fields in the object t (type), e (event), p (params) 
            json_t *t, *e, *p, *type, *location, *quantity, *value ;
            const char *event_text, *command;
            char str[80];
            
            t = json_object_get (root, "t");
            e = json_object_get (root, "e");
            p = json_object_get (root, "p");
            // check if event is portux...
            if (!json_is_string(e)) {
                fprintf (stderr, "error: event is not a string\n");
                json_decref(root);
                return;
            }
            
            event_text = json_string_value(e);
            if (strcmp (event_text, "portux") == 0) {
                // yes, we have portux in the message... now we need to work on the next params
                type = json_object_get (p, "type");
                location = json_object_get (p, "location");
                quantity = json_object_get (p, "quantity");
                value = json_object_get (p, "value");
                
                // we act on type = Switch, quantity is command, and value is true or false
                if (strcmp(json_string_value(type), "Switch") == 0) {
                    // Create the command to be executed
                    command = json_string_value(quantity);
                    strcpy (str, command);
                    if (json_is_true(value)) {
                        strcat (str, "On;");
                    } else {
                        strcat (str, "Off;");
                    }
                    if (bStderr) fprintf(stderr, "%s\n", str);
                    // write string to serial port
                    writestring (str);
               }      
            }
         }
    }
}

void connectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        fprintf(stderr, "Error: %s\n", c->errstr);
        return;
    }
    fprintf(stderr, "Connected...\n");
}

void disconnectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        fprintf(stderr, "Error: %s\n", c->errstr);
        return;
    }
    fprintf(stderr, "Disconnected...\n");
}

int main (int argc, char **argv) {
    int ret;
    signal(SIGPIPE, SIG_IGN);

    // sleep for a while, before starting up, so that the system settles down
    sleep (30);

    redisAsyncContext *c = redisAsyncConnect("portux.local", 6379);
    if (c->err) {
        fprintf(stderr, "error: %s\n", c->errstr);
        return 1;
    }

	progname = argv[0];
    // open serial port
	fprintf(stderr, "%s started\n", progname);
	// open device for writing only
	if ((ret = initport(1)) == 0) {
		fprintf(stderr, "device opened ok\n");
	} else {
		fprintf(stderr, "failed to initialize device\n");
		close(fdDevice);
		fflush(stderr);
		exit(1);
	}

    // Setup the first setting of the device
    // So that it does not try to send IR commands
    writestring("TransmitSettings RF;");

    redisLibevAttach(EV_DEFAULT_ c);
    redisAsyncSetConnectCallback(c,connectCallback);
    redisAsyncSetDisconnectCallback(c,disconnectCallback);    
    
    redisAsyncCommand(c, onMessage, NULL, "SUBSCRIBE ss:event");
    ev_loop(EV_DEFAULT_ 0);
    
	close(fdDevice);
	fprintf(stderr, "%s finished\n", progname);
    fflush(stderr);

    return 0;
}
