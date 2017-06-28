/* $Header: /home/student1/dos_cvrt/RCS/unix2dos.c,v 1.2 1998/12/06 21:34:06 student1 Exp $
 * Basil Fawlty     $Date: 1998/12/06 21:34:06 $
 *
 * UNIX to DOS text format conversion.
 *
 * $Log: unix2dos.c,v $
 * Revision 1.2  1998/12/06 21:34:06  student1
 * Added error checking.
 *
 * Revision 1.1  1998/11/27 02:29:21  student1
 * Initial revision
 */
static const char rcsid[] =
    "$Id: unix2dos.c,v 1.2 1998/12/06 21:34:06 student1 Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "dos_cvrt.h"

/*
 * Convert file named by pathname to DOS
 * text format, on standard output:
 */
int
unix2dos(const char *pathname) {
    int ch;                 /* Current input character */
    int rc = 0;             /* Return code */
    FILE *in = 0;           /* Input file */

    if ( !(in = fopen(pathname,"r")) ) {
        fprintf(stderr,"%s: opening %s for read.\n",
            strerror(errno),pathname);
        return RC_OPENERR;
    }

    while ( (ch = fgetc(in)) != EOF ) {
        if ( ch == '\n' )
            if ( put_ch('\r') ) {
                rc = RC_WRITERR;    /* Write failed */
                goto xit;
            }
        if ( put_ch(ch) ) {
            rc = RC_WRITERR;        /* Write failed */
            goto xit;
        }
    }

    /*
     * Test for a read error:
     */
    if ( ferror(in) ) {
        fprintf(stderr,"%s: reading %s\n",
            strerror(errno),pathname);
        rc = RC_READERR;
    }

xit:fclose(in);
    return rc;
}

/* End $Source: /home/student1/dos_cvrt/RCS/unix2dos.c,v $ */
