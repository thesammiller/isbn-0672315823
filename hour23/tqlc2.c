/* tqlc2.c :
 * Sams Teach Yourself LINUX Programming in 24 Hours
 * ISBN 0-672-31582-3
 * Warren W. Gay VE3WWG
 */
static const char rcsid[] =
    "$Id: tqlc2.c,v 1.1 1999/02/20 21:32:09 wwg Exp $";

#include "tqlc.h"

/*
 * This value is used when the row data is too wide
 * to display across. This value represents the
 * widest column name.
 */
static int name_width = 0;

/*
 * Display column headings, across if possible,
 * otherwise we label the headings for a
 * vertical display instead:
 *
 * r1   - Holds the max column widths for all
 *        rows of data coming our way.
 * r2   - Holds the list of column heading
 *        names.
 */
static Bool
dispHdgs(Resp1 *r1,Resp2 *r2) {
    short x;
    char *p;
    int w;
    int line_width = 0;
    Bool bAcross = True;    /* Assume across */

    putchar('\n');

    /*
     * Sum up the width if we go across :
     */
    for ( x=0; x<r1->n_slist; ++x )
        line_width += r1->col_width[x] + 1;

    /*
     * If we go over 79 columns, switch
     * to displaying vertically:
     */
    if ( line_width > 79 )
        bAcross = False;

    /*
     * Display headings:
     */
    if ( bAcross ) {
        /*
         * Show column headings across the
         * screen:
         */
        p = r2->colhdg;
        for ( x=0; x<r1->n_slist; ++x ) {
            w = r1->col_width[x];
            printf("%-*.*s ",w,w,p);
            p += strlen(p) + 1;
        }
        puts("\n");

    } else {
        /*
         * Show column headings vertically,
         * so we must now determine the
         * max width of all column names:
         */
        name_width = 0;
        p = r2->colhdg;
        for ( x=0; x<r1->n_slist; ++x ) {
            w = strlen(p);
            p += w + 1;
            if ( w > name_width )
                name_width = w;
        }
        /*
         * Insist on a minimum width
         * of 7 to fit "COLUMN":
         */
        if ( name_width < 7 )
            name_width = 7;
        /*
         * Now display the vertical
         * headings:
         */
        printf("%-*.*s: VALUE\n",
            name_width,
            name_width,
            "COLUMN");
    }

    return bAcross;
}

/*
 * Display Column Data :
 *
 * r1   - holds column widths
 * r2   - holds column headings
 * rdat - holds column data
 * bAcross -
 *      True    - show across
 *      False   - show vertically
 */
static void
dispDat(Resp1 *r1,Resp2 *r2,Resp3 *rdat,Bool bAcross) {
    int x;
    char *p, *cp;
    int w;

    /*
     * The strings for each column's data are in
     * packed one after each other's NUL byte:
     */
    p = rdat->coldat;

    if ( bAcross ) {
        /*
         * Show data across:
         */
        for ( x=0; x<r1->n_slist; ++x ) {
            w = r1->col_width[x];
            printf("%-*.*s%s",
                w,w,p,
                x+1<r1->n_slist?" ":"");
            p += strlen(p) + 1;
        }
        putchar('\n');

    } else {
        /*
         * Show data vertically :
         */
        cp = r2->colhdg;
        putchar('\n');
        for ( x=0; x<r1->n_slist; ++x ) {
            printf("%-*.*s: %s\n",
                name_width,
                name_width,
                cp,
                p);
            cp += strlen(cp) + 1;
            p += strlen(p) + 1;
        }
    }
}

/*
 * Process Column Data :
 *
 * r1   - usually has R_COLHDR
 *      - but can be R_END if only COUNT(*)
 *        data is to be displayed.
 */
int
doColHdr(Resp1 *r1) {
    Resp r;
    Resp2 r2;
    Bool bAcross;

    /*
     * Check if we just got the summary :
     */
    if ( r1->rtype == R_END ) {
        /* Just a count returned */
        r.end = ((Resp *)r1)->end;
        goto count;
    }

    /*
     * Fetch Column Headings :
     */
    if ( rcvMsq(msqid,&r,0) )
        return -1;

    TRACEF(1,MAIN,("Received response rtype=%d\n",
        r.com.rtype))

    switch ( r.com.rtype ) {
    case R_COLHDG :
        r2 = r.colhdg;  /* Save this info */
        break;          /* Headings */
    case R_END :
        goto count;     /* Just COUNT(*) */
    default :    
        return -1;      /* Server problem */
    }

    /*
     * We have the column widths in r1
     * and now the column headings in
     * r2. Display the headings:
     */
    bAcross = dispHdgs(r1,&r.colhdg);

    /*
     * Now display all row data:
     */
    do  {
        /*
         * Get next row of data :
         */
        if ( rcvMsq(msqid,&r,0) )
            return -1;

        TRACEF(1,MAIN,("Received response rtype=%d\n",
            r.com.rtype))

        /*
         * Dispatch according to received msg:
         */
        switch ( r.com.rtype ) {
        case R_COLDAT :
            /* Display row data */
            dispDat(r1,&r2,&r.coldat,bAcross);
            break;
        case R_END :
            break;
        default :    
            return -1;
        }
    } while ( r.com.rtype == R_COLDAT );
    
    /*
     * Display Summary Count :
     */
count:
    if ( r.com.rtype == R_END )
        printf("\n%ld Rows\n",r.end.rows);

    else if ( r.com.rtype == R_ERROR )
        printf("\nSQL ERROR %d: %s\n",
            r.error.sqler,
            sqErrorText(r.error.sqler));

    else {
        printf("\nPROTOCOL ERROR: RTYPE=%d\n",r.com.rtype);
        return -1;
    }

    return 0;
}

/* end tqlc2.c */
