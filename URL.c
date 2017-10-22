#include "URL.h"
#include "FileTypeRules.h"
#include "IPRegion.h"


typedef enum {BLACKLIST_HOST, WHITELIST_HOST, BLACKLIST_IP, WHITELIST_IP, BLACKLIST_HOSTLIST, WHITELIST_HOSTLIST, BLACKLIST_IPLIST, WHITELIST_IPLIST, BLACKLIST_REGION, WHITELIST_REGION} EURLRuleTypes;

ListNode *URLRules=NULL;


void URLRuleAdd(int Type, const char *Arg)
{
if (! URLRules) URLRules=ListCreate();
ListAddTypedItem(URLRules, Type, "", CopyStr(NULL, Arg));
}


void URLParseRule(const char *Rule)
{
    int Type=-1, SafeEvil=-1;
    char *Match=NULL, *Token=NULL, *Argument=NULL;
    const char *ptr;

    ptr=GetToken(Rule,"\\S", &Match, GETTOKEN_QUOTES);
    ptr=GetToken(ptr,"\\S", &Argument, GETTOKEN_QUOTES);
    ptr=GetToken(ptr,"\\S", &Token, GETTOKEN_QUOTES);

    if (strcasecmp(Match, "ip")==0)
    {
        if (strcasecmp(Token,"safe")==0) Type=WHITELIST_IP;
        else Type=BLACKLIST_IP;
    }
    else if (strcasecmp(Match,"host")==0)
    {
        if (strcasecmp(Token,"safe")==0) Type=WHITELIST_HOST;
        else Type=BLACKLIST_HOST;
    }
    else if (strcasecmp(Match, "iplist")==0)
    {
        if (strcasecmp(Token,"safe")==0) Type=WHITELIST_IPLIST;
        else Type=BLACKLIST_IPLIST;
    }
    else if (strcasecmp(Match,"hostlist")==0)
    {
        if (strcasecmp(Token,"safe")==0) Type=WHITELIST_HOSTLIST;
        else Type=BLACKLIST_HOSTLIST;
    }
    else if (strcasecmp(Match,"region")==0)
    {
        if (strcasecmp(Token,"safe")==0) Type=WHITELIST_REGION;
        else Type=BLACKLIST_REGION;
    }


    URLRuleAdd(Type, Argument);

    DestroyString(Argument);
    DestroyString(Token);
    DestroyString(Match);
}


int InFileList(const char *Path, const char *Item)
{
    STREAM *S;
    char *Tempstr=NULL, *Token=NULL;
    const char *ptr;

    S=STREAMOpen(Path, "r");
    Tempstr=STREAMReadLine(Tempstr, S);
    while (Tempstr)
    {
        StripTrailingWhitespace(Tempstr);
        StripLeadingWhitespace(Tempstr);
        ptr=GetToken(Tempstr, "\\S", &Token, 0);
        if (strcmp(Token, Item)==0)
        {
            DestroyString(Tempstr);
            DestroyString(Token);
            return(TRUE);
        }
        Tempstr=STREAMReadLine(Tempstr, S);
    }
    STREAMClose(S);

    DestroyString(Tempstr);
    DestroyString(Token);

    return(FALSE);
}


int URLRegionCheck(const char *Config, const char *IP, char **RegionRegistrar, char **RegionCountry)
{
    const char *ptr;
    char *Tempstr=NULL, *Token=NULL;
    int result=FALSE;

    if (! (*RegionRegistrar))
    {
        Tempstr=RegionLookup(Tempstr, IP);
        ptr=GetToken(Tempstr,":",RegionRegistrar,0);
        ptr=GetToken(ptr,":",RegionCountry,0);
    }

    ptr=GetToken(Config, ",", &Token, 0);
    while (ptr)
    {
        if (
            (strcasecmp(Token, *RegionRegistrar)==0) ||
            (strcasecmp(Token, *RegionCountry)==0)
        )
        {
            result=TRUE;
            break;
        }
        ptr=GetToken(ptr, ",", &Token, 0);
    }

    DestroyString(Tempstr);
    DestroyString(Token);

    return(result);
}



