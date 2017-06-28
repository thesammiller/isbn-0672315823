union {
    struct S_PrintReq {
        long    mtype;      /* Print priority */
        pid_t   req_PID;    /* Requesting Process ID */
        uid_t   uid;        /* User requesting the print */
        gid_t   gid;        /* The group id of the user */
        char    path[128];  /* File to print */
    } req_print;
    struct msgbuf msg;
} anymsg;
int msqid;                  /* Message Queue IPC ID */
int maxsz;                  /* Message Size */
int z;                      /* Returned status */

maxsz = sizeof anymsg - sizeof(long);

do  {
    z = msgrcv(msqid,&anymsg.msg,maxsz,-16,0);
} while ( z == -1 && errno == EINTR );

if ( z == -1 ) {
    perror("msgrcv()");
    abort();
}
