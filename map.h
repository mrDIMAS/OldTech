#ifndef _MAP_
#define _MAP_

#include "common.h"

OLDTECH_BEGIN_HEADER

typedef struct SMap {
    struct TEntity * root;
    struct TEntity * body;
} TMap;

TMap * Map_LoadFromFile( const char * fileName );
#endif

OLDTECH_END_HEADER