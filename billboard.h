#ifndef _BILLBOARD_
#define _BILLBOARD_

#include "entity.h"
#include "Texture.h"

OLDTECH_BEGIN_HEADER

typedef struct TBillboard {
    float width;
    float height;
    struct TEntity * owner;
    TTexture * texture;
} TBillboard;

TBillboard * Billboard_Create( float w, float h );

OLDTECH_END_HEADER

#endif