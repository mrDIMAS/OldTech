#ifndef _VERTEX_
#define _VERTEX_

#include "vector2.h"
#include "vector3.h"

typedef struct {
    TVec3 p;
    TVec3 n;
    TVector2 t;
    TVector2 t2;
    TVec3 tg;
} TVertex;

#endif