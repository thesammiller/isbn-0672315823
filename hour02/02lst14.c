/* dos_cvrt.c */

static const char rcsid[] = "$Id: dos_cvrt.c,v 1.3 1998/11/21 22:49:39 student1 Exp student1 $";

/* REVISION HISTORY:
 * $Log: dos_cvrt.c,v $
 * Revision 1.3  1998/11/21 22:49:39  student1
 * Demonstration of RCS substitutions
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main(int argc,char **argv) {
    puts("dos_cvrt.c version $Revision: 1.3 $");
    return 0;
}
