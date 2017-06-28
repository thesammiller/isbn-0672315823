#include <string.h>

char *
Basename(char *pathname) {
    char *cp;

    if ( ( cp = strrchr(pathname,'/') ) != 0 )
        return cp + 1;  /* Return basename ptr */
    return pathname;    /* No directory */
}
