#include "fnmatch.h"
#include "FileTypeRules.h"
#include "DocumentStrings.h"
#include "FileExtensions.h"

typedef struct
{
int Flags;
char *ContentType;
char *Contains;
char *Equivalent;
} TFileRule;


ListNode *Rules=NULL;
ListNode *HeaderRules=NULL;

void FileRulesAdd(const char *MimeType, int Flags, const char *Contains, const char *Equivalent)
{
ListNode *Node, *Head;
TFileRule *Rule;

if (! Rules) Rules=ListCreate();
if (! HeaderRules) HeaderRules=ListCreate();

Rule=(TFileRule *) calloc(1,sizeof(TFileRule));
Rule->Flags=Flags;
Rule->ContentType=CopyStr(Rule->ContentType, MimeType);
Rule->Equivalent=CopyStr(Rule->Equivalent, Equivalent);

if (Flags & RULE_CONTAINER)
{
	if (StrValid(Contains)) Rule->Contains=CopyStr(Rule->Contains, Contains);
	else Rule->Contains=CopyStr(Rule->Contains, "*");
	ListAddNamedItem(Rules, Rule->ContentType, Rule);
}
else if (Flags & RULE_HEADER) 
{
	if (StrValid(Contains)) Rule->Contains=CopyStr(Rule->Contains, Contains);
	else Rule->Contains=CopyStr(Rule->Contains, "*");
	ListAddItem(HeaderRules, Rule);
}
else ListAddNamedItem(Rules, Rule->ContentType, Rule);
}




void FileTypeRuleParse(const char *Data, int Flags)
{
char *Match=NULL, *Equiv=NULL, *Contains=NULL, *Token=NULL, *ptr;

	ptr=GetToken(Data,"\\S",&Match,GETTOKEN_QUOTES);
	ptr=GetToken(ptr,"\\S",&Token,GETTOKEN_QUOTES);

	while (ptr)
	{
		if (strcasecmp(Token,"evil")==0) Flags |= RULE_EVIL;
		if (strcasecmp(Token,"safe")==0) Flags |= RULE_SAFE;
		if (strcasecmp(Token,"container")==0) Flags |= RULE_CONTAINER;
		if (strcasecmp(Token,"allow-blank-ctype")==0) Flags |= RULE_BLANK_CONTYPE;
		if (strcasecmp(Token,"allow-blank-magic")==0) Flags |= RULE_BLANK_MAGIC;
		if (strcasecmp(Token,"extn=magic")==0) Flags |= RULE_EXTN_MATCHES_MAGIC;
		if (strcasecmp(Token,"ctype=magic")==0) Flags |= RULE_CONTYPE_MATCHES_MAGIC;
		if (strncasecmp(Token,"equiv=",6)==0) Equiv=CopyStr(Equiv, Token+6);
		if (strncasecmp(Token,"contains=",9)==0) 
		{
			Contains=CopyStr(Contains, Token+9);
			Flags |= RULE_CONTAINER;
		}
	

	ptr=GetToken(ptr,"\\S",&Token,GETTOKEN_QUOTES);
	}

	FileRulesAdd(Match, Flags, Contains, Equiv);

DestroyString(Token);
DestroyString(Equiv);
DestroyString(Contains);
DestroyString(Match);
}


void FileExtnRuleParse(const char *Data)
{
char *ContentType=NULL, *Extn=NULL, *ptr, *eptr;

	ptr=GetToken(Data,"\\S",&ContentType,GETTOKEN_QUOTES);
	ptr=GetToken(ptr,"\\S",&Extn,GETTOKEN_QUOTES);
	while (ptr)
	{
		eptr=Extn;
		if (*eptr=='.') eptr++;
		if (StrValid(Extn)) FileExtensionsAdd(eptr, ContentType);
		ptr=GetToken(ptr,"\\S",&Extn,GETTOKEN_QUOTES);
	}

DestroyString(ContentType);
DestroyString(Extn);
}



void FileRulesLoadPostProcess()
{
ListNode *Curr, *Node;
TFileRule *Rule, *OtherRule;
int tmpFlags;

Curr=ListGetNext(Rules);
while (Curr)
{
Rule=(TFileRule *) Curr->Item;
if (StrValid(Rule->Equivalent))
{
	Node=ListFindNamedItem(Rules, Rule->Equivalent);
	if (Node)
	{
		OtherRule=(TFileRule *) Node->Item;
		tmpFlags=Rule->Flags;
		Rule->Flags=OtherRule->Flags |= tmpFlags;
		Rule->Contains=MCatStr(Rule->Contains, ",", OtherRule->Contains, NULL);

	}
}

Curr=ListGetNext(Curr);
}

}

