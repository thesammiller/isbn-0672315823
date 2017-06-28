/* $Header: /home/student1/dos_cvrt/RCS/dos2unix.c,v 1.2 1998/12/06 21:33:38 student1 Exp $
 * Basil Fawlty     $Date: 1998/12/06 21:33:38 $
 *
 * The DOS to UNIX text conversion:
 *
 * $Log: dos2unix.c,v $
 * Revision 1.2  1998/12/06 21:33:38  student1
 * Added error checking.
 *
 * Revision 1.1  1998/11/27 02:29:37  student1
 * Initial revision
 */
static const char rcsid[] =
    "$Id: dos2unix.c,v 1.2 1998/12/06 21:33:38 student1 Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "dos_cvrt.h"

/*
 * Convert file named by pathname, to
 * UNIX text file format on standard output:
 */
int
dos2unix(const char *pathname) {
    int rc = 0;             /* Return code */
    int ch;                 /* Current input character */
    int cr_flag;            /* True when CR prev. encountered */
    FILE *in = 0;           /* Input file */

    if ( !(in = fopen(pathname,"r")) ) {
        fprintf(stderr,"%s: opening %s for read.\n",
            strerror(errno),pathname);
        return RC_OPENERR;  /* Open failure */
    }

    cr_flag = 0;	/* No CR encountered yet */
    while ( (ch = fgetc(in)) != EOF ) {
        if ( cr_flag && ch != '\n' ) {
            /* This CR did not preceed LF */
            if ( put_ch('\r') ) {
                rc = RC_WRITERR;    /* Write failed */
                goto xit;
            }
        }
        if ( !(cr_flag = ch == '\r') )
            if ( put_ch(ch) ) {
                rc = RC_WRITERR;    /* Write failed */
                goto xit;
            }
    }

    /*
     * Check for read errors:
     */
    if ( ferror(in) ) {
        fprintf(stderr,"%s: reading %s\n",
            strerror(errno),pathname);
        rc = RC_READERR;
    }

xit:fclose(in);
    return rc;
}

/* End $Source: /home/student1/dos_cvrt/RCS/dos2unix.c,v $ */
