CC=     gcc
STD=    -D_GNU_SOURCE

.c.o:
	$(CC) $(CFLAGS) -c $(STD) -Wall $<

OBJ=    log.o msq.o trace.o sqerr.o
CLNOBJ= tqlc.o tqlc2.o token.o $(OBJ)
SRVOBJ= tqld.o prepare.o server.o table.o row.o select.o compare.o sort.o $(OBJ)

all:    tqld tqlc

tqld:   $(SRVOBJ)
	$(CC) $(SRVOBJ) -o tqld

tqlc:	$(CLNOBJ)
	$(CC) $(CLNOBJ) -o tqlc

clean:  
	rm -f *.o core *.t tql.log

clobber: clean
	rm -f a.out tqld tqlc
