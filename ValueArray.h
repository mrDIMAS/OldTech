#ifndef _VALUE_ARRAY_
#define _VALUE_ARRAY_

#include "common.h"

typedef struct {
    char * name;
    char * string;
    float number;
} TValue;

typedef struct {
    int count;
    TValue * values;
} TValueArray;

void ValueArray_Create( TValueArray * array );
TValue * ValueArray_GetValueByName( TValueArray * array, const char * name );
void ValueArray_Free( TValueArray * var );

#endif