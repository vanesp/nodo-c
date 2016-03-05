CC_OPTS = -Wall -lev -lhiredis -ljansson -ggdb3 -I/usr/local/include/hiredis -std=c99
LIB = -L/usr/local/lib

all: publishnodo cmdreceiver

clean:
	rm -f *.o

publishnodo: publishnodo.c globals.h comms.o
	$(CC) $(CC_OPTS) $(LIB) -o $@ comms.o $<

cmdreceiver: cmdreceiver.c  globals.h comms.o
	$(CC) $(CC_OPTS) $(LIB) -o $@ comms.o $<

comms.o: comms.c comms.h
	$(CC) -c $<

# install must be run as SU
install:
	cp publishnodo.sh /etc/init.d/publishnodo
	cp cmdreceiver.sh /etc/init.d/cmdreceiver
	update-rc.d publishnodo defaults
	update-rc.d cmdreceiver defaults

	