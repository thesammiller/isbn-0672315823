#include <stdio.h>
#include <errno.h>
#include <string.h>

extern int sys_nerr;

int main(int argc,char **argv) {
    int x;
    static int ecodes[] = 
        { -1, EIO, 0 };

    /* Get maximum code */
    ecodes[2] = sys_nerr;

    for ( x=0; x<3; ++x )
        printf("%4d = '%s'\n",
            ecodes[x],
            strerror(ecodes[x]));

    return 0;
}
