#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int
main(int argc,char **argv) {
    char *cp;                       /* Work ptr */
    char buf[256];                  /* Input buffer */
    FILE *p = popen(cp="ps f","r"); /* Input pipe */

    if ( !p ) {
        fprintf(stderr,
            "%s: Opening pipe(%s) for read.\n",
            strerror(errno),cp);
        return 13;
    }

    /*
     * Read the output of the pipe:
     */
    while ( fgets(buf,sizeof buf,p) != 0 )
        fputs(buf,stdout);

    if ( pclose(p) ) {
        fprintf(stderr,"%s: pclose()\n",
            strerror(errno));
        return 13;
    }

    return 0;
}
