#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main(int argc,char *argv[]) {
    int nf;             /* Number of fields converted */
    int n;              /* # of characters scanned */
    int mm, dd, yyyy;   /* Month, day and year */
    static char sdate[] = "   1 /  2  /  2000  ";

    printf("Extracting from '%s'\n",sdate);

    nf = sscanf(sdate,"%d /%d /%d%n",
        &mm, &dd, &yyyy, &n);

    printf("%02d/%02d/%04d nf=%d, n=%d\n",
        mm, dd, yyyy, nf, n);

    if ( nf >= 3 )
        printf("Remainder = '%s'\n",&sdate[n]);

    return 0;
}
