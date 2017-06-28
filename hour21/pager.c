/* Pager.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

typedef struct {
    long    mtype;          /* Message type */
    char    phone[20];      /* Pager phone number */
    char    callbk[20];     /* Call back number */
} Page;

typedef union {
    struct msgbuf generic;  /* Generic Message */
    Page    page;           /* Page request */
} Msg;

static char *cmdopt_s = 0;  /* Device to page to */
static key_t cmdopt_k = 0;  /* IPC Key */
static int cmdopt_p = 20;   /* Priority */
static int msqid = -1;      /* Message Queue */

static void
usage(char *cmd) {
    char *cp = strrchr(cmd,'/');

    if ( cp )
        cmd = cp + 1;
    fprintf(stderr,"Server Usage: %s -s tty -k key\n"
        "\tto start server.\n\n",cmd);
    fprintf(stderr,"Client Usage: %s -k key [-p priority] "
        "page#/callbk#...\n"
        "\tto queue a page request\n\n",
        cmd);
    fputs("\t-s tty\t\tDefault /dev/tty\n",stderr);
    fputs("\t-k key\t\tIPC key for server\n",stderr);
    fputs("\t-p priority\tDefault priority=20\n",stderr);
    fputs("\t\t\t(range 1 to 100)\n",stderr);
    fputs("Example:\n",stderr);
    fputs("\t%s -k 23456 -p5 416-555-1212/"
        "416-555-5555x4687\n",stderr);
}

/*
 * Create or access a message queue :
 */
static int
createMsq(key_t key,int perms) {
    int msqid;

    if ( perms )
        perms |= IPC_CREAT|IPC_EXCL;

    if ( (msqid = msgget(key,perms)) == -1 ) {
        fprintf(stderr,"%s: msgget()\n",strerror(errno));
        exit(13);
    }
    return msqid;
}

/*
 * Remove a message queue :
 */
static void
rmMsq(int *pmsqid) {

    if ( msgctl(*pmsqid,IPC_RMID,0) == -1 ) {
        fprintf(stderr,"%s: msgctl(%d,IPC_RMID)\n",
            strerror(errno),*pmsqid);
        *pmsqid = -1;
    }
}

/*
 * Queue a Message :
 */
static void
sendMsg(Msg *msg,int msgsz) {
    int msz = msgsz - sizeof(long);
    int z;

    do  {
        z = msgsnd(msqid,&msg->generic,msz,0);
    } while ( z == -1 && errno == EINTR );

    if ( z == -1 ) {
        fprintf(stderr,"%s: msgsnd() mtype=%ld\n",
            strerror(errno),msg->generic.mtype);
        exit(13);
    }
}

/*
 * Receive a Message :
 */
static void
recvMsg(Msg *msg,long mtype) {
    int msz = sizeof *msg - sizeof(long);
    int z;

    do  {
        z = msgrcv(msqid,&msg->generic,msz,mtype,0);
    } while ( z == -1 && errno == EINTR );

    if ( z < 0 ) {
        fprintf(stderr,"%s: msgrcv() mtype=%ld\n",
            strerror(errno),mtype);
        exit(13);
    }
}        

/*
 * Paging server :
 */
static void
server(void) {
    Msg req;

    for (;;) {
        recvMsg(&req,-100L); /* Recv in priority sequence */
        if ( !strcasecmp(req.page.phone,"shutdown") )
            break;
        printf("ATDT%s,,%s\n",
            req.page.phone,
            req.page.callbk);
        sleep(2);           /* Simulate modem delay */
    }
    rmMsq(&msqid);          /* Server removes msgq */
}

/*
 * Client code:
 */
static void
client(int argc,char *argv[]) {
    int x;
    char *cp;
    Msg req;    /* Page request message */

    req.page.mtype = cmdopt_p;  /* Priority */

    for ( x=optind; x<argc; ++x ) { /* Queue each arg */
        while ( (cp = strchr(argv[x],'x')) != 0
          ||    (cp = strchr(argv[x],'X')) != 0 )
            *cp = '*';  /* Dial * instead of 'X' */

        /* Slash separates call from callback number */
        if ( (cp = strchr(argv[x],'/')) != 0 ) {
            *cp = 0;
            strncpy(req.page.phone,argv[x],
                sizeof req.page.phone)
                [sizeof req.page.phone-1] = 0;
            strncpy(req.page.callbk,++cp,
                sizeof req.page.callbk)
                [sizeof req.page.callbk-1] = 0;
            sendMsg(&req,sizeof req.page);
        } else
            fprintf(stderr,
                "No call back number given! (%s)\n",
                argv[x]);
    }
}

/*
 * Paging Program:
 */
int
main(int argc,char *argv[]) {
    int rc = 0;
    int optch;
    const char cmdopts[] = "s:k:p:h";

    /*
     * Parse command line arguments :
     */
    while ( (optch = getopt(argc,argv,cmdopts)) != -1 )
        switch ( optch ) {
        case 'h' :  /* -h */
            usage(argv[0]);
            exit(0);
        case 's' :  /* -s /dev/tty */
            cmdopt_s = optarg;
            break;
        case 'k' :  /* -k key */
            cmdopt_k = (key_t) atoi(optarg);
            break;
        case 'p' :  /* -p priority (1 to 100) */
            cmdopt_p = atoi(optarg);
            if ( cmdopt_p < 1 )
                cmdopt_p = 1;
            else if ( cmdopt_p > 100 )
                cmdopt_p = 100;
            break;
        default  :
            rc = 1;
        }

    /*
     * Test for command line argument errors:
     */
    if ( rc )
        exit(1);        /* Bad command options */
    if ( cmdopt_s && optind < argc ) {
        fputs("Server mode does not accept arguments.\n",
            stderr);
        usage(argv[0]);
        exit(1);
    }
    if ( !cmdopt_k ) {
        fputs("Must supply IPC key in -k option.\n",
            stderr);
        usage(argv[0]);
        exit(1);
    }

    /*
     * Now perform server/client function:
     */
    if ( cmdopt_s ) {
        msqid = createMsq(cmdopt_k,0666);
        server();
    } else {
        msqid = createMsq(cmdopt_k,0);
        client(argc,argv);
    }

    return 0;
}    

/* End pager.c */
