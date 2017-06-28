#include <stdio.h>

#define TRACE    printf("%s @ %u\n",__FILE__,__LINE__)

int main(int argc,char *argv[]) {
    int x;

    TRACE;
    for ( x=0; x<argc; ++x ) {
        TRACE;
        printf("argv[%d] = '%s'\n",x,argv[x]);
    }
    TRACE;
    return 0;
}
