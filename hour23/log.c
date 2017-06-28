/* log.c :
 * Sams Teach Yourself LINUX Programming in 24 Hours
 * ISBN 0-672-31582-3
 * Warren W. Gay VE3WWG
 */
static const char rcsid[] =
    "$Id: log.c,v 1.1 1999/02/20 21:32:09 wwg Exp $";

#include "tql.h"

static FILE *lf = NULL;     /* Current log file */

/*
 * Use stderr for the log file:
 *
 * This function is called by the client program
 * to log messages to stderr, rather than a file.
 */
void
logStderr(void) {

    if ( lf && lf != stderr )
        fclose(lf);
    lf = stderr;
}

/*
 * Format a log time :
 */
char *
logTime(void) {
    static char buf[128];
    struct tm *tp;
    time_t t;
    static char *logfmt = NULL;

    /*
     * The first time this function is called we
     * establish a default for the environment
     * variable, if one is not defined.
     */
    if ( logfmt == NULL ) {         /* First time? */
        setenv(LOGFMT,LOGFMTDFT,0); /* Set default */
        logfmt = getenv(LOGFMT);    /* Get value */
    }

    /*
     * Obtain the time, and call strftime()
     * to format a date and time stamp.
     */
    time(&t);                       /* Get time */
    tp = localtime(&t);             /* Local time */
    strftime(buf,sizeof buf-1,logfmt,tp); /* Format */

    return buf;     /* Return ptr to static buf[] */
}

/*
 * Do printf() styled logging to a log file.
 *
 * If logging to stderr, we do not format a time
 * stamp. Entries logged to the file however do
 * get a date and time stamp.
 *
 * This routine is called by logf().
 */
void
vlogf(const char *format,va_list ap) {
    char bigbuf[2048];
    static Bool nl = True;
    int e = errno;              /* Save errno for appl */

    /*
     * If no log file is open presently, we open
     * it now.
     */
    if ( lf == NULL && (lf = fopen(TQLLOG,"w")) == NULL )
        lf = stderr;            /* Try stderr */

    /*
     * If not logging to stderr, we format and
     * include a date and time stamp:
     */
    if ( lf != stderr && nl != 0 )
        strcat(strcpy(bigbuf,logTime())," ");
    else
        *bigbuf = 0;            /* No time stamp */

    /*
     * Now finish the log entry in the string
     * buffer to be logged:
     */
    vsprintf(bigbuf+strlen(bigbuf),format,ap);

    /*
     * Write the log entry to the file/stderr:
     */
    fputs(bigbuf,lf);
    fflush(lf);                 /* Flush */

    /*
     * Don't put out time stamp next time if no
     * newline character has been written yet.
     */
    nl = ( strchr(bigbuf,'\n') != 0 ) ? True : False;
    errno = e;                  /* Restore errno */
}

/*
 * Format a log file entry:
 */
void
logf(const char *format,...) {
    va_list ap;
    int e = errno;              /* Save errno */

    /*
     * The work is done by vlogf()
     */
    va_start(ap,format);
    vlogf(format,ap);
    va_end(ap);

    errno = e;                  /* Restore errno */
}

/*
 * Close the log file :
 */
void
logClose(void) {
    if ( lf != NULL && lf != stderr )
        fclose(lf);
    lf = NULL;
}

/*
 * Flush the current log file:
 */
void
logFlush(void) {
    if ( lf )
        fflush(lf);
}

/* end log.c */
