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
size_t buffor = 256;

void init(int argc, char *args[])
{
    int argument;
    char *cwd;
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

void syncDir()
{
    char *p = "/";
    syncDirPath(p);
}

//./function -s ./test -d ./test_copy
void syncDirPath(char *subDir)
{
    int err = 0;
    DIR *source;
    struct dirent *ep;
    char *destinationFilePath;
    char *sourceFilePath;
    char *copyPath = (char *)malloc(sizeof(char) * buffor);
    char *copySubDir = (char *)malloc(sizeof(char) * buffor);
    strcpy(copyPath, sourcePath);
    strcat(copyPath, subDir);
    source = opendir(copyPath);
    if (source == NULL)
        exitFailure("Couldn't open the directory");

    sourceFilePath = (char *)malloc(sizeof(char) * buffor);
    destinationFilePath = (char *)malloc(sizeof(char) * buffor);

    while (ep = readdir(source))
    {
        if (strcmp(".", ep->d_name) == 0 || strcmp("..", ep->d_name) == 0)
            continue;
        strcpy(sourceFilePath, copyPath);
        strcat(sourceFilePath, "/");
        strcat(sourceFilePath, ep->d_name);

        if (isFile(sourceFilePath))
        {
            strcpy(destinationFilePath, destinationPath);
            strcat(destinationFilePath, subDir);
            syncFile(sourceFilePath, destinationFilePath, ep->d_name);
        }
        else if (isDir(sourceFilePath) == 1 && recursivePathFlag == 1)
        {
            strcpy(copySubDir, subDir);
            strcat(copySubDir, ep->d_name);
            strcat(copySubDir, "/");
            syncDirPath(copySubDir);
        }
    }
    free(copySubDir);
    free(sourceFilePath);
    free(destinationFilePath);
    free(copyPath);
    closedir(source);
}

void syncFile(char *src, char *dest, char *file)
{
    int fds[2];
    int err = 0;
    pid_t pid;
    if (pipe(fds) == -1)
    {
        exitFailure(strerror(errno));
    }

    pid = fork();
    if (pid < 0)
    {
        exitFailure(strerror(errno));
    }

    if (pid == (pid_t)0)
    {
        DIR *test = opendir(dest);
        if (errno == ENOENT)
        {
            mkdir(dest, S_IRUSR | S_IWUSR | S_IXUSR);
        }
        closedir(test);
        strcat(dest, file);
        writeToFile(fds, dest);
        return;
    }
    else
    {
        readFromFile(fds, src);
    }
}

void writeToFile(int fds[2], char *file)
{
    int err;
    err = close(fds[1]);
    if (err == -1)
    {
        exitFailure(strerror(errno));
    }

    int fd = open(file, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        exitFailure(strerror(errno));
    }

    int writeBuffer;
    char *text = (char *)malloc(sizeof(char) * (buffor + 1));
    while ((writeBuffer = read(fds[0], text, buffor)) > 0)
    {
        write(fd, text, writeBuffer);
    }
    close(fd);
    close(fds[0]);
}

void readFromFile(int fds[2], char *file)
{
    int err;
    err = close(fds[0]);
    if (err == -1)
    {
        exitFailure(strerror(errno));
    }
    int od = open(file, O_RDWR);
    int readbuffer;
    char *text = (char *)malloc(sizeof(char) * (buffor + 1));
    while ((readbuffer = read(od, text, buffor)) > 0)
    {
        write(fds[1], text, readbuffer);
    }
    close(od);
    close(fds[1]);
}
