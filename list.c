#include "list.h"
#include "common.h"
#include "thread.h"

void List_Create( TList * list ) {
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

void List_Free( TList * list ) {
	TListNode * current = list->head;
    while( current ) {
		TListNode * del = current;
        current = current->next;
		Memory_Free( del );
    }
}

void List_Add( TList * list, void * data ) {
    TListNode * newElement = Memory_New( TListNode );
    newElement->data = data;
    newElement->next = NULL;
    newElement->prev = list->tail;
    if( list->tail ) {
        list->tail->next = newElement;
    }
    if( !list->head ) {
        list->head = newElement;
    }
    list->tail = newElement;
    list->size++;
}

TListNode * List_Find( TList * list, void * data ) {
    for( TListNode * node = list->head; node; node = node->next ) {
        if( node->data == data ) {
            return node;
        }
    }
    return 0;    
}

int List_Count( TList * list ) {
    TListNode * current = list->head;
    int count = 0;
    while( current ) {
        count++;
        current = current->next;
    }
    
    return count;
}

void List_RemoveNth( TList * list, int n, char freeData ) {
    int i = 0;
    TListNode * current = list->head;
    while( current ) {
        if( i == n ) {
            if( current->next ) {
                current->next->prev = current->prev;
            }
            if( current->prev ) {
                current->prev->next = current->next;
            }
            if( current == list->head ) {
                if( current->next ) {
                    list->head = current->next;
                } else {
                    list->head = NULL;
                }
            }
            if( current == list->tail ) {
                if( current->prev ) {
                    list->tail = current->prev;
                } else {
                    list->tail = NULL;
                }
            }
            Memory_Free( current );
            if( freeData ) {
                Memory_Free( current->data );
            }
            list->size--;
            break;
        }
        i++;
        current = current->next;
    }
}

void List_Clear( TList * list, char freeData ) {
    TListNode * current = list->head;
    while( current ) {
        if( freeData ) {
            if( current->data ) {
                Memory_Free( current->data );
            }
        }
        TListNode * next = current->next;
        Memory_Free( current );
        current = next;
    }
    list->size = 0;
}

void * List_GetNodeData( TList * list, int n ) {
    TListNode * current = list->head;
    int count = 0;
    while( current ) {
        if( count == n ) {
            return current->data;
        }
        count++;
        current = current->next;
    }
    return NULL;
}

TListNode * List_Remove( TList * list, void * data ) {
    TListNode * current = list->head;
	TListNode * nextOfDeleted = NULL;
    while( current ) {
        if( current->data == data ) {
            if( current->next ) {
                current->next->prev = current->prev;
            }
            if( current->prev ) {
                current->prev->next = current->next;
            }
			nextOfDeleted = current->next;
            if( current == list->head ) {
                if( current->next ) {
                    list->head = current->next;
                } else {
                    list->head = NULL;
                }
            }
            if( current == list->tail ) {
                if( current->prev ) {
                    list->tail = current->prev;
                } else {
                    list->tail = NULL;
                }
            }
            Memory_Free( current );
            list->size--;
            break;
        }
        current = current->next;
    }
	return nextOfDeleted;
}