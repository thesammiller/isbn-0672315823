#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>

static char *tf_path = 0;      /* Temp. File Pathname */

static void
myCleanup(void) {
    puts("myCleanup() called.");
    if ( tf_path != 0 ) {
        printf("Cleaning up temp. file %s\n",tf_path);
        unlink(tf_path);
        free(tf_path);
    }
}

int
main(int argc,char *argv[]) {
    FILE *tmpf = 0;         /* Temp. File stream */

    atexit(myCleanup);      /* Register our cleanup func */

    if ( !(tf_path = tempnam("./my_tmp","tmp-")) ) {
        fprintf(stderr,"%s: generating a temp file name.\n",
            strerror(errno));
        abort();
    }

    printf("Temp. file name is %s\n",tf_path);

    if ( !(tmpf = fopen(tf_path,"w+")) ) {
        fprintf(stderr,"%s: opening %s for I/O\n",
            strerror(errno),tf_path);
        abort();
    }

    fprintf(tmpf,"PID %ld was here.\n",(long)getpid());
    fclose(tmpf);

    return 0;
}
