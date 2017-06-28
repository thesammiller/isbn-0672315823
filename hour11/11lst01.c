#include <stdio.h>
#include <string.h>

int
main(int argc,char **argv) {
    char *my_argv[64];  /* Collected argv[] array */
    int my_argc;        /* My argc value */
    char *svptr;        /* strtok_r() updates this */
    char *s;            /* Returned strtok_r() pointer */
    char buf[256];      /* Command buffer */
    int x;              /* Work index */
    static const char delim[] = " \t\n"; /* Delimiters */
    
    strcpy(buf,"rm -fr core d\n");  /* Fake a command */

    s = strtok_r(buf,delim,&svptr); /* First call */
    my_argc = 0;                    /* Start on my_argv[0] */

    while ( s != 0 ) {
        my_argv[my_argc++] = s;     /* Save argv[] value */
        s = strtok_r(NULL,delim,&svptr); /* Parse next tkn */
    }

    my_argv[my_argc] = 0;           /* null in last entry */

    /*
     * Print out collected values:
     */
    for ( x=0; x<=my_argc; ++x )
        if ( my_argv[x] != 0 )
            printf("argv[%d] = '%s';\n",x,my_argv[x]);
        else
            printf("argv[%d] = NULL;\n",x);

    return 0;
}
