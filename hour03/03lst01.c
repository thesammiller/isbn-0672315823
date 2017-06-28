/* dos_cvrt.c */

static const char rcsid[] =
    "$Id: dos_cvrt.c,v 1.3 1998/11/21 22:49:39 student1 Exp student1 $";

/* REVISION HISTORY:
 * $Log: dos_cvrt.c,v $
 * Revision 1.3  1998/11/21 22:49:39  student1
 * Demonstration of RCS substitutions
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int
main(int argc,char **argv) {
    char *base_name = 0;    /* Basename of command */

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
     * Now that we know the basename of the command
     * name used, we can determine which function we
     * must carry out here.
     */
    if ( !strcmp(base_name,"unix_cvrt") ) {
        /* Perform a UNIX -> DOS text conversion */
        puts("UNIX to DOS conversion");
    } else {
        /* Perform a DOS -> UNIX text conversion */
        puts("DOS to UNIX conversion");
    }

    return 0;
}
