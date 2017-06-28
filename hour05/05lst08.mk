# $Source: /home/student1/dos_cvrt/RCS/Makefile,v $
# $Revision: 1.1 $
# $Date: 1998/11/27 22:17:18 $

CC=	gcc
STD=	_GNU_SOURCE
OBJS=	dos_cvrt.o unix2dos.o dos2unix.o

.c.o:
	$(CC) -c -Wall $(CFLAGS) -D$(STD) $<

all:	dos_cvrt unix_cvrt

dos_cvrt: $(OBJS)
	$(CC) $(OBJS) -o dos_cvrt

unix_cvrt: dos_cvrt
	rm -f unix_cvrt
	ln dos_cvrt unix_cvrt

clean:
	rm -f *.o core

clobber: clean
	rm -f dos_cvrt unix_cvrt

# End Makefile
