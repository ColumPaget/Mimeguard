#ifndef LIBUSEFUL_TIME_H
#define LIBUSEFUL_TIME_H

#include "includes.h"
#include "Process.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HOURSECS 3600
#define DAYSECS (24 * 3600)
#define LU_STD_DATE_FMT "%Y/%m/%d %H:%M:%S"

#define TIME_MILLISECS 1
#define TIME_CENTISECS 2
#define TIME_CACHED 1024

uint64_t GetTime(int Flags);
char *GetDateStrFromSecs(const char *DateFormat, time_t Secs, const char *TimeZone);
char *GetDateStr(const char *DateFormat, const char *TimeZone);
time_t DateStrToSecs(const char *DateFormat, const char *Str, const char *TimeZone);

//this sets a SIGALRM timer, causing a signal to be sent to our process after 'timeout' seconds. 
//You can either pass a signal handler function, or pass NULL to use the default libUseful internal
//signal handler (SIGNAL_HANDLER_FUNC is of the form 'void MyHandler(int sig)' )
void SetTimeout(int timeout, SIGNAL_HANDLER_FUNC);

#ifdef __cplusplus
}
#endif



#endif
