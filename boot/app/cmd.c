#include <stdint.h>
#include <stdio.h>


int cmd_hello_world(int argc, char *argv[])
{
    shellPrint(shellGetCurrent(), "%s\n","Hello, World!");
    return 0;
}
