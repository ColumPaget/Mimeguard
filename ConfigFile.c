#include "ConfigFile.h"
#include "FileTypeRules.h"
#include "FileExtensions.h"
#include "DocumentStrings.h"
#include "Magic.h"




void ConfigFileParseFileStringRule(const char *Data)
{
char *ContentType=NULL, *String=NULL, *ptr;

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
char *Header=NULL, *Value=NULL, *ptr;

	ptr=GetToken(Data,"\\S",&Header,GETTOKEN_QUOTES);
	ptr=GetToken(ptr,"\\S",&Value,GETTOKEN_QUOTES);
	FileRulesAdd(Header, RULE_HEADER, Value, ptr);

DestroyString(Header);
DestroyString(Value);
}




void ConfigFileLoad(const char *Path)
{
STREAM *S;
char *Tempstr=NULL, *Token=NULL, *ptr;

S=STREAMFileOpen(Path, SF_RDONLY);
if (S)
{
Tempstr=STREAMReadLine(Tempstr, S);
while (Tempstr)
{
StripLeadingWhitespace(Tempstr);
StripTrailingWhitespace(Tempstr);

ptr=GetToken(Tempstr,"\\S",&Token,GETTOKEN_QUOTES);
if (strcmp(Token,"MagicsFile")==0) MagicLoadFile(ptr);
if (strcmp(Token,"MimeTypesFile")==0) FileExtensionsLoad(ptr);
if (strcmp(Token,"FileType")==0) FileTypeRuleParse(ptr,0);
if (strcmp(Token,"FileName")==0) FileTypeRuleParse(ptr, RULE_FILENAME);
if (strcmp(Token,"Extn")==0) FileExtnRuleParse(ptr);
if (strcmp(Token,"String")==0) ConfigFileParseFileStringRule(ptr);
if (strcmp(Token,"Header")==0) ConfigFileParseMimeHeaderRule(ptr);


Tempstr=STREAMReadLine(Tempstr, S);
}

STREAMClose(S);
}

FileRulesLoadPostProcess();

DestroyString(Tempstr);
DestroyString(Token);
}
