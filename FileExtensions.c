#include "common.h"

ListNode *MimeTypes=NULL;

void FileExtensionsAdd(const char *Extn, const char *MimeType)
{
	char *Tempstr=NULL, *Token=NULL, *ptr;

	if (! MimeTypes) MimeTypes=ListCreate();
	Tempstr=CopyStr(Tempstr,Extn);
	strlwr(Tempstr);
	ptr=GetToken(Tempstr,"\\S",&Token,0);
	while (ptr)
	{
	SetVar(MimeTypes, Token, MimeType);
	ptr=GetToken(ptr,"\\S",&Token,0);
	}

	DestroyString(Tempstr);
	DestroyString(Token);
}


void FileExtensionsLoad(const char *Path)
{
STREAM *S;
char *Tempstr=NULL, *MimeType=NULL, *ptr;

if (! MimeTypes) MimeTypes=ListCreate();

S=STREAMFileOpen(Path, SF_RDONLY);
if (S)
{
	Tempstr=STREAMReadLine(Tempstr, S);
	while (Tempstr)
	{
		StripTrailingWhitespace(Tempstr);
		StripLeadingWhitespace(Tempstr);

		if (*Tempstr != '#') 
		{
			ptr=GetToken(Tempstr, "\\S", &MimeType,0);
			FileExtensionsAdd(ptr, MimeType);
		}
		Tempstr=STREAMReadLine(Tempstr, S);
	}

STREAMClose(S);
}

DestroyString(Tempstr);
DestroyString(MimeType);
}


char *FileExtensionLookup(const char *Extn)
{
const char *ptr;

if ((! Extn) || (*Extn=='\0')) return("");
ptr=Extn;
if (*ptr=='.') ptr++;

return(GetVar(MimeTypes, ptr));
}


void FileExtensionsLoadDefaults()
{
FileExtensionsAdd("csv", "text/csv");
FileExtensionsAdd("htm html", "text/html");
FileExtensionsAdd("txt", "text/plain");
FileExtensionsAdd("pdf", "application/pdf");
FileExtensionsAdd("lnk", "application/x-ms-shortcut");
FileExtensionsAdd("rtf", "application/rtf");
FileExtensionsAdd("ai eps ps", "application/postscript");
FileExtensionsAdd("exe dll com msi bin", "application/x-msdownload");
FileExtensionsAdd("pl perl pm", "application/perl");
FileExtensionsAdd("lua", "application/lua");
FileExtensionsAdd("js", "application/javascript");
FileExtensionsAdd("jar", "application/java-archive");
FileExtensionsAdd("class", "application/java-vm");
FileExtensionsAdd("doc dot docm", "application/msword");
FileExtensionsAdd("xls xlt xlm xlsm xltm", "application/ms-excel");
FileExtensionsAdd("ppt pps pot ppam pptm potm", "application/vnd.ms-powerpoint");
FileExtensionsAdd("swf", "application/x-shockwave-flash");
FileExtensionsAdd("vb vbe vbs vba vbx", "application/visualbasic");
FileExtensionsAdd("scr wsh ws wsc bat ", "application/x-ms-script");
FileExtensionsAdd("docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document");
FileExtensionsAdd("xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet");
FileExtensionsAdd("xppt", "application/vnd.openxmlformats-officedocument.presentationml.presentation");
}
