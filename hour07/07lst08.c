/* dos_cvrt.c */

static const char rcsid[] = 
    "$Id: dos_cvrt.c,v 1.11 1998/12/06 21:29:58 student1 Exp $";

/* REVISION HISTORY:
 * $Log: dos_cvrt.c,v $
 * Revision 1.11  1998/12/06 21:29:58  student1
 * Added error checking.
 *
 * Revision 1.10  1998/12/04 04:43:10  student1
 * Added GNU long options.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include "dos_cvrt.h"

extern char *optarg;    /* Option argument */
extern int optind;      /* Option/arg index */
extern int opterr;      /* Error handling flg */

/*
 * Show brief command usage help:
 */
static void
usage(const char *cmd) {

    fprintf(stderr,"Usage: %s [-h | --help] [--version] "
        "[-u] infile..\n",cmd);
    fputs("\t-h (or --help)\tGives this help display.\n",stderr);
    fputs("\t--version\tDisplays program version.\n",stderr);
    fputs("\t-u\t\tSpecifies UNIX to DOS conversion.\n\n",stderr);
    fputs("Command unix_cvrt converts UNIX to DOS text.\n",stderr);
    fputs("while dos_cvrt converts DOS to UNIX, except "
        "when -u is used.\n\n",stderr);
}

/*
 * Return the pointer to the basename component
 * of a pathname:
 */
static char *
Basename(const char *path) {
    char *base_name = 0;

    if ( ( base_name = strrchr(path,'/') ) != 0 )
        ++base_name;        		/* Skip over '/' */
    else
        base_name = (char *) path; 	/* No dir. */
    return base_name;
}

int
main(int argc,char **argv) {
    char *base_name = 0;    /* Basename of command */
    int rc = 0;             /* Command return code */
    int (*conv)(const char *pathname); /* Conv. Func. Ptr */
    int x;
    int cmdopt_u = 0;       /* -u ; True if UNIX -> DOS */
    static int cmdopt_v = 0;/* --version */
    int cmdx = 0;           /* lopts[] index */
    int optch;              /* Current option character */
    static char stropts[] = "hu"; /* Supported options */
    static struct option lopts[] = {
        { "help", 0, 0, 'h' },  /* --help */
        { "version", 0, &cmdopt_v, 1 }, /* --version */
        { 0, 0, 0, 0 }          /* No more options */
    };

    /*
     * Determine the basename of the command name. This
     * is necessary since this command could be invoked
     * with a pathname. For example, argv[0] could be
     * "/usr/local/bin/unix_cvrt" if it was installed
     * in the system that way.
     */
    base_name = Basename(argv[0]);
    if ( !strcmp(base_name,"unix_cvrt") )
        cmdopt_u = 1;       /* Pretend that -u was given */

    /*
     * Process all command line options :
     */
    while ( (optch = getopt_long(argc,argv,stropts,
      lopts,&cmdx)) != EOF )

        switch ( optch ) {
        case 0 :            /* Processed lopts[cmdx] */
            break;
        case 'h' :          /* -h or --help */
            usage(base_name);
            return 0;
        case 'u' :          /* -u ; Specifies UNIX -> DOS */
            cmdopt_u = 1;
            break;
        default :           /* Unsupported option */
            fputs("Use --help for help.\n",stderr);
            return 1;
        }

    if ( cmdopt_v != 0 ) {
        fprintf(stderr,"Version 1.0\n");
	return 0;
    }

    /*
     * Check for missing input file:
     */
    if ( argc - optind < 1 ) {
        fprintf(stderr,"Missing input file(s).\n");
        fputs("Use --help for help.\n",stderr);
        return RC_CMDOPTS;
    }

    /*
     * -u determines the direction of conversion :
     */
    if ( cmdopt_u != 0 )
        conv = unix2dos;
    else
        conv = dos2unix;

    /*
     * Perform a text conversion
     */
    for ( x=optind; x<argc; ++x )
        if ( (rc = conv(argv[x])) != 0 )
            break;      /* An error occured */

    /*
     * Check that our possibly buffered stdout
     * gets flushed without errors:
     */
    fflush(stdout);
    if ( ferror(stdout) ) {
        fprintf(stderr,"%s: writing standard output.\n",
            strerror(errno));
        rc = RC_WRITERR;
    }

    return rc;
}
