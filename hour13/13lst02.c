#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

int
main(int argc,char *argv[]) {
    time_t td;          /* Current Date */
    struct tm dc;       /* Date components */
    char sdate[64];     /* Formatted date result */
    char fmt[128];      /* Input line/format string */
    char *cp;

    dc.tm_mon = 3;      /* April */
    dc.tm_mday = 1;     /* 1, */
    dc.tm_year = 100;   /* Year 2000 */
    dc.tm_hour = 11;    /* Just before noon */
    dc.tm_min  = 59;
    dc.tm_sec  = 30;
    dc.tm_isdst= 0;     /* Not D.S.T. */

    td = mktime(&dc);   /* Make time_t */

    while ( fgets(fmt,sizeof fmt,stdin) ) {
        if ( (cp = strchr(fmt,'\n')) != 0 )
            *cp = 0;    /* Remove the newline */
        strftime(sdate,sizeof sdate,fmt,&dc);
        printf("Format='%s':\n%s\n\n", fmt,sdate);
    }

    return 0;
}                        
