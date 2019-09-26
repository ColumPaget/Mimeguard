#include "URL.h"
#include "FileTypeRules.h"
#include "IPRegion.h"


typedef struct 
{
int Type;
int Flags;
int ExitVal;
char *Argument;
} TURLRule;


typedef enum {BLACKLIST_HOST, WHITELIST_HOST, BLACKLIST_IP, WHITELIST_IP, BLACKLIST_HOSTLIST, WHITELIST_HOSTLIST, BLACKLIST_IPLIST, WHITELIST_IPLIST, BLACKLIST_REGION, WHITELIST_REGION} EURLRuleTypes;

ListNode *GlobalURLRules=NULL;
ListNode *DocumentURLRules=NULL;
int ExitVal=0;


void URLRuleDestroy(void *p_Rule)
{
TURLRule *Rule;

if (! p_Rule) return;
Rule=(TURLRule *) p_Rule;

Destroy(Rule->Argument);
free(Rule);
}


void URLRulesClear(ListNode *URLRules)
{
ListClear(URLRules, URLRuleDestroy);
}

void URLParseRule(ListNode *URLRules, const char *Declare)
{
    int SafeEvil=-1;
    char *MatchTypeStr=NULL, *Token=NULL;
    const char *ptr;
		TURLRule *Rule;

		Rule=(TURLRule *) calloc(1, sizeof(TURLRule));
    ptr=GetToken(Declare,"\\S", &MatchTypeStr, GETTOKEN_QUOTES);
    ptr=GetToken(ptr,"\\S", &Rule->Argument, GETTOKEN_QUOTES);
    ptr=GetToken(ptr,"\\S", &Token, GETTOKEN_QUOTES);
		while (ptr)
		{
		if (strcasecmp(Token, "safe")==0) SafeEvil = RULE_SAFE;
		if (strncasecmp(Token, "exit=", 5)==0) Rule->ExitVal=atoi(Token+5);
		
    ptr=GetToken(ptr,"\\S", &Token, GETTOKEN_QUOTES);
		}

    if (strcasecmp(MatchTypeStr, "ip")==0)
    {
        if (SafeEvil==RULE_SAFE) Rule->Type=WHITELIST_IP;
        else Rule->Type=BLACKLIST_IP;
    }
    else if (strcasecmp(MatchTypeStr,"host")==0)
    {
        if (SafeEvil==RULE_SAFE) Rule->Type=WHITELIST_HOST;
        else Rule->Type=BLACKLIST_HOST;
    }
    else if (strcasecmp(MatchTypeStr, "iplist")==0)
    {
        if (SafeEvil==RULE_SAFE) Rule->Type=WHITELIST_IPLIST;
        else Rule->Type=BLACKLIST_IPLIST;
    }
    else if (strcasecmp(MatchTypeStr,"hostlist")==0)
    {
        if (SafeEvil==RULE_SAFE) Rule->Type=WHITELIST_HOSTLIST;
        else Rule->Type=BLACKLIST_HOSTLIST;
    }
    else if (strcasecmp(MatchTypeStr,"region")==0)
    {
        if (SafeEvil==RULE_SAFE) Rule->Type=WHITELIST_REGION;
        else Rule->Type=BLACKLIST_REGION;
    }

    if (! URLRules) URLRules=ListCreate();
    ListAddItem(URLRules, Rule);

    Destroy(MatchTypeStr);
    Destroy(Token);
}


static int InFileList(const char *Path, const char *Item)
{
    STREAM *S;
    char *Tempstr=NULL, *Token=NULL;
    const char *ptr;
		int result=FALSE;

    S=STREAMOpen(Path, "r");
		if (S)
		{
    Tempstr=STREAMReadLine(Tempstr, S);
    while (Tempstr)
    {
        StripTrailingWhitespace(Tempstr);
        StripLeadingWhitespace(Tempstr);
        ptr=GetToken(Tempstr, "\\S", &Token, 0);
        if (strcmp(Token, Item)==0)
        {
					result=TRUE;
					break;
        }
        Tempstr=STREAMReadLine(Tempstr, S);
    }
    STREAMClose(S);
		}

    Destroy(Tempstr);
    Destroy(Token);

    return(result);
}


