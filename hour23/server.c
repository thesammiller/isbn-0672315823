/* server.c :
 * Sams Teach Yourself LINUX Programming in 24 Hours
 * ISBN 0-672-31582-3
 * Warren W. Gay VE3WWG
 *
 * This module is the main server loop:
 */
static const char rcsid[] =
    "$Id: server.c,v 1.1 1999/02/20 21:32:09 wwg Exp $";

#include "server.h"

/*
 * Send an error back to the client :
 *
 * ARGS:
 *  sqe     input   TQL error code.
 *  pid     input   The process ID of the client to
 *                  return the message to.
 * RETURNS:
 *  0       Successful
 *  errno   Failed
 */
static int
sendError(int sqe,pid_t pid) {
    Resp0 r0;

    r0.mtype = (long)pid;     /* Process ID of client */
    r0.rtype = R_ERROR;               /* Message type */
    r0.sqler = sqe;         /* The returned SQL error */
    return sendMsq(tqld.msqid,&r0,sizeof r0,0);
}

/*
 * Execute the client message :
 *
 * ARGS:
 *  msg     input   The input statement from the TQL
 *                  client process to execute.
 */
static void
execute(Msg *msg) {
    Select sel;                      /* Select object */
    int sqe = SQE_OK;               /* Assume success */

    initSelect(&sel);        /* Initialize the object */

    /*
     * Log and check the process ID :
     */
    logf("TQL query for PID %ld.\n",msg->PID);
    if ( msg->PID <= 3L ) {
        logf("Invalid PID=%ld received!\n",msg->PID);
        return;
    }    

    TRACEF(5,XEQ,("Statement token=%d;\n",msg->sql[0]))

    /*
     * Determine what the request is :
     */
    switch ( msg->sql[0] ) {

        /*
         * SELECT statement :
         */
    case TKN_SELECT :
        sqe = prepare(msg,&sel);  /* Prep SELECT stmt */
        if ( !sqe )                       /* Prep OK? */
            sqe = xeqSelect(&sel,msg->PID); /* XEQ it */
        if ( sqe )                      /* SQL Error? */
            sendError(sqe,msg->PID); /* Send err. rsp */
        break;

        /*
         * SHUTDOWN request from client:
         */
    case TKN_SHUTDN :
        logf("PID %ld requested server shutdown\n",msg->PID);
        tqld.signo = -2;           /* Normal shutdown */
        break;

        /*
         * Unsupported Command :
         */
    default :
        sqe = SQE_SYNTAX;             /* Syntax error */
        sendError(sqe,msg->PID);   /* Send err. resp. */
    }

    /*
     * Report Errors to Log:
     */
    if ( sqe != SQE_OK )
        logf("%s: statement completed.\n",
            sqErrorText(sqe));

    disposeSelect(&sel); /* Dispose of object content */
}

/*
 * Main TQL Server Loop :
 *
 * In this function we do:
 *
 *  1.  Get a message from any client
 *  2.  Check for EINTR and signal.
 *  3.  If signal, then shutdown.
 *  4.  If no signal, process the statement.
 *  5.  Repeat step 1
 */
void
server(void) {
    Msg msg;
    int z;

    do  {
        /*
         * 1. Get a message from any client process:
         */
        do  {
            z = msgrcv(tqld.msqid,(struct msgbuf *)&msg,
                MAXSQL,1L,0);
            /*
             * 2. Check for signal after EINTR:
             */
            if ( tqld.signo != -1 )
                break;      /* Got signal */
        } while ( z == -1 && errno == EINTR );

        /*
         * Check for receipt of a signal :
         */
        if ( tqld.signo < 0 ) {
            /*
             * No signal received: Check for an
             * error from msgrcv().
             */
            if ( z < 0 ) {
                logf("%s: msgrcv(msqid=%d)\n",
                    strerror(errno),
                    tqld.msqid);
                break;
            }

            /*
             * We have a valid message to process:
             */
            if ( z < sizeof(pid_t) ) {
                logf("Bad message! Too short (%d bytes)\n",z);
                continue;
            }

            /*
             * 4. Process the message :
             */
            z -= sizeof(pid_t);
            msg.sql[z] = 0;
            execute(&msg);
	}

    } while ( tqld.signo == -1 );

    /*
     * Termination Message :
     */
    if ( tqld.signo >= 0 )
        logf("Received signal: %s\n",strsignal(tqld.signo));
    else
        logf("Shutting down.\n");
}

/* End server.c */
