#include "PDF.h"
#include "FileTypeRules.h"
#include "DocumentStrings.h"

#define STATE_NONE 0
#define STATE_STREAM 1

#define PDF_SEPARATORS " |[|]|\n|\r|<<|>>|/"

//Gets called recursively
int PDFProcessChunk(STREAM *S, const char *Chunk, ListNode *PDFStrings, ListNode *FoundStrings);


//processes one token from the stream. As '/' counts as a token to the tokenizer, but 
//commands in PDF have the form '/Command' so we prepend the previous token. If it's a '/' it'll
//give the full command string

int PDFProcessToken(const char *PrevToken, const char *Token, ListNode *PDFStrings, ListNode *FoundStrings)
{
char *HexStr=NULL, *UnQuote=NULL;
const char *ptr;
int len=0, LineLen;
int RetVal=RULE_NONE;

HexStr=SetStrLen(HexStr, 3);
HexStr[2]='\0';
if (strcmp(PrevToken,"/")==0) 
{
	UnQuote=AddCharToBuffer(UnQuote,len,'/');
	len++;
}

for (ptr=Token; *ptr!='\0' ; ptr++)
{
	if (*ptr=='#') 
	{
		ptr++;
		HexStr[0]=*ptr;
		ptr++;
		HexStr[1]=*ptr;
		UnQuote=AddCharToBuffer(UnQuote,len,strtol(HexStr,NULL,16) & 0xFF);
	}
	else UnQuote=AddCharToBuffer(UnQuote,len, *ptr);
	len++;
}

if ((Flags & FLAG_DEBUG) && StrValid(HexStr)) printf("HEX: [%s] [%s]\n",Token, UnQuote);

RetVal=DocumentStringsCheck(PDFStrings, UnQuote);
if (RetVal==RULE_EVIL)
{
	RetVal=RULE_EVIL;
	SetVar(FoundStrings, UnQuote, "");
	if (Flags & FLAG_DEBUG) printf("Illegal String: %s\n",UnQuote);
}

DestroyString(UnQuote);
DestroyString(HexStr);

return(RetVal);
}





int PDFProcessSubType(TMimeItem *Item, const char *Data)
{
char *Token=NULL, *Token2=NULL, *SubType=NULL, *ptr;

ptr=GetToken(Data, PDF_SEPARATORS, &Token, GETTOKEN_MULTI_SEPARATORS| GETTOKEN_INCLUDE_SEP);
StripTrailingWhitespace(Token);
while (ptr && (!StrValid(Token)))
{ 
ptr=GetToken(ptr,PDF_SEPARATORS, &Token, GETTOKEN_MULTI_SEPARATORS| GETTOKEN_INCLUDE_SEP);
StripTrailingWhitespace(Token);
}

if (strcmp(Token,"/")==0)
{
	ptr=GetToken(ptr,PDF_SEPARATORS, &Token2, GETTOKEN_MULTI_SEPARATORS| GETTOKEN_INCLUDE_SEP);
	//SubType=PDFProcessToken(SubType, Token, Token2);
}
//else SubType=PDFProcessToken(SubType, "", Token);


DestroyString(SubType);
DestroyString(Token);
DestroyString(Token2);

return(FALSE);
}



int PDFProcessStream(STREAM *S, ListNode *PDFStrings, ListNode *FoundStrings)
{
char *Tempstr=NULL, *Compressed=NULL, *Decompressed=NULL;
int total=0, len;
int RetVal=RULE_NONE;

Tempstr=SetStrLen(Tempstr,BUFSIZ);
len=STREAMReadBytesToTerm(S, Tempstr,BUFSIZ,'\n');
while (len > 0)
{
Tempstr[len]='\0';
if (strncmp(Tempstr,"endstream\r",10)==0) 
{
	len=DeCompressBytes(&Decompressed, "zlib", Compressed, total);
	if (PDFProcessChunk(S, Decompressed, PDFStrings, FoundStrings)==RULE_EVIL) RetVal=RULE_EVIL;
	if (PDFProcessChunk(S, Tempstr+10, PDFStrings, FoundStrings)==RULE_EVIL) RetVal=RULE_EVIL;

	DestroyString(Decompressed);
	DestroyString(Compressed);
	DestroyString(Tempstr);

	return(RetVal);
}

Compressed=realloc(Compressed, (total+len) *2);
memcpy(Compressed+total,Tempstr,len);
total+=len;

len=STREAMReadBytesToTerm(S, Tempstr,BUFSIZ,'\n');
}

DestroyString(Decompressed);
DestroyString(Compressed);
DestroyString(Tempstr);
return(RetVal);
}


