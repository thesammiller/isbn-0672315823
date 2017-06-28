/* trace.c :
 * Sams Teach Yourself LINUX Programming in 24 Hours
 * ISBN 0-672-31582-3
 * Warren W. Gay VE3WWG
 *
 * This module provides subsystem trace support:
 */
static const char rcsid[] =
    "$Id: trace.c,v 1.1 1999/02/20 21:32:09 wwg Exp $";

#include "tql.h"

char *trace_subsys = 0;             /* Trace subsystems */
int trace_level = -1;               /* Trace level */

/*
 * Trace support :
 *
 * ARGS:
 *  level       input   Debug level of this trace (0-9)
 *  subsys      input   Subsystem name of this trace
 *
 * RETURNS:
 *  True    Do trace
 *  False   Do not trace
 */
Bool
traceSubsys(short level,const char *subsys) {
    char *cp;           /* Work pointer */
    char vbuf[128];     /* Buffer for variable */

    /*
     * One time initialization : Test for the presence
     * of the environment variables TRACE_LEVEL and
     * TRACE_SUBSYS.
     */
    if ( trace_level == -1 ) {
        trace_subsys = getenv("TRACE_SUBSYS");
        if ( (cp = getenv("TRACE_LEVEL")) != 0 )
            trace_level = atoi(cp); /* Trace level required */
        else
            trace_level = 0;        /* No trace */
    }

    /*
     * If the TRACE_LEVEL is lower than the macro
     * call, then return False.
     */
    if ( trace_level < level )
        return False;               /* Don't trace */

    /*
     * Return True if no TRACE_SUBSYS environment
     * value is defined.
     */
    if ( !trace_subsys )
        return True;                /* Trace all */
        
    /*
     * Copy string so we don't modify env. variable:
     */
    strncpy(vbuf,trace_subsys,sizeof vbuf-1)
        [sizeof vbuf-1] = 0;

    /*
     * Scan if we have a matching subsystem token :
     */
    for ( cp=strtok(vbuf,","); cp != 0; cp=strtok(0,",") )
        if ( !strcasecmp(subsys,cp) )
            return True;            /* Do trace */

    return False;                   /* Don't trace */
}

/* end trace.c */
