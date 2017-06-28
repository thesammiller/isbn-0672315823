#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

int
main(int argc,char *argv[]) {
    time_t td;          /* Current Date */
    struct tm dc;       /* Date components */

    dc.tm_mon = 0;      /* January */
    dc.tm_mday = 1;     /* 1, */
    dc.tm_year = 100;   /* Year 2000 */
    dc.tm_hour = 0;     /* Midnight */
    dc.tm_min  = 0;
    dc.tm_sec  = 0;
    dc.tm_isdst= 0;     /* Not D.S.T. */

    td = mktime(&dc);   /* Make time_t */

    /* Verify : */
    printf("Date: %s",ctime(&td));

    return 0;
}                        
