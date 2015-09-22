#ifndef _FACE_
#define _FACE_

#include "common.h"

OLDTECH_BEGIN_HEADER

typedef unsigned short TIndex16;

typedef struct {
    TIndex16 index[3];
    int lightmapIndex;
} TFace;

OLDTECH_END_HEADER

#endif