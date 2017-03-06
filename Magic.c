#include "Magic.h"
#include "OLE.h"
#include "RTF.h"

typedef struct
{
int ByteOffset;
char *Match;
int MatchLen;
char *ContentType;
} TMagic;

ListNode *Magics=NULL;



void MagicConvertString(TMagic *Magic, const char *String)
{
const char *ptr;
char *Token=NULL;

for (ptr=String; *ptr != '\0'; ptr++)
{
	if (*ptr=='\\')
	{
		ptr++;
		switch (*ptr)
		{
		case '\\':
		Magic->Match=AddCharToBuffer(Magic->Match, Magic->MatchLen++, '\\');
		break;

		case 'x':
		ptr++;
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
		Token=CopyStrLen(Token,ptr,3);
		Magic->Match=AddCharToBuffer(Magic->Match, Magic->MatchLen++, strtol(Token, NULL, 8));
		ptr+=2; //only +2, because the ptr++ in the for loop will make it +3
		break;
		}
	}
	else Magic->Match=AddCharToBuffer(Magic->Match, Magic->MatchLen++, *ptr);
}

DestroyString(Token);
}


void MagicAdd(const char *String, char *ContentType)
{
TMagic *Magic;

	if (! Magics) Magics=ListCreate();
	Magic=(TMagic *) calloc(1,sizeof(TMagic));
	ListAddItem(Magics, Magic);
	MagicConvertString(Magic, String);
	Magic->ContentType=CopyStr(Magic->ContentType, ContentType);
}


void MagicLoad(const char *Data)
{
char *Token=NULL, *ContentType=NULL, *ptr;

ptr=GetToken(Data,"\\S",&Token,0);
ptr=GetToken(ptr,"\\S",&Token,0);
if (strcmp(Token,"string")==0)
{
	while (isspace(*ptr)) ptr++;
	ptr=GetToken(ptr,"\\S",&Token,0);
	ptr=GetToken(ptr,"\\S",&ContentType,0);
	MagicAdd(Token, ContentType);
}

DestroyString(ContentType);
DestroyString(Token);
}



void MagicLoadFile(const char *Path)
{
char *Tempstr=NULL, *Token=NULL, *ptr;
STREAM *S;

S=STREAMFileOpen(Path, SF_RDONLY);
if (S)
{
	Tempstr=STREAMReadLine(Tempstr, S);
	while (Tempstr)
	{
		StripTrailingWhitespace(Tempstr);
		StripLeadingWhitespace(Tempstr);
		ptr=GetToken(Tempstr,"\\S",&Token,0);

		if (isdigit(*Token)) MagicLoad(Tempstr);
	Tempstr=STREAMReadLine(Tempstr, S);
	}
}

DestroyString(Tempstr);
DestroyString(Token);
}


void MagicLoadDefaults()
{
MagicAdd("MIME-Version: 1.0", "multipart/mixed");
MagicAdd("PK\\003\\004", "application/zip");
MagicAdd("PK\\005\\006", "application/zip");
MagicAdd("PK\\007\\008", "application/zip");
MagicAdd("%PDF-", "application/pdf");
MagicAdd("BZh", "application/x-bzip2");
MagicAdd("\\xFD7zXZ\000", "application/x-xz");
MagicAdd("MZ", "application/x-msdownload");
MagicAdd("\\x7FELF", "application/elf");
MagicAdd("Rar!", "application/x-rar-compressed");
MagicAdd(OLE_MAGIC, "application/x-ole-storage");
MagicAdd(RTF_MAGIC, "application/rtf");
}


const char *MagicLookupContentType(char *Data)
{
ListNode *Curr;
TMagic *Magic;

Curr=ListGetNext(Magics);
while (Curr)
{
Magic=(TMagic *) Curr->Item;

if (strncmp(Data, Magic->Match, Magic->MatchLen)==0) return(Magic->ContentType);
Curr=ListGetNext(Curr);
}

return(NULL);
}
