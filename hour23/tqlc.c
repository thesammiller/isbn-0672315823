/* tqlc.c :
 * Sams Teach Yourself LINUX Programming in 24 Hours
 * ISBN 0-672-31582-3
 * Warren W. Gay VE3WWG
 */
static const char rcsid[] =
    "$Id: tqlc.c,v 1.1 1999/02/20 21:32:09 wwg Exp $";

#include "tqlc.h"

char *skey = SKEYDFLT;  /* Default IPC Key */
key_t key = 0;          /* Binary IPC Key */
int msqid = -1;         /* Message Queue ID */

/*
 * Display usage information:
 */
static void
usage(const char *cmd) {
    char *basename = strrchr(cmd,'/');

    if ( basename )
        ++basename;     /* Skip over '/' */
    else
        basename = (char *) cmd; /* basename already */
    
    printf("Usage:  %s [-x lvl] [-X str] [-h] [-v]\n",
        basename);
    printf("or:     %s [--debug lvl] [--trace str]"
        " [--help] [--version]\n",basename);
    puts("\n    -h  or  --help              This usage info");
    puts("    -v  or  --version           Version info");
    puts("    -x lvl  or  --debug lvl     "
        "Debug/trace level 0-9");
    puts("    -X str  or  --trace str     "
        "Subsystem(s) to trace");
}

/*
 * TQL Client Main Program :
 */
int
main(int argc,char *argv[],const char *env[]) {
    Msg msg;            /* Message struct */
    size_t n;           /* Packed SQL message length */
    size_t n2;          /* sendMsq() message length */
    Resp resp;          /* Response message */
    int cmdopt_x = 0;   /* -x or --debug */
    int cmdopt_h = 0;   /* -h or --help */
    int cmdopt_v = 0;   /* -v or --version */
    char *p;            /* work pointer */
    int rc = 0;         /* Return Code */
    char tbuf[32];      /* Work buffer */
    int optch;          /* Option character */
    int lx;             /* Long option index */
    static const char cmdopts[] = "x:X:hv";
    static struct option lopts[] = {
        { "help", 0, 0, 'h' },
        { "version",0,0,'v' },
        { "debug", 0, 0, 'x' },
        { "trace", 0, 0, 'X' },
        { 0, 0, 0, 0 }
    };

    logStderr();        /* Log to stderr */

    /*
     * Parse command line options if any:
     */
    while ( (optch =
             getopt_long(argc,argv,cmdopts,lopts,&lx)) != -1 )
        switch ( optch ) {
        case 'x' :      /* -x n  or  --debug n */
            cmdopt_x = (int) strtol(optarg,&p,10);
            if ( *p ) {
                logf("Invalid number: -x '%s'\n",optarg);
                rc = 1;
            }
            if ( cmdopt_x < 0 || cmdopt_x > 9 ) {
                logf("Invalid range: -x %ld -- must be 0-9\n",
                    optarg);
                rc = 1;
    	    }
            if ( !rc ) {
                sprintf(tbuf,"%d",cmdopt_x);
                setenv(TRACE_LEVEL,tbuf,True);
            }
            break;
        case 'X' :      /* -X str  or  --trace str */
            setenv(TRACE_SUBSYS,optarg,True);
            break;
        case 'h' :      /* -h  or  --help */
            cmdopt_h = True;
            break;
        case 'v' :      /* -v  or  --version */
            cmdopt_v = True;
            break;
        default :
            rc = 1;     /* Bad options used */
        }

    /*
     * Give help regardless of error(s):
     */
    if ( cmdopt_h ) {
        usage(argv[0]);
        return rc;
    }

    /*
     * If there were command line option errors, then
     * offer some advice before we exit:
     */
    if ( rc ) {
        logf("Use --help for more information.\n");
        return rc;
    }

    /*
     * Give version information if requested:
     */
    if ( cmdopt_v ) {
        printf("TQL Version %s\n",TQLC_VERSION);
        printf("Written by Warren W. Gay VE3WWG\n");
        return rc;
    }

    /*
     * Get the message queue connection to the server :
     */
    if ( (msqid = newMsq(&skey,&key,0)) == -1 ) {
        fputs("No TQL server available.\n",stderr);
        return 1;
    }

    /*
     * Prompt loop:
     */
    msg.mtype = 1L;         /* To server */
    msg.PID = getpid();     /* From our PID */

    while ( (n = lexSQL(msg.sql,MAXSQL)) > 0 ) {
        /*
         * Send the message to the server :
         */
        TRACEF(1,MAIN,("Packed SQL is %d bytes\n",n))
        n2 = sizeof(long) + sizeof(pid_t) + n;
        if ( sendMsq(msqid,&msg,n2,0) )
            break;

        if ( msg.sql[0] == TKN_SHUTDN )
            break;       /* The server will not reply */

        /*
         * Wait for message from the server:
         */
        if ( rcvMsq(msqid,&resp,0) )
            break;

        TRACEF(1,MAIN,("Received response rtype=%d\n",
            resp.com.rtype))

        switch ( resp.com.rtype ) {

            /*
             * An error was received from the server:
             */
        case R_ERROR :
            printf("SQLERROR %d: %s\n",
                resp.error.sqler,
                sqErrorText(resp.error.sqler));
            continue;

            /*
             * The SELECT statement sends us responses
             * R_COLHDR or R_END if only the COUNT(*)
             * is to be displayed.
             */
        case R_COLHDR :
        case R_END :
            if ( doColHdr(&resp.colhdr) )
                goto svrerr;
            break;

            /*
             * This should not happen :
             */
        default :
            printf("Unknown response RTYPE=%d\n",resp.com.rtype);
            goto svrerr;
        }
    }

    /*
     * Exit TQL Client :
     */
    logClose();
    putchar('\n');
    return 0;

    /*
     * Server error exit :
     */
svrerr:
    putchar('\n');
    logf("SERVER ERROR.\n");
    return 13;
}

/* end tql.c */
