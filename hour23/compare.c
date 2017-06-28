/* compare.c :
 * Sams Teach Yourself LINUX Programming in 24 Hours
 * ISBN 0-672-31582-3
 * Warren W. Gay VE3WWG
 *
 * This module does data comparisons for the WHERE
 * clause support:
 */
static const char rcsid[] =
    "$Id: compare.c,v 1.2 1999/02/20 21:32:09 wwg Exp $";

#include "server.h"

/*
 * Obtain the string value of a reference :
 *
 * ARGS:
 *  tclass  input   Identifies the classification of the
 *                  token passed in "sym".
 *  sym     input   The token string to fetch a value for.
 *  row     input   The row to fetch a column value from.
 *
 * RETURNS:
 *  ptr     The allocated string containing the value
 *          being sought. The caller must free this
 *          string when it is no longer needed.
 * ABORTS:
 *  Aborts can happen if tclass is not a supported
 *  type.
 */
char *
valueOf(char tclass,char *sym,Row *row) {
    char *cp = 0;

    switch ( tclass ) {
    case TKN_STRING :
        cp = strdup(sym);       /* Quoted value string */
        break;
    case TKN_NUMBER :
        cp = strdup(sym);       /* Bare numeric string */
        break;
    case TKN_IDENT :            /* Column in row */
    case TKN_COUNT :            /* COUNT(*) value */
        cp = gcolRow(row,sym);  /* Fetched Value */
    }

    TRACEF(8,COMPARE,("valueOf(%d,%s)='%s';\n",
        tclass,sym,
        cp ? cp : "<NULL>"))

    if ( !cp )
        abort();
    return cp;                  /* Return the value str */
}

/*
 * Numeric Compare :
 *
 * ARGS:
 *  wc      input   Input where clause to be evaluated.
 *  row     input   Row to obtain column info from.
 *
 * RETURNS:
 *  True    Where clause evaluates as True.
 *  False   Where clause evaluates as False.
 */
Bool
numCmp(WhereClause *wc,Row *row) {
    char *s1, *s2, *ep;
    long v1, v2;
    Bool b;
    
    /*
     * Obtain the string values for both
     * operands:
     */
    s1 = valueOf(wc->tcl_op1,wc->op1,row);
    s2 = valueOf(wc->tcl_op2,wc->op2,row);

    /*
     * Convert string 1 to long:
     */
    v1 = strtol(s1,&ep,10);
    if ( *ep )
        v1 = 0;

    /*
     * Convert string 2 to long:
     */
    v2 = strtol(s2,&ep,10);
    if ( *ep )
        v2 = 0;

    /*
     * Free strings we no long need:
     */
    free(s1);
    free(s2);

    TRACEF(5,COMPARE,("numCmp(%ld,%ld,%d(%c))\n",
        v1,v2,wc->tcl_opr,wc->tcl_opr>' '
            ? wc->tcl_opr : ' '))

    /*
     * Perform the numeric comparisons :
     */
    switch ( wc->tcl_opr ) {
    case TKN_NE :
        b = ( v1 != v2 ? True : False );
        break;
    case TKN_LE :
        b = ( v1 <= v2 ? True : False );
        break;
    case TKN_GE :
        b = ( v1 >= v2 ? True : False );
        break;
    case '<' :
        b = ( v1 < v2 ? True : False );
        break;
    case '=' :
        b = ( v1 == v2 ? True : False );
        break;
    case '>' :
        b = ( v1 > v2 ? True : False );
        break;
    default :
        TRACE(1,NUMCMP);
        abort();
    }

    /*
     * Return the boolean result:
     */
    TRACEF(5,COMPARE,("numCmp() => b = %s;\n",
        b ? "TRUE" : "FALSE"))
    return b;
}

/*
 * String Compare :
 *
 * ARGS:
 *  wc      input   Input where clause to be evaluated.
 *  row     input   Row to obtain column info from.
 *
 * RETURNS:
 *  True    Where clause evaluates as True.
 *  False   Where clause evaluates as False.
 */
Bool
strCmp(WhereClause *wc,Row *row) {
    int b;
    char *s1, *s2;
    
    /*
     * Obtain the string values of the two
     * operangs:
     */
    s1 = valueOf(wc->tcl_op1,wc->op1,row);
    s2 = valueOf(wc->tcl_op2,wc->op2,row);

    /*
     * Perform the string comparison:
     */
    b = strcmp(s1,s2);

    /*
     * Convert to a Boolean Result:
     */
    switch ( wc->tcl_opr ) {
    case TKN_NE :
        break;              /* b already set */
    case TKN_LE :
        b = b <= 0;
        break;
    case TKN_GE :
        b = b >= 0;
        break;
    case '<' :
        b = b < 0;
        break;
    case '=' :
        b = !b;
        break;
    case '>' :
        b = b > 0;
        break;
    default :
        TRACEF(1,COMPARE,("tcl_opr = %d;\n",wc->tcl_opr))
        abort();
    }

    /*
     * Free the temporary strings:
     */
    TRACEF(7,COMPARE,("strCmp('%s','%s') => %d;\n",s1,s2,b));
    free(s1);
    free(s2);

    /*
     * Return the Boolean Result:
     */
    return b ? True : False;
}

/*
 * Evaluate one where clause, regardless of types.
 *
 * ARGS:
 *  wc      input   Input where clause to be evaluated.
 *  row     input   Row to obtain column info from.
 *
 * RETURNS:
 *  True    Where clause evaluates as True.
 *  False   Where clause evaluates as False.
 */
Bool
colCmp(WhereClause *wc,Row *row) {

    /*
     * Perform the comparison:
     */
    if ( isNumeric(colIndex(wc->op1)) )
        wc->b = numCmp(wc,row);
    else
        wc->b = strCmp(wc,row);

    /*
     * Return the Boolean result:
     */
    TRACEF(5,COMPARE,("colCmp() => b=%s;\n",
        wc->b ? "TRUE" : "FALSE"))
    return wc->b;
}

/*
 * Evaluate the entire where clause with conjunctions:
 *
 * ARGS:
 *  wc      input   Input where clause to be evaluated.
 *  n       input   The number of clauses in wc.
 *  row     input   Row to obtain column info from.
 *
 * RETURNS:
 *  True    Where clause evaluates as True.
 *  False   Where clause evaluates as False.
 */
Bool
compare(WhereClause *wc,int n,Row *row) {
    short x;
    Bool b = True;

    for ( x=0; x<n; ++x ) {
        if ( x > 0 ) {
            /*
             * If the previous clause used AND, and
             * we have evaluated a False already, then
             * there is no need to go further - the
             * result is going to be False.
             */
            if ( wc[x-1].tcl_conj == TKN_AND && !b )
                return False;
            /*
             * If the previous clause used OR, and
             * we have evaluated a True already, then
             * we already know the result will be
             * True.
             */
            if ( wc[x-1].tcl_conj == TKN_OR && b )
                return True;
        }
        /*
         * Evaluate clause "x" :
         */
        b = colCmp(&wc[x],row);
    }

    /*
     * Return the results of this WHERE clause:
     */
    return b;
}

/* end compare.c */
