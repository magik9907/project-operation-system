#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "function.h"
int main(int argc, char *argv[])
{
    init(argc, argv);
    start();

    return 0;
}
