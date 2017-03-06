#include "process.h"
#define __GNU_SOURCE
#include "errno.h"
#include "includes.h"
#include "Time.h"
#include <pwd.h>
#include <sys/file.h>

/*This is code to change the command-line of a program as visible in ps */

extern char **environ;
char *TitleBuffer=NULL;
int TitleLen=0;


//The command-line args that we've been passed (argv) will occupy a block of contiguous memory that
//contains these args and the environment strings. In order to change the command-line args we isolate
//this block of memory by iterating through all the strings in it, and making copies of them. The
//pointers in 'argv' and 'environ' are then redirected to these copies. Now we can overwrite the whole
//block of memory with our new command-line arguments.
void ProcessTitleCaptureBuffer(char **argv)
{
char *end=NULL, *tmp;
int i;

TitleBuffer=*argv;
end=*argv;
for (i=0; argv[i] !=NULL; i++)
{
//if memory is contiguous, then 'end' should always wind up
//pointing to the next argv
if (end==argv[i])
{
	while (*end != '\0') end++;
	end++;
}
}

//we used up all argv, environ should follow it
if (argv[i] ==NULL)
{
	for (i=0; environ[i] !=NULL; i++)
	if (end==environ[i])
	{
	while (*end != '\0') end++;
	end++;
	}
}

//now we replace argv and environ with copies
for (i=0; argv[i] != NULL; i++) argv[i]=strdup(argv[i]);
for (i=0; environ[i] != NULL; i++) environ[i]=strdup(environ[i]);

//These might point to argv[0], so make copies of these too
#ifdef __GNU_LIBRARY__
extern char *program_invocation_name;
extern char *program_invocation_short_name;

program_invocation_name=strdup(program_invocation_name);
program_invocation_short_name=strdup(program_invocation_short_name);
#endif


TitleLen=end-TitleBuffer;
}


void ProcessSetTitle(char *FmtStr, ...)
{
va_list args;

		if (! TitleBuffer) return;
		memset(TitleBuffer,0,TitleLen);

    va_start(args,FmtStr);
		vsnprintf(TitleBuffer,TitleLen,FmtStr,args);
    va_end(args);
}




int WritePidFile(char *ProgName)
{
char *Tempstr=NULL;
int fd;


if (*ProgName=='/') Tempstr=CopyStr(Tempstr,ProgName);
else Tempstr=FormatStr(Tempstr,"/var/run/%s.pid",ProgName);

fd=open(Tempstr,O_CREAT | O_WRONLY,0600);
if (fd > -1)
{
  fchmod(fd,0644);
  if (flock(fd,LOCK_EX|LOCK_NB) ==0)
  {
  ftruncate(fd,0);
  Tempstr=FormatStr(Tempstr,"%d\n",getpid());
  write(fd,Tempstr,StrLen(Tempstr));
  }
  else
  {
    close(fd);
    fd=-1;
  }
}

//Don't close 'fd'!

DestroyString(Tempstr);

return(fd);
}


void CloseOpenFiles()
{
      int i;

      for (i=3; i < 1024; i++) close(i);
}



int SwitchUser(const char *NewUser)
{
int uid;
char *ptr;

	uid=LookupUID(NewUser);
  if ((uid==-1) || (setreuid(uid,uid) !=0))
	{
		syslog(LOG_ERR,"ERROR: Switch to user '%s' failed. Error was: %s",NewUser,strerror(errno));
		ptr=LibUsefulGetValue("SwitchUserAllowFail");
		if (ptr && (strcasecmp(ptr,"yes")==0)) return(FALSE);
		exit(1);
	}
  return(TRUE);
}


int SwitchGroup(const char *NewGroup)
{
int gid;
char *ptr;

	gid=LookupUID(NewGroup);
	if ((gid==-1) && (setgid(gid) !=0))
	{
		syslog(LOG_ERR,"ERROR: Switch to group '%s' failed. Error was: %s",NewGroup,strerror(errno));
		ptr=LibUsefulGetValue("SwitchGroupAllowFail");
		if (ptr && (strcasecmp(ptr,"yes")==0)) return(FALSE);
		exit(1);
	}
  return(TRUE);
}




char *GetCurrUserHomeDir()
{
struct passwd *pwent;

    pwent=getpwuid(getuid());
    if (! pwent) return(NULL);
    return(pwent->pw_dir);
}



void ColLibDefaultSignalHandler(int sig)
{

}


int CreateLockFile(char *FilePath, int Timeout)
{
int fd, result;

SetTimeout(Timeout);
fd=open(FilePath, O_CREAT | O_RDWR, 0600);
if (fd <0) return(-1);
result=flock(fd,LOCK_EX);
alarm(0);

if (result==-1)
{
  close(fd);
  return(-1);
}
return(fd);
}
