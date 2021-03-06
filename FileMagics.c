#include "FileMagics.h"
#include "OLE.h"
#include "RTF.h"

typedef struct
{
    int ByteOffset;
    char *Match;
    int MatchLen;
    char *ContentType;
} TFileMagics;

ListNode *FileMagics=NULL;



void FileMagicsConvertString(TFileMagics *Magic, const char *String)
{
    const char *ptr;
    char *Token=NULL;

    for (ptr=String; *ptr != '\0'; ptr++)
    {
        if (*ptr=='\\')
        {
            switch (*(ptr+1))
            {
            case '\\':
                Magic->Match=AddCharToBuffer(Magic->Match, Magic->MatchLen++, '\\');
								ptr++;
                break;

            case 'x':
                ptr+=2;
                Token=CopyStrLen(Token,ptr,2);
                Magic->Match=AddCharToBuffer(Magic->Match, Magic->MatchLen++, strtol(Token,NULL, 16));
                ptr++; //only ++, because the for loop will do another ++
                break;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
								ptr++;
                Token=CopyStrLen(Token,ptr,3);
                Magic->Match=AddCharToBuffer(Magic->Match, Magic->MatchLen++, strtol(Token, NULL, 8));
                ptr+=2; //only +2, because the ptr++ in the for loop will make it +3
                break;
            }
        }
        else Magic->Match=AddCharToBuffer(Magic->Match, Magic->MatchLen++, *ptr);
    }

    Destroy(Token);
}


void FileMagicsAdd(const char *String, char *ContentType)
{
    TFileMagics *Magic;

    if (! FileMagics) FileMagics=ListCreate();
    Magic=(TFileMagics *) calloc(1,sizeof(TFileMagics));
    ListAddItem(FileMagics, Magic);
    FileMagicsConvertString(Magic, String);
    Magic->ContentType=CopyStr(Magic->ContentType, ContentType);
}


void FileMagicsParse(const char *Data)
{
    char *Token=NULL, *ContentType=NULL;
    const char *ptr;

    ptr=GetToken(Data,"\\S",&Token,0);
    ptr=GetToken(ptr,"\\S",&Token,0);
    if (strcmp(Token,"string")==0)
    {
        while (isspace(*ptr)) ptr++;
        ptr=GetToken(ptr,"\\S",&Token,0);
        ptr=GetToken(ptr,"\\S",&ContentType,0);
        FileMagicsAdd(Token, ContentType);
    }

    Destroy(ContentType);
    Destroy(Token);
}



void FileMagicsLoadFile(const char *Path)
{
    char *Tempstr=NULL, *Token=NULL;
    const char *ptr;
    STREAM *S;

    Tempstr=FileListExpand(Tempstr, Path);
    S=STREAMOpen(Tempstr, "r");
    if (S)
    {
        Tempstr=STREAMReadLine(Tempstr, S);
        while (Tempstr)
        {
            StripTrailingWhitespace(Tempstr);
            StripLeadingWhitespace(Tempstr);
            ptr=GetToken(Tempstr,"\\S",&Token,0);

            if (isdigit(*Token)) FileMagicsParse(Tempstr);
            Tempstr=STREAMReadLine(Tempstr, S);
        }
    }

    Destroy(Tempstr);
    Destroy(Token);
}



void FileMagicsLoad(const char *Path)
{
    char *Tempstr=NULL, *Token=NULL;
    const char *ptr;

    Tempstr=FileListExpand(Tempstr, Path);
    ptr=GetToken(Tempstr,",",&Token, GETTOKEN_QUOTES);
    while (ptr)
    {
        StripTrailingWhitespace(Token);
        StripLeadingWhitespace(Token);
        FileMagicsLoadFile(Token);
        ptr=GetToken(ptr,",",&Token, GETTOKEN_QUOTES);
    }

    Destroy(Tempstr);
    Destroy(Token);
}


void FileMagicsLoadDefaults()
{
    FileMagicsAdd("MIME-Version: 1.0", "multipart/mixed");
    FileMagicsAdd("PK\\003\\004", "application/zip");
    FileMagicsAdd("PK\\005\\006", "application/zip");
    FileMagicsAdd("PK\\007\\008", "application/zip");
    FileMagicsAdd("%PDF-", "application/pdf");
    FileMagicsAdd("BZh", "application/x-bzip2");
    FileMagicsAdd("\\xFD7zXZ\000", "application/x-xz");
    FileMagicsAdd("MZ", "application/x-msdownload");
    FileMagicsAdd("\\x7FELF", "application/elf");
    FileMagicsAdd("Rar!", "application/x-rar-compressed");
    FileMagicsAdd("<html", "text/html");
    FileMagicsAdd("<HTML", "text/html");
    FileMagicsAdd("<?xml", "application/xml");
    FileMagicsAdd(OLE_MAGIC, "application/x-ole-storage");
    FileMagicsAdd(RTF_MAGIC, "application/rtf");
    FileMagicsAdd("{\\\\rt", "application/rtf");
}


const char *FileMagicsLookupContentType(char *Data)
{
    ListNode *Curr;
    TFileMagics *Magic;

    Curr=ListGetNext(FileMagics);
    while (Curr)
    {
        Magic=(TFileMagics *) Curr->Item;

				if (Config->Flags & FLAG_DEBUG) printf("FileMagics: compare %d bytes of [%s] with [%s]\n",Magic->MatchLen, Magic->Match, Data);
        if (strncmp(Data, Magic->Match, Magic->MatchLen)==0) return(Magic->ContentType);
        Curr=ListGetNext(Curr);
    }

    return(NULL);
}


int FileMagicsIsText(const char *Data)
{
		const char *ptr;
		int IsText=TRUE;

		for (ptr=Data; *ptr !='\0'; ptr++)
		{
			if ( ((*ptr < 32) || (*ptr > 127)) && (*ptr != '\n') && (*ptr != '\r') ) IsText=FALSE;
		}
			
		return(IsText);
}


char *FileMagicsExamine(char *RetStr, STREAM *S, int *Flags)
{
    char *Tempstr=NULL;
		int result;

    Tempstr=SetStrLen(Tempstr, 256);
    result=STREAMPeekBytes(S,Tempstr,255);
		if (result > 0) Tempstr[result]='\0';
		else Tempstr[0]='\0';

    RetStr=CopyStr(RetStr, FileMagicsLookupContentType(Tempstr));
		if (Flags && FileMagicsIsText(Tempstr)) *Flags |= MIMEFLAG_ISTEXT;
	

    Destroy(Tempstr);

    return(RetStr);
}