int URLRuleCheck(TMimeItem *Item, const char *URL)
{
    ListNode *Curr;
    char *Proto=NULL, *Host=NULL, *PortStr=NULL, *Doc=NULL, *IP=NULL;
    char *RegionRegistrar=NULL, *RegionCountry=NULL;
    char *Tempstr=NULL;
		const char *ptr;
    int result=FALSE;

    ParseURL(URL, &Proto, &Host, &PortStr, NULL, NULL, &Doc, NULL);
		if (StrValid(Host))
		{
		ptr=GetTypedVar(g_KeyValueStore, Host, KV_IP);
    if (! StrValid(ptr)) 
		{
			ptr=LookupHostIP(Host);
			if (StrValid(ptr)) SetDetailVar(g_KeyValueStore, Host, ptr, KV_IP, time(NULL) + 5);
		}
		IP=CopyStr(IP, ptr);

    if (g_Flags & FLAG_DEBUG) printf("URL: %s %s\n",IP, URL);

    Curr=ListGetNext(URLRules);
    while (Curr)
    {
            switch (Curr->ItemType)
            {
            case BLACKLIST_HOST:
                if (pmatch(Curr->Item, Host, StrLen(Host), NULL, 0))
                {
                    Tempstr=MCopyStr(Tempstr, "host ", Host, " in ", Curr->Item, NULL);
                    SetTypedVar(Item->Errors,Tempstr,"",ERROR_BASIC);
                    result=TRUE;
                }
                break;

            case WHITELIST_HOST:
                if (pmatch(Curr->Item, Host, StrLen(Host), NULL, 0)) result=FALSE;
                break;

            case BLACKLIST_IP:
                if (StrValid(IP) && pmatch(Curr->Item, IP, StrLen(IP), NULL, 0))
                {
                    Tempstr=MCopyStr(Tempstr, "ip ", IP, " in ", Curr->Item, NULL);
                    SetTypedVar(Item->Errors,Tempstr,"",ERROR_BASIC);
                    result=TRUE;
                }
                break;

            case WHITELIST_IP:
                if (StrValid(IP) && pmatch(Curr->Item, Host, StrLen(Host), NULL, 0)) result=FALSE;
                break;

            case BLACKLIST_HOSTLIST:
                if (InFileList(Curr->Item, Host))
                {
                    Tempstr=MCopyStr(Tempstr, "host ", Host, " in ", Curr->Item, NULL);
                    SetTypedVar(Item->Errors,Tempstr,"",ERROR_BASIC);
                    result=TRUE;
                }
                break;

            case WHITELIST_HOSTLIST:
                if (InFileList(Curr->Item, URL)) result=FALSE;
                break;

            case BLACKLIST_IPLIST:
                if (StrValid(IP) && InFileList(Curr->Item, IP))
                {
                    Tempstr=MCopyStr(Tempstr, "ip ", IP, " in ", Curr->Item, NULL);
                    SetTypedVar(Item->Errors,Tempstr,"",ERROR_BASIC);
                    result=TRUE;
                }
                break;

            case WHITELIST_IPLIST:
                if (StrValid(IP) && InFileList(Curr->Item, IP)) result=FALSE;
                break;


            case BLACKLIST_REGION:
                if (URLRegionCheck(Curr->Item, IP, &RegionRegistrar, &RegionCountry))
                {
                    result=TRUE;
                    Tempstr=MCopyStr(Tempstr, "ip ",IP, " region ",RegionRegistrar,":",RegionCountry,NULL);
                    SetTypedVar(Item->Errors,Tempstr,"",ERROR_BASIC);
                }
                break;

            case WHITELIST_REGION:
                if (URLRegionCheck(Curr->Item, IP, &RegionRegistrar, &RegionCountry)) result=FALSE;
                break;
            }
        Curr=ListGetNext(Curr);
    }


    if (result)
    {
        Item->RulesResult &= ~RULE_SAFE;
        Item->RulesResult |= RULE_EVIL | RULE_MACROS;
    }
		}

    DestroyString(RegionRegistrar);
    DestroyString(RegionCountry);
    DestroyString(Tempstr);
    DestroyString(PortStr);
    DestroyString(Proto);
    DestroyString(Host);
    DestroyString(Doc);
    DestroyString(IP);


    return(result);
}


