static void fun1(void) {
    char *cp = 0;

    *cp = '!'; /* Take that! */
}
int main(int argc,char **argv) {
    fun1();
    return 0;
}
