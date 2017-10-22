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
		//if (StrValid(Equivalent)) SetTypedVar(g_KeyValueStore, MimeType, Equivalent, KV_EQUIV_MIMETYPE);
		

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
    char *Match=NULL, *Equiv=NULL, *Contains=NULL, *Token=NULL;
    const char *ptr;

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
        if (strcasecmp(Token,"allow-empty")==0) Flags |= RULE_ALLOW_EMPTY;
        if (strcasecmp(Token,"allow-macros")==0) Flags |= RULE_ALLOW_MACROS;
        if (strcasecmp(Token,"allow-encrypt")==0) Flags |= RULE_ALLOW_ENCRYPTED;
        if (strcasecmp(Token,"allow-encrypted")==0) Flags |= RULE_ALLOW_ENCRYPTED;
        if (strncasecmp(Token,"equiv=",6)==0) Equiv=CopyStr(Equiv, Token+6);
        if (strncasecmp(Token,"contains=",9)==0)
        {
            Contains=CopyStr(Contains, Token+9);
            Flags |= RULE_CONTAINER;
        }
        if (strcasecmp(Token,"strip")==0) Flags |= RULE_STRIP;


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
    char *ContentType=NULL, *Extn=NULL;
    const char *ptr, *eptr;

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

const char *TranslateMimeTypeEquivalent(const char *MimeType)
{
    ListNode *Node;
    TFileRule *Rule;

    if (! StrValid(MimeType)) return("");
    //Node=ListFindTypedItem(g_KeyValueStore, KV_EQUIV_MIMETYPE, MimeType);
    Node=ListFindNamedItem(Rules, MimeType);
    if (! Node)
    {
        if (g_Flags & FLAG_DEBUG) printf("TranslateMimeType: %s no matches\n",MimeType);
        return("");
    }

		Rule=(TFileRule *) Node->Item;
    if (g_Flags & FLAG_DEBUG) printf("TranslateMimeType: %s -> %s\n",MimeType, Rule->Equivalent);

    return((const char *) Rule->Equivalent);
}


