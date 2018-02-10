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



char *OutputItemErrors(char *RetStr, TMimeItem *Item)
{
    ListNode *Curr;
    int HasStrings=FALSE;
    char *Tempstr=NULL;

    //to ensure RetStr isn't NULL
    RetStr=CatStr(RetStr,"");

    if (Item->RulesResult & RULE_CONTAINER) RetStr=CommaList(RetStr,"not allowed in container");
    else if (Item->RulesResult & RULE_ENCRYPTED) RetStr=CommaList(RetStr,"encrypted");
    else if (Item->RulesResult & RULE_EMPTY) RetStr=CommaList(RetStr,"empty container");


    if (Item->RulesResult & (RULE_MACROS | RULE_MALFORMED | RULE_CONTAINER | RULE_EMPTY))
    {
        Curr=ListGetNext(Item->Errors);
        while (Curr)
        {
            if (Curr->ItemType==ERROR_BASIC) RetStr=CommaList(RetStr, Curr->Tag);
            if (Curr->ItemType==ERROR_STRING) HasStrings=TRUE;
            Curr=ListGetNext(Curr);
        }

        if (HasStrings)
        {
            Curr=ListGetNext(Item->Errors);
            while (Curr)
            {
                if (Curr->ItemType==ERROR_STRING) Tempstr=CommaList(Tempstr, Curr->Tag);
                Curr=ListGetNext(Curr);
            }
            RetStr=MCatStr(RetStr," ForbiddenStrings{",Tempstr,"}",NULL);
        }
    }


    return(RetStr);
}

void OutputItem(TMimeItem *Top, TMimeItem *Item, int Level, int Safe)
{
    ListNode *Curr;
    int i, val, Show=FALSE;
    char *Prefix=NULL, *Additional=NULL, *p_startANSI="", *p_endANSI="", *p_Status="?";

    Prefix=CopyStr(Prefix,"");
    for (i=0; i < Level; i++) Prefix=CatStr(Prefix,"  ");

    if (Level > 0) Show=TRUE;
    if (Safe && (g_Flags & FLAG_SHOW_SAFE)) Show=TRUE;
    if ((! Safe) && (g_Flags & FLAG_SHOW_EVIL)) Show=TRUE;

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

    if ((! StrValid(Item->FileName) ) && (! StrValid(Item->ContentType) ) && (! StrValid(Item->FileMagicsType) ) ) Show=FALSE;

    if (Show)
    {
        if (Level==0) OutputHeaders(Top);
        Additional=OutputItemErrors(Additional, Item);
        printf("%s %s%s%s content=%s magic=%s extn=%s   %s %s\n",p_Status, Prefix, p_startANSI, Item->FileName, Item->ContentType, Item->FileMagicsType, Item->ExtnType, Additional, p_endANSI);


        //print out subitems
        Curr=ListGetNext(Item->SubItems);
        while (Curr)
        {
            OutputItem(Top, (TMimeItem *) Curr->Item, Level+1, 0);
            Curr=ListGetNext(Curr);
        }

        if (Level==0) printf("\n");
    }

    Destroy(Prefix);
    Destroy(Additional);
}


