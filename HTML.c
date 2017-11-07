#include "HTML.h"
#include "FileTypeRules.h"
#include "DocumentStrings.h"
#include "URL.h"

char *HTMLReformatURL(char *RetStr, const char *URL)
{
const char *ptr;
STREAM *S;

RetStr=CopyStr(RetStr, URL);

//get past 'http://'. Then if there's no '/' ending the URL then add one
ptr=strchr(RetStr, ':');
if (ptr)
{
	ptr++;
	while (*ptr=='/') ptr++;
	
	if (! strchr(ptr, '/')) RetStr=CatStr(RetStr, "/");
}

S=STREAMOpen(URL,"r");
if (S)
{
RetStr=CopyStr(RetStr, STREAMGetValue(S, "HTTP:URL"));
STREAMClose(S);
}

return(RetStr);
}

int HTMLTagWithURL(TMimeItem *Item, const char *TagData)
{
    char *Name=NULL, *Value=NULL, *Tempstr=NULL;
		TMimeItem *SubItem;
    const char *ptr;

    ptr=GetNameValuePair(TagData, "\\S", "=", &Name, &Value);
    while (ptr)
    {
        if ((strcasecmp(Name,"href")==0) || (strcasecmp(Name,"src")==0)) 
				{
					Tempstr=HTMLReformatURL(Tempstr, Value);
					//Don't consider mailto URLs
					if (strncmp(Tempstr,"mailto:",7) !=0)
					{
					URLRuleCheck(Item, Tempstr);
					if (g_Flags & FLAG_DEBUG) printf("URL: %s\n",Value);
				  SubItem=MimeItemCreate(Tempstr,"","");
					if (StrValid(SubItem->ExtnType)) SubItem->ContentType=CopyStr(SubItem->ContentType, SubItem->ExtnType);
					else SubItem->ContentType=CopyStr(SubItem->ContentType, "text/html");
   				ListAddItem(Item->SubItems, SubItem);
					}
				}
        ptr=GetNameValuePair(ptr, "\\S", "=", &Name, &Value);
    }

    DestroyString(Name);
    DestroyString(Value);
    DestroyString(Tempstr);
}


int HTMLProcess(STREAM *S, TMimeItem *Item)
{
    char *Tempstr=NULL, *Line=NULL, *TagName=NULL, *TagData=NULL;
    const char *ptr;
    int RetVal=RULE_NONE;
    ListNode *HTMLStrings;

    HTMLStrings=DocumentStringsGetList("text/html");
    Line=STREAMReadLine(Line, S);
    while (Line)
    {
        StripTrailingWhitespace(Line);
        Tempstr=CatStr(Tempstr, Line);
        if (S->State & SS_EMBARGOED) exit(1);
        Line=STREAMReadLine(Line, S);
    }

    ptr=XMLGetTag(Tempstr, NULL, &TagName, &TagData);
    while (ptr)
    {
        switch (*TagName)
        {
        case 'a':
        case 'A':
            if (strcasecmp(TagName, "a")==0) HTMLTagWithURL(Item, TagData);
            break;

        case 'i':
        case 'I':
            if (strcasecmp(TagName, "img")==0) HTMLTagWithURL(Item, TagData);
            break;

        default:
            if (DocumentStringsCheck(HTMLStrings, TagName)==RULE_EVIL)
            {
                Item->RulesResult |= RULE_EVIL | RULE_MACROS;
                SetTypedVar(Item->Errors, TagName, "", ERROR_STRING);
            }
            break;
        }

        ptr=XMLGetTag(ptr, NULL, &TagName, &TagData);
    }


    DestroyString(Tempstr);
    DestroyString(TagName);
    DestroyString(TagData);
    DestroyString(Line);

    return(Item->RulesResult);
}



int HTMLFileProcess(const char *Path, TMimeItem *Item)
{
    int result, i;
    int RetVal=RULE_SAFE;
    ListNode *Curr;
    char *Tempstr=NULL;
    STREAM *S;

    if ((g_Flags & FLAG_DEBUG)) printf("Check HTML: [%s]\n",Path);
    S=STREAMFileOpen(Path, SF_RDONLY);
    if (S)
    {
        RetVal=HTMLProcess(S, Item);
    }

    DestroyString(Tempstr);

    return(RetVal);
}

