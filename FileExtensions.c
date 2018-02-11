#include "common.h"

void FileExtensionsAdd(const char *Extn, const char *MimeType)
{
    char *Tempstr=NULL, *Token=NULL;
    const char *ptr, *tptr;;

    Tempstr=CopyStr(Tempstr,Extn);
    strlwr(Tempstr);
    ptr=GetToken(Tempstr,"\\S",&Token,0);
    while (ptr)
    {
        tptr=Token;
        while ((*tptr=='.') || isspace(*tptr)) tptr++;
        SetTypedVar(g_KeyValueStore, tptr, MimeType, KV_FILE_EXTN);
        ptr=GetToken(ptr,"\\S",&Token,0);
    }

    Destroy(Tempstr);
    Destroy(Token);
}


void FileExtensionsLoadFile(const char *Path)
{
    STREAM *S;
    char *Tempstr=NULL, *MimeType=NULL;
    const char *ptr;

    S=STREAMOpen(Path, "r");
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

    Destroy(Tempstr);
    Destroy(MimeType);
}


void FileExtensionsLoad(const char *Path)
{
    char *Tempstr=NULL, *Token=NULL;
    const char *ptr;

    Tempstr=FileListExpand(Tempstr, Path);
    ptr=GetToken(Tempstr,",",&Token, GETTOKEN_QUOTES);
    while (ptr)
    {
        StripTrailingWhitespace(Token);
        StripLeadingWhitespace(Token);
        FileExtensionsLoadFile(Token);
        ptr=GetToken(ptr,",",&Token, GETTOKEN_QUOTES);
    }

    Destroy(Tempstr);
    Destroy(Token);
}



const char *FileExtensionLookup(const char *Extn)
{
    const char *ptr;

    if ((! Extn) || (*Extn=='\0')) return("");
    ptr=Extn;
    if (*ptr=='.') ptr++;

    return(GetTypedVar(g_KeyValueStore, ptr, KV_FILE_EXTN));
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

    FileExtensionsAdd("zip", "application/zip");

    FileExtensionsAdd("jpg jpeg", "image/jpeg");
    FileExtensionsAdd("gif", "image/gif");
    FileExtensionsAdd("png", "image/png");
    FileExtensionsAdd("bmp", "image/bmp");
    FileExtensionsAdd("tif tiff", "image/tiff");
}