int IsEquivalentMimeType(TFileRule *Rule, const char *MimeType)
{
    char *Token=NULL;
    const char *ptr;
    int result=FALSE;

    if (fnmatch(Rule->ContentType, MimeType, 0)==0) return(TRUE);
//    if (fnmatch(Rule->ContentType, TranslateMimeTypeEquivalent(MimeType), 0)==0) return(TRUE);
		

    ptr=GetToken(Rule->Equivalent ,",",&Token,0);
    while (ptr)
    {
				StripLeadingWhitespace(Token);
				StripTrailingWhitespace(Token);
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


//This only checks 'Contains' for the item
int ProcessContainedItem(const char *Contains, TMimeItem *Item)
{
    char *Token=NULL;
    const char *ptr, *p_pattern;
    int result=RULE_NONE;

    if (! StrValid(Contains)) return(RULE_NONE);
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

    if (result != RULE_SAFE) Item->RulesResult=RULE_CONTAINER;

    DestroyString(Token);
    return(result);
}


void ProcessContainerItems(TFileRule *Rule, ListNode *SubItems)
{
    ListNode *Curr;
    TMimeItem *Item;

    Curr=ListGetNext(SubItems);
    while (Curr)
    {
        Item=(TMimeItem *) Curr->Item;
        if (ProcessContainedItem(Rule->Contains, Item) == RULE_CONTAINER) Item->RulesResult |= RULE_CONTAINER;
        FileRulesConsider(Item);
        Curr=ListGetNext(Curr);
    }

}



int FileRulesProcessRule(TFileRule *Rule, TMimeItem *Item)
{
    ListNode *Curr;
    int ContentMatches=FALSE, FileMagicsMatches=FALSE, ExtnMatches=FALSE;
    int result;
    const char *ptr;

    if (Rule->Flags & RULE_FILENAME)
    {
        ptr=GetBasename(Item->FileName);
        if (fnmatch(Rule->ContentType, ptr, 0) != 0) return(RULE_NONE);
    }
    else
    {
        if (StrValid(Item->FileMagicsType))
        {
            FileMagicsMatches=IsEquivalentMimeType(Rule, Item->FileMagicsType);
            if (FileMagicsMatches && (Rule->Flags & RULE_EVIL)) return(RULE_EVIL);
        }

        if (StrValid(Item->ExtnType))
        {
            ExtnMatches=IsEquivalentMimeType(Rule, Item->ExtnType);
            if (ExtnMatches && (Rule->Flags & RULE_EVIL)) return(RULE_EVIL);
        }

        ContentMatches=IsEquivalentMimeType(Rule, Item->ContentType);


        //if noting matches then ignore this rule
        if (! (ContentMatches || FileMagicsMatches || ExtnMatches)) return(RULE_NONE);

        //if (Rule->Flags & RULE_STRIP) return(RULE_STRIP);

//printf("IS EQIV: %s %s %d %d %d\n",Item->ContentType, Rule->ContentType, ContentMatches, FileMagicsMatches, ExtnMatches);

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
            //	if (StrValid(Item->FileMagicsType) || StrValid(Item->ExtnType)) return(RULE_NONE);

            //do nothing. This prevents a mismatch being declared when we have a magic type or a file extention type
            //but no 'content type' declared in the email
        }
        else if (! ContentMatches) return(RULE_MISMATCH);
        else
        {
            if (Rule->Flags & RULE_EVIL) return(RULE_EVIL);
        }

        if ((Rule->Flags & RULE_BLANK_MAGIC) && (! StrValid(Item->FileMagicsType)))
        {
            //do nothing
        }
        else if ((! ExtnMatches) && (StrValid(Item->ExtnType)))  return(RULE_MISMATCH);


        Curr=ListGetNext(Item->SubItems);
        if (Curr)
        {
            //Item has subitems but items of this type are not declared to be containers
            if  (! (Rule->Flags & RULE_CONTAINER))
            {
                //root item can contain anything!
                if (! (Item->Flags & MIMEFLAG_ROOT))	return(RULE_CONTAINER);
            }
            else ProcessContainerItems(Rule, Item->SubItems);
        }
        //item is a container but is empty
        else if (Rule->Flags & RULE_CONTAINER)
        {
           if (! (Rule->Flags & RULE_ALLOW_EMPTY)) return(Item->RulesResult | RULE_EMPTY | RULE_EVIL);
        }
    }

    if (Rule->Flags & RULE_EVIL) return(RULE_EVIL);
    if (Rule->Flags & (RULE_ALLOW_MACROS | RULE_ALLOW_EMPTY | RULE_ALLOW_ENCRYPTED | RULE_SAFE)) return(Rule->Flags);

    return(RULE_NONE);
}




int FileRulesConsider(TMimeItem *Item)
{
    ListNode *Curr;
    TFileRule *Rule;
    int val;

    Curr=ListGetNext(Rules);
    while (Curr)
    {
        Rule=(TFileRule *) Curr->Item;
        val=FileRulesProcessRule(Rule, Item);

        //if we got a rule that says 'evil' then set that
        if (val & RULE_EVIL) Item->RulesResult |= (val & ~RULE_SAFE);
        else if ((val & RULE_SAFE) && (! (Item->RulesResult & (RULE_MALFORMED | RULE_MACROS | RULE_ENCRYPTED)))) Item->RulesResult=RULE_SAFE;

        //These can be applied independantly of anything else
        if ((val & RULE_STRIP) ) Item->RulesResult |= RULE_STRIP;
        if ((val & RULE_ALLOW_MACROS) ) Item->RulesResult &= ~RULE_MACROS;
        if ((val & RULE_ALLOW_EMPTY) ) Item->RulesResult &= ~RULE_EMPTY;
        if ((val & RULE_ALLOW_ENCRYPTED) ) Item->RulesResult &= ~RULE_ENCRYPTED;
        Curr=ListGetNext(Curr);
    }

    return(Item->RulesResult);
}


void HeaderRulesConsider(TMimeItem *Item, const char *Header, const char *Value)
{
    ListNode *Curr, *Node;
    TFileRule *Rule;
    int val, result=RULE_NONE;
    char *Token=NULL;
    const char *ptr;

    if (! StrValid(Header)) return;
    if (! Item->Headers) Item->Headers=ListCreate();
    Node=ListAddNamedItem(Item->Headers, Header, CopyStr(NULL, Value));

    Curr=ListGetNext(HeaderRules);
    while (Curr)
    {
        Rule=(TFileRule *) Curr->Item;

        //For a header rule 'ContentType' is the header name, 'Contains' is the header value
        if ((pmatch(Rule->ContentType, Header,StrLen(Header), NULL, 0)) && (pmatch(Rule->Contains, Value, StrLen(Value), NULL, 0 )))
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

//if we marked the item as either having macros or being encrypted, and not other rule
//has unmarked this, then it's time to mark it 'evil' now
    if (Item->RulesResult & (RULE_MACROS | RULE_ENCRYPTED)) Item->RulesResult |= RULE_EVIL;

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
