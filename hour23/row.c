/* row.c :
 * Sams Teach Yourself LINUX Programming in 24 Hours
 * ISBN 0-672-31582-3
 * Warren W. Gay VE3WWG
 *
 * This source module manages Row objects:
 */
static const char rcsid[] =
    "$Id: row.c,v 1.1 1999/02/20 21:32:09 wwg Exp $";

#include "server.h"

long RowCount = 0L;    /* Value used for COUNT(*) ref */

/*
 * Create a new empty Row Object:
 *
 * ARGS:
 *  none
 *
 * RETURNS:
 *  ptr     To new Row object. Use deleteRow() to
 *          delete it later.
 */
Row *
newRow(void) {
    Row *row = (Row *) malloc(sizeof *row);

    initRow(row);          /* Initialize its contents */
    return row;           /* Return pointer to object */
}

/*
 * Initialize a Row Object :
 *
 * ARGS:
 *  row     output  Row object to initialize. The
 *                  current object contents are
 *                  assumed to be garbage.
 *
 * RETURNS:
 *  ptr     To the initialized Row object.
 */
Row *
initRow(Row *row) {
    row->pw.pw_name = NULL;
    row->pw.pw_passwd = NULL;
    row->pw.pw_uid = (uid_t) 0;
    row->pw.pw_gid = (gid_t) 0;
    row->pw.pw_gecos = NULL;
    row->pw.pw_dir = NULL;
    row->pw.pw_shell = NULL;
    return row;
}    

/*
 * Clone new Row contents from struct passwd.
 *
 * ARGS:
 *  dest    output  Row object to receive the
 *                  contents of the src object.
 *  src     input   struct passwd data that is
 *                  to be copied to the dest row.
 *
 * RETURNS:
 *  ptr     To the dest Row object.
 */
Row *
passwdRow(Row *dest,struct passwd *src) {
    dest->pw.pw_name = strdup(src->pw_name);
    dest->pw.pw_passwd = strdup(src->pw_passwd);
    dest->pw.pw_uid = src->pw_uid;
    dest->pw.pw_gid = src->pw_gid;
    dest->pw.pw_gecos = strdup(src->pw_gecos);
    dest->pw.pw_dir = strdup(src->pw_dir);
    dest->pw.pw_shell = strdup(src->pw_shell);
    return dest;
}

/*
 * Clone new Row contents from another Row object:
 *
 * ARGS:
 *  dest    output  Row object to receive the
 *                  contents of the src object.
 *  src     input   Source Row data that is
 *                  to be copied to the dest row.
 *
 * RETURNS:
 *  ptr     To the dest Row object.
 */
Row *
cloneRow(Row *dest,Row *src) {
    passwdRow(dest,&src->pw);
    return dest;
}

/*
 * Free the contents, and NULL out the pointer:
 *
 * ARGS:
 *  freep   I/O     Pointer to (char *) to free
 *                  if non-null. If freed, the
 *                  value at *freep is set to
 *                  NULL.
 */
static void
Free(char **freep) {
    if ( *freep )
        free(*freep);
    *freep = NULL;
}

/*
 * Dispose of the contents of a Row object, but not
 * the row object itself. The state of the
 * remaining Row object is "garbage".
 *
 * ARGS:
 *  row     output  Pointer to the object that
 *                  is to be "disposed" -- i.e.
 *                  partially destructed.
 * RETURNS:
 *  ptr     The row object that was disposed of.
 */
Row *
disposeRow(Row *row) {
    Free(&row->pw.pw_name);
    Free(&row->pw.pw_passwd);
    Free(&row->pw.pw_gecos);
    Free(&row->pw.pw_dir);
    Free(&row->pw.pw_shell);
    return row;
}

/*
 * Delete a Row Object:
 *
 * ARGS:
 *  row     I/O     The pointer to the row object to be
 *                  destructed. This must be a pointer
 *                  returned previously by newRow().
 */
void
deleteRow(Row *row) {
    disposeRow(row);    /* Dispose of contents */
    free(row);          /* Destruct container */
}

/*
 * Return string rep. of (long) value :
 *
 * ARGS:
 *  lv      input   The long value to be converted into
 *                  string format.
 *
 * RETURNS:
 *  ptr     To the allocated string containing the the
 *          string form of the long value.
 */
char *
longDup(long lv) {
    char buf[64];

    sprintf(buf,"%ld",lv);    /* Simple long -> ASCII */
    return strdup(buf);    /* Return allocated string */
}

/*
 * Get a column from a row :
 *
 * ARGS:
 *  row         input   The row object to extract the
 *                      value from.
 *  colname     input   The column name required.
 *
 * RETURNS:
 *  ptr     A pointer to an allocated string holding the
 *          value requested. The caller must free this
 *          value when it is no longer required.
 */
char *
gcolRow(Row *row,const char *colname) {
    short colx;
    
    /*
     * The "COUNT(*)" reference must come from the
     * global value RowCount, that has been established
     * before we use it here:
     */
    if ( (colx = colIndex(colname)) <= COLX_COUNT )
        return longDup(RowCount);

    /*
     * All other row values actually come from the row
     * itself:
     */
    switch ( colx ) {
    case COLX_PW_NAME :
        return strdup(row->pw.pw_name);
    case COLX_PW_PASSWORD :
        return strdup(row->pw.pw_passwd);
    case COLX_PW_UID :
        return longDup(row->pw.pw_uid);
    case COLX_PW_GID :
        return longDup(row->pw.pw_gid);
    case COLX_PW_GECOS :
        return strdup(row->pw.pw_gecos);
    case COLX_PW_DIR :
        return strdup(row->pw.pw_dir);
    case COLX_PW_SHELL :
        return strdup(row->pw.pw_shell);
    }
   return NULL;     /* Unknown column */
}

/*
 * Put a value into a row column :
 *
 * ARGS:
 *  row         output  The Row object to be modified
 *                      with the new column value.
 *  colname     input   The name of the column to
 *                      update.
 *  str         input   The ASCII string form of the
 *                      column data.
 * RETURNS:
 *  Ptr     Pointer to the row affected is returned.
 *  NULL    Unrecognized column name given.
 */
Row *
pcolRow(Row *row,const char *colname,const char *str) {
    short colx;             /* Column index */
    char **sp;              /* Ptr to string pointer */
    char *cp;               /* Work pointer */
    long lv;                /* Temp. long value */
    
    /*
     * Look up the column to be updated:
     */
    colx = colIndex(colname);

    switch ( colx ) {
    case COLX_PW_NAME :
        sp = &row->pw.pw_name;
        break;
    case COLX_PW_PASSWORD :
        sp = &row->pw.pw_passwd;
        break;
    case COLX_PW_UID :
        lv = strtoul(str,&cp,10);
        if ( *cp )
            return NULL;                    /* Failed */
        row->pw.pw_uid = (uid_t) lv;
        return row;
    case COLX_PW_GID :
        lv = strtoul(str,&cp,10);
        if ( *cp )
            return NULL;                    /* Failed */
        row->pw.pw_gid = (uid_t) lv;
        return row;
    case COLX_PW_GECOS :
        sp = &row->pw.pw_gecos;
        break;
    case COLX_PW_DIR :
        sp = &row->pw.pw_dir;
        break;
    case COLX_PW_SHELL :
        sp = &row->pw.pw_shell;
        break;
    default :
        return NULL;               /* Unknown column */
    }

    /*
     * All string types are updated here:
     */
    Free(sp);                  /* Free prior contents */
    *sp = strdup(str);    /* Replace with new content */
    return row;
}

/* end row.c */