char *TranslateMimeTypeEquivalent(const char *MimeType)
{
ListNode *Node;
TFileRule *Rule;

Node=ListFindNamedItem(Rules, MimeType);

if (! Node)
{
	if (Flags & FLAG_DEBUG) printf("TranslateMimeType: %s no matches\n",MimeType);
	return("");
}

Rule=(TFileRule *) Node->Item;
if (Flags & FLAG_DEBUG) printf("TranslateMimeType: %s no -> %s\n",MimeType, Rule->Equivalent);

return(Rule->Equivalent);
}


//Both an Item's ContentType and it's Magic Type must match the rules type
//or it's equivalent
int IsEquivalentMimeType(TFileRule *Rule, const char *MimeType)
{
char *Token=NULL, *ptr;
int result=FALSE;

if (fnmatch(Rule->ContentType, MimeType, 0)==0) return(TRUE);

ptr=GetToken(Rule->Equivalent,",",&Token,0);
while (ptr)
{
	if (StrValid(Token) && fnmatch(Token, MimeType, 0)==0) 
	{
	result=TRUE;
	break;
	}
	ptr=GetToken(ptr,",",&Token,0);
}

DestroyString(Token);

return(result);
}


int ProcessContainedItem(const char *Contains, TMimeItem *Item)
{
char *Token=NULL, *ptr, *p_pattern;
int result=RULE_NONE;

ptr=GetToken(Contains,",",&Token,0);
while (ptr)
{
	p_pattern=Token;
	if (*p_pattern=='!') p_pattern++;

	if (fnmatch(p_pattern,Item->ContentType,0)==0) 
	{
		if (*Token=='!') result=RULE_EVIL | RULE_CONTAINER;
		else if (result==RULE_NONE) result=RULE_SAFE;
	}
ptr=GetToken(ptr,",",&Token,0);
}

FileRulesConsider(Item);
if ((result==RULE_SAFE) && (Item->RulesResult==RULE_NONE)) Item->RulesResult=result;
else Item->RulesResult = Item->RulesResult | result;

DestroyString(Token);
return(result);
}


int ProcessContainerItems(TFileRule *Rule, ListNode *SubItems)
{
ListNode *Curr;
TMimeItem *Item;
int RetVal=RULE_NONE, result;

Curr=ListGetNext(SubItems);
while (Curr)
{
	Item=(TMimeItem *) Curr->Item;
	result=ProcessContainedItem(Rule->Contains, Item);

	if ((RetVal==RULE_NONE) || (RetVal==RULE_SAFE)) 
	{
		RetVal=result;
	}
	Curr=ListGetNext(Curr);
}

return(RetVal);
}



