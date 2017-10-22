#include "../libUseful.h"

main()
{
int FNotify, flags;
struct timeval tv;
char *FileName=NULL;
pid_t pid;
int fd;
STREAM *S;

//FNotify=FileNotifyInit("/home", FNOTIFY_RECURSE | FNOTIFY_ALL);
FNotify=FileNotifyInit("/etc/hosts", FNOTIFY_ALL | FNOTIFY_PERMIT);

while (1)
{
	tv.tv_sec=5;
	tv.tv_usec=0;
	if (FDSelect(FNotify, SELECT_READ, &tv))
	{
		flags=FileNotifyNext(FNotify, &FileName, &pid, &fd);
		if (flags & FNOTIFY_OPEN) printf("pid %d opened %s\n",pid,FileName);
		if (flags & FNOTIFY_READ) printf("pid %d read %s\n",pid,FileName);
		if (flags & FNOTIFY_WRITE) printf("pid %d modified %s\n",pid,FileName);
		if (flags & FNOTIFY_CLOSE) printf("pid %d closed %s\n",pid,FileName);
		if (flags & FNOTIFY_PERMIT) 
		{
			printf("ALLOW pid %d %s\n",pid,FileName);
			FileNotifyPermit(FNotify, fd, TRUE);
		}
	}
}

}
