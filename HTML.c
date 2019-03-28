#include "HTML.h"
#include "FileTypeRules.h"
#include "DocumentStrings.h"
#include "URL.h"

char *HTMLReformatURL(char *RetStr, const char *URL)
{
    char *ptr;
    STREAM *S;

    RetStr=CopyStr(RetStr, URL);

		ptr=strrchr(RetStr, '?');
		if (ptr) *ptr='\0';

//get past 'http://'. Then if there's no '/' ending the URL then add one
    ptr=strchr(RetStr, ':');
    if (ptr)
    {
        ptr++;
        while (*ptr=='/') ptr++;
    }
		else ptr=RetStr;

    if (! strchr(ptr, '/')) RetStr=CatStr(RetStr, "/");
		
/*
    S=STREAMOpen(URL,"r");
    if (S)
    {
        RetStr=CopyStr(RetStr, STREAMGetValue(S, "HTTP:URL"));
        STREAMClose(S);
    }
*/

    return(RetStr);
}



TMimeItem *HTMLAddURLSubItem(TMimeItem *Parent, const char *URL)
{
    TMimeItem *SubItem;
    char *Doc=NULL;

    ParseURL(URL, NULL, NULL, NULL, NULL, NULL, &Doc, NULL);

    //All the extraction of ExtnType etc has to be done on the Doc part of the URL so that 'http://somewhere.com' isn't
    //misidentified as a windows command file
    SubItem=MimeItemCreate(Doc,"","");
    if (StrValid(SubItem->ExtnType)) SubItem->ContentType=CopyStr(SubItem->ContentType, SubItem->ExtnType);
    else SubItem->ContentType=CopyStr(SubItem->ContentType, "text/html");
    SubItem->FileName=CopyStr(SubItem->FileName, URL);
    ListAddItem(Parent->SubItems, SubItem);

    Destroy(Doc);
}


int HTMLTagWithURL(TMimeItem *Item, const char *TagData)
{
    char *Name=NULL, *Value=NULL, *Tempstr=NULL, *Proto=NULL;
		const char *IgnoreProtos[]={"mailto","x-msg","cid",NULL};
    const char *ptr;
		

    ptr=GetNameValuePair(TagData, "\\S", "=", &Name, &Value);
    while (ptr)
    {
        if ((strcasecmp(Name,"href")==0) || (strcasecmp(Name,"src")==0))
        {
            Tempstr=HTMLReformatURL(Tempstr, Value);
						GetToken(Tempstr, ":", &Proto, 0);
            //Don't consider mailto URLs
            if (MatchTokenFromList(Proto, IgnoreProtos, 0)==-1)
            {
                if (Config->Flags & FLAG_DEBUG) printf("URL: %s\n", Tempstr);
                if (! (Config->Flags & FLAG_NO_URL_CHECKS)) URLRuleCheck(Item, Tempstr);
                HTMLAddURLSubItem(Item, Tempstr);
            }
        }
        ptr=GetNameValuePair(ptr, "\\S", "=", &Name, &Value);
    }

    Destroy(Name);
    Destroy(Value);
    Destroy(Tempstr);
}


int HTMLProcess(STREAM *S, TMimeItem *Item)
{
    char *Tempstr=NULL, *Line=NULL, *TagName=NULL, *TagData=NULL;
    const char *ptr;
    int RetVal=RULE_NONE;
    const char *HTMLStrings;

    HTMLStrings=DocumentStringsGetList("text/html");
    Line=STREAMReadLine(Line, S);
    while (Line)
    {
        StripTrailingWhitespace(Line);

				if ((Item->Flags & MIMEFLAG_BASE64) && (! (Item->Flags & MIMEFLAG_DECODED)))
				{
					DecodeBytes(&TagData, Line, ENCODE_BASE64);
        	Tempstr=CatStr(Tempstr, TagData);
				}
        else Tempstr=CatStr(Tempstr, Line);

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


    Destroy(Tempstr);
    Destroy(TagName);
    Destroy(TagData);
    Destroy(Line);

    return(Item->RulesResult);
}



int HTMLFileProcess(const char *Path, TMimeItem *Item)
{
    int result, i;
    int RetVal=RULE_SAFE;
    ListNode *Curr;
    char *Tempstr=NULL;
    STREAM *S;

    if ((Config->Flags & FLAG_DEBUG)) printf("Check HTML: [%s]\n",Path);
    S=STREAMFileOpen(Path, SF_RDONLY);
    if (S)
    {
        RetVal=HTMLProcess(S, Item);
    }

    Destroy(Tempstr);

    return(RetVal);
}

