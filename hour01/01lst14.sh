bash$ gcc -c -Wall -D_GNU_SOURCE hello.c -o hello.o
bash$ gcc -c hello.o -o hello
bash$ ./hello
Hello World!
bash$ echo $?
0
bash$
