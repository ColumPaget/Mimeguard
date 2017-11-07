#include "Mime.h"
#include "FileMagics.h"
#include "FileExtensions.h"
#include "FileTypeRules.h"
#include "DocumentTypes.h"
#include "Export.h"
#include "Zip.h"
#include "PDF.h"
#include "RTF.h"
#include "OLE.h"


TMimeItem *MimeItemCreate(const char *FileName, const char *ContentType, const char *FileMagicsType)
{
    TMimeItem *Item;
		const char *ptr;

    Item=(TMimeItem *) calloc(1,sizeof(TMimeItem));
    Item->ContentType=CopyStr(Item->ContentType, ContentType);
    Item->FileMagicsType=CopyStr(Item->FileMagicsType, FileMagicsType);
    Item->ExtnType=CopyStr(Item->ExtnType, "");
    Item->FileName=CopyStr(Item->FileName, FileName);
    Item->SubItems=ListCreate();
    Item->Errors=ListCreate();
    if (StrValid(FileName))
		{
			ptr=strrchr(FileName, '.');
			if (ptr) Item->ExtnType=CopyStr(Item->ExtnType, FileExtensionLookup(ptr));
		}

    return(Item);
}

void MimeItemDestroy(void *pItem)
{
    TMimeItem *Item;

    if (! pItem) return;
    Item=(TMimeItem *) pItem;
    DestroyString(Item->ContentType);
    DestroyString(Item->FileMagicsType);
    DestroyString(Item->ExtnType);
    DestroyString(Item->FileName);
    ListDestroy(Item->SubItems, MimeItemDestroy);
    ListDestroy(Item->Headers, DestroyString);
    ListDestroy(Item->Errors, DestroyString);
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
    char *Token=NULL, *Tempstr=NULL;
    const char *ptr;

    ptr=GetToken(Data, ";", &Item->ContentType, GETTOKEN_QUOTES);
    strlwr(Item->ContentType);
//Item->FileMagicsType=CopyStr(Item->FileMagicsType, Item->ContentType);
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
                Item->FileName=DecodeMailText(Item->FileName, Token + 9);
                StripQuotes(Item->FileName);
            }
            break;

        case 'n':
        case 'N':
            if (strncasecmp(Token, "name=", 5)==0)
            {
                Item->FileName=DecodeMailText(Item->FileName, Token + 5);
                StripQuotes(Item->FileName);
            }
            break;
        }
    }
    while (ptr);

    DestroyString(Token);
}




void MimeParseContentDisposition(const char *Data, TMimeItem *Item)
{
    char *Token=NULL, *Tempstr=NULL;
    const char *ptr;

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
                Item->FileName=DecodeMailText(Item->FileName, Token + 9);
                StripQuotes(Item->FileName);
            }
            break;
        }
    }
    while (ptr);

    DestroyString(Token);
}




TMimeItem *MimeReadHeaders(STREAM *S)
{
    char *Tempstr=NULL, *Name=NULL, *Value=NULL;
    const char *ptr;
    TMimeItem *Item=NULL;

    Tempstr=MimeHeaderReadLine(Tempstr, S);
    while (! StrValid(Tempstr))
    {

        if (! Tempstr) return(NULL);
        Tempstr=MimeHeaderReadLine(Tempstr, S);
    }

    Item=MimeItemCreate("","","");

    while (StrValid(Tempstr))
    {
        /*
        	if (g_Rewrite)
        	{
        	STREAMWriteLine(Tempstr, g_Rewrite);
        	STREAMWriteLine("\n", g_Rewrite);
        	}
        */

        ptr=GetToken(Tempstr, ":",&Name,0);
        StripTrailingWhitespace(Name);
        while (isspace(*ptr)) ptr++;

        Value=DecodeMailText(Value, ptr);
        HeaderRulesConsider(Item, Name, Value);
        if (strcasecmp(Name, "Content-Type")==0) MimeParseContentType(Value, Item);
        else if (strcasecmp(Name, "Content-Disposition")==0) MimeParseContentDisposition(Value, Item);
        else if (strcasecmp(Name, "Content-Transfer-Encoding")==0)
        {
            if (strcasecmp(Value,"base64")==0) Item->Flags |=MIMEFLAG_BASE64;
            else if (strcasecmp(Value,"quoted-printable")==0) Item->Flags |=MIMEFLAG_QUOTEDPRINTABLE;
        }
        else if (strcasecmp(Name, "Content-Location")==0) Item->FileName=DecodeMailText(Item->FileName, Value);
        else if (
            (strcasecmp(Name, "Date")==0) ||
            (strcasecmp(Name, "Subject")==0) ||
            (strcasecmp(Name, "From")==0)
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
    DestroyString(Name);
    DestroyString(Value);

    return(Item);
}



STREAM *MimeReadDocument(STREAM *S, TMimeItem *Item, const char *Boundary, char **SavePath)
{
    char *Tempstr=NULL, *Data=NULL;
    int lines=0, result, fd=-1, BoundLen;
    STREAM *Doc=NULL;

    BoundLen=StrLen(Boundary);

    if (g_Flags & FLAG_DEBUG) printf("MIME-DECODE: %s\n",*SavePath);
    fd=ExportOpen(Item, SavePath);
    if (fd > -1) Doc=STREAMFromFD(fd);

    Tempstr=STREAMReadLine(Tempstr, S);
    while (Tempstr)
    {
        if (BoundLen && (strncmp(Tempstr,Boundary, BoundLen)==0))
        {
            if (g_Flags & FLAG_DEBUG) printf("MIME-BOUNDARY: %s\n",Tempstr);
            break;
        }

        result=DecodeDocumentLine(Tempstr, Item->Flags & MIMEFLAG_ENCODING, &Data);

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
				if (StrEnd(Item->ContentType) && StrEnd(Item->FileName)) Item->ContentType=CopyStr(Item->ContentType,"text/plain");
        ListAddItem(Outer->SubItems, Item);

        Doc=MimeReadDocument(S, Item, Outer->Boundary, &Tempstr);
        //MimeFileFileMagics(Doc, Item);
        if (Doc) DocTypeProcess(Doc, Item, Tempstr);
        if (! (g_Flags & FLAG_EXPORT)) unlink(Tempstr);
        STREAMClose(Doc);
        Item=MimeReadHeaders(S);
    }
    DestroyString(Tempstr);
}


void MimeFileFileMagics(STREAM *S, TMimeItem *Item)
{
    char *Tempstr=NULL, *Data=NULL;
    int result;

    Tempstr=SetStrLen(Tempstr, 255);
    STREAMPeekBytes(S,Tempstr,255);

    result=DecodeDocumentLine(Tempstr, Item->Flags & MIMEFLAG_ENCODING, &Data);

    Item->FileMagicsType=CopyStr(Item->FileMagicsType, FileMagicsLookupContentType(Data));

    DestroyString(Tempstr);
    DestroyString(Data);
}



