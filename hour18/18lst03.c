#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

int
main(int argc,char **argv) {
    pid_t pid;          /* Process ID of the child process */
    pid_t wpid;         /* Process ID from wait() */
    int status;         /* Exit status from wait() */

    pid = fork();       /* Create a new child process */

    if ( pid == -1 ) {
        fprintf(stderr,
            "%s: Failed to fork()\n",
            strerror(errno));
        exit(13);
    } else if ( pid == 0 ) {
        printf("PID %ld: Child started, parent is %ld.\n",
            (long)getpid(),     /* Our PID */
            (long)getppid());   /* Parent PID */
    } else  {
        printf("PID %ld: Started child PID %ld.\n",
            (long)getpid(),     /* Our PID */
            (long)pid);         /* Child's PID */
        wpid = wait(&status);   /* Child's exit status */
        if ( wpid == -1 ) {
            fprintf(stderr,"%s: wait()\n",
                strerror(errno));
            return 1;
        } else if ( wpid != pid )
            abort(); /* Should never happen in this prog. */
        else {
            printf("Child PID %ld exited status 0x%04X\n",
                (long)pid,      /* Child PID */
                status);        /* Exit status */
        }
    }       

    return 0;
}