static int URLRegionCheck(const char *Config, const char *IP, char **RegionRegistrar, char **RegionCountry)
{
    const char *ptr;
    char *Tempstr=NULL, *Token=NULL;
    int result=FALSE;

		if (strcmp(Config, "*")==0) return(TRUE);
    *RegionRegistrar=CopyStr(*RegionRegistrar, "");
    *RegionCountry=CopyStr(*RegionCountry, "");

    Tempstr=RegionLookup(Tempstr, IP);
		if (StrValid(Tempstr))
		{
    ptr=GetToken(Tempstr,":",RegionRegistrar,0);
    ptr=GetToken(ptr,":",RegionCountry,0);

    ptr=GetToken(Config, ",", &Token, 0);
    while (ptr)
    {
        if (
            (strcasecmp(Token, Tempstr)==0) ||
            (strcasecmp(Token, *RegionRegistrar)==0) ||
            (strcasecmp(Token, *RegionCountry)==0)
        )
        {
            result=TRUE;
            break;
        }
        ptr=GetToken(ptr, ",", &Token, 0);
    }
		}

    Destroy(Tempstr);
    Destroy(Token);

    return(result);
}



int URLRulesProcess(ListNode *URLRules, int RetVal, TMimeItem *Item, const char *Host, const char *IP, const char *URL)
{
char *RegionRegistrar=NULL, *RegionCountry=NULL, *Tempstr=NULL;
ListNode *Curr;
TURLRule *Rule;

Curr=ListGetNext(URLRules);
while (Curr)
{
	Rule=(TURLRule *) Curr->Item;
            switch (Rule->Type)
            {
            case BLACKLIST_HOST:
                if (pmatch(Rule->Argument, Host, StrLen(Host), NULL, 0))
                {
                    Tempstr=MCopyStr(Tempstr, "host ", Host, " in ", Rule->Argument, NULL);
                    SetTypedVar(Item->Errors,Tempstr,"",ERROR_BASIC);
                    RetVal=RULE_EVIL;
										if (Rule->ExitVal > 0) ExitVal=Rule->ExitVal;
                }
                break;

            case WHITELIST_HOST:
                if (pmatch(Rule->Argument, Host, StrLen(Host), NULL, 0)) 
								{
									RetVal=RULE_SAFE;
								}
                break;

            case BLACKLIST_IP:
                if (StrValid(IP) && pmatch(Rule->Argument, IP, StrLen(IP), NULL, 0))
                {
                    Tempstr=MCopyStr(Tempstr, "ip ", IP, " in ", Rule->Argument, " (", URL, ")", NULL);
                    SetTypedVar(Item->Errors,Tempstr,"",ERROR_BASIC);
                    RetVal=RULE_EVIL;
										if (Rule->ExitVal > 0) ExitVal=Rule->ExitVal;
                }
                break;

            case WHITELIST_IP:
                if (StrValid(IP) && pmatch(Rule->Argument, IP, StrLen(IP), NULL, 0)) 
								{
								RetVal=RULE_SAFE;
								}
                break;

            case BLACKLIST_HOSTLIST:
                if (InFileList(Rule->Argument, Host))
                {
                    Tempstr=MCopyStr(Tempstr, "host ", Host, " in ", Rule->Argument, NULL);
                    SetTypedVar(Item->Errors,Tempstr,"",ERROR_BASIC);
                    RetVal=RULE_EVIL;
										if (Rule->ExitVal > 0) ExitVal=Rule->ExitVal;
										if (Config->Flags & FLAG_DEBUG) printf("URLCHECK: FOUND host %s in %s\n", Host, Rule->Argument);
								} else if (Config->Flags & FLAG_DEBUG) printf("URLCHECK: CLEAR host %s not in %s\n", Host, Rule->Argument);
                break;

            case WHITELIST_HOSTLIST:
                if (InFileList(Rule->Argument, Host)) 
								{
									RetVal=RULE_SAFE;
								}
                break;

            case BLACKLIST_IPLIST:
                if (StrValid(IP) && InFileList(Rule->Argument, IP))
                {
                    Tempstr=MCopyStr(Tempstr, "ip ", IP, " in ", Rule->Argument, NULL);
                    SetTypedVar(Item->Errors,Tempstr,"",ERROR_BASIC);
                    RetVal=RULE_EVIL;
										if (Rule->ExitVal > 0) ExitVal=Rule->ExitVal;
										if (Config->Flags & FLAG_DEBUG) printf("IP CHECK: FOUND ip %s in %s\n", IP, Rule->Argument);
								} else if (Config->Flags & FLAG_DEBUG) printf("IP CHECK: CLEAR ip %s not in %s\n", IP, Rule->Argument);
                break;

            case WHITELIST_IPLIST:
                if (StrValid(IP) && InFileList(Rule->Argument, IP)) RetVal=RULE_SAFE;
                break;

            case BLACKLIST_REGION:
                if (URLRegionCheck(Rule->Argument, IP, &RegionRegistrar, &RegionCountry))
                {
                    RetVal=RULE_EVIL;
										if (Rule->ExitVal > 0) ExitVal=Rule->ExitVal;
                    Tempstr=MCopyStr(Tempstr, "ip ",IP, " region ",RegionRegistrar,":",RegionCountry, " (", URL, ")", NULL);
                    SetTypedVar(Item->Errors,Tempstr,"",ERROR_BASIC);
                }
                break;

            case WHITELIST_REGION:
                if (URLRegionCheck(Rule->Argument, IP, &RegionRegistrar, &RegionCountry)) 
								{
									RetVal=RULE_SAFE;
								}
                break;
            }
            Curr=ListGetNext(Curr);
        }

    Destroy(RegionRegistrar);
    Destroy(RegionCountry);
    Destroy(Tempstr);


return(RetVal);
}



