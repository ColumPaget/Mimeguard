#include "DocumentStrings.h"
#include "FileTypeRules.h"

ListNode *DocumentStrings=NULL, *OverrideDocumentStrings=NULL;

void DocumentStringsAdd(const char *DocType, const char *String, int Flags)
{
ListNode *Head, *Node, *Items;

if (! DocumentStrings) DocumentStrings=ListCreate();
if (! OverrideDocumentStrings) OverrideDocumentStrings=ListCreate();

if (Flags & RULE_OVERRIDE) Head=OverrideDocumentStrings;
else Head=DocumentStrings;

Node=ListFindNamedItem(Head,DocType);
if (! Node) Node=ListAddNamedItem(Head,DocType,ListCreate());

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

Node=ListFindNamedItem(OverrideDocumentStrings, DocType);
if (Node) return((ListNode *) Node->Item);
Node=ListFindNamedItem(DocumentStrings, DocType);
if (Node) return((ListNode *) Node->Item);

return(NULL);
}


int DocumentStringsCheck(ListNode *Items, const char *String)
{
ListNode *Curr;

Curr=ListGetNext(Items);
while (Curr)
{

if (fnmatch(Curr->Tag,String,0)==0) 
{
	return(Curr->ItemType);
}
Curr=ListGetNext(Curr);
}

return(RULE_NONE);
}


void DocumentStringsClearOverrides()
{
ListClear(OverrideDocumentStrings, DestroyString);
}
