#include "ConfigFile.h"
#include "FileTypeRules.h"
#include "FileExtensions.h"
#include "DocumentStrings.h"
#include "FileMagics.h"
#include "URL.h"
#include "IPRegion.h"




void ConfigFileParseFileStringRule(const char *Data)
{
    char *ContentType=NULL, *String=NULL;
    const char *ptr;

    ptr=GetToken(Data,"\\S",&ContentType,GETTOKEN_QUOTES);
    ptr=GetToken(ptr,"\\S",&String,GETTOKEN_QUOTES);
    while (ptr)
    {
        if (StrValid(String)) DocumentStringsAdd(ContentType, String, RULE_EVIL);

        ptr=GetToken(ptr,"\\S",&String,GETTOKEN_QUOTES);
    }

    DestroyString(ContentType);
    DestroyString(String);
}


void ConfigFileParseMimeHeaderRule(const char *Data)
{
    char *Header=NULL, *Value=NULL;
    const char *ptr;

    ptr=GetToken(Data,"\\S",&Header,GETTOKEN_QUOTES);
    ptr=GetToken(ptr,"\\S",&Value,GETTOKEN_QUOTES);
    FileRulesAdd(Header, RULE_HEADER, Value, ptr);

    DestroyString(Header);
    DestroyString(Value);
}




void ConfigFileLoad(const char *Path)
{
    char *Tempstr=NULL, *Token=NULL;
    const char *ptr;
    STREAM *S;

    S=STREAMFileOpen(Path, SF_RDONLY);
    if (S)
    {
        Tempstr=STREAMReadLine(Tempstr, S);
        while (Tempstr)
        {
            StripLeadingWhitespace(Tempstr);
            StripTrailingWhitespace(Tempstr);

            ptr=GetToken(Tempstr,"\\S",&Token,GETTOKEN_QUOTES);
            if (strcasecmp(Token,"FileMagicsFile")==0) FileMagicsLoadFile(ptr);
            if (strcasecmp(Token,"FileMagicsFiles")==0) FileMagicsLoadFile(ptr);
            if (strcasecmp(Token,"MimeTypesFile")==0) FileExtensionsLoad(ptr);
            if (strcasecmp(Token,"MimeTypesFiles")==0) FileExtensionsLoad(ptr);
            if (strcasecmp(Token,"RegionFile")==0) RegionSetFiles(ptr);
            if (strcasecmp(Token,"RegionFiles")==0) RegionSetFiles(ptr);
            if (strcasecmp(Token,"FileType")==0) FileTypeRuleParse(ptr,0);
            if (strcasecmp(Token,"FileName")==0) FileTypeRuleParse(ptr, RULE_FILENAME);
            if (strcasecmp(Token,"Extn")==0) FileExtnRuleParse(ptr);
            if (strcasecmp(Token,"String")==0) ConfigFileParseFileStringRule(ptr);
            if (strcasecmp(Token,"Header")==0) ConfigFileParseMimeHeaderRule(ptr);
            if (strcasecmp(Token,"URLRule")==0) URLParseRule(ptr);


            Tempstr=STREAMReadLine(Tempstr, S);
        }

        STREAMClose(S);
    }
    else printf("ERROR: Failed to open config file %s\n",Path);

    FileRulesLoadPostProcess();

    DestroyString(Tempstr);
    DestroyString(Token);
}
