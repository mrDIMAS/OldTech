#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <malloc.h>
#include "memory.h"
#include "utils.h"
#include <string.h>
#include "thread.h"
#include "log.h"

typedef struct SMemoryNode {
    void * data;
    int size;
    struct SMemoryNode * next;
    struct SMemoryNode * prev;
} TMemoryNode;


TMemoryNode * g_rootAllocNode = NULL;
TMemoryNode * g_lastAllocNode = NULL;
int totalAllocatedMemory = 0;


// to make memory management thread-safe we must use critical section
TCriticalSection criticalSection = NULL;

void Memory_EnterCriticalSection( void ) {
    while( !CriticalSection_TryEnter( criticalSection )) {
        // this message is mostly for debug purposes
        Log_Write( "Memory: - Waiting critical section" );
    }
}

void Memory_LeaveCriticalSection( void ) {
    CriticalSection_Leave( criticalSection );
}

void * Memory_CommonAllocation( int size, bool clear ) {
    if( !criticalSection ) {
        criticalSection = CriticalSection_Create();
    }
    
    Memory_EnterCriticalSection();
    
    void * block = malloc( size );
    if( !block ) {
        Util_RaiseError( "Unable to allocate %d bytes. Not enough memory! Allocation failed!", size );
    }
    if( clear ) {
        memset( block, 0, size );
    }
    totalAllocatedMemory += size;
    TMemoryNode * newAllocNode = malloc( sizeof( TMemoryNode ));
    newAllocNode->data = block;
    newAllocNode->next = NULL;
    newAllocNode->size = size;
    newAllocNode->prev = g_lastAllocNode;
    if( g_lastAllocNode ) {
        g_lastAllocNode->next = newAllocNode;
    }
    if( !g_rootAllocNode ) {
        g_rootAllocNode = newAllocNode;
    }
    g_lastAllocNode = newAllocNode;
    
    Memory_LeaveCriticalSection();
    
    return block;
}

void * Memory_Allocate( int size ) {
    return Memory_CommonAllocation( size, false );
}

void * Memory_AllocateClean( int size ) {
    return Memory_CommonAllocation( size, true );
}

int Memory_GetAllocated( void ) {
    return totalAllocatedMemory;
}

void * Memory_Reallocate( void * data, int newSize ) {
    Memory_EnterCriticalSection();
    // find existing memory block
    TMemoryNode * current = g_rootAllocNode;
    void * newData = 0;
    while( current ) {
        if( current->data == data ) {
            newData = realloc( data, newSize );
            if( !newData ) {
                Util_RaiseError( "Memory reallocation failed!" );
            }
            // got pointer to another place, rewrite it in the memblock
            if( newData != data ) {
                current->data = newData;
            }
        }
        current = current->next;
    }
    Memory_LeaveCriticalSection();
    return newData;
}

void Memory_Free( void * data ) {
    Memory_EnterCriticalSection();
    TMemoryNode * current = g_rootAllocNode;
    while( current ) {
        if( current->data == data ) {
            if( current->next ) {
                current->next->prev = current->prev;
            }
            if( current->prev ) {
                current->prev->next = current->next;
            }
            if( current == g_rootAllocNode ) {
                if( current->next ) {
                    g_rootAllocNode = current->next;
                } else {
                    g_rootAllocNode = NULL;
                }
            }
            if( current == g_lastAllocNode ) {
                if( current->prev ) {
                    g_lastAllocNode = current->prev;
                } else {
                    g_lastAllocNode = NULL;
                }
            }
            totalAllocatedMemory -= current->size;
            free( current );
            free( data );
            CriticalSection_Leave( criticalSection );
            return;
        }
        current = current->next;
    }
    Memory_LeaveCriticalSection();
    Util_RaiseError( "Attempt to free memory %d allocated without Memory_Allocate[Clean]", (int)data );
}


void Memory_CollectGarbage( ) {
    CriticalSection_Delete( criticalSection );
    TMemoryNode *next, *current;
    for( current = g_rootAllocNode; current; current = next ) {
        next = current->next;
        totalAllocatedMemory -= current->size;
        free( current->data );
        free( current );
        current = next;
    }
}