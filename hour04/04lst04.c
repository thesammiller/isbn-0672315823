/* $Header: /home/student1/dos_cvrt/RCS/unix2dos.c,v 1.1 1998/11/27 02:29:21 student1 Exp $
 * Basil Fawlty     $Date: 1998/11/27 02:29:21 $
 *
 * UNIX to DOS text format conversion.
 *
 * $Log: unix2dos.c,v $
 * Revision 1.1  1998/11/27 02:29:21  student1
 * Initial revision
 *
 */
static const char rcsid[] =
    "$Id: unix2dos.c,v 1.1 1998/11/27 02:29:21 student1 Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "dos_cvrt.h"

/*
 * Convert file named by pathname to DOS
 * text format, on standard output:
 */
int
unix2dos(const char *pathname) {
    int ch;                 /* Current input character */
    FILE *in = 0;           /* Input file */

    if ( !(in = fopen(pathname,"r")) ) {
        fprintf(stderr,"Cannot open input file.\n");
        return 2;
    }

    while ( (ch = fgetc(in)) != EOF ) {
        if ( ch == '\n' )
            putchar('\r');
        putchar(ch);
    }

    fclose(in);
    return 0;
}

/* End $Source: /home/student1/dos_cvrt/RCS/unix2dos.c,v $ */
