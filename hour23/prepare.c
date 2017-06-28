/* prepare.c :
 * Sams Teach Yourself LINUX Programming in 24 Hours
 * ISBN 0-672-31582-3
 * Warren W. Gay VE3WWG
 *
 * This module prepares a statement for execution
 * by the server.
 */
static const char rcsid[] =
    "$Id: prepare.c,v 1.1 1999/02/20 21:32:09 wwg Exp $";

#include "server.h"

/*
 * SELECT Column Names:
 */
static struct {
    char    *colname;           /* Column Name */
    Bool    numeric;            /* Numeric or String */
} cols[] = {
    { "count",      True },     /* COUNT(*) */
    { "pw_name",    False },    /* Column Names... */
    { "pw_passwd",  False },
    { "pw_uid",     True },
    { "pw_gid",     True },
    { "pw_gecos",   False },
    { "pw_dir",     False },
    { "pw_shell",   False },
    { NULL,         },
};

/*
 * Return the column name for the index :
 *
 * RETURNS:
 *  ptr     to string name of column
 *  NULL    unknown column name
 */
char *
colName(int colx) {

    if ( colx < 0 || colx > COLX_PW_SHELL )
        return NULL;
    return cols[colx].colname;
}

/*
 * Return True if column is numeric :
 *
 * RETURNS:
 *  True    Column is numeric
 *  False   Column is string
 *
 * NOTES:
 *  False is assumed if the column is not known.
 */
Bool
isNumeric(int colx) {

    if ( colx < 0 || colx > COLX_PW_SHELL )
        return False;
    return cols[colx].numeric;
}

/*
 * Return the index of the column, or -1 :
 *
 * RETURNS:
 *  >= 0    Index of the column in cols[]
 *  -1      Unknown column or aggregate
 */
int
colIndex(const char *col) {
    short x;

    for ( x=0; cols[x].colname != NULL; ++x )
        if ( !strcasecmp(col,cols[x].colname) )
            return x;
    return -1;                  /* Not supported */
}

/*
 * Return the value of a string in decimal:
 *
 * ARGS:
 *  str     input   string to be converted
 *  longp   output  returned value (can be null)
 *
 * RETURNS:
 *  True    Conversion succeeded
 *  False   Conversion failed
 */
Bool
value(const char *str,long *longp) {
    long lv;
    char *cp;

    lv = strtol(str,&cp,10);
    if ( *cp )                  /* Leftovers? */
        return False;           /* then failed */

    if ( longp )                /* Return ptr? */
        *longp = lv;            /* Yes, return value */

    return True;                /* Succeeded */
}

/*
 * Prepare SELECT columns :
 *
 * ARGS:
 *  pp      I/O     parsing pointer
 *  slist   out     digested columns list
 *  pn      out     returned # of columns
 *
 * RETURNS:
 *  True    SELECT columns list is valid
 *  False   SELECT columns list is invalid
 */
static Bool
prepSelList(char **pp,SelList *slist,int *pn) {
    char *p = *pp;  /* Get parse pointer */
    short x;

    *pn = 0;        /* Initially no columns */

    /*
     * The SELECT * case :
     *
     * This causes the slist[] array to be filled with
     * all the columns possible for this table.
     */
    if ( *p == '*' ) {
        /* Load all columns */
        for ( x=COLX_PW_NAME; cols[x].colname; ++x ) {
            slist[*pn].tclass = TKN_IDENT;
            slist[*pn].ident = cols[x].colname;
            slist[(*pn)++].colx = x;
        }
        *pp = ++p;  /* Return parse pointer */
        goto xit;   /* Return results */
    }

    /*
     * The SELECT col,col,col case :
     *
     * Here we parse out each column listed, and record
     * its presence in the slist[] array, keeping count
     * in *pn.
     */
    do  {
        /*
         * Make sure we don't overflow slist[]
         */
        if ( *pn >= MAX_SELLIST )
            return False;       /* Overflow */

        /*
         * Check for column COUNT(*) :
         */
        if ( *p == TKN_COUNT ) {
            /*
             * We have COUNT token...
             */
            if ( p[1] != '('
            ||   p[2] != '*'
            ||   p[3] != ')' )
                return False;       /* Bad syntax */

            /*
             * We have a valid COUNT(*) column reference:
             */
            slist[*pn].tclass = *p;
            slist[(*pn)++].ident = "count";
            p += 4;

        } else if ( *p == TKN_IDENT
               ||   *p == TKN_STRING
               ||   *p == TKN_NUMBER ) {
            /*
             * Here we have a column name, a number
             * token or a string constant token:
             */
            slist[*pn].ident = p + 1;
            slist[(*pn)++].tclass = *p++;
            p += strlen(p) + 1;
        } else
            return False;           /* Bad syntax */

    } while ( *p++ == ',' );        /* Continue if commas */

    /*
     * Backup one for the p++ above, and store the
     * updated parse pointer back into the calling
     * function:
     */
    *pp = --p;

    /*
     * Return successful result:
     */
xit:
    TRACEF(5,SELPREP,("Return True;\n"))
    return True;
}

