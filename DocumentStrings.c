#include "DocumentStrings.h"
#include "FileTypeRules.h"

void DocumentStringsAdd(const char *DocType, const char *String, int Flags)
{
    ListNode *Node, *Items;
    int Type;

    if (Flags & RULE_OVERRIDE) KV_DOCSTRINGS_OVERRIDE;
    else Type=KV_DOCSTRINGS;

    Node=ListFindTypedItem(g_KeyValueStore, Type, DocType);
    if (! Node) Node=ListAddTypedItem(g_KeyValueStore, Type, DocType, ListCreate());

    Items=(ListNode *) Node->Item;

    if (! ListFindNamedItem(Items,String))
    {
        Node=ListAddNamedItem(Items, String, NULL);
        Node->ItemType=Flags & (RULE_SAFE | RULE_EVIL);
    }
}


ListNode *DocumentStringsGetList(const char *DocType)
{
    ListNode *Node;

    Node=ListFindTypedItem(g_KeyValueStore, KV_DOCSTRINGS_OVERRIDE, DocType);
    if (Node) return((ListNode *) Node->Item);
    Node=ListFindTypedItem(g_KeyValueStore, KV_DOCSTRINGS, DocType);
    if (Node) return((ListNode *) Node->Item);

    return(NULL);
}


int DocumentStringsCheck(ListNode *Items, const char *String)
{
    ListNode *Curr;

    Curr=ListGetNext(Items);
    while (Curr)
    {

        if (pmatch(Curr->Tag,String,StrLen(String), NULL, 0))
        {
            return(Curr->ItemType);
        }
        Curr=ListGetNext(Curr);
    }

    return(RULE_NONE);
}