int PDFProcessChunk(STREAM *S, const char *Chunk, ListNode *PDFStrings, ListNode *FoundStrings)
{
char *Token=NULL, *P1Token=NULL, *P2Token=NULL;
const char *ptr;
int RetVal=RULE_NONE;

//so we don't have to check for NULL later
P1Token=CopyStr(P1Token,"");
P2Token=CopyStr(P2Token,"");

if (Flags & FLAG_DEBUG) printf("%s",Chunk);
ptr=GetToken(Chunk, PDF_SEPARATORS, &Token, GETTOKEN_MULTI_SEPARATORS| GETTOKEN_INCLUDE_SEP);

while (ptr)
{
	StripTrailingWhitespace(Token);
	if (strcmp(Token,"stream")==0) 
	{
		Token=CopyStr(Token,"");
		if (PDFProcessStream(S, PDFStrings, FoundStrings)==RULE_EVIL) RetVal=RULE_EVIL;
		ptr=NULL;
	}
	else if (strcmp(Token,"endstream")==0) break;
		
	if (StrValid(Token))
	{
		//This not only unquotes any hex quoted strings, but also adds the '/' token
		//back onto the string if the string is a command in the form: /Command
		if (PDFProcessToken(P1Token, Token, PDFStrings, FoundStrings)==RULE_EVIL) RetVal=RULE_EVIL;
		//if ((strcmp(UnQuote,"/Subtype")==0) && PDFProcessSubType(Item, ptr)) RetVal=RULE_EVIL;
	}

	//P1 and P2 tokens are Previous tokens, used to recombine things like '/' onto a string
	P2Token=CopyStr(P2Token,P1Token);
	P1Token=CopyStr(P1Token,Token);
	ptr=GetToken(ptr, PDF_SEPARATORS, &Token, GETTOKEN_MULTI_SEPARATORS | GETTOKEN_INCLUDE_SEP);
}

DestroyString(Token);
DestroyString(P1Token);
DestroyString(P2Token);

return(RetVal);
}



int PDFProcessCommands(STREAM *S, TMimeItem *Item, ListNode *FoundStrings)
{
char *Tempstr=NULL;
		//P1 and P2 tokens are Previous tokens, used to recombine things like '/' onto a string
int RetVal=RULE_NONE, len;
ListNode *PDFStrings;


	PDFStrings=DocumentStringsGetList("application/pdf");
	Tempstr=SetStrLen(Tempstr,BUFSIZ);
	len=STREAMReadBytesToTerm(S, Tempstr,BUFSIZ,'\n');
	while (len > 0)
	{
	Tempstr[len]='\0';
	if (PDFProcessChunk(S, Tempstr, PDFStrings, FoundStrings)==RULE_EVIL) RetVal=RULE_EVIL;
	len=STREAMReadBytesToTerm(S, Tempstr,BUFSIZ,'\n');
	}

DestroyString(Tempstr);

return(RetVal);
}




int PDFFileProcess(const char *Path, TMimeItem *Item)
{
int result, i;
ListNode *FoundStrings, *Curr;
char *Tempstr=NULL;
STREAM *S;

FoundStrings=ListCreate();
if ((Flags & FLAG_DEBUG)) printf("Check PDF: [%s]\n",Path);
S=STREAMFileOpen(Path, SF_RDONLY);
if (S)
{
 result=PDFProcessCommands(S, Item, FoundStrings);
 if (result & RULE_EVIL) Item->RulesResult=result | RULE_MACROS;
}


Curr=ListGetNext(FoundStrings);
if (Curr)
{
	while (Curr)
	{
		Tempstr=CommaList(Tempstr,Curr->Tag);
		Curr=ListGetNext(Curr);
	}
	Item->ResultInfo=MCopyStr(Item->ResultInfo,"Forbidden Commands [",Tempstr,"]",NULL);
}

ListDestroy(FoundStrings, DestroyString);
DestroyString(Tempstr);

STREAMClose(S);

return(Item->RulesResult);
}

