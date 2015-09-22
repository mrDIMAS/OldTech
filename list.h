#ifndef _LIST_
#define _LIST_

typedef struct SListNode {
    void * data;
    struct SListNode * prev;
    struct SListNode * next;
} TListNode;

typedef struct {
    TListNode * head;
    TListNode * tail;
    int size;
} TList;

#define for_each( type, node, list ) type * (node) = (list).head ? (type*)(list).head->data : 0; \
    for( TListNode * (type##ListNode) = (list).head; (type##ListNode); (node) = ((type##ListNode)->next) ? (type*)(type##ListNode)->next->data : 0, (type##ListNode) = (type##ListNode)->next )
    
void List_Create( TList * list );
void List_Free( TList * list );
void List_Add( TList * list, void * data );
TListNode * List_Remove( TList * list, void * data );
int List_Count( TList * list );
void List_RemoveNth( TList * list, int n, char freeData );
void * List_GetNodeData( TList * list, int n );
void List_Clear( TList * list, char freeData );
TListNode * List_Find( TList * list, void * data );
#endif