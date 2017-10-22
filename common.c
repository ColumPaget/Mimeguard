#include "common.h"
#include "Mime.h"
#include <glob.h>

int Flags=0;
char *ExportPath=NULL;

//nothing to see here, move along
STREAM *g_Rewrite=NULL;

//global policies map, into which we put anything that we need to 
//look up by name
ListNode *g_KeyValueStore=NULL;

char *DecodeMailText(char *RetStr, const char *Text)
{
    const char *sptr, *eptr;

    sptr=Text;
    if (*sptr=='"') sptr++;
    while (isspace(*sptr)) sptr++;
    if (strncmp(sptr, "=?UTF-8?B?", 10)==0)
    {
        sptr+=10;
        eptr=strchr(sptr,'?');
        if (eptr) DecodeBytes(&RetStr, sptr, ENCODE_BASE64);
    }
    else RetStr=CopyStr(RetStr, Text);

    return(RetStr);
}


char *DecodeQuotedPrintable(char *Data, const char *Line)
{
    const char *ptr;
    char *Token=NULL;
    int len=0;

    for (ptr=Line; *ptr !='\0'; ptr++)
    {
        switch (*ptr)
        {
        case '=':
            ptr++;
            if (*ptr=='\n')
            {
                //do nothing. This is a line continuation
            }
            {
                Token=CopyStrLen(Token,ptr,2);
                Data=AddCharToBuffer(Data,len,strtol(Token,NULL,16));
                len++;
                ptr++;
            }
            break;

        default:
            Data=AddCharToBuffer(Data,len,*ptr);
            len++;
            break;
        }
    }

    DestroyString(Token);

    return(Data);
}


int DecodeDocumentLine(const char *Line, int Encoding, char **Data)
{
    int result=-1;

    switch (Encoding)
    {
    case MIMEFLAG_BASE64:
        *Data=SetStrLen(*Data, StrLen(Line));
        result=from64tobits(*Data, Line);
        if (result > -1) (*Data)[result]='\0';
        break;

    case MIMEFLAG_QUOTEDPRINTABLE:
        *Data=DecodeQuotedPrintable(*Data, Line);
        result=StrLen(*Data);
        break;

    default:
        result=StrLen(Line);
        *Data=CopyStrLen(*Data, Line, result);
        break;
    }

    return(result);
}



const char *FileListExtractProto(const char *Path, char **Proto)
{
    const char *ptr;

    *Proto=CopyStr(*Proto, "");
    ptr=strchr(Path,':');
    if (ptr && (strncmp(Path,"mmap",ptr-Path)==0) )
    {
        ptr++;
        *Proto=CopyStrLen(*Proto,Path,ptr-Path);
    }
    else ptr=Path;

    return(ptr);
}

char *FileListExpand(char *RetStr, const char *FilesList)
{
    char *Token=NULL, *Proto=NULL;
    const char *ptr, *p_Path;
    glob_t Glob;
    int i;


    ptr=GetToken(FilesList, ",",&Token,GETTOKEN_QUOTES);
    while (ptr)
    {
        p_Path=FileListExtractProto(Token, &Proto);
        glob(p_Path, 0, 0, &Glob);
        for (i=0; i < Glob.gl_pathc; i++)
        {
            Token=MCopyStr(Token,Proto,Glob.gl_pathv[i],"");
            RetStr=CommaList(RetStr, Token);
        }
        globfree(&Glob);
        ptr=GetToken(ptr, ",",&Token,GETTOKEN_QUOTES);
    }

    DestroyString(Token);
    DestroyString(Proto);

	return(RetStr);
}
