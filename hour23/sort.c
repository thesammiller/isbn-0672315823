/* sort.c :
 * Sams Teach Yourself LINUX Programming in 24 Hours
 * ISBN 0-672-31582-3
 * Warren W. Gay VE3WWG
 *
 * This module performs the sorting required by the
 * ORDER BY clause.
 */
static const char rcsid[] =
    "$Id: sort.c,v 1.1 1999/02/20 21:32:09 wwg Exp $";

#include "server.h"

char *path_sort = NULL;   /* Pathname of sort command */

static char pos_args[128];    /* Used for sort_argv[] */
static char *sort_argv[64] = {         /* Sort argv[] */
    "sort",                                /* argv[0] */
    "-t:",                                 /* argv[1] */
};          /* The remaining args specify sort fields */

/*
 * This function makes sure we can locate
 * and execute the sort command :
 *
 * RETURNS:
 *  True    If the sort command was found, and was
 *          executable by this process.
 *  False   If no executable sort command could be
 *          found.
 */
Bool
initSort(void) {
    char *cp = getenv("PATH");/* PATH environment var */
    char *path = NULL;          /* Local copy of PATH */
    char *buf = NULL;                  /* work buffer */
    struct stat sbuf;           /* stat() info buffer */

    if ( !cp )                   /* no PATH variable? */
        cp = "/bin:/usr/bin";    /* No?, Assume these */
    path = strdup(cp);     /* Duplicate PATH for mods */

    buf = malloc(strlen(path) + 16); /* For pathnames */

    /*
     * Search the PATH looking for a sort command. It
     * must be executable also.
     */
    cp = strtok(path,":"); /* First directory to srch */
    do  {
        strcat(strcpy(buf,cp),"/sort");  /* make path */
        if ( !stat(buf,&sbuf)         /* Is it there? */
        &&   S_ISREG(sbuf.st_mode)   /* Is it a file? */
                     /* Does it have executable bits? */
        &&   (S_IXUSR|S_IXGRP|S_IXOTH) & sbuf.st_mode
        &&   !access(buf,X_OK) ) {  /* Can we xeq it? */
            path_sort = strdup(buf); /* Save its path */
            break;                            /* Done */
	}    
    } while ( (cp = strtok(NULL,":")) != 0 );

    free(path);                /* No longer need this */
    free(buf);           /* Done with path buffer too */

    if ( path_sort ) {             /* Did we find it? */
        /* Report what we're using to the log */
        logf("Found sort at %s\n",path_sort);
        return True;
    }

    /*
     * Log why the server is going down.
     */
    logf("Unable to find/xeq the sort command.\n");
    return False;
}

/*
 * Sort the results according to the ORDER BY clause:
 *
 * ARGS:
 *  tbl     I/O     Initially is the table that we are
 *                  sorting from. Later, a switch of
 *                  files is performed.
 *  olist   input   The olist[] array indicates the sort
 *                  order required.
 *  n       input   Indicates how many sort fields there
 *                  are.
 * RETURNS:
 *  True    Succeeded.
 *  False   Sort failed.
 */
