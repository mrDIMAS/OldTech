#ifndef _ARRAY_
#define _ARRAY_

#include "common.h"

OLDTECH_BEGIN_HEADER

typedef struct {
    int elementSize;
    int elementCount;
    int capacity;    
    void *writePtr;
    char *elements;
} TArray;

#define for_each_a(type,name,array) type name = *((type*)(array).elements);\
    for( type* ptr##name = ((type*)(array).elements); ptr##name < ((type*)(array).elements) + (array).elementCount; (ptr##name)++, name = *(ptr##name) )
        
TArray Array_Create( int elementSize );
void Array_Destroy( TArray * array );
void Array_Add( TArray * array, void * element );
void Array_Erase( TArray * array, int n );
void * Array_Get( TArray * array, int n );
void Array_Clean( TArray * array );
int Array_Find( TArray * array, void * data );


// tests
void Test_Array( void );

OLDTECH_END_HEADER

#endif