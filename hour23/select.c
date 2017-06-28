/* select.c :
 * Sams Teach Yourself LINUX Programming in 24 Hours
 * ISBN 0-672-31582-3
 * Warren W. Gay VE3WWG
 *
 * This module carries out the prepared SELECT statement
 * on the TQL server.
 */
static const char rcsid[] =
    "$Id: select.c,v 1.1 1999/02/20 21:32:09 wwg Exp $";

#include "server.h"

/*
 * Current SELECT process Object:
 */
typedef struct {
    Table   *table;     /* Temp file */
    short   col_width[COLX_PW_SHELL+1];
    long    rows;       /* Row count */
} Selection;

/*
 * Initialize the Selection Object:
 *
 * ARGS:
 *  _this       output  The object to initialize. The
 *                      present state of the object is
 *                      assumed to be garbage.
 *
 * RETURNS:
 *  Ptr     To the initialized object.
 */
static Selection *
initSelection(Selection *_this) {
    short x;        /* Iterator */

    /*
     * Create a temp file :
     */
    if ( !(_this->table = tqlOpenTable("#TEMP00")) ) {
        logf("%s: tmpfile()!\n",strerror(errno));
        abort();    /* Should not normally fail */
    }
    
    /*
     * Initialize all column widths : The minimum
     * widths for all columns is the width of the
     * column name itself.
     */
    for ( x=0; x<= COLX_PW_SHELL; ++x )
        _this->col_width[x] = strlen(colName(x))
            + (x == COLX_COUNT ? 3 : 0);

    _this->rows = 0;    /* No rows selected yet */
    return _this;
}

/*
 * Dispose of the Selection Object's contents:
 *
 * ARGS:
 *  _this       output  Ptr to the Selection Object to
 *                      dispose of its contents.
 */
static void
disposeSelection(Selection *_this) {
    tqlCloseTable(_this->table);  /* Close & rlse tbl */
    _this->table = NULL;        /* Forget about table */
}

/*
 * Initialize a Select Object:
 *
 * ARGS:
 *  _this       output  The ptr to the object to be
 *                      initialized. The object is
 *                      assume to contain garbage.
 * RETURNS:
 *  ptr     To the initialized object.
 */
Select *
initSelect(Select *_this) {
    memset(_this,0,sizeof *_this);
    return _this;
}

/*
 * Dispose of the contents of the Select object:
 *
 * ARGS:
 *  _this       output  The object whose contents
 *                      are to be disposed of. The
 *                      state of the object is
 *                      "garbage" afterwards.
 * RETURNS:
 *  ptr     To the disposed of object.
 */
Select *
disposeSelect(Select *_this) {
    short x;

    /*
     * The flist[] array has some allocated
     * storage in it that should be closed
     * and released.
     */
    for ( x=0; x<_this->n_flist; ++x ) {
        if ( _this->flist[x].table != NULL ) {
            tqlCloseTable(_this->flist[x].table);
            _this->flist[x].table = NULL; /* For safety */
        }
        if ( _this->flist[x].row != NULL ) {
            deleteRow(_this->flist[x].row);
            _this->flist[x].row = NULL;   /* For safety */
        }
    }
    return _this;
}

/*
 * Put a selected row into the selection pool :
 *
 * ARGS:
 *  _this       output  The Selection object that
 *                      is to be modified and used
 *                      to "put" the row.
 *  row         input   The row to be "put" into
 *                      the selected rows temp
 *                      table.
 */
static void
putSelection(Selection *_this,Row *row) {
    int z;              /* Status int */
    short x;            /* Work index */
    char *cp;           /* Work ptr */
    int slen;           /* String length */
    char tbuf[32];      /* Work buffer */

    /*
     * First write the provided row out to the 
     * temp table.
     */
    if ( (z = tqlWriteTable(_this->table,row)) )
        abort();        /* This shouldn't fail */

    /*
     * Check the string width of each column we
     * just put.
     */
    for ( x=COLX_PW_NAME; x<=COLX_PW_SHELL; ++x ) {
        cp = gcolRow(row,colName(x));    /* Get value */
        slen = strlen(cp);           /* Get its width */
        free(cp);         /* Release the value string */

        if ( slen > _this->col_width[x] )   /* Wider? */
            _this->col_width[x] = slen; /* Yes,adjust */
    }

    /*
     * Update Count Info :
     */
    ++_this->rows;

    /*
     * Check the width of the row count also:
     */
    sprintf(tbuf,"%ld",_this->rows);
    slen = strlen(tbuf);
    if ( slen > _this->col_width[COLX_COUNT] )
        _this->col_width[COLX_COUNT] = slen;
}

/*
 * Carry out the select :
 *
 * ARGS:
 *  sel         I/O     The selection object to perform
 *                      the select with.
 *  client_pid  input   The client program requesting
 *                      a service.
 *
 * RETURNS:
 *
 *
 */
int
xeqSelect(Select *sel,pid_t client_pid) {
    short x;                /* Work index */
    int z;                  /* Work status */
    int sqerr = SQE_OK;     /* TQL SQL error code */
    char *p, *cp;           /* Work pointers */
    Selection s;            /* Selection object */
    Resp1 resp1;            /* Col Hdr Response */
    Resp2 resp2;            /* Col Hdg Response */
    Resp3 resp3;            /* Col Data Response */
    Resp4 resp4;            /* End Response */
    Bool notCount;          /* True if not count */

    initSelection(&s);      /* Initialize object */

    /*
     * First make sure we can open all tables found
     * in the FROM clause.
     */
    if ( sel->n_flist > 1 )
        return SQE_MULFROM; /* Not yet supported */

    for ( x=0; x<sel->n_flist; ++x ) {
        sel->flist[x].table =
            tqlOpenTable(sel->flist[x].tabnam);
        if ( !sel->flist[x].table ) {
            sqerr = SQE_NOTABLE;
            goto xit;       /* Do exit cleanup */
        }
    }

    /*
     * Allocate a row object to each Table in
     * the from list:
     */
    for ( x=0; x<sel->n_flist; ++x )
        sel->flist[x].row = newRow();
        
    /*
     * Select loop :
     */
    for (;;) {
        /*
         * Read in a row from the table:
         */
        if ( (z = tqlReadTable(sel->flist[0].table,
            sel->flist[0].row)) == EOF )
            break;              /* End-File */
        else if ( z )
            goto ioerr;         /* I/O Error */

        /*
         * If the WHERE clause yields a final
         * result of True, we write the row into
         * the temp table.
         */
        if ( compare(sel->wlist,sel->n_wlist,
          sel->flist[0].row) )
            /*
             * Write the row to the temp table:
             */
            putSelection(&s,sel->flist[0].row);
    }

    /*
     * Update the external that is used to define
     * the column COUNT(*) for:
     */
    RowCount = s.rows;

    /*
     * Apply the sort procedure, if the ORDER BY clause
     * was used:
     */
    if ( sel->n_olist > 0 )
        /*
         * We must sort: The end result is that the table
         * s.table is switched to another temp file that
         * contains the sorted result.
         */
        if ( !doOrderBy(s.table,sel->olist,sel->n_olist) ) {
            sqerr = SQE_SORT;          /* Sort failed */
            goto xit;              /* Do exit cleanup */
        }

    /*
     * Send Header Message to Client : This message
     * tells the client program what the column widths
     * are for each of the requested columns:
     */
    resp1.mtype = client_pid;    /* Client process ID */
    resp1.rtype = R_COLHDR;    /* Col Header Response */
    resp1.n_slist = sel->n_slist;     /* # of columns */
    notCount = False;         /* Assume COUNT(*) only */

    /*
     * Set the column widths in the response message:
     */
    for ( x=0; x<sel->n_slist; ++x ) {
        switch ( sel->slist[x].tclass ) {
        case TKN_IDENT :
            resp1.col_width[x] = 
                s.col_width[sel->slist[x].colx];
            if ( colIndex(sel->slist[x].ident)
            !=   COLX_COUNT )
                notCount = True;    /* Not just COUNT(*) */
            break;
        default :
            if ( sel->slist[x].colx != COLX_COUNT )
                cp = valueOf(sel->slist[x].tclass,
                    sel->slist[x].ident,
                    NULL);
            else
                cp = longDup(resp4.rows);
            resp1.col_width[x] = strlen(cp);
            free(cp);
	}
    }

    if ( !notCount )
        goto co;       /* Only sending COUNT(*) value */

    /*
     * Send the heading sizes to client:
     */
    if ( sendMsq(tqld.msqid,&resp1,sizeof resp1,0) )
        goto ioerr;

    /*
     * Send the column headings to Client. Here the
     * names of the columns requested are returned
     * back to the client:
     */
    resp2.mtype = client_pid;    /* Client process ID */
    resp2.rtype = R_COLHDG;       /* Headings message */
    *(p = resp2.colhdg) = NUL;          /* Initialize */

    /*
     * Pack multiple NUL terminated strings to name
     * each column:
     */
    for ( x=0; x<sel->n_slist; ++x ) {
        strcpy(p,sel->slist[x].ident);
        p += strlen(p) + 1;
    }

    /*
     * Send the heading names to client:
     */
    if ( snd2Msq(tqld.msqid,&resp2,p,0) )
        goto ioerr;

    /*
     * Send Column Data to Client :
     */
    resp3.mtype = client_pid;
    resp3.rtype = R_COLDAT;

    if ( tqlRewindTable(s.table) )
        goto ioerr;

    while ( !(z = tqlReadTable(s.table,sel->flist[0].row)) ) {
        *(p = resp3.coldat) = NUL;      /* Initialize */

        for ( x=0; x<sel->n_slist; ++x ) {
            switch ( sel->slist[x].colx ) {
            case COLX_COUNT :         /* Get COUNT(*) */
                cp = longDup(resp4.rows);
                break;
            default :      /* All other column values */
                cp = valueOf(sel->slist[x].tclass,
                    sel->slist[x].ident,
                    sel->flist[0].row);
            }
            strcpy(p,cp);   /* Copy to client message */
            p += strlen(p) + 1; /* Skip over NUL byte */
            free(cp);            /* Free string value */
        }
        /*
         * Send the row data to the client:
         */
        if ( snd2Msq(tqld.msqid,&resp3,p,0) )
            goto ioerr;
    }

    /*
     * The last message to the client after a SELECT
     * is the COUNT(*) value :
     */
co: resp4.mtype = client_pid;    /* Client process ID */
    resp4.rtype = R_END;              /* Last message */
    resp4.rows = RowCount;               /* Row count */

    /*
     * Send the Row Count (END) message to Client:
     */
    if ( sendMsq(tqld.msqid,&resp4,sizeof resp4,0) )
        goto ioerr;

    /*
     * Whew! Success.
     */
    sqerr = SQE_OK;

xit:disposeSelection(&s); /* Dispose of obj s content */
    return sqerr;             /* Return error, if any */

    /*
     * I/O Error :
     */
ioerr:
    sqerr = SQE_IOERR;            /* Report I/O error */
    goto xit;                      /* Do exit cleanup */
}

/* end select.c */
