#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/*
 * Trace by TRACE_LEVEL & TRACE_SUBSYS macro :
 */
#define TRACE_SUBSYSF(level,subsys,msg) { \
    if ( testSubsystem(level,subsys) ) { \
        printf("%s @ %u : ",__FILE__,__LINE__); \
        printf msg; \
        fflush(stdout); \
    } \
}

/*
 * Support function for TRACE_SUBSYSF() macro :
 *
 * Environment variables :
 *  TRACE_LEVEL=n           Sets debug level
 *  TRACE_SUBSYS=sys1,sys2  Defines which subsystems will
 *                          be traced.
 *
 * If TRACE_LEVEL is defined, but TRACE_SUBSYS is not, then
 * all subsystems will be traced at the appropriate level.
 */
static int
testSubsystem(short level,const char *subsys) {
    char *cp;                     /* Work pointer */
    char vbuf[128];               /* Buffer for variable */
    static short trace_level = -1;/* Trace level after init */
    static char *trace_subsys = 0;/* Pointer to environment val*/

    /*
     * One time initialization : Test for the presence
     * of the environment variables TRACE_LEVEL and
     * TRACE_SUBSYS.
     */
    if ( trace_level == -1 ) {
        trace_subsys = getenv("TRACE_SUBSYS"); /* Get variable */
        if ( (cp = getenv("TRACE_LEVEL")) != 0 )
            trace_level = atoi(cp);         /* Trace level */
        else
            trace_level = 0;                /* No trace */
    }

    /*
     * If the TRACE_LEVEL is lower than this macro
     * call, then return false :
     */
    if ( trace_level < level )    /* Tracing at lower lvl? */
        return 0;                 /* Yes, No trace required */

    /*
     * Return TRUE if no TRACE_SUBSYS environment
     * value is defined :
     */
    if ( !trace_subsys )          /* TRACE_SUBSYS defined? */
        return 1;                 /* No, Trace ALL subsystems */

    /*
     * Copy string so we don't modify env. variable :
     */
    strncpy(vbuf,trace_subsys,sizeof vbuf);
    vbuf[sizeof vbuf - 1] = 0;    /* Enforce nul byte */

    /*
     * Scan if we have a matching subsystem token :
     */
    for ( cp=strtok(vbuf,","); cp != 0; cp=strtok(NULL,",") )
        if ( !strcmp(subsys,cp) ) /* Compare strings? */
            return 1;             /* Yes, trace this call */
    return 0;                     /* Not in trace list */
}

int main(int argc,char *argv[]) {

    TRACE_SUBSYSF(5,"SYSA",("argv[0]='%s'\n",argv[0]));
    puts("Program 09LST02.c");
    TRACE_SUBSYSF(3,"SYSB",("argc=%d\n",argc));
    return 0;
}
