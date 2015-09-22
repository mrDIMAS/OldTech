#include "ValueArray.h"

void ValueArray_Create( TValueArray * array ) {
    array->values = NULL;
    array->count = 0;
}


TValue * ValueArray_GetValueByName( TValueArray * array, const char * name ) {
    int i;
    for( i = 0; i < array->count; i++ ) {
        if( strcmp( name, array->values[i].name ) == 0 ) {
            return &array->values[i];
        }
    }

    Util_RaiseError( "ArrayGetValueByName: Unable to find '%s' value!", name );

    return 0;
}

void ValueArray_Free( TValueArray * var ) {
    int i;
    for( i = 0; i < var->count; i++ ) {
        Memory_Free( var->values[i].string );
        Memory_Free( var->values[i].name );
    }
    Memory_Free( var->values );
}