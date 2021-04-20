#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <syslog.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "function.h"
char *sourcePath = NULL;
char *destinationPath = NULL;
int sleepTime = 5 * 60;         //time in second
int borderFileSize = 10;        // in bytes
bool recursivePathFlag = false; //recursive synchronise files
size_t buffor = 256;
const char *daemonName = "DirSync";

void exitFailure(const char *mess)
{
    write(1, mess, strlen(mess));
    logger(mess);
    exit(EXIT_FAILURE);
}

void logger(const char *message)
{
    time_t now;
    time(&now);
    syslog(LOG_INFO, "%s: %s", ctime(&now), message);
}

void handler(int signum)
{
    logger("user active synchronise dir");
    syncDir();
}

void init(int argc, char *args[])
{
    if (argc < 5)
    {
        exitFailure("Not given arguments\n");
    }

    // Open logs here
    setlogmask(LOG_UPTO(LOG_INFO));
    openlog(daemonName, LOG_NDELAY, LOG_USER);
    logger("Initialize program\n");
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
        logger("Initialize failed: source path\n");
        exitFailure("Source path not found\n");
    }
    if (destinationPath == NULL || strcmp("", destinationPath) == 0)
    {
        logger("Initialize failed: destination path\n");
        exitFailure("Destination path not found\n");
    }
    //checking if paths are folders
    if (!isDir(sourcePath))
    {
        logger("Initialize failed: source aren't folder\n");
        exitFailure("Source path is not folder\n");
    }
    if (!isDir(destinationPath))
    {
        if (errno == ENOTDIR || errno == ENOENT)
        {
            if (mkdir(destinationPath, S_IRUSR | S_IWUSR | S_IXUSR) == -1)
            {
                logger(strerror(errno));
                return;
            }
        }
        else
        {
            logger("Initialize failed: destination isn't folder\n");
            exitFailure("Destionation path is not folder\n");
        }
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

void signal_kill()
{
    logger("Deamon stop, kill PID");
    closelog();
    exit(EXIT_SUCCESS);
}

void start()
{

    signal(SIGTERM, signal_kill);
    /* Our process ID and Session ID */
    pid_t pid, sid, pidTest;

    /* Fork off the parent process */
    pid = fork();
    if (pid < 0)
    {
        logger("Creating process error\n");
        exit(EXIT_FAILURE);
    }
    /* If we got a good PID, then
           we can exit the parent process. */
    if (pid > 0)
    {
        logger("creating process success\n");
        exit(EXIT_SUCCESS);
    }
    /* Change the file mode mask */
    umask(0);
    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0)
    {
        /* Log the failure */
        logger("Creation of the daemon %s has failed\n");
        exit(EXIT_FAILURE);
    }

    /* Change the current working directory */
    if ((chdir("/")) < 0)
    {
        /* Log the failure */
        logger("Could not change the directory to /");
        exit(EXIT_FAILURE);
    }

    /* Close out the standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    /* Daemon-specific initialization goes here */
    signal(SIGUSR1, handler);
    /* The Big Loop */
    while (1)
    {
        /* Do some task here ... */
        syncDir();
        logger("Daemon has slept\n");
        sleep(sleepTime);
        logger("Daemon has woken up\n");
    }
    logger("Daemon close\n");
    closelog();
    exit(EXIT_SUCCESS);
}

const bool isDir(char *path)
{
    DIR *i = opendir(path);
    if (i == NULL)
    {
        return false;
    }
    closedir(i);
    struct stat st;
    stat(path, &st);
    if (S_ISDIR(st.st_mode))
        return true;
    return false;
}

//check if file is regular file
const bool isFile(char *path)
{
    struct stat st;
    stat(path, &st);
    if (S_ISREG(st.st_mode))
        return true;
    return false;
}

void syncDir()
{
    DIR *source;
    source = opendir(sourcePath);
    if (source == NULL)
    {
        logger("Syncing file: source dir not found");
        rmDestination("/");
    }
    else
    {
        char *p = "/";
        checkExist(p);
        syncDirPath(p);
    }
}

bool pathExist(char *p)
{
    struct stat info;
    if (stat(p, &info) != 0)
    {
        if (errno == ENOENT || errno == EACCES)
            return false;
    }
    return true;
}

void logWithFileName(const char *mess, const char *file)
{
    char *fullMess = (char *)malloc(sizeof(char) * (strlen(mess) + strlen(file) + 2));
    strcpy(fullMess, mess);
    strcat(fullMess, file);
    logger(fullMess);
    free(fullMess);
}

void checkExist(char *subDir)
{
    int err = 0, i;
    char *logMess;
    char *destinationFilePath = (char *)malloc(sizeof(char) * buffor);
    char *sourceFilePath = (char *)malloc(sizeof(char) * buffor);
    char *copyPath = (char *)malloc(sizeof(char) * buffor);
    char *copySubDir = (char *)malloc(sizeof(char) * buffor);
    strcpy(copyPath, destinationPath);
    strcat(copyPath, subDir);
    struct dirent **fileList;
    int noFiles = scandir(copyPath, &fileList, NULL, alphasort);

    for (i = 0; i < noFiles; i++)
    {
        if (strcmp(".", fileList[i]->d_name) != 0 && strcmp("..", fileList[i]->d_name) != 0)
        {
            strcpy(sourceFilePath, sourcePath);
            strcat(sourceFilePath, subDir);
            strcat(sourceFilePath, fileList[i]->d_name);

            strcpy(destinationFilePath, copyPath);
            strcat(destinationFilePath, fileList[i]->d_name);
            if (pathExist(sourceFilePath) == 0)
            {
                if (isDir(destinationFilePath) == 1)
                {
                    strcpy(copySubDir, subDir);
                    strcat(copySubDir, fileList[i]->d_name);
                    strcat(copySubDir, "/");
                    checkExist(copySubDir);
                    logWithFileName("Removing directory ", destinationFilePath);
                    if (rmdir(destinationFilePath) != 0)
                    {
                        logger("rmdir not completed");
                        return;
                    }
                }
                else
                {
                    logWithFileName("Removing file ", destinationFilePath);
                    if (remove(destinationFilePath) != 0)
                    {
                        logger("remove file not completed");
                        return;
                    }
                }
            }
            else if (isDir(sourceFilePath) == 1)
            {
                strcpy(copySubDir, subDir);
                strcat(copySubDir, fileList[i]->d_name);
                strcat(copySubDir, "/");
                checkExist(copySubDir);
            }
        }
        free(fileList[i]);
    }
    free(fileList);
    free(copySubDir);
    free(sourceFilePath);
    free(destinationFilePath);
    free(copyPath);
}

