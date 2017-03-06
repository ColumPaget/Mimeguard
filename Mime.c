#include "Mime.h"
#include "Magic.h"
#include "FileExtensions.h"
#include "FileTypeRules.h"
#include "DocumentTypes.h"
#include "Export.h"
#include "Zip.h"
#include "PDF.h"
#include "RTF.h"
#include "OLE.h"


TMimeItem *MimeItemCreate(const char *FileName, const char *ContentType, const char *MagicType)
{
TMimeItem *Item;

Item=(TMimeItem *) calloc(1,sizeof(TMimeItem));
Item->ContentType=CopyStr(Item->ContentType, ContentType);
Item->MagicType=CopyStr(Item->MagicType, MagicType);
Item->ExtnType=CopyStr(Item->ExtnType, "");
Item->FileName=CopyStr(Item->FileName, FileName);
Item->SubItems=ListCreate();

return(Item);
}

void MimeItemDestroy(void *pItem)
{
TMimeItem *Item;

if (! pItem) return;
Item=(TMimeItem *) pItem;
DestroyString(Item->ContentType);
DestroyString(Item->MagicType);
DestroyString(Item->ExtnType);
DestroyString(Item->FileName);
DestroyString(Item->ResultInfo);
ListDestroy(Item->SubItems, MimeItemDestroy);
ListDestroy(Item->Headers, DestroyString);
free(Item);
}


char *MimeHeaderReadLine(char *RetStr, STREAM *S)
{
char *Tempstr=NULL;
int val;

RetStr=STREAMReadLine(RetStr, S);
if (! RetStr) return(RetStr);
StripTrailingWhitespace(RetStr);

val=STREAMPeekChar(S);
if ((val==' ') || (val=='\t'))
{
	Tempstr=STREAMReadLine(Tempstr, S);
	RetStr=CatStr(RetStr,Tempstr);
	StripTrailingWhitespace(RetStr);
}

DestroyString(Tempstr);
return(RetStr);
}



void MimeParseContentType(const char *Data, TMimeItem *Item)
{
char *Token=NULL, *ptr, *Tempstr=NULL;

ptr=GetToken(Data, ";", &Item->ContentType, GETTOKEN_QUOTES);
strlwr(Item->ContentType);
//Item->MagicType=CopyStr(Item->MagicType, Item->ContentType);
do
{
	while (isspace(*ptr)) ptr++;
	ptr=GetToken(ptr, ";", &Token, GETTOKEN_QUOTES);

	switch (*Token)
	{
		case 'b':
		case 'B':
		if (strncasecmp(Token, "boundary=", 9)==0) 
		{
			Tempstr=CopyStr(Tempstr, Token + 9);
			StripTrailingWhitespace(Tempstr);
			StripQuotes(Tempstr);
			Item->Boundary=MCopyStr(Item->Boundary,"--",Tempstr,NULL);
		}
		break;

		case 'f':
		case 'F':
		if (strncasecmp(Token, "filename=", 9)==0) 
		{
			Item->FileName=CopyStr(Item->FileName, Token + 9);
			StripQuotes(Item->FileName);
		}
		break;

		case 'n':
		case 'N':
		if (strncasecmp(Token, "name=", 5)==0) 
		{
			Item->FileName=CopyStr(Item->FileName, Token + 5);
			StripQuotes(Item->FileName);
		}
		break;
	}
} while (ptr);

DestroyString(Token);
}




void MimeParseContentDisposition(const char *Data, TMimeItem *Item)
{
char *Token=NULL, *ptr, *Tempstr=NULL;

ptr=GetToken(Data, ";", &Item->Disposition, GETTOKEN_QUOTES);
do
{
	while (isspace(*ptr)) ptr++;
	ptr=GetToken(ptr, ";", &Token, GETTOKEN_QUOTES);

	switch (*Token)
	{
		case 'f':
		case 'F':
		if (strncasecmp(Token, "filename=", 9)==0) 
		{
			Item->FileName=CopyStr(Item->FileName, Token + 9);
			StripQuotes(Item->FileName);
		}
		break;
	}
} while (ptr);

DestroyString(Token);
}