int FileRulesProcessRule(TFileRule *Rule, TMimeItem *Item)
{
ListNode *Curr;
int ContentMatches=FALSE, MagicMatches=FALSE, ExtnMatches=FALSE;
int result;
const char *ptr;

if (Rule->Flags & RULE_FILENAME)
{
	ptr=GetBasename(Item->FileName);
	if (fnmatch(Rule->ContentType, ptr, 0) != 0) return(RULE_NONE);
}
else
{
	if (StrValid(Item->MagicType))
	{
		MagicMatches=IsEquivalentMimeType(Rule, Item->MagicType);
		if (MagicMatches && (Rule->Flags & RULE_EVIL)) return(RULE_EVIL);
	}
	
	if (StrValid(Item->ExtnType))
	{
	ExtnMatches=IsEquivalentMimeType(Rule, Item->ExtnType);
	if (ExtnMatches && (Rule->Flags & RULE_EVIL)) return(RULE_EVIL);
	}
	
	ContentMatches=IsEquivalentMimeType(Rule, Item->ContentType);
	
	//if noting matches then ignore this rule
	if (! (ContentMatches || MagicMatches || ExtnMatches)) return(RULE_NONE);


	// TO GO ANY FURTHER THE RULE MUST MATCH IN SOME WAY

	if (
//	as we just wound up turning 'black content type' on for everything, we've made it default
//		(Rule->Flags & RULE_BLANK_CONTYPE) && 
		(
			(StrValid(Item->ContentType)==0) || 
			(strcmp(Item->ContentType, "application/octet-stream")==0) ||
			(strcmp(Item->ContentType, "application/download")==0) 
		)
	)
	{
	//	if (StrValid(Item->MagicType) || StrValid(Item->ExtnType)) return(RULE_NONE);

	//do nothing. This prevents a mismatch being declared when we have a magic type or a file extention type
	//but no 'content type' declared in the email
	}
	else if (! ContentMatches) return(RULE_MISMATCH);
	else 
	{
		if (Rule->Flags & RULE_EVIL) return(RULE_EVIL);
	}
	
	if ((Rule->Flags & RULE_BLANK_MAGIC) && (! StrValid(Item->MagicType)))
	{
		//do nothing
	}
	else if ((! ExtnMatches) && (StrValid(Item->ExtnType)))  return(RULE_MISMATCH);
	
	
	Curr=ListGetNext(Item->SubItems);
	if (Curr)
	{
		//Item has subitems but items of this type are not declared to be containers
		if  (! (Rule->Flags & RULE_CONTAINER)) return(RULE_CONTAINER);
		ProcessContainerItems(Rule, Item->SubItems);
	}
	//item is a container but is empty
	else if (Rule->Flags & RULE_CONTAINER)
	{
		return(Item->RulesResult | RULE_EMPTY | RULE_EVIL);
	}
}


if (Rule->Flags & RULE_SAFE) return(RULE_SAFE);
if (Rule->Flags & RULE_EVIL) return(RULE_EVIL);
return(RULE_NONE);
}




void FileRulesConsider(TMimeItem *Item)
{
ListNode *Curr;
TFileRule *Rule;
int val;

Curr=ListGetNext(Rules);
while (Curr)
{
	Rule=(TFileRule *) Curr->Item;
	val=FileRulesProcessRule(Rule, Item);

	if (val & (RULE_SAFE | RULE_EVIL))
	{
		if (! (Item->RulesResult & RULE_MALFORMED))
		{
		if (! (Item->RulesResult & RULE_MACROS)) Item->RulesResult=val;
		}
	}
	Curr=ListGetNext(Curr);
}

}


void HeaderRulesConsider(TMimeItem *Item, const char *Header, const char *Value)
{
ListNode *Curr, *Node;
TFileRule *Rule;
int val, result=RULE_NONE;
char *Token=NULL, *ptr;

if (! Item->Headers) Item->Headers=ListCreate();
Node=ListAddNamedItem(Item->Headers, Header, CopyStr(NULL, Value));

Curr=ListGetNext(HeaderRules);
while (Curr)
{
	Rule=(TFileRule *) Curr->Item;

	//For a header rule 'ContentType' is the header name, 'Contains' is the header value
	if ((pmatch(Rule->ContentType, Header,StrLen(Header), 0, NULL)) && (pmatch(Rule->Contains, Value, StrLen(Value), 0, NULL)))
	{

		//Equivalent stores action string
		ptr=GetToken(Rule->Equivalent," ",&Token,0);

		if (strcasecmp(Token, "FileType")==0) FileTypeRuleParse(ptr, 0);
		if (strcasecmp(Token, "FileExtn")==0) FileExtnRuleParse(ptr);

		if (strcasecmp(Token, "string")==0) 
		{
			ptr=GetToken(ptr," ",&Token,0);
			DocumentStringsAdd(Token, ptr, RULE_OVERRIDE);
		}

		if (strcasecmp(Token, "show")==0) Node->ItemType |= RULE_ECHO;
	}
	Curr=ListGetNext(Curr);
}

DestroyString(Token);
}



int IsItSafe(TMimeItem *Item)
{
TMimeItem *SubItem;
ListNode *Curr;
int result;

if (Item->RulesResult & RULE_EVIL) return(Item->RulesResult);
Curr=ListGetNext(Item->SubItems);
while (Curr)
{
  SubItem=(TMimeItem *) Curr->Item;
	//if any flag other than safe is set, then return those other flags
	//this eliminates any situation where both safe and unsafe flags have
	//gotten set together
  result=IsItSafe(SubItem) & ~RULE_SAFE;
	if (result) return(result);
  Curr=ListGetNext(Curr);
}

if (Item->RulesResult & RULE_SAFE) return(Item->RulesResult);

return(RULE_NONE);
}