/*
 * Prepare FROM clause :
 *
 * ARGS:
 *  pp      I/O     Parse pointer
 *  flist   output  Extracted FROM clause info
 *  pn      output  Count of flist[] items
 *
 * RETURNS:
 *  True    A valid FROM clause was parsed
 *  False   Syntax error or other problems
 */
static Bool
prepFromList(char **pp,FromList *flist,int *pn) {
    char *p = *pp;  /* Get parse pointer */

    *pn = 0;        /* Initially no items */

    /*
     * Here we parse the construct:
     *  FROM table, table, table
     */
    do  {
        /*
         * Don't overflow the flist[] array:
         */
        if ( *pn >= MAX_FROMLIST )
            return False;       /* Overflow */

        /*
         * Must have a table name:
         */
        if ( *p == TKN_IDENT ) {
            flist[*pn].table = 0;       /* Set later */
            flist[*pn].row = 0;         /* Set later */
            flist[*pn].tabnam= p + 1;   /* table name */
            flist[(*pn)++].tclass = *p++; /* TKN_IDENT */
            p += strlen(p) + 1;         /* Skip to comma */
        } else
            return False;               /* Syntax error */

    } while ( *p++ == ',' );            /* more if ',' */

    /*
     * Return results:
     */
    *pp = --p;
    TRACEF(5,SELPREP,("Return True;\n"))
    return True;
}

/*
 * Prepare WHERE clause :
 *
 * ARGS:
 *  pp      I/O     Parse pointer
 *  wlist   output  Extracted where clause info.
 *  pn      output  wlist[] item count
 *
 * RETURNS:
 *  True    Valid WHERE clause was extracted.
 *  False   Syntax error or other problems.
 */
static Bool
prepWhereClause(char **pp,WhereClause *wlist,int *pn) {
    char *p = *pp;  /* Get the parse pointer */

    *pn = 0;        /* Initially no items */

    /*
     * Here we extract the following clauses:
     *
     * WHERE op1 opr op2
     * AND/OR op1 opr op2
     * AND/OR op1 opr op2
     */
    do  {
        /*
         * Make sure we don't overflow the wlist[] array:
         */
        if ( *pn >= MAX_WHRECLSE )
            return False;       /* Overflow */

        /*
         * Extract operand one :
         */
        switch ( *p ) {
        case TKN_IDENT :                /* Column ref */
        case TKN_STRING :               /* String constant */
        case TKN_NUMBER :               /* Number */
            wlist[*pn].tcl_op1 = *p++;  /* tclass */
            wlist[*pn].op1 = p;         /* token */
            p += strlen(p) + 1;         /* Skip to next */
            break;
        default :
            return False;               /* Syntax error */
        }

        /*
         * Binary Operator :
         */
        switch ( *p ) {
        case TKN_NE :       /* <>  or  != */
        case TKN_LE :       /* <= */
        case TKN_GE :       /* >= */
        case '<' :
        case '=' :
        case '>' :
            wlist[*pn].tcl_opr = *p++;
            break;
        default :
            return False;               /* Syntax error */
        }

        /*
         * Operand 2 :
         */
        switch ( *p ) {
        case TKN_IDENT :                /* Column ref. */
        case TKN_STRING :               /* String constant */
        case TKN_NUMBER :               /* Number */
            wlist[*pn].tcl_op2 = *p++;  /* tclass */
            wlist[*pn].op2 = p;         /* token */
            p += strlen(p) + 1;         /* skip to next */
            break;
        default :
            return False;               /* Syntax error */
        }

        /*
         * Optional Conjunction :
         */
        switch ( *p ) {
        case TKN_AND :                  /* AND */
        case TKN_OR :                   /* OR */
            wlist[*pn].tcl_conj = *p++; /* tclass */
            break;
        default :
            wlist[*pn].tcl_conj = TKN_END; /* End of where */
        }

    } while ( wlist[(*pn)++].tcl_conj != TKN_END );

    /*
     * Return success:
     */
    TRACEF(5,SELPREP,("Return True;\n"))
    *pp = p;            /* Update caller's parse pointer */
    return True;        /* Success */
}

