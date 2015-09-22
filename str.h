#ifndef _STR_
#define _STR_

#include "memory.h"
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

char gStringFormatBuffer[8192];

static inline char * String_Duplicate( const char * str ) {
    int size = strlen( str );
    char * mem = Memory_AllocateClean( size + 1 );
    strncpy( mem, str, size );
    mem[size] = 0;
    return mem;
}

static inline void String_Free( char * str ) {
    Memory_Free( str );
}

static inline float String_ToFloat( const char * str ) {
    return atof( str );
}

static inline int String_ToInteger( const char * str ) {
    return atoi( str );
}

static inline char * String_Format( const char * format, ... ) {
    va_list	argumentList;
    va_start( argumentList, format);
    vsprintf( gStringFormatBuffer, format, argumentList );
    va_end( argumentList );
    return String_Duplicate( gStringFormatBuffer );
}

#endif