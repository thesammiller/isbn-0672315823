$ cc -c -D_HPUX_SOURCE hello.c -o hello.o
$ cc -c hello.o -o hello
$ ./hello
Hello World!
$ echo $?
0
$
