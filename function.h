#ifndef FUNCTION_H
#define FUNCTION_H
typedef enum Bool
{
    false = 0,
    true = 1
} bool; //bool type

void init (int argc,char* args[]);
const bool isDir(char *path);
void exitFailure(const char *mess);
void syncDir();
void copyFileFromDir(int sourceFile, int destinationFile);

#endif