#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "function.h"

char *sourcePath = NULL;
char *destinationPath = NULL;
int sleepTime = 10;      //time in second
int borderFileSize = 10; // in bytes

void init(int argc, char *args[])
{
    int argument;
    int index;

    while ((argument = getopt(argc, args, "s:d:t:r:")) != -1)
    {
        switch (argument)
        {
        case 's': //source path
            sourcePath = optarg;
            break;
        case 'd': //destination path
            destinationPath = optarg;
            break;
        case 't': //freeze time
            sleepTime = atoi(optarg);
            break;
        case 'r': //file size (border of type file copy method )
            borderFileSize = atoi(optarg);
            break;
        case '?':
            write(1, "unknown parameters\n", 20);
            break;
        }
    }

    //Source and destination path check if exists
    if (sourcePath == NULL || strcmp("",sourcePath))
    {
        exitFailure("Source path not found\n");
    }
    if (destinationPath == NULL || strcmp("",destinationPath))
    {
        exitFailure("Destination path not found\n");
    }

    //checking if paths are folders
    if (!isDir(sourcePath))
    {
        exitFailure("Source path is not folder\n");
    }
    if (!isDir(destinationPath))
    {
        exitFailure("Destionation path is not folder\n");
    }
}

const bool isDir(char *path)
{
    struct stat st;
    stat(path, &st);
    if (S_ISDIR(st.st_mode))
        return true;
    return false;
}

void exitFailure(const char *mess)
{
    write(1, mess, strlen(mess));
    exit(EXIT_FAILURE);
}