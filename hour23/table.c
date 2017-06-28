/* table.c :
 * Sams Teach Yourself LINUX Programming in 24 Hours
 * ISBN 0-672-31582-3
 * Warren W. Gay VE3WWG
 */
static const char rcsid[] =
    "$Id: table.c,v 1.1 1999/02/20 21:32:09 wwg Exp $";

#include "server.h"

/*
 * This routine creates a local copy of the password
 * database so we don't work with the real thing:
 */
static void
copyPasswd(const char *table_name) {
    FILE *etc_pw = fopen("/etc/passwd","r");
    FILE *new_pw = 0;
    char buf[2048];

    if ( !etc_pw ) {
        logf("%s: Unable to copy /etc/passwd to temp file.\n",
            strerror(errno));
        return;         /* We cannot help */
    }

    new_pw = fopen(table_name,"w");
    if ( !new_pw ) {
        logf("%s: opening '%s' for w\n",
            strerror(errno),table_name);
        fclose(etc_pw);
        return;
    }

    while ( fgets(buf,sizeof buf,etc_pw) )
        fputs(buf,new_pw);
    fclose(new_pw);
    fclose(etc_pw);
}

/*
 * Open a table: When /etc/passwd is selected,
 * only open it with read!
 */
Table *
tqlOpenTable(const char *table_name) {
    FILE *f = 0;
    Table *table = 0;
    char *rw = "r";
    struct stat etc_pw;
    struct stat cur_tb;
    int ze, z;

    /*
     * Make sure we know about the real /etc/passwd file:
     * NOTE: Temp tables start with '#'
     */
    if ( *table_name != '#' ) {
        /*
         * Non-temp table :
         */
        if ( (ze = stat("/etc/passwd",&etc_pw)) == -1 )
            logf("%s: stat(/etc/passwd)\n",
                strerror(errno));

        if ( (z = stat(table_name,&cur_tb)) == -1 ) {
            logf("%s: stat(%s)\n",
                strerror(errno),table_name);
            if ( !strcmp(table_name,"passwd") )
                copyPasswd(table_name);
            z = stat(table_name,&cur_tb);
        }

        /*
         * Make sure we only open with "r+" if we know
         * the real /etc/passwd file is not being
         * referenced!
         */
        if ( ze == 0 ) {        /* Got stat on /etc/passwd */
            if ( z == -1 )      /* Cur file does not stat */
                rw = "r+";      /* New table */
            else if ( cur_tb.st_dev != etc_pw.st_dev
                   || cur_tb.st_ino != etc_pw.st_ino )
                 rw = "r+";     /* Not /etc/passwd file */
        }

        /*
         * Log read-only access:
         */
        if ( !strcmp(rw,"r") )
            logf("Will assume read access"
                " only for table %s.\n",
                table_name);

        /*
         * Open the file (table) :
         */
        if ( !(f = fopen(table_name,rw)) )
                return NULL;
    } else if ( !(f = tmpfile()) )
        return NULL;

    /*
     * Create a table entry :
     */
    table = (Table *)malloc(sizeof *table);
    table->name = strdup(table_name);
    table->file = f;

    return table;
}

/*
 * Rewind the table:
 */
int
tqlRewindTable(Table *table) {
    int e;

    fflush(table->file);
    rewind(table->file);
    e = errno;
    if ( ferror(table->file) ) {
        clearerr(table->file);
        return e;
    }

    return 0;
}

/*
 * Read row from table :
 */
int
tqlReadTable(Table *table,Row *row) {
    struct passwd *p;
    
    disposeRow(row);
    errno = 0;
    p = fgetpwent(table->file);
    if ( !p && !errno )
        return EOF;     /* EOF */
    if ( !p )
        return errno;   /* Error */

    passwdRow(row,p);   /* Copy record */

    return 0;           /* Successful */
}

/*
 * Write row to table :
 */
int
tqlWriteTable(Table *table,Row *row) {
    if ( putpwent(&row->pw,table->file) == -1 )
        return errno;
    return 0;
}

/*
 * Close a table:
 */
void
tqlCloseTable(Table *table) {
    fclose(table->file);
    free(table->name);
    free(table);
}

/*
 * Return the row count for the table :
 */
long
tqlCountTable(Table *table) {
    Row row;
    long count = 0;

    if ( tqlRewindTable(table) )
        return 0L;
    initRow(&row);

    while ( !tqlReadTable(table,&row) )
        ++count;

    return count;
}

/* end table.c */