/*
 * Prepare ORDER BY clause :
 *
 * ARGS:
 *  pp      I/O     Parse pointer
 *  olist   output  Extracted order by info
 *  pn      output  Count of olist[] items
 *
 * RETURNS:
 *  True    An ORDER BY clause was sucessfully extracted
 *  False   Syntax error or other problems.
 */
static Bool
prepOrderBy(char **pp,OrderBy *olist,int *pn) {
    char *p = *pp;  /* Get parse pointer */

    *pn = 0;        /* Initially no items */

    /*
     * Test and skip over BY keyword :
     */
    if ( *p != TKN_BY )
        return False;           /* Missing BY keyword */
    else
       ++p;                     /* Skip BY keyword */

    /*
     * Here we extract column references or numeric
     * references to columns:
     *
     * ORDER BY
     *  column, column, number, column etc.
     *
     * Example:
     *  ORDER BY pw_name,pw_uid,4,5
     */
    do  {
        /*
         * Make sure we do not overflow the olist[] array:
         */
        if ( *pn >= MAX_ORDERBY )
            return False;       /* Overflow */

        /*
         * Operand 1 :
         */
        switch ( *p ) {
        case TKN_IDENT :        /* Column refernce by name */
        case TKN_NUMBER :       /* Reference by number */
            olist[*pn].tclass = *p++;   /* tclass */
            olist[(*pn)++].ref = p;     /* reference */
            p += strlen(p) + 1;         /* skip to next */
            break;
        default :
            return False;       /* Syntax error */
        }

    } while ( *p++ == ',' );

    /*
     * Backup p due to p++ above, and update the caller's
     * parse pointer. Then return success.
     */
    TRACEF(5,SELPREP,("Return True;\n"))
    *pp = --p;                  /* update parse ptr */
    return True;                /* Success */
}

/*
 * Prepare a SELECT statement :
 *
 * ARGS:
 *  msg     input   Contains compacted SQL statement
 *  sel     output  Extracted SELECT info.
 *
 * RETURNS:
 *  True    A valid SELECT statement was prepared.
 *  False   Syntax error or other errors.
 */