TMimeItem *MimeReadHeaders(STREAM *S)
{
char *Tempstr=NULL, *Token=NULL, *ptr;
TMimeItem *Item=NULL;

Tempstr=MimeHeaderReadLine(Tempstr, S);
StripTrailingWhitespace(Tempstr);
while (! StrValid(Tempstr))
{

if (! Tempstr) return(NULL);
Tempstr=MimeHeaderReadLine(Tempstr, S);
StripTrailingWhitespace(Tempstr);
}

Item=MimeItemCreate("","text/html","");

while (StrValid(Tempstr))
{
	ptr=GetToken(Tempstr, ":",&Token,0);
	StripTrailingWhitespace(Token);
	while (isspace(*ptr)) ptr++;
	
	HeaderRulesConsider(Item, Token, ptr);
	if (strcasecmp(Token, "Content-Type")==0) MimeParseContentType(ptr, Item);
	else if (strcasecmp(Token, "Content-Disposition")==0) MimeParseContentDisposition(ptr, Item);
	else if (strcasecmp(Token, "Content-Transfer-Encoding")==0) 
	{
		if (strcasecmp(ptr,"base64")==0) Item->Encoding=MIMEENC_BASE64;
		else if (strcasecmp(ptr,"quoted-printable")==0) Item->Encoding=MIMEENC_QUOTEDPRINTABLE;
	}
	else if (strcasecmp(Token, "Content-Location")==0) Item->FileName=CopyStr(Item->FileName, ptr);
	else if (
						(strcasecmp(Token, "Date")==0) ||
						(strcasecmp(Token, "Subject")==0) ||
						(strcasecmp(Token, "From")==0) 
			)
	{
		if (! StrValid(Item->ContentType)) Item->ContentType=CopyStr(Item->ContentType, "message/rfc822");
	}
	
	Tempstr=MimeHeaderReadLine(Tempstr, S);
}

if (StrValid(Item->FileName))
{
	ptr=strrchr(Item->FileName, '.');
	Item->ExtnType=CopyStr(Item->ExtnType, FileExtensionLookup(ptr));
}

DestroyString(Tempstr);
DestroyString(Token);

return(Item);
}



STREAM *MimeReadDocument(STREAM *S, TMimeItem *Item, const char *Boundary, char **SavePath)
{
char *Tempstr=NULL, *Data=NULL;
int lines=0, result, fd=-1, BoundLen;
STREAM *Doc=NULL;

BoundLen=StrLen(Boundary);

if (Flags & FLAG_DEBUG) printf("MIME-DECODE: %s\n",*SavePath);
fd=ExportOpen(Item, SavePath);
if (fd > -1) Doc=STREAMFromFD(fd);

Tempstr=STREAMReadLine(Tempstr, S);
while (Tempstr)
{
	if (BoundLen && (strncmp(Tempstr,Boundary, BoundLen)==0))
	{
		if (Flags & FLAG_DEBUG) printf("MIME-BOUNDARY: %s\n",Tempstr);
		break;
	}

	result=DecodeDocumentLine(Tempstr, Item->Encoding, &Data);

	if (Flags & FLAG_DEBUG) 
	{
		if (result < 0) printf("MIME-BYTES: %d %s\n",result,Tempstr);
	}

	lines++;
	if (Doc && (result > 0)) 
	{
		STREAMWriteBytes(Doc, Data, result);
	}

	Tempstr=STREAMReadLine(Tempstr, S);
}


if (Doc) 
{
	STREAMFlush(Doc);
	STREAMSeek(Doc,0,SEEK_SET);
}
	
DestroyString(Tempstr);
DestroyString(Data);

return(Doc);
}




void MimeReadMultipart(STREAM *S, TMimeItem *Outer)
{
char *Tempstr=NULL;
TMimeItem *Item, *tmpItem;
STREAM *Doc;


	Item=MimeReadHeaders(S);
	while (Item)
	{
		ListAddItem(Outer->SubItems, Item);

		Doc=MimeReadDocument(S, Item, Outer->Boundary, &Tempstr);
		//MimeFileMagic(Doc, Item);
		if (Doc) DocTypeProcess(Doc, Item, Tempstr);
    if (! (Flags & FLAG_EXPORT)) unlink(Tempstr);

	Item=MimeReadHeaders(S);
	}

DestroyString(Tempstr);
}


void MimeFileMagic(STREAM *S, TMimeItem *Item)
{
char *Tempstr=NULL, *Data=NULL;
int result;

	Tempstr=SetStrLen(Tempstr, 255);
	STREAMPeekBytes(S,Tempstr,255);

	result=DecodeDocumentLine(Tempstr, Item->Encoding, &Data);
	
	Item->MagicType=CopyStr(Item->MagicType, MagicLookupContentType(Data));

	DestroyString(Tempstr);
	DestroyString(Data);
}



