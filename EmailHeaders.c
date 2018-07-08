#include "EmailHeaders.h"
#include "FileTypeRules.h"
#include "IPRegion.h"
#include "DocumentStrings.h"

ListNode *HeaderRules=NULL;


void EmailHeaderRulesAdd(const char *Header, const char *Value, const char *Action)
{
char *Tempstr=NULL;

Tempstr=FormatStr(Tempstr,"%s:%s",Header,Value);
if (! HeaderRules) HeaderRules=ListCreate();
ListAddNamedItem(HeaderRules, Tempstr, CopyStr(NULL, Action));

Destroy(Tempstr);
}


char *EmailHeadersExtractIP(char *IP, const char *ReceivedHeader)
{
char *Token=NULL;
const char *optr, *ptr;

IP=CopyStr(IP, "");
ptr=GetToken(ReceivedHeader, "\\S", &Token, 0);
while (ptr)
{
if (strcasecmp(Token, "from")==0) 
{
	ptr=GetToken(ptr, "\\S", &Token, 0);
	while (ptr)
	{
		if (*Token == '[') 
		{
			GetToken(Token+1,"]",&IP,0);
		}
		ptr=GetToken(ptr, "\\S", &Token, 0);
	}
}
ptr=GetToken(ptr, "\\S", &Token, 0);
}

Destroy(Token);

return(IP);
}



void EmailHeaderCheckSource(TMimeItem *Item, const char *Rules)
{
char *Tempstr=NULL, *Name=NULL, *Value=NULL, *IP=NULL;
ListNode *Node;
const char *ptr;

Node=ListFindNamedItem(Item->Headers, "Received");
if (Node)
{
	IP=EmailHeadersExtractIP(IP, Node->Item);
	if (Config->Flags & FLAG_DEBUG) printf("SENDER IP: %s\n",IP);
	ptr=GetNameValuePair(Rules, "\\S", "=", &Name, &Value);
	while (ptr)
	{
		if (strcasecmp(Name, "ip")==0)
		{
      if (pmatch(Value, IP, StrLen(IP), NULL, 0))
			{
				Item->RulesResult |= RULE_EVIL | RULE_IP;	
				if (Config->Flags & FLAG_DEBUG) printf("SENDER IP RULE FAIL: required=%s got=%s\n",Value, IP);
			}
		}
		else if (strcasecmp(Name, "region")==0)
		{
			Tempstr=RegionLookup(Tempstr, IP);
      if (pmatch(Value, Tempstr, StrLen(Tempstr), NULL, 0))
			{
				Item->RulesResult |= RULE_EVIL | RULE_IPREGION;	
				if (Config->Flags & FLAG_DEBUG) printf("SENDER REGION RULE FAIL: required=%s got=%s\n",Value, Tempstr);
			}
		}
		ptr=GetNameValuePair(ptr, "\\S", "=", &Name, &Value);
	}
}

Destroy(Name);
Destroy(Value);
Destroy(Tempstr);
Destroy(IP);
}


void EmailHeaderRulesConsider(TMimeItem *Item, const char *Header, const char *Value)
{
    ListNode *Curr, *Node;
    int val, result=RULE_NONE;
    char *HeadName=NULL, *Token=NULL;
    const char *ptr;

		//this gets called for every header, and adds them as it goes
    if (! StrValid(Header)) return;
    if (! Item->Headers) Item->Headers=ListCreate();
    Node=ListAddNamedItem(Item->Headers, Header, CopyStr(NULL, Value));

    Curr=ListGetNext(HeaderRules);
    while (Curr)
    {
				ptr=GetToken(Curr->Tag,":",&HeadName,0);
        if ((pmatch(HeadName, Header, StrLen(Header), NULL, 0)) && (pmatch(ptr, Value, StrLen(Value), NULL, 0 )))
        {
            //Equivalent stores action string
            ptr=GetToken((char *) Curr->Item, " ",&Token,0);

						//These rules add filetype rules that are processed later
            if (strcasecmp(Token, "FileType")==0) FileTypeRuleParse(ptr, 0);
            if (strcasecmp(Token, "FileExtn")==0) FileExtnRuleParse(ptr);

						//These rules add/override the permitted documents strings
            if (strcasecmp(Token, "string")==0)
            {
                ptr=GetToken(ptr," ",&Token,0);
                DocumentStringsAdd(Token, ptr, RULE_OVERRIDE);
            }
		
						//source ipadddress check	
            if (strcasecmp(Token, "source")==0) EmailHeaderCheckSource(Item, ptr);

						//show a header
            if (strcasecmp(Token, "show")==0) Node->ItemType |= RULE_ECHO;
        }
        Curr=ListGetNext(Curr);
    }

    Destroy(HeadName);
    Destroy(Token);
}