Bool
doOrderBy(Table *tbl,OrderBy *olist,int n) {
    short x;
    short ax;               /* Index into sort_argv[] */
    char *cp;                             /* Work ptr */
    FILE *tempf;                         /* Temp file */
    pid_t chpid;                         /* Child PID */
    pid_t wtpid;                   /* PID from wait() */
    int termstat;               /* Termination status */

    /*
     * Begin building the argv[] vector for the sort
     * command. We start by adding argv[2]:
     */
    *(cp = pos_args) = NUL;    /* Buffer for arg data */
    ax = 2;                       /* Start at argv[2] */

    /*
     * For each ORDER by reference:
     */
    for ( x=0; x<n; ++x ) {
        sprintf(cp,"+%d%s",       /* argv[] = '+POSn' */
            olist[x].fldno, /* Field # in passwd file */
            olist[x].num ? "n" : "");/* Numeric or not*/

        sort_argv[ax++] = cp;     /* Assign to argv[] */
        cp += strlen(cp) + 1;  /* Skip arg & NUL byte */

        sprintf(cp,"-%d",          /* argv[] = '-POS' */
            olist[x].fldno+1);

        sort_argv[ax++] = cp;     /* Assign to argv[] */
        cp += strlen(cp) + 1;  /* Skip arg & NUL byte */
    }

    sort_argv[ax] = NULL; /* Last argv[] must be NULL */

    /*
     * Create another temp file to contains the sorted
     * results:
     */
    if ( !(tempf = tmpfile()) ) {    /* New temp file */
        logf("%s: tmpfile()\n",strerror(errno));
        return False;                  /* Sort failed */
    }

    /*
     * Rewind the table :
     */
    if ( tqlRewindTable(tbl) )  /* Rewind input table */
        goto bail;             /* This shouldn't fail */

    /*
     * Flush pending buffers:
     */
    fflush(stdout);
    fflush(stdin);
    fflush(stderr);
    fflush(tbl->file);
    logFlush();

    /*
     * Fork to create our sort process:
     */
    if ( (chpid = fork()) == -1 ) {
        logf("%s: fork()\n",strerror(errno));
        goto bail;   /* System must be short on procs */

    } else if ( !chpid ) {
        extern char **environ;     /* Our environment */

        /*
         * Child process:
         *
         * Close our standard input, so that we can make
         * the input table the standard input instead:
         */
        close(0);   /* stdin */
        if ( dup2(fileno(tbl->file),0) != 0 )
            _exit(10);                      /* Failed */

        /*
         * Close our standard output, so that we can
         * make the standard output the new temp file.
         */
        close(1);   /* stdout */
        if ( dup2(fileno(tempf),1) != 1 )
            _exit(11);                      /* Failed */

        /*
         * At this point in the child process, we have
         * as our standard input the table to be sorted
         * and the new temp table as our standard output.
         * Now we just execute the sort command, with
         * the arguments we set up, and "presto" our
         * new temp file gets the sorted results.
         */
        execve(path_sort,sort_argv,environ);
        _exit(12);                /* Failed to exec() */
    }

    /*
     * Parent Process :
     *
     * Wait for the sort process to complete.
     */
    do  {
        wtpid = wait(&termstat);
    } while ( wtpid != chpid );

    TRACEF(3,SORT,("Sort PID %ld completed 0x%04lX\n",
        (long)wtpid,termstat))

    /*
     * See if the sort was successful or not :
     */
    if ( WIFEXITED(termstat) ) {
        x = WEXITSTATUS(termstat);
        if ( x != 0 ) {
            logf("Sort failed with exit code %d\n",x);
            goto bail;
        }
        /* Sort succeeded */

    } else if ( WIFSIGNALED(termstat) ) {
        x = WTERMSIG(termstat);
        logf("Sort failed with signal: %s\n",strsignal(x));
        goto bail;

    } else if ( WIFSTOPPED(termstat) ) {
        x = WSTOPSIG(termstat);
        logf("Sort stopped with signal: %s\n",strsignal(x));
        goto bail;

    } else  {
        logf("Sort failure for X-Files. Status 0x%04lX\n",
            termstat);
        goto bail;
    }

    /*
     * Rewind the new temp file with the sorted results :
     */
    rewind(tempf);
    if ( ferror(tempf) ) {
        logf("%s: Cannot rewind sort tempf\n",
            strerror(errno));
        goto bail;
    }

    /*
     * Pull a switch on the table : The provided table
     * is a temp file, which we can close and consequently
     * allow the system to release. We then put the new
     * sorted temp file in its place.
     */
    fclose(tbl->file);    /* Close unsorted temp file */
    tbl->file = tempf;     /* Put sorted temp file in */
    return True;               /* Sorted successfully */

bail:
    fclose(tempf);         /* Close unused temp. file */
    return False;                   /* Return failure */
}

/* end sort.c */
