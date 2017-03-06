#include "RTF.h"
#include "FileTypeRules.h"
#include "DocumentStrings.h"

#define RTF_SEPARATORS " |	|\n|\r|\\\\|{|}|;"


char *RTFProcessToken(char *RetStr, const char *PrevToken, const char *Token)
{
if (strcmp(PrevToken,"\\")==0) 
{
	RetStr=MCopyStr(RetStr,"\\",Token,NULL);
}
else RetStr=CopyStr(RetStr,Token);

return(RetStr);
}


char *RTFParseSubtype(char *Data, char **SubType)
{
char *Token=NULL, *ptr;

	*SubType=CopyStr(*SubType, "");
	ptr=GetToken(Data, RTF_SEPARATORS, &Token, GETTOKEN_MULTI_SEPARATORS | GETTOKEN_INCLUDE_SEP | GETTOKEN_BACKSLASH);
	while (ptr && (strcmp(Token,"}") !=0) )
	{
		*SubType=CatStr(*SubType, Token);
		ptr=GetToken(ptr, RTF_SEPARATORS, &Token, GETTOKEN_MULTI_SEPARATORS | GETTOKEN_INCLUDE_SEP | GETTOKEN_BACKSLASH);
	}

return(Token);
}



int RTFProcessCommands(STREAM *S, TMimeItem *Item, ListNode *FoundStrings)
{
char *Tempstr=NULL, *Token=NULL, *UnQuote=NULL, *P1Token=NULL, *P2Token=NULL, *ptr;
char *SubType=NULL;
int RetVal=RULE_NONE;
ListNode *RTFStrings;

P1Token=CopyStr(P1Token,"");
P2Token=CopyStr(P2Token,"");

RTFStrings=DocumentStringsGetList("application/rtf");

	Tempstr=STREAMReadLine(Tempstr,S);
	while (Tempstr)
	{
		ptr=GetToken(Tempstr, RTF_SEPARATORS, &Token, GETTOKEN_MULTI_SEPARATORS| GETTOKEN_INCLUDE_SEP | GETTOKEN_BACKSLASH);
		while (ptr)
		{
		StripTrailingWhitespace(Token);
		if (StrValid(Token))
		{
			UnQuote=RTFProcessToken(UnQuote, P1Token, Token);

			if (DocumentStringsCheck(RTFStrings, UnQuote)==RULE_EVIL)
			{
				printf("Illegal string: %s\n",UnQuote);
				RetVal=RULE_EVIL;
				SetVar(FoundStrings, UnQuote, "");
			}

			if (strcasecmp(UnQuote,"\\objclass")==0) 
			{
					ptr=RTFParseSubtype(ptr, &SubType);
			}
		}

		P2Token=CopyStr(P2Token,P1Token);
		P1Token=CopyStr(P1Token,Token);
		ptr=GetToken(ptr, RTF_SEPARATORS, &Token, GETTOKEN_MULTI_SEPARATORS | GETTOKEN_INCLUDE_SEP | GETTOKEN_BACKSLASH);
		}
	  Tempstr=STREAMReadLine(Tempstr,S);
	}

DestroyString(Tempstr);
DestroyString(UnQuote);
DestroyString(Token);
DestroyString(P1Token);
DestroyString(P2Token);
DestroyString(SubType);

return(RetVal);
}


STREAM *RTFOpen(const char *Path, TMimeItem *Item)
{
char *Tempstr=NULL;
STREAM *S;

S=STREAMFileOpen(Path, SF_RDONLY);
if (S) 
{
	Tempstr=SetStrLen(Tempstr, 20);
	STREAMPeekBytes(S, Tempstr, 20);
	if (strncmp(Tempstr,RTF_MAGIC,6)!=0)
	{
	Item->RulesResult=RULE_MALFORMED | RULE_EVIL;
  Item->ResultInfo=CommaList(Item->ResultInfo,"malformed: bad header signature");
	STREAMClose(S);
	S=NULL;
	}
}

DestroyString(Tempstr);
return(S);
}


int RTFFileProcess(const char *Path, TMimeItem *Item)
{
int result, i;
int RetVal=RULE_SAFE;
ListNode *FoundStrings, *Curr;
char *Tempstr=NULL;
STREAM *S;

FoundStrings=ListCreate();
if ((Flags & FLAG_DEBUG)) printf("Check RTF: [%s]\n",Path);
S=RTFOpen(Path, Item);
if (S)
{
	RetVal=RTFProcessCommands(S, Item, FoundStrings);
	Curr=ListGetNext(FoundStrings);
	if (Curr)
	{
		while (Curr)
		{
			Tempstr=CommaList(Tempstr,Curr->Tag);
			Curr=ListGetNext(Curr);
		}
		Item->ResultInfo=MCatStr(Item->ResultInfo,"Forbidden Commands [",Tempstr,"[ ",NULL);
	}
}

ListDestroy(FoundStrings, DestroyString);
DestroyString(Tempstr);

return(RetVal);
}

