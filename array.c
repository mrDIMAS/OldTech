#include "array.h"
#include "lightmap.h"

void Array_ReInit( TArray * array ) {
    array->elementCount = 0;
    array->capacity = 1;
    array->elements = Memory_Allocate( array->elementSize );
    array->writePtr = array->elements;
}

TArray Array_Create( int elementSize ) {
    TArray array;
    array.elementSize = elementSize;
    Array_ReInit( &array );
    return array;
}

void Array_Clean( TArray * array ) {
    Memory_Free( array->elements );
    Array_ReInit( array );
}

void Array_Destroy( TArray * array ) {
    array->elementSize = 0;    
    array->elementCount = 0;
    array->capacity = 0;
    Memory_Free( array->elements );
    array->elements = NULL;
}

void Array_Add( TArray * array, void * element ) {       
    if( (array->elementCount + 1) > array->capacity ) {        
        array->capacity = ceilf( array->capacity * 1.5f );
        array->elements = Memory_Reallocate( array->elements, array->capacity * array->elementSize );
    }   
    array->writePtr = array->elements + array->elementCount * array->elementSize;    
    memcpy( array->writePtr, element, array->elementSize );  
    array->elementCount++;    
}

void Array_Erase( TArray * array, int n ) {
    if( n == 0 ) {
        // move data
        memcpy( array->elements, array->elements + array->elementSize, (array->elementCount - 1) * array->elementSize );
    } else if( n == (array->elementCount - 1)) {
        // keep last element in array
    } else {
        // n-th element in [0; elementCount] bounds
        void * writePtr = array->elements + n * array->elementSize;
        int moveSize = (array->elementCount - n) * array->elementSize;
        void * movePtr = array->elements + (n + 1) * array->elementSize;
        memcpy( writePtr, movePtr, moveSize );
    }    
    array->elementCount--;
}

int Array_Find( TArray * array, void * data ) {
    for( int i = 0; i < array->elementCount; i++ ) {
        if( (array->elements + array->elementSize * i) == data ) {
            return i;
        }
    }
    return -1;
}

void * Array_Get( TArray * array, int n ) {
    return array->elements + n * array->elementSize;
}


typedef struct TComplex {
    float real;
    float img;
} TComplex;

void Test_ArrayPrint( TArray * array ) {
    int i = 0;
    for_each_a( TComplex*, complex, *array ) {
        printf( "%f\t%f\n", complex->real, complex->img );
        Array_Erase( array, i );
        i++;
    }  
}

TArray Test_ArrayFill( TArray * array ) {
    for( int i = 0; i < 9; i++ ) {
        TComplex * complex = Memory_New( TComplex );
        complex->real = 1.0f;
        complex->img = i;
        Array_Add( array, &complex );
    }
    
    TArray a2 = Array_Create(sizeof(TComplex*));
    
    // do copy
    for_each_a( TComplex*, complex, *array ) {
        Array_Add( &a2, &complex );
    }
    return a2;
}


        
void Test_Array( ) {
    
    TArray array = Array_Create(sizeof(TComplex*));
    TArray other = Test_ArrayFill( &array );
    Test_ArrayPrint( &other );
    for_each_a( TComplex*, complex, array ) {
        printf( "%f\t%f\n", complex->real, complex->img );
    }
    
    /*
    TArray array;
    Array_Create( &array, sizeof(TComplex));
    for( int i = 0; i < 10; i++ ) {
        TComplex * complex = Memory_New( TComplex );
        complex->real = 1.0f;
        complex->img = rand() % 20;
        Array_Add( &array, complex );
    }   
    foreach( TComplex, complex, array ) {
        printf( "%f\t%f\n", complex.real, complex.img );
    }*/
    Array_Clean( &array );
}