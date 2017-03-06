#include "Zip.h"
#include "FileExtensions.h"
#include <wait.h>

void ZipFileProcess(const char *Path, TMimeItem *Parent)
{
char *Tempstr=NULL, *Token=NULL, *UnzipExe=NULL, *ptr;
TMimeItem *Item;
int result, i;
STREAM *S;

if ((Flags & FLAG_DEBUG)) printf("Check Zip: [%s]\n",Path);
UnzipExe=FindFileInPath(UnzipExe, "unzip", getenv("PATH"));
Tempstr=MCopyStr(Tempstr, UnzipExe, " -l ",Path,NULL);
S=STREAMSpawnCommand(Tempstr, COMMS_BY_PIPE, "");
if (S)
{

//Read PastHeader
Tempstr=STREAMReadLine(Tempstr,S);
while (Tempstr && (*Tempstr != '-'))
{
Tempstr=STREAMReadLine(Tempstr,S);
}

Tempstr=STREAMReadLine(Tempstr,S);
while (Tempstr && (*Tempstr != '-'))
{
	StripTrailingWhitespace(Tempstr);
	ptr=Tempstr;
	while (isspace(*ptr)) ptr++;
	ptr=GetToken(ptr,"\\S",&Token,0);
	while (isspace(*ptr)) ptr++;
	ptr=GetToken(ptr,"\\S",&Token,0);
	while (isspace(*ptr)) ptr++;
	ptr=GetToken(ptr,"\\S",&Token,0);

if ((Flags & FLAG_DEBUG)) printf("         : [%s]\n",ptr);
	Item=MimeItemCreate(ptr,"","");
	ListAddItem(Parent->SubItems, Item);
	ptr=strrchr(Item->FileName, '.');
	Item->ExtnType=CopyStr(Item->ExtnType, FileExtensionLookup(ptr));
	Item->ContentType=CopyStr(Item->ContentType, Item->ExtnType);

	Tempstr=STREAMReadLine(Tempstr,S);
}

STREAMClose(S);
}

//collect child processes
for (i=0; i < 10; i++) waitpid(-1,NULL,WNOHANG);

DestroyString(Tempstr);
DestroyString(UnzipExe);
DestroyString(Token);
}
