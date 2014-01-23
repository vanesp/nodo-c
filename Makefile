CC_OPTS = -Wall -lev -lhiredis -ljansson -ggdb3 -I/usr/include/hiredis -std=c99

all: publishnodo cmdreceiver

clean:
	rm -f *.o

publishnodo: publishnodo.c globals.h comms.o
	$(CC) $(CC_OPTS) -o $@ comms.o $<

cmdreceiver: cmdreceiver.c  globals.h comms.o
	$(CC) $(CC_OPTS) -o $@ comms.o $<

comms.o: comms.c comms.h
	$(CC) -c $<
