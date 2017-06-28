/* sqerr.c :
 * Sams Teach Yourself LINUX Programming in 24 Hours
 * ISBN 0-672-31582-3
 * Warren W. Gay VE3WWG
 *
 * This module provides SQE error to message mapping
 * support.
 */
static const char rcsid[] =
    "$Id: sqerr.c,v 1.1 1999/02/20 21:32:09 wwg Exp $";

#include "tql.h"

/*
 * SQE error mapping table:
 */
static struct {
    int     sqlerr; /* Error code */
    char    *text;  /* Error Text */
} sqerrs[] = {
    { SQE_OK,       "No error" },
    { SQE_SYNTAX,   "Syntax error" },
    { SQE_COLNAME,  "Unsupported column name" },
    { SQE_COLREF,
        "Bad column reference in ORDER BY clause" },
    { SQE_NOTABLE,  "Unable to open table" },
    { SQE_IOERR,    "I/O Error" },
    { SQE_SORT,     "Sort failure" },
    { SQE_MULFROM,  "Multiple tables not yet supported" },
    { SQE_END,      0 }
};

/*
 * Return error text for an SQL error :
 *
 * ARGS:
 *  sqerr   input   SQE_* error
 *
 * RETURNS:
 *  ptr     to error text,
 *          or to the string "Unknown SQL error"
 */
char *
sqErrorText(int sqerr) {
    short x;

    for ( x=0; sqerrs[x].sqlerr != SQE_END; ++x )
        if ( sqerrs[x].sqlerr == sqerr )
            return sqerrs[x].text;

    return "Unknown SQL error";
}

/* end sqerr.c */
