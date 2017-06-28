#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int
main(int argc,char *argv[]) {
    char tf_path[64];   /* Temp. File Pathname */
    int tfd = -1;       /* UNIX File Descriptor */
    FILE *tmpf = 0;     /* Opened temporary FILE */

    strcpy(tf_path,"/tmp/01-XXXXXX"); /* Init template */

    if ( (tfd = mkstemp(tf_path)) < 0 ) {
        fprintf(stderr,"%s: generating a temp file name.\n",
            strerror(errno));
        abort();
    }

    printf("Using temp file: %s\n",tf_path);

    tmpf = fdopen(tfd,"w+");        /* Use FILE I/O */
    fprintf(tmpf,"Written by PID=%ld\n", (long)getpid());
    fclose(tmpf);

    return 0;
}
