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
int sleepTime = 10;             //time in second
int borderFileSize = 10;        // in bytes
bool recursivePathFlag = false; //recursive synchronise files

void init(int argc, char *args[])
{
    int argument;
    int index;

    while ((argument = getopt(argc, args, "s:d:t:m:R")) != -1)
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
        case 'm': //file size (border of type file copy method )
            borderFileSize = atoi(optarg);
            break;
        case 'R': //recursive searching file
            recursivePathFlag = true;
            break;
        case '?':
            write(1, "unknown parameters\n", 20);
            break;
        }
    }

    //Source and destination path check if exists
    if (sourcePath == NULL || strcmp("", sourcePath) == 0)
    {
        exitFailure("Source path not found\n");
    }
    if (destinationPath == NULL || strcmp("", destinationPath) == 0)
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

//85f5f1aacbf7a57766211a49488c8ab2b80ede68

const bool isDir(char *path)
{
    struct stat st;
    stat(path, &st);
    if (S_ISDIR(st.st_mode))
        return true;
    return false;
}

//check if file is regular file
const bool isFile(char *path)
{
    //    printf("\n%s\n", path);
    struct stat st;
    stat(path, &st);
    if (S_ISREG(st.st_mode))
        return true;
    return false;
}

void exitFailure(const char *mess)
{
    write(1, mess, strlen(mess));
    exit(EXIT_FAILURE);
}

//./function -s ./test -d ./test_copy
void syncDir()
{
    DIR *source;
    struct dirent *ep;

    source = opendir(sourcePath);
    if (source == NULL)
        exitFailure("Couldn't open the directory");

    while (ep = readdir(source))
    {
        char *sourceFilePath = (char *)malloc(sizeof(char));
        char *destinationFilePath = (char *)malloc(sizeof(char));

        strcat(strcat(strcat(sourceFilePath, sourcePath), "/"), ep->d_name);
        //write(1, ep->d_name, strlen(ep->d_name));
        // printf("%d\n", isFile(sourcePath, ep->d_name));
        if (isFile(sourceFilePath) == 0)
            continue;

        int sourceFile = open(sourceFilePath, O_RDONLY);
        if (sourceFile == -1)
        {
            free(sourceFilePath);
            close(sourceFile);
            exitFailure("source file open error");
        }

        strcat(strcat(strcat(destinationFilePath, destinationPath), "/"), ep->d_name);
        int destinationFile = open(destinationFilePath, O_CREAT | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);
        if (destinationFile == -1)
        {
            close(sourceFile);
            free(destinationFilePath);
            close(destinationFile);
            free(sourceFilePath);
            exitFailure("destination file open error");
        }

        unsigned char buffer[1024];
        ssize_t bytes_read, bytes_write;

        do
        {
            buffer[0] = '\0';
            bytes_read = read(sourceFile, buffer, sizeof(buffer));
            if (bytes_read == -1)
            {
                close(sourceFile);
                free(sourceFilePath);
                close(destinationFile);
                free(destinationFilePath);
                exitFailure("read from source file error");
            }
            bytes_write = write(destinationFile, buffer, bytes_read);
            if (bytes_write == -1)
            {
                close(sourceFile);
                free(sourceFilePath);
                close(destinationFile);
                free(destinationFilePath);
                exitFailure("write to destination file error");
            }
        } while (bytes_read == sizeof(buffer));

        close(sourceFile);
        free(sourceFilePath);
        close(destinationFile);
        free(destinationFilePath);
    }
    (void)closedir(source);
}