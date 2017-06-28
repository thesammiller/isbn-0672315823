struct S_PrintReq {
    long    mtype;          /* Print priority */
    pid_t   req_PID;        /* Requesting Process ID */
    uid_t   uid;            /* User requesting the print */
    gid_t   gid;            /* The group id of the user */
    char    path[128];      /* File to print */
} req_print;
int msqid;                  /* Message Queue IPC ID */
int msgsz;                  /* Message Size */
int z;                      /* Returned status */

msgsz = sizeof req_print - sizeof req_print.mtype;

req_print.mtype = 1;        /* Set top priority */

do  {
    z = msgsnd(msqid,(struct msgbuf *)&req_print,msgsz,0);
} while ( z == -1 && errno == EINTR );

if ( z == -1 ) {
    perror("msgsnd()");
    abort();
}
