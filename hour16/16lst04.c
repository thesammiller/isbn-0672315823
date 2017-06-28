#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int
main(int argc,char *argv[]) {
    FILE *tmpf = 0;     /* Opened temp. file */
    char buf[128];      /* Input buffer */

    if ( !(tmpf = tmpfile()) ) {
        fprintf(stderr,"%s: generating a temp file name.\n",
            strerror(errno));
        abort();
    }

    fprintf(tmpf,"PID %ld was here.\n",(long)getpid());
    fflush(tmpf);

    rewind(tmpf);
    fgets(buf,sizeof buf,tmpf);
    printf("GOT: '%s';\n",buf);

    return 0;
}
