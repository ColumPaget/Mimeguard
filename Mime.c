#include "Mime.h"
#include "FileMagics.h"
#include "FileExtensions.h"
#include "FileTypeRules.h"
#include "DocumentTypes.h"
#include "EmailHeaders.h"
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
        if (ptr) 
				{
					Item->ExtnType=CopyStr(Item->ExtnType, FileExtensionLookup(ptr));
					if (! StrValid(Item->ContentType)) Item->ContentType=CopyStr(Item->ContentType, Item->ExtnType);
				}
    }
		

    return(Item);
}

void MimeItemDestroy(void *pItem)
{
    TMimeItem *Item;

    if (! pItem) return;
    Item=(TMimeItem *) pItem;
    Destroy(Item->ContentType);
    Destroy(Item->FileMagicsType);
    Destroy(Item->ExtnType);
    Destroy(Item->FileName);
    ListDestroy(Item->SubItems, MimeItemDestroy);
    ListDestroy(Item->Headers, Destroy);
    ListDestroy(Item->Errors, Destroy);
    free(Item);
}


char *MimeItemGetContentType(TMimeItem *Item)
{
    if ((! StrValid(Item->ContentType)) || (strcasecmp(Item->ContentType,"octet-stream")==0) || (strcasecmp(Item->ContentType,"application/octet-stream")==0) )
    {
        if (StrValid(Item->FileMagicsType)) return(Item->FileMagicsType);
        if (StrValid(Item->ExtnType)) return(Item->ExtnType);
    }

    return(Item->ContentType);
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

    Destroy(Tempstr);
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

    Destroy(Token);
}




void MimeParseContentDisposition(const char *Data, TMimeItem *Item)
{
    char *Token=NULL, *Tempstr=NULL;
    const char *ptr, *tptr;

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

    Destroy(Token);
}



void MimeHeaderDecode(const char *Header, char **Name, char **Value)
{
const char *ptr;
char *Tempstr=NULL, *wptr;

	*Value=CopyStr(*Value, "");
	ptr=GetToken(Header, ":",Name,0);
	StripTrailingWhitespace(*Name);
	if (ptr)
	{
	while (isspace(*ptr)) ptr++;

	Tempstr=DecodeMailText(Tempstr, ptr);
	if (strcmp(*Name, "From")==0)
	{
		ptr=strchr(Tempstr,'<');
		if (ptr)
		{
			*Value=CopyStr(*Value, ptr+1);
			wptr=strchr(*Value,'>');
			if (wptr) *wptr='\0';
		}
		else *Value=CopyStr(*Value, Tempstr);
	}
	else *Value=CopyStr(*Value, Tempstr);
	}

DestroyString(Tempstr);
}



TMimeItem *MimeReadHeaders(STREAM *S, int CopyToRewrite)
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
				MimeHeaderDecode(Tempstr, &Name, &Value);

        EmailHeaderRulesConsider(Item, Name, Value);
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

			if (CopyToRewrite && g_Rewrite)
			{
				STREAMWriteLine(Tempstr, g_Rewrite);
				if (strcasecmp(Name,"Subject")==0) STREAMWriteLine("  (attachments stripped)", g_Rewrite);
				STREAMWriteLine("\r\n", g_Rewrite);
			}

       Tempstr=MimeHeaderReadLine(Tempstr, S);
    }

		// if rewriting we need to add a blank line to end the headers
		if (g_Rewrite) STREAMWriteLine("\r\n", g_Rewrite);

    ptr=strrchr(Item->FileName, '.');
    if (ptr) Item->ExtnType=CopyStr(Item->ExtnType, FileExtensionLookup(ptr));


    Destroy(Tempstr);
    Destroy(Name);
    Destroy(Value);

    return(Item);
}




STREAM *MimeReadDocument(STREAM *S, TMimeItem *Item, const char *Boundary, char **SavePath)
{
    char *Tempstr=NULL, *Data=NULL;
    int lines=0, result, fd=-1, BoundLen;
    STREAM *Doc=NULL;

    BoundLen=StrLen(Boundary);

    if (Config->Flags & FLAG_DEBUG) printf("MIME-DECODE: %s\n",*SavePath);
    fd=ExportOpen(Item, SavePath);
    if (fd > -1) Doc=STREAMFromFD(fd);

		Item->Flags |= MIMEFLAG_DECODED;
    Tempstr=STREAMReadLine(Tempstr, S);
    while (Tempstr)
    {
        if (BoundLen && (strncmp(Tempstr,Boundary, BoundLen)==0))
        {
            if (Config->Flags & FLAG_DEBUG) printf("MIME-BOUNDARY: %s\n",Tempstr);
            break;
        }

        result=DecodeDocumentLine(Tempstr, Item->Flags & MIMEFLAG_ENCODING, &Data);

				//if this is the first line of then do FileMagic detection on it
				if (lines==0) 
				{
				Item->FileMagicsType=CopyStr(Item->FileMagicsType, FileMagicsLookupContentType(Data));
				if (FileMagicsIsText(Data)) Item->Flags |=MIMEFLAG_ISTEXT;
				}

        lines++;
        if (Doc && (result > 0))
        {
						if (! FileMagicsIsText(Data)) Item->Flags &= ~MIMEFLAG_ISTEXT;
            STREAMWriteBytes(Doc, Data, result);
        }

        Tempstr=STREAMReadLine(Tempstr, S);
    }

    if (Doc)
    {
        STREAMFlush(Doc);
        STREAMSeek(Doc,0,SEEK_SET);
    }

    Destroy(Tempstr);
    Destroy(Data);

    return(Doc);
}




void MimeReadMultipart(STREAM *S, TMimeItem *Outer)
{
    char *Tempstr=NULL;
    TMimeItem *Item, *tmpItem;
    STREAM *Doc;

    Item=MimeReadHeaders(S,FALSE);
    while (Item)
    {
        if (StrEnd(Item->ContentType) && StrEnd(Item->FileName)) Item->ContentType=CopyStr(Item->ContentType,"text/plain");
        ListAddItem(Outer->SubItems, Item);

        Doc=MimeReadDocument(S, Item, Outer->Boundary, &Tempstr);
        if (Doc) DocTypeProcess(Doc, Item, Tempstr);
        STREAMClose(Doc);

				if (g_Rewrite && (Outer->Flags & MIMEFLAG_ROOT)) RewriteCopyDocument(Outer, Item, Tempstr);
				if (! (Config->Flags & FLAG_EXPORT)) unlink(Tempstr);
        Item=MimeReadHeaders(S, FALSE);
    }
    Destroy(Tempstr);
}


