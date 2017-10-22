#ifndef LIB_USEFUL_LIST
#define LIB_USEFUL_LIST

#include <time.h>

#define LIST_FLAG_DELETE 1
#define LIST_FLAG_CASE 2
#define LIST_FLAG_SELFORG 4
#define LIST_FLAG_ORDERED 8
#define LIST_FLAG_CACHE 16
#define LIST_FLAG_TIMEOUT 32
#define LIST_FLAG_MAP_HEAD   64
#define LIST_FLAG_MAP_CHAIN 128
#define LIST_FLAG_MAP (LIST_FLAG_MAP_HEAD | LIST_FLAG_MAP_CHAIN)
#define LIST_FLAG_STATS 256

#define LIST_FLAG_USER1 1024
#define LIST_FLAG_USER2 2048
#define LIST_FLAG_USER3 4096
#define LIST_FLAG_USER4 8192
#define LIST_FLAG_USER5 16384
#define LIST_FLAG_DEBUG 32768

#define ANYTYPE -1

typedef struct 
{
time_t Time;
//In the 'head' item 'Hits' is used to hold the count of items in the list
unsigned long Hits;
} ListStats;

//attempt to order items in likely use order, this *might* help the L1 cache
//is 32 bytes, so should fit in one cache line
typedef struct lnode
{
struct lnode *Next;
struct lnode *Prev;
char *Tag;
//in map heads ItemType is used to hold the number of buckets
uint16_t ItemType;
uint16_t Flags;
struct lnode *Head;
void *Item;
struct lnode *Side;
ListStats *Stats;
} ListNode;


//things not worthy of a full function or which pull tricks with macros

//this allows 'ListCreate' to be called with flags or without
#define ListCreate(F) (ListInit(F + 0))

//if L isn't NULL then return L->Head, it's fine if L->Head is null
#define ListGetHead(Node) ((Node) ? (Node)->Head : NULL)

#define ListSize(node) (((node) && (node)->Head && (node)->Head->Stats) ? (node)->Head->Stats->Hits : 0)
#define ListNodeGetHits(node) ((node)->Stats ? (node)->Stats->Hits : 0)
#define ListNodeGetTime(node) ((node)->Stats ? (node)->Stats->Time : 0)


//#define ListGetNext(Node) (Node ? (((Node)->Head->Flags & (LIST_FLAG_MAP|LIST_FLAG_MAP_CHAIN)) ? MapGetNext(Node) : (Node)->Next) : NULL)
#define ListGetNext(Node) ((Node) ? MapGetNext(Node) : NULL)

//listDeleteNode handles ListFindItem returning NULL, so no problems here
#define ListDeleteItem(list, item) (ListDeleteNode(ListFindItem((list), (item))))

#define ListInsertItem(list, item) (ListInsertTypedItem((list), 0, "", (item)))
#define ListAddItem(list, item) (ListAddTypedItem((list), 0, "", (item)))

#define ListInsertNamedItem(list, name, item) (ListInsertTypedItem((list), 0, (name), (item)))
#define ListAddNamedItem(list, name, item) (ListAddTypedItem((list), 0, (name), (item)))


//Again, NULL is handled here by ListInsertTypedItem
#define OrderedListAddNamedItem(list, name, item) (ListInsertTypedItem(ListFindNamedItemInsert((list), (name)), 0, (name), (item)))

//in map heads ItemType is used to hold the number of buckets
#define MapChainCount(Head) (((Head)->Flags & LIST_FLAG_MAP) ? (Head)->ItemType : 0)

#ifdef __cplusplus
extern "C" {
#endif


typedef void (*LIST_ITEM_DESTROY_FUNC)(void *);
typedef void *(*LIST_ITEM_CLONE_FUNC)(void *);

void MapDumpSizes(ListNode *Head);

ListNode *ListInit(int Flags);
ListNode *MapCreate(int Buckets, int Flags);
ListNode *MapGetNthChain(ListNode *Map, int n);
ListNode *MapGetChain(ListNode *Map, const char *Key);

void ListDestroy(ListNode *, LIST_ITEM_DESTROY_FUNC);
void ListClear(ListNode *, LIST_ITEM_DESTROY_FUNC);

void ListSetFlags(ListNode *List, int Flags);
void ListNodeSetTime(ListNode *Node, time_t When);
void ListNodeSetHits(ListNode *Node, int Hits);
void ListNodeAddHits(ListNode *Node, int Hits);


void ListThreadNode(ListNode *Prev, ListNode *Node);
void ListUnThreadNode(ListNode *Node);

ListNode *ListAddTypedItem(ListNode *, uint16_t, const char *Name, void *);
ListNode *ListInsertTypedItem(ListNode *,uint16_t, const char *,void *);
ListNode *SortedListInsertItem(ListNode *, void *, int (*LessThanFunc)(void *, void *, void *));
ListNode *MapGetNext(ListNode *);
ListNode *ListGetPrev(ListNode *);
ListNode *ListGetLast(ListNode *);
ListNode *ListGetNth(ListNode *Head, int n);
ListNode *ListFindNamedItemInsert(ListNode *Head, const char *Name);
ListNode *ListFindTypedItem(ListNode *Head, int Type, const char *Name);
ListNode *ListFindNamedItem(ListNode *Head, const char *Name);
ListNode *ListFindItem(ListNode *Head, void *Item);
ListNode *ListJoin(ListNode *, ListNode *);
ListNode *ListClone(ListNode *, LIST_ITEM_CLONE_FUNC);
void ListAppendItems(ListNode *Dest, ListNode *Src, LIST_ITEM_CLONE_FUNC ItemCloner);
void ListSort(ListNode *, void *Data, int (*LessThanFunc)(void *, void *, void *));
void ListSortNamedItems(ListNode *List);

void *ListDeleteNode(ListNode *);

void ListSwapItems(ListNode *, ListNode *);

#ifdef __cplusplus
}
#endif


#endif