static Bool
prepareSelect(Msg *msg,Select *sel) {
    short x;            /* Work index */
    short colx;         /* Column index */
    char *p = msg->sql; /* Parse pointer */
    long lv;            /* long value */

    /*
     * Clear the select extract table:
     */
    memset(sel,0,sizeof sel);

    /*
     * Check that we've been called correctly:
     */
    if ( *p != TKN_SELECT )
        return SQE_SYNTAX;  /* Not a select statement! */
    ++p;                    /* Skip SELECT keyword */

    /*
     * Prepare the SELECT column references list:
     */
    if ( !prepSelList(&p,sel->slist,&sel->n_slist) )
        return SQE_SYNTAX;

    /*
     * Prepare the required FROM clause:
     */
    if ( *p++ != TKN_FROM
      || !prepFromList(&p,sel->flist,&sel->n_flist) )
        return SQE_SYNTAX;

    /*
     * Prepare the optional WHERE clause:
     */
    if ( *p == TKN_WHERE ) {
        ++p;                    /* Skip WHERE token */
        if ( !prepWhereClause(&p,sel->wlist,&sel->n_wlist) )
            return SQE_SYNTAX;  /* Failed */
    } else
        sel->n_wlist = 0;       /* No WHERE clause present */

    /*
     * Prepare the optional ORDER BY clause:
     */
    if ( *p == TKN_ORDER ) {
        ++p;                    /* Skip ORDER keyword */
        if ( !prepOrderBy(&p,sel->olist,&sel->n_olist) )
            return SQE_SYNTAX;  /* Failed */
    } else
        sel->n_olist = 0;       /* No ORDER BY clause */

    /*
     * We should reach the statement end now:
     */
    if ( *p != TKN_END )
        return SQE_SYNTAX;      /* Nope, syntax error */

    /*
     * The SELECT statement has been prepared successfully.
     * However, we must now check the listed column names:
     */
    TRACEF(5,SELPREP,("Have %d columns to select.\n",
        sel->n_slist))

    /*
     * For each column listed that is not a numeric or
     * string constant, make sure it is a known
     * column name:
     */
    for ( x=0; x<sel->n_slist; ++x ) {
        TRACEF(5,SELPREP,
            ("Checking column '%s'\n",sel->slist[x].ident))

        if ( sel->slist[x].tclass == TKN_IDENT ) {
            sel->slist[x].colx = 
                colIndex(sel->slist[x].ident);  /* x or -1 */
            if ( sel->slist[x].colx < 0 )
                return SQE_COLNAME;         /* unknown col */
        } else
            sel->slist[x].colx = -1;        /* constant */
    }

    TRACEF(5,SELPREP,("Have %d tables to select from.\n",
        sel->n_flist))
    TRACEF(5,SELPREP,("Have %d where clauses.\n",
        sel->n_wlist))

    /*
     * Check refs in WHERE clause : here for each reference
     * that is not a constant, make sure the name is a
     * recognized column name. Do not permit the reference
     * to "count" stand.
     */
    for ( x=0; x<sel->n_wlist; ++x ) {    
        if ( sel->wlist[x].tcl_op1 == TKN_IDENT
        &&   colIndex(sel->wlist[x].op1) <= COLX_COUNT )
            return SQE_COLNAME; /* Bad column ref in op1 */

        if ( sel->wlist[x].tcl_op2 == TKN_IDENT
        &&   colIndex(sel->wlist[x].op2) <= COLX_COUNT )
            return SQE_COLNAME; /* Bad column ref in op2 */
    }

    TRACEF(5,SELPREP,("Have %d order by refs.\n",
        sel->n_olist))

    /*
     * Check refs in ORDER BY clause :
     */
    for ( x=0; x<sel->n_olist; ++x ) {
        switch ( sel->olist[x].tclass ) {
        case TKN_IDENT :
            TRACEF(5,SELPREP,("col = '%s'\n",
                sel->olist[x].ref))

            colx = colIndex(sel->olist[x].ref);

            if ( colx < 0 )
                return SQE_COLNAME;        /* Bad col ref */
            sel->olist[x].num = isNumeric(colx);  /* Type */
            sel->olist[x].fldno = colx - 1;      /* Field */
            break;

        case TKN_NUMBER :
            TRACEF(5,SELPREP,("no. = '%s'\n",
                sel->olist[x].ref))

            if ( !value(sel->olist[x].ref,&lv)  /* numbr? */
              || lv < 1L                        /* range? */
              || lv > sel->n_slist )            /* range? */
                return SQE_COLREF;      /* Bad column ref */

            --lv;               /* Zero base the refernce */
            if ( sel->slist[lv].tclass != TKN_IDENT )
                return SQE_COLNAME;      /* only col refs */

            colx = colIndex(sel->slist[lv].ident);
            if ( colx <= COLX_COUNT )
                return SQE_COLNAME;  /* can't do COUNT(*) */

            sel->olist[x].num = isNumeric(colx);/* numeric? */
            sel->olist[x].fldno = colx - 1;   /* Sort field */
            break;

        default : /* This should not happen: bug if it does */
            return SQE_SYNTAX;
        }
    }

    /*
     * Return Success:
     */
    TRACEF(5,SELPREP,("Prepared: SQE_OK.\n"))
    return SQE_OK;
}

/*
 * Prepare a TQL statement :
 *
 * ARGS:
 *  msg     input
 *  sel     output
 *
 * RETURNS:
 *  SQE_OK  Successfully prepared statement
 *  SQE_*   Other TQL SQL errors.
 */
int
prepare(Msg *msg,Select *sel) {

    /*
     * Currently, only the SELECT statement is
     * supported.
     */
    switch ( msg->sql[0] ) {
    case TKN_SELECT :
        return prepareSelect(msg,sel);
        break;
    }

    /*
     * Failed to prepare:
     */
    return False;
}

/* end prepare.c */
