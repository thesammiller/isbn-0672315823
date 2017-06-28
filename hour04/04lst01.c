/* dos_cvrt.c */

static const char rcsid[] = 
    "$Id: dos_cvrt.c,v 1.6 1998/11/27 01:01:28 student1 Exp $";

/* REVISION HISTORY:
 * $Log: dos_cvrt.c,v $
 * Revision 1.6  1998/11/27 01:01:28  student1
 * Separated conversions into unix2dos() and
 * dos2unix() functions.
 *
 * Revision 1.5  1998/11/23 05:32:21  student1
 * Completed utility & tested ok.
 *
 * Revision 1.4  1998/11/23 05:04:23  student1
 * Coded basename test
 *
 * Revision 1.3  1998/11/21 22:49:39  student1
 * Demonstration of RCS substitutions
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

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

int
main(int argc,char **argv) {
    char *base_name = 0;    /* Basename of command */
    int rc = 0;             /* Command return code */

    /*
     * Determine the basename of the command name. This
     * is necessary since this command could be invoked
     * with a pathname. For example, argv[0] could be
     * "/usr/local/bin/unix_cvrt" if it was installed
     * in the system that way.
     */
    if ( ( base_name = strrchr(argv[0],'/') ) != 0 )
        ++base_name;        /* Skip over '/' */
    else
        base_name = argv[0];/* No dir. component */

    /*
     * Check for missing input file:
     */
    if ( argc != 2 ) {
        fprintf(stderr,"Missing input file.\n");
        return 1;
    }

    /*
     * Now that we know the basename of the command
     * name used, we can determine which function we
     * must carry out here.
     */
    if ( !strcmp(base_name,"unix_cvrt") ) {
        /* Perform a UNIX -> DOS text conversion */
	rc = unix2dos(argv[1]);
    } else {
        /* Perform a DOS -> UNIX text conversion */
        rc = dos2unix(argv[1]);
    }

    return rc;
}
