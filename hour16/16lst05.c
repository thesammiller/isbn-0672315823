#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>

int
main(int argc,char *argv[]) {
    char *tf_path = 0;      /* Temp. File Pathname */
    FILE *tmpf = 0;         /* Temp. File stream */

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

    free(tf_path);      /* Free malloc()'d string */

    return 0;
}
