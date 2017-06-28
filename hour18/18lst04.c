#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
 * If the ps command is not located at /bin/ps
 * on your system, then change the pathname
 * defined for PS_PATH below.
 */
#define PS_PATH "/bin/ps"

extern char **environ;  /* Our environment array */

/*
 * Replace this process with the ps command :
 */
static void
exec_ps_cmd(void) {
    static char *argv[] =
        { "ps", "f", NULL };

    /*
     * Exec the ps command: ps f
     */
    execve(PS_PATH,argv,environ);

    /*
     * If control reaches here, then the execve()
     * call has failed!
     */
    fprintf(stderr,"%s: execve()\n",
        strerror(errno));
}

/*
 * Main program :
 */
int
main(int argc,char **argv) {
    pid_t pid;          /* Process ID of the child process */
    pid_t wpid;         /* Process ID from wait() */
    int status;         /* Exit status from wait() */

    /*
     * Create a new child process :
     */
    pid = fork();

    if ( pid == -1 ) {
        /*
         * Fork failed to create a process :
         */
        fprintf(stderr,
            "%s: Failed to fork()\n",
            strerror(errno));
        exit(13);

    } else if ( pid == 0 ) {
        /*
         * This is the child process running :
         */
        printf("PID %ld: Child started, parent is %ld.\n",
            (long)getpid(),     /* Our PID */
            (long)getppid());   /* Parent PID */
        exec_ps_cmd();          /* Start the ps command */

    } else  {
        /*
         * This is the parent process running :
         */
        printf("PID %ld: Started child PID %ld.\n",
            (long)getpid(),     /* Our PID */
            (long)pid);         /* Child's PID */

        /*
         * Wait for the child process to terminate :
         */
        wpid = wait(&status);   /* Child's exit status */
        if ( wpid == -1 ) {
            /*
             * The wait() call failed :
             */
            fprintf(stderr,"%s: wait()\n",
                strerror(errno));
            return 1;

        } else if ( wpid != pid )
            /* Should never happen in this program: */
            abort();

        else {
            /*
             * The child process has terminated:
             */
            if ( WIFEXITED(status) ) {
                /*
                 * Normal exit -- print status
                 */
                printf("Exited: $? = %d\n",
                    WEXITSTATUS(status));

            } else if ( WIFSIGNALED(status) ) {
                /*
                 * Process abort, kill or signal:
                 */
                printf("Signal: %d%s\n",
                    WTERMSIG(status),
                    WCOREDUMP(status)
                        ? " with core file."
                        : "");
            } else {
                /*
                 * Stopped child process:
                 */
                printf("Stopped.\n");
            }
        }
    }       

    return 0;
}
