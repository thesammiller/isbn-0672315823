/* tql.h :
 * Sams Teach Yourself LINUX Programming in 24 Hours
 * ISBN 0-672-31582-3
 * Warren W. Gay VE3WWG
 * $Id: tql.h,v 1.2 1999/02/20 21:34:38 wwg Exp $
 */
#ifndef _tql_h_
#define _tql_h_ "$Revision: 1.2 $"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include <malloc.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <memory.h>
#include <pwd.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>

/*
 * Software Versions:
 */
#define TQLC_VERSION "1.0"      /* Client Program version */
#define TQLD_VERSION "1.0"      /* Server version */

/*
 * Defaults :
 */
#define TQLLOG      "tql.log"   /* TQL log file */
#define LOGFMTDFT   "%Y/%m/%d %H:%M" /* Log time format */
#define SKEYDFLT    "0x57305536"/* Default IPC Key Value */

/*
 * Environment Variables:
 */
#define TQLKEY      "TQLKEY"    /* IPC Key Env Variable */
#define LOGFMT      "TQLLOGFMT" /* TQL log time format */
#define TRACE_LEVEL "TRACE_LEVEL"   /* Trace level 0-9 */
#define TRACE_SUBSYS "TRACE_SUBSYS" /* Trace subsystems */

/*
 * Limits :
 */
#define MAXSQL          2048        /* Bytes */
#define MAX_SELLIST     16      /* Max select cols */
#define MAX_FROMLIST    16      /* Max from tables */
#define MAX_WHRECLSE    16      /* Max where clauses */
#define MAX_ORDERBY     16      /* Max order by refs */

/*
 * Tokens :
 */
#define TKN_END         0       /* No token */
#define TKN_EOF         1       /* End input */
#define TKN_STRING      2       /* String constant */
#define TKN_IDENT       3       /* Identifier */
#define TKN_NUMBER      4       /* Number */
#define TKN_OPR         5       /* Operator */
#define TKN_NE          6       /* Not equal */
#define TKN_LE          7       /* <= */
#define TKN_GE          8       /* >= */
#define TKN_SELECT      9       /* SELECT keyword */
#define TKN_FROM        10      /* FROM keyword */
#define TKN_WHERE       11      /* WHERE keyword */
#define TKN_AND         12      /* AND keyword */
#define TKN_OR          13      /* OR keyword */
#define TKN_ORDER       14      /* ORDER keyword */
#define TKN_BY          15      /* BY keyword */
#define TKN_COUNT       16      /* COUNT keyword */
#define TKN_SHUTDN      17      /* SHUTDOWN keyword */

/*
 * TQL SQL errors codes:
 */
#define SQE_OK          0       /* Successful */
#define SQE_SYNTAX      (-1)    /* Unsupported syntax */
#define SQE_COLNAME     (-2)    /* Unsupported column name */
#define SQE_COLREF      (-3)    /* Bad col ref */
#define SQE_NOTABLE     (-4)    /* Unable to open table */
#define SQE_IOERR       (-5)    /* I/O Error */
#define SQE_SORT        (-6)    /* Sort failure */
#define SQE_MULFROM     (-7)    /* Multiple from tables */
#define SQE_END         (-999)  /* End of errors list */

/*
 * General Macros:
 */
#define NUL         ((char)0)   /* Null Byte */
#define True            1       /* Boolean True */
#define False           0       /* Boolean False */

/*
 * Data Types:
 */
typedef short           Bool;

/*
 * Server Request Message:
 */
typedef struct {
    long    mtype;                      /* Message Type */
    long    PID;                        /* PID of sender */
    char    sql[MAXSQL];                /* Message */
} Msg;

/*
 * Server Error Response:
 */
#define R_ERROR         0               /* Resp0 */
typedef struct {
    long    mtype;                      /* PID of client */
    short   rtype;                      /* R_ERROR */
    int     sqler;                      /* SQL Error */
} Resp0;

/*
 * Server Column Widths Response:
 */
#define R_COLHDR        1               /* Resp1 */
typedef struct {
    long    mtype;                      /* PID of client */
    short   rtype;                      /* R_COLHDR */
    short   n_slist;                    /* # columns */
    int     col_width[MAX_SELLIST];     /* Col widths */
} Resp1;

/*
 * Server Column Headings Response:
 */
#define R_COLHDG        2               /* Resp2 */
typedef struct {
    long    mtype;                      /* PID of client */
    short   rtype;                      /* R_COLHDG */
    char    colhdg[MAXSQL];
} Resp2;

/*
 * Server Column Data Response:
 */
#define R_COLDAT        3               /* Resp3 */
typedef struct {
    long    mtype;                      /* PID of client */
    short   rtype;                      /* R_COLDAT */
    char    coldat[MAXSQL];
} Resp3;

/*
 * Server Row Count Response:
 */
#define R_END           4               /* Resp4 */
typedef struct {
    long    mtype;                      /* PID of client */
    short   rtype;                      /* R_END */
    long    rows;                       /* Count of rows */
} Resp4;

/*
 * Union of all Server messages to Client:
 */
typedef union {
    struct  {
        long    mtype;                  /* Message type (PID) */
        short   rtype;                  /* Response type */
    }       com;                        /* Fields in common */
    Resp0   error;                      /* R_ERROR  */
    Resp1   colhdr;                     /* R_COLHDR */
    Resp2   colhdg;                     /* R_COLHDG */
    Resp3   coldat;                     /* R_COLDAT */
    Resp4   end;                        /* R_END */
} Resp;                                 /* Response */

/*
 * External Function Prototypes:
 */
extern char *sqErrorText(int sqerr);

extern void logStderr(void);
extern char *logTime(void);
extern void vlogf(const char *format,va_list ap);
extern void logf(const char *format,...);
extern void logClose(void);
extern void logFlush(void);

extern int newMsq(char **pskey,key_t *pkey,int msgflgs);
extern int delMsq(int msqid);
extern int sendMsq(int msqid,void *data,size_t bytes,int flgs);
extern int snd2Msq(int msqid,void *ptr1,void *ptr2,int flgs);
extern int rcvMsq(int msqid,Resp *r,int flags);

/*
 * Debug and Trace Facilities :
 *
 * TRACE: Just traces where the execution has been.
 */
#define TRACE(level,subsys) { \
    if ( traceSubsys(level,#subsys) ) { \
        logf("%s @ %u : trace\n",__FILE__,__LINE__); \
    } \
}

/*
 * Formatted Trace:
 */
#define TRACEF(level,subsys,msg) { \
    if ( traceSubsys(level,#subsys) ) { \
        logf("%s @ %u : ",__FILE__,__LINE__); \
        logf msg; \
    } \
}

/*
 * Trace support :
 */
extern char *trace_subsys;
extern int trace_level;
extern Bool traceSubsys(short level,const char *subsys);

#endif /* _tql_h */

/* end tql.h */
