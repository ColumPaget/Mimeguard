#ifndef LIBUSEFUL_PROCESS_H
#define LIBUSEFUL_PROCESS_H

void CloseOpenFiles();
int SwitchUser(const char *User);
int SwitchGroup(const char *Group);
char *GetCurrUserHomeDir();
int WritePidFile(char *ProgName);
int CreateLockFile(char *FilePath,int Timeout);
void ProcessTitleCaptureBuffer(char **argv);
void ProcessSetTitle(char *FmtStr, ...);
void ColLibDefaultSignalHandler(int sig);

#endif