static int URLRuleCheckHost(TMimeItem *Item, const char *Host, const char *URL)
{
    char *Tempstr=NULL, *IP=NULL;
    const char *ptr;
    int RetVal=RULE_NONE;

    if ( 
					(ListSize(GlobalURLRules)==0) &&
					(ListSize(DocumentURLRules)==0) 
				)return(FALSE);

		ExitVal=0;

        ptr=GetTypedVar(g_KeyValueStore, Host, KV_IP);
        if (! StrValid(ptr))
        {
            ptr=LookupHostIP(Host);
            if (StrValid(ptr)) SetDetailVar(g_KeyValueStore, Host, ptr, KV_IP, time(NULL) + 5);
        }

        IP=CopyStr(IP, ptr);

		RetVal=URLRulesProcess(GlobalURLRules, RetVal, Item, Host, IP, URL);
		RetVal=URLRulesProcess(DocumentURLRules, RetVal, Item, Host, IP, URL);

        if (RetVal==RULE_EVIL)
        {
            Item->RulesResult &= ~RULE_SAFE;
            Item->RulesResult |= RULE_EVIL | RULE_MACROS;
						if (ExitVal > 0) EvilExitStatus=ExitVal;
        }
        else if (RetVal==RULE_SAFE)
        {
            Item->RulesResult &= ~(RULE_EVIL | RULE_MACROS);
            Item->RulesResult |= RULE_SAFE;
        }



	Destroy(Tempstr);
  Destroy(IP);

  return(RetVal);
}


int URLRuleCheck(TMimeItem *Item, const char *URL)
{
    char *Proto=NULL, *Host=NULL, *PortStr=NULL, *Doc=NULL;
    int result=FALSE;

		if (strncasecmp(URL, "mailto:", 7)==0) return(RULE_SAFE);

    ParseURL(URL, &Proto, &Host, &PortStr, NULL, NULL, &Doc, NULL);
    if (StrValid(Host))
		{
			 result=URLRuleCheckHost(Item, Host, URL);
//       if (Config->Flags & (FLAG_DEBUG | FLAG_SHOWURL)) printf("URL: %s\n", URL);
		}

  Destroy(PortStr);
  Destroy(Proto);
  Destroy(Host);
  Destroy(Doc);

	return(result);
}