void rmDestination(char *subDir)
{
    DIR *dest;
    dest = opendir(destinationPath);
    if (dest == NULL)
    {
        logger("Syncing file: destination dir not found");
        return;
    }
    int err = 0, i;
    char *logMess;
    char *destinationFilePath = (char *)malloc(sizeof(char) * buffor);
    char *copyPath = (char *)malloc(sizeof(char) * buffor);
    char *copySubDir = (char *)malloc(sizeof(char) * buffor);
    strcpy(copyPath, destinationPath);
    strcat(copyPath, subDir);
    logWithFileName("in directory ", subDir);
    struct dirent **fileList;
    int noFiles = scandir(copyPath, &fileList, NULL, alphasort);
    for (i = 0; i < noFiles; i++)
    {
        if (strcmp(".", fileList[i]->d_name) != 0 && strcmp("..", fileList[i]->d_name) != 0)
        {
            strcpy(destinationFilePath, copyPath);
            strcat(destinationFilePath, "/");
            strcat(destinationFilePath, fileList[i]->d_name);

            if (isDir(destinationFilePath) == 1)
            {
                strcpy(copySubDir, subDir);
                strcat(copySubDir, fileList[i]->d_name);
                strcat(copySubDir, "/");
                rmDestination(copySubDir);
                logWithFileName("Removing directory ", destinationFilePath);
                if (rmdir(destinationFilePath) != 0)
                {
                    logWithFileName("rmdir not completed", destinationFilePath);
                    return;
                }
            }
            else
            {
                logWithFileName("Removing file ", destinationFilePath);
                if (remove(destinationFilePath) != 0)
                {
                    logWithFileName("remove file not completed", destinationFilePath);
                    logger(strerror(errno));
                    return;
                }
            }
        }
        free(fileList[i]);
    }
    free(fileList);
    free(copySubDir);
    free(destinationFilePath);
    free(copyPath);
    logWithFileName("Removing destination path ", destinationPath);
    free(destinationFilePath)
    if (rmdir(destinationPath) != 0)
    {
        logger("rmdir on destination file not completed");
    }
}

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
    {
        logger("Syncing file path: source dir not found");
        return;
    }
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
            strcpy(destinationFilePath, destinationPath);
            strcat(destinationFilePath, subDir);
            strcat(destinationFilePath, ep->d_name);
            DIR *test = opendir(destinationFilePath);
            if (test == NULL)
            {
                logWithFileName("Creating directory ", destinationFilePath);
                if (mkdir(destinationFilePath, S_IRUSR | S_IWUSR | S_IXUSR) == -1)
                {
                    closedir(test);
                    logger(strerror(errno));
                    return;
                }
            }
            closedir(test);
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

void syncLargeFile(size_t length, char *src, char *dest)
{
    logWithFileName("Creating or opening a large file called ", dest);
    int fdr = open(src, O_RDONLY);
    char *addr = mmap(NULL, length, PROT_READ, MAP_PRIVATE, fdr, 0);
    if (addr == MAP_FAILED)
    {
        logger(strerror(errno));
        return;
    }
    int fdw = open(dest, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    int wSize = write(fdw, addr, length);
    if (wSize != length)
    {
        if (wSize == -1)
        {
            logger(strerror(errno));
            return;
        }
    }
    munmap(addr, length);
    close(fdr);
    close(fdw);
}

void syncFile(char *src, char *dest, char *file)
{
    strcat(dest, file);
    struct stat bufSrc, bufDest;
    if (stat(src, &bufSrc) == -1)
        if (errno != ENOENT)
        {
            logger(strerror(errno));
            return;
        }
    if (stat(dest, &bufDest) == -1)
        if (errno != ENOENT)
        {
            logger(strerror(errno));
            return;
        }
    time_t *timeSrc = &bufSrc.st_mtime;
    time_t *timeDest = &bufDest.st_mtime;
    if ((*timeDest) > (*timeSrc))
    {
        return;
    }
    size_t length = bufSrc.st_size;
    if (length >= borderFileSize)
    {
        syncLargeFile(length, src, dest);
        return;
    }
    int fds[2];
    int err = 0;
    pid_t pid;
    if (pipe(fds) == -1)
    {
        logger(strerror(errno));
        return;
    }

    pid = fork();
    if (pid < 0)
    {
        logger(strerror(errno));
        return;
    }

    if (pid == (pid_t)0)
    {
        writeToFile(fds, dest);
        exit(EXIT_SUCCESS);
    }
    else
    {
        readFromFile(fds, src);
        wait(pid);
    }
}

void writeToFile(int fds[2], char *file)
{
    logWithFileName("Creating or changing ", file);
    int err;
    err = close(fds[1]);
    if (err == -1)
    {
        logger(strerror(errno));
        return;
    }

    int fd = open(file, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        logger(strerror(errno));
        return;
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
        logger(strerror(errno));
        return;
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
