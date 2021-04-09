#ifndef FUNCTION_H
#define FUNCTION_H
typedef enum Bool
{
    false = 0,
    true = 1
} bool; //bool type

void init(int argc, char *args[]);
const bool isDir(char *path);
void exitFailure(const char *mess);
void syncDir();
void syncDirPath(char *subDir);
void copyFileFromDir(int sourceFile, int destinationFile);
void recursiveSyncDir(char *folderPath);
void readFromFile(int fds[2], char *file);
void writeToFile(int fds[2], char *file);
void syncFile(char *src, char *dest, char *file);
void syncLargeFile(size_t length, char *src, char *dest);
void checkExist(char *subDir);
#endif
