#ifndef _MEMORY_
#define _MEMORY_

// allocate memory without cleaning
void * Memory_Allocate( int size );
// allocate clean memory (filled with zeros)
void * Memory_AllocateClean( int size );
void * Memory_Reallocate( void * data, int newSize );
// safe memory freeing, attempt to free data not created with Memory_Allocate will cause error
// this functions is not thread safe!
void Memory_Free( void * data );
// call this function when your program ends for cleanup
void Memory_CollectGarbage( void );
// retrieve allocated memory size
int Memory_GetAllocated( void );
// some useful macros
// allocate memory for type with clearing allocated memory with zeros
#define Memory_New( typeName ) ((typeName*)Memory_AllocateClean( sizeof(typeName )))
// allocate array of type with clearing allocated memory with zeros
#define Memory_NewCount( count, typeName ) ((typeName*)Memory_AllocateClean( count * sizeof(typeName )))

#endif