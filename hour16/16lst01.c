#include <stdio.h>
#include <string.h>
#include <errno.h>

int
main(int argc,char *argv[]) {
    char *tmp_pathname; /* Temp. File Pathname */
    FILE *tmpf = 0;     /* Opened temp. file */

    if ( !(tmp_pathname = tmpnam(0)) ) {
        fprintf(stderr,"%s: generating a temp file name.\n",
            strerror(errno));
        abort();
    }

    printf("Using temp file: %s\n",tmp_pathname);

    if ( !(tmpf = fopen(tmp_pathname,"w")) ) {
        fprintf(stderr,"%s: creating temp file %s\n",
            strerror(errno), tmp_pathname);
        abort();
    }

    return 0;
}
