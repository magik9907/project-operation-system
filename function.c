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
    char *cwd;
    size_t buffor = 250;

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
    //set working directory
    if (sourcePath[0] == '.')
    {
        cwd = (char *)malloc(sizeof(char) * buffor);
        cwd = getcwd(cwd, buffor);
        sourcePath = strcat(cwd, &sourcePath[1]);
    }

    if (destinationPath[0] == '.')
    {
        cwd = (char *)malloc(sizeof(char) * buffor);
        cwd = getcwd(cwd, buffor);
        destinationPath = strcat(cwd, &destinationPath[1]);
    }
    chdir("/");
}

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
    size_t buffor = 250;
    char *destinationFilePath;
    int sourceFile, destinationFile;

    source = opendir(sourcePath);
    if (source == NULL)
        exitFailure("Couldn't open the directory");

    destinationFilePath = (char *)malloc(sizeof(char) * buffor);

    while (ep = readdir(source))
    {
        if (strcmp(".", ep->d_name) == 0 || strcmp("..", ep->d_name) == 0)
            continue;

        if (isFile(ep->d_name))
        {
            sourceFile = open(ep->d_name, O_RDONLY);
            if (sourceFile == -1)
            {
                close(sourceFile);
                exitFailure("source file open error");
            }

            // write(1, destinationPath, 1);
            strcpy(destinationFilePath, destinationPath);
            strcat(destinationFilePath, "/");
            strcat(destinationFilePath, ep->d_name);
            destinationFile = open(destinationPath, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
            if (destinationFile == -1)
            {
                free(destinationFilePath);
                close(sourceFile);
                close(destinationFile);
                exitFailure("destination file open error");
            }
            copyFileFromDir(sourceFile, destinationFile);

            close(sourceFile);
            close(destinationFile);
        }
    }
    free(destinationFilePath);
    (void)closedir(source);
}

void copyFileFromDir(int sourceFile, int destinationFile)
{
    unsigned char buffor[1024];
    ssize_t bytes_read, bytes_write;

    do
    {
        buffor[0] = '\0';
        bytes_read = read(sourceFile, buffor, sizeof(buffor));
        if (bytes_read == -1)
        {
            close(sourceFile);
            close(destinationFile);
            exitFailure("read from source file error");
        }
        bytes_write = write(destinationFile, buffor, bytes_read);
        if (bytes_write == -1)
        {
            close(sourceFile);
            close(destinationFile);
            exitFailure("write to destination file error");
        }
    } while (bytes_read == sizeof(buffor));
}
