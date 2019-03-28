#include "common.h"
#include "Mime.h"
#include <glob.h>

TConfig *Config=NULL;
char *ExportPath=NULL;
int EvilExitStatus=1; //default exit status if a mail is found to have a problem

//nothing to see here, move along
STREAM *g_Rewrite=NULL;



//global policies map, into which we put anything that we need to
//look up by name
ListNode *g_KeyValueStore=NULL;



void ConfigInit()
{
    Config=(TConfig *) calloc(1, sizeof(TConfig));
    Config->Flags = FLAG_SHOW_SAFE | FLAG_SHOW_EVIL | FLAG_SHOW_CURR;
    Config->SmtpAddress=CopyStr(Config->SmtpAddress, "tcp:127.0.0.1:25");
}


char *DecodeMailText(char *RetStr, const char *Text)
{
    const char *sptr, *eptr;
    char *wptr;

    sptr=Text;
    if (*sptr=='"') sptr++;
    while (isspace(*sptr)) sptr++;
    if (strncasecmp(sptr, "=?UTF-8?B?", 10)==0)
    {
        sptr+=10;
        eptr=strchr(sptr,'?');
        if (eptr) DecodeBytes(&RetStr, sptr, ENCODE_BASE64);
        for (wptr=RetStr; *wptr !='\0'; wptr++) if ((*wptr < 32) || (*wptr > 126)) *wptr='?';
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

    Destroy(Token);

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

    Destroy(Token);
    Destroy(Proto);

    return(RetStr);
}
