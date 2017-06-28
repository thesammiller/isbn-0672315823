/* msq.c :
 * Sams Teach Yourself LINUX Programming in 24 Hours
 * ISBN 0-672-31582-3
 * Warren W. Gay VE3WWG
 *
 * This module performs Message Queue functions. 
 */
static const char rcsid[] =
    "$Id: msq.c,v 1.1 1999/02/20 21:32:09 wwg Exp $";

#include "tql.h"

/*
 * Access/Create Message Queue :
 *
 * RETURNS:
 *  >= 0    IPC ID of the message queue
 *  -1      Failed, check errno
 */
int
newMsq(char **pskey,key_t *pkey,int msgflgs) {
    char *cp;
    key_t key;
    int msqid;

    /*
     * Get the message queue IPC Key :
     */
    if ( (cp = getenv(TQLKEY)) && pskey )
        *pskey = cp;        /* Pass back string */

    /*
     * Convert the IPC Key string to numeric :
     */
    key = (key_t) strtoul(*pskey,&cp,0);
    if ( *cp ) {
        logf("Invalid number: IPC Key %s='%s'\n",TQLKEY,*pskey);
        goto bail;
    }

    /*
     * Access/Create Message Queue:
     */
    if ( (msqid = msgget(key,msgflgs)) >= 0 )
        return msqid;       /* Successful */

    /*
     * Log an error:
     */
    logf("%s: msgget(key=0x%08lX,%04o)\n",
        strerror(errno),
        (long)key,
        msgflgs);

    /*
     * Error Exit:
     */
bail:
    *pkey = 0;
    return -1;
}

/*
 * Destroy the message queue :
 *
 * RETURNS:
 *  0       Successful
 *  errno   Failed
 */
int
delMsq(int msqid) {

    if ( msgctl(msqid,IPC_RMID,0) == -1 ) {
        /*
         * Log the error:
         */
        logf("%s: msgctl(%d,IPC_RMID)\n",
            strerror(errno),
            msqid);
        return errno;
    }

    return 0;   /* Successful */
}

/*
 * Send a message :
 *
 * RETURNS:
 *  0       Successful
 *  errno   Failed
 */
int
sendMsq(int msqid,void *data,size_t bytes,int flgs) {
    int z;
    size_t n = bytes - sizeof(long);

    /*
     * Send a message, EINTR is ignored here.
     */
    do  {
        z = msgsnd(msqid,(struct msgbuf *)data,n,flgs);
    } while ( z == -1 && errno == EINTR );

    /*
     * Report an error if any.
     */
    if ( z == -1 ) {
        logf("%s: msgsnd(%d,,%d bytes,%04o)\n",
            strerror(errno),n,flgs);
        return errno;
    }

    return 0;       /* Successful */
}

/*
 * Send a message of variable size, of the difference
 * between two pointers:
 *
 * RETURNS:
 *  0       Successful
 *  errno   Failed
 */
int
snd2Msq(int msqid,void *ptr1,void *ptr2,int flgs) {
    size_t bytes = ((char *)ptr2 - (char *)ptr1);

    return sendMsq(msqid,ptr1,bytes,flgs);
}

/*
 * Client side Recv Message :
 *
 * RETURNS:
 *  0       Successful
 *  errno   Failed
 */
int
rcvMsq(int msqid,Resp *r,int flags) {
    long pid = (long)getpid();          /* Get our process ID */
    size_t n = sizeof *r - sizeof(long);
    int z;

    /*
     * Receive a message, ignoring EINTR:
     */
    do  {
        z = msgrcv(msqid,(struct msgbuf *)r,n,pid,flags);
    } while ( z == -1 && errno == EINTR );

    /*
     * Log the error if any:
     */
    if ( z == -1 ) {
        logf("%s: msgrcv(%d,,,%04o)\n",
            strerror(errno), msqid, flags);
        return errno;
    }

    return 0;   /* Successful */
}

/* msq.c */
