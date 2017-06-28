# $Source: /home/student1/dos_cvrt/RCS/Makefile,v $
# $Revision: 1.2 $
# $Date: 1998/12/06 21:34:26 $

CC=	gcc
STD=	_GNU_SOURCE
OBJS=	dos_cvrt.o unix2dos.o dos2unix.o putch.o

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
