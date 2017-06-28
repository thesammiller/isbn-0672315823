/* tqld.c :
 * Sams Teach Yourself LINUX Programming in 24 Hours
 * ISBN 0-672-31582-3
 * Warren W. Gay VE3WWG
 */
static const char rcsid[] =
    "$Id: tqld.c,v 1.1 1999/02/20 21:32:09 wwg Exp $";

#include "server.h"

/*
 * Server externals :
 */
struct S_TQLD tqld = {
    SKEYDFLT,           /* skey */
    0,                  /* key */
    -1,                 /* msqid */
    -1,                 /* signo */
};

/*
 * Server cleanup function :
 */
static void
cleanup(void) {

    if ( tqld.msqid >= 0 )
        delMsq(tqld.msqid);
}

/*
 * Signal catcher :
 */
void
catcher(int signo) {

    if ( tqld.signo == -1 )
        tqld.signo = signo;
}

/*
 * Server main program :
 */
int
main(int argc,const char *argv[],const char *envp[]) {
    int rc = 1;
    struct sigaction sa;
    pid_t pid;
    
    printf("TQL Server version %s\n",TQLD_VERSION);

    /*
     * Fork to become a daemon:
     */
    if ( (pid = fork()) == (pid_t) -1 ) {
        logf("%s: fork()\n",strerror(errno));
        exit(13);
    } else if ( pid )
        exit(0);              /* Parent process exits */

    /*
     * Close all possible tty file units:
     */
    freopen("/dev/null","r",stdin);
    freopen("/dev/null","w",stdout);
    freopen("/dev/null","w",stderr);
    setsid();              /* Become a session leader */

    /*
     * Catch signals SIGTERM, SIGINT and SIGHUP :
     */
    sa.sa_handler = catcher;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGTERM,&sa,0);
    sigaction(SIGINT,&sa,0);
    sigaction(SIGHUP,&sa,0);

    /*
     * Cleanup message queue on exit :
     */
    atexit(cleanup);

    /*
     * Initialize for sort :
     */
    logf("TQL Server version %s\n",TQLD_VERSION);
    logf("Written by Warren W. Gay VE3WWG\n");
    logf("Started as PID %ld\n",(long)getpid());

    if ( !initSort() )
        goto xit;       /* Can't find sort */

    /*
     * Create the message queue :
     */
    tqld.msqid = newMsq(&tqld.skey,&tqld.key,
        IPC_CREAT|IPC_EXCL|0666);
    if ( tqld.msqid < 0 )
        goto xit;       /* Failed */

    /*
     * Now service clients :
     */
    server();
    rc = 0;

xit:
    logf("Exit RC=%d\n",rc);
    logClose();

    return rc;
}
