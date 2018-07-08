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

    Destroy(Argument);
    Destroy(Token);
    Destroy(Match);
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
            Destroy(Tempstr);
            Destroy(Token);
            return(TRUE);
        }
        Tempstr=STREAMReadLine(Tempstr, S);
    }
    STREAMClose(S);

    Destroy(Tempstr);
    Destroy(Token);

    return(FALSE);
}


int URLRegionCheck(const char *Config, const char *IP, char **RegionRegistrar, char **RegionCountry)
{
    const char *ptr;
    char *Tempstr=NULL, *Token=NULL;
    int result=FALSE;

    *RegionRegistrar=CopyStr(*RegionRegistrar, "");
    *RegionCountry=CopyStr(*RegionCountry, "");

    Tempstr=RegionLookup(Tempstr, IP);
    ptr=GetToken(Tempstr,":",RegionRegistrar,0);
    ptr=GetToken(ptr,":",RegionCountry,0);

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

    Destroy(Tempstr);
    Destroy(Token);

    return(result);
}



int URLRuleCheckHost(TMimeItem *Item, const char *Host)
{
    ListNode *Curr;
    char *IP=NULL, *RegionRegistrar=NULL, *RegionCountry=NULL;
    char *Tempstr=NULL;
    const char *ptr;
    int result=FALSE;

    if (ListSize(URLRules)==0) return(FALSE);

        ptr=GetTypedVar(g_KeyValueStore, Host, KV_IP);
        if (! StrValid(ptr))
        {
            ptr=LookupHostIP(Host);
            if (StrValid(ptr)) SetDetailVar(g_KeyValueStore, Host, ptr, KV_IP, time(NULL) + 5);
        }

        IP=CopyStr(IP, ptr);


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
										if (Config->Flags & FLAG_DEBUG) printf("URLCHECK: FOUND host %s in %s\n", Host, Curr->Item);
								} else if (Config->Flags & FLAG_DEBUG) printf("URLCHECK: CLEAR host %s not in %s\n", Host, Curr->Item);
                break;

            case WHITELIST_HOSTLIST:
                if (InFileList(Curr->Item, Host)) result=FALSE;
                break;

            case BLACKLIST_IPLIST:
                if (StrValid(IP) && InFileList(Curr->Item, IP))
                {
                    Tempstr=MCopyStr(Tempstr, "ip ", IP, " in ", Curr->Item, NULL);
                    SetTypedVar(Item->Errors,Tempstr,"",ERROR_BASIC);
                    result=TRUE;
										if (Config->Flags & FLAG_DEBUG) printf("IP CHECK: FOUND ip %s in %s\n", IP, Curr->Item);
								} else if (Config->Flags & FLAG_DEBUG) printf("IP CHECK: CLEAR ip %s not in %s\n", IP, Curr->Item);
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

    Destroy(RegionRegistrar);
    Destroy(RegionCountry);
    Destroy(Tempstr);
    Destroy(IP);


    return(result);
}


int URLRuleCheck(TMimeItem *Item, const char *URL)
{
    char *Proto=NULL, *Host=NULL, *PortStr=NULL, *Doc=NULL;
    int result=FALSE;

    ParseURL(URL, &Proto, &Host, &PortStr, NULL, NULL, &Doc, NULL);
    if (StrValid(Host))
		{
       if (Config->Flags & FLAG_DEBUG) printf("URL: %s\n", URL);
			 result=URLRuleCheckHost(Item, Host);
		}

  Destroy(PortStr);
  Destroy(Proto);
  Destroy(Host);
  Destroy(Doc);

	return(result);
}
