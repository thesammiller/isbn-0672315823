/* $Header: /home/student1/dos_cvrt/RCS/dos2unix.c,v 1.1 1998/11/27 02:29:37 student1 Exp $
 * Basil Fawlty     $Date: 1998/11/27 02:29:37 $
 *
 * The DOS to UNIX text conversion:
 *
 * $Log: dos2unix.c,v $
 * Revision 1.1  1998/11/27 02:29:37  student1
 * Initial revision
 *
 */
static const char rcsid[] =
    "$Id: dos2unix.c,v 1.1 1998/11/27 02:29:37 student1 Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "dos_cvrt.h"

/*
 * Convert file named by pathname, to
 * UNIX text file format on standard output:
 */
int
dos2unix(const char *pathname) {
    int ch;                 /* Current input character */
    int cr_flag;            /* True when CR prev. encountered */
    FILE *in = 0;           /* Input file */

    if ( !(in = fopen(pathname,"r")) ) {
        fprintf(stderr,"Cannot open input file.\n");
        return 2;
    }

    cr_flag = 0;	/* No CR encountered yet */
    while ( (ch = fgetc(in)) != EOF ) {
        if ( cr_flag && ch != '\n' ) {
            /* This CR did not preceed LF */
            putchar('\r');
        }
        if ( !(cr_flag = ch == '\r') )
            putchar(ch);
    }

    fclose(in);
    return 0;
}

/* End $Source: /home/student1/dos_cvrt/RCS/dos2unix.c,v $ */
