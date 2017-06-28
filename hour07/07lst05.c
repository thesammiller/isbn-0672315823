/* $Header: /home/student1/dos_cvrt/RCS/putch.c,v 1.1 1998/12/06 21:32:49 student1 Exp $
 *
 * Put characters to stdout with error checking:
 *
 * $Log: putch.c,v $
 * Revision 1.1  1998/12/06 21:32:49  student1
 * Initial revision
 *
 */
static const char rcsid[] =
    "$Id: putch.c,v 1.1 1998/12/06 21:32:49 student1 Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

/*
 * Put a character to stdout, with proper
 * error checking:
 *
 * RETURNS: 0 == success
 *         -1 == Failed
 */
int
put_ch(int ch) {

    putchar(ch);
    if ( ferror(stdout) ) {
        fprintf(stderr,"%s: writing to stdout\n",
            strerror(errno));
        return -1;          /* Failed write */
    }
    return 0;               /* Successful write */
}

/* End $Source: /home/student1/dos_cvrt/RCS/putch.c,v $ */
