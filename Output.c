#include "Output.h"
#include "FileTypeRules.h"

void OutputHeaders(TMimeItem *Item)
{
ListNode *Curr;

Curr=ListGetNext(Item->Headers);
while (Curr)
{
if (Curr->ItemType & RULE_ECHO) printf("%s: %s\n",Curr->Tag,(char *) Curr->Item);
Curr=ListGetNext(Curr);
}

}



void OutputItem(TMimeItem *Top, TMimeItem *Item, int Level, int Safe)
{
ListNode *Curr;
int i, val, Show=FALSE;
char *Prefix=NULL, *Additional=NULL, *p_startANSI="", *p_endANSI="", *p_Status="?";

Prefix=CopyStr(Prefix,"");
for (i=0; i < Level; i++) Prefix=CatStr(Prefix,"  ");

if (Level > 0) Show=TRUE;
if (Safe && (Flags & FLAG_SHOW_SAFE)) Show=TRUE; 
if ((! Safe) && (Flags & FLAG_SHOW_EVIL)) Show=TRUE; 

if (Show)
{
	if (Item->RulesResult & RULE_EVIL)
	{
		if (isatty(1))
		{
		p_startANSI=ANSICode(ANSI_RED, ANSI_NONE, 0);
		p_endANSI=ANSI_NORM;
		}
		p_Status="!";
	}
	else if (Item->RulesResult & RULE_SAFE)
	{
		if (isatty(1))
		{
		p_startANSI=ANSICode(ANSI_GREEN, ANSI_NONE, 0);
		p_endANSI=ANSI_NORM;
		}
		p_Status=" ";
	}
	else if (Item->RulesResult & RULE_MISMATCH)
	{
		if (isatty(1))
		{
		p_startANSI=ANSICode(ANSI_MAGENTA, ANSI_NONE, 0);
		p_endANSI=ANSI_NORM;
		}
		p_Status="M";
	}
	else if (Item->RulesResult & RULE_CONTAINER)
	{
		if (isatty(1))
		{
		p_startANSI=ANSICode(ANSI_BLUE, ANSI_NONE, 0);
		p_endANSI=ANSI_NORM;
		}
		p_Status="C";
	}
}

if ((! StrValid(Item->FileName) ) && (! StrValid(Item->ContentType) ) && (! StrValid(Item->MagicType) ) ) Show=FALSE;

if (Show)
{
	if (Level==0) OutputHeaders(Top);

	if (Item->RulesResult & RULE_CONTAINER) Item->ResultInfo=CommaList(Item->ResultInfo,"not allowed in container");
	else if (Item->RulesResult & RULE_ENCRYPTED) Item->ResultInfo=CommaList(Item->ResultInfo,"encrypted");
	else if (Item->RulesResult & RULE_EMPTY) Item->ResultInfo=CommaList(Item->ResultInfo,"empty container");


	Additional=CopyStr(Additional,"");
	if (
				(Item->RulesResult & (RULE_MACROS | RULE_MALFORMED | RULE_CONTAINER | RULE_EMPTY)) &&
				(StrValid(Item->ResultInfo))
		) Additional=MCopyStr(Additional,"(",Item->ResultInfo,")",NULL);

	printf("%s %s%s%s content=%s magic=%s extn=%s %s %s\n",p_Status, Prefix, p_startANSI, Item->FileName, Item->ContentType, Item->MagicType, Item->ExtnType, Additional, p_endANSI);


	//print out subitems
	Curr=ListGetNext(Item->SubItems);
	while (Curr)
	{
		OutputItem(Top, (TMimeItem *) Curr->Item, Level+1, 0);
		Curr=ListGetNext(Curr);
	}

	if (Level==0) printf("\n");
}

DestroyString(Prefix);
DestroyString(Additional);
}


