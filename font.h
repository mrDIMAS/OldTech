#ifndef _FONT_
#define _FONT_

#include "common.h"
#include "texture.h"

OLDTECH_BEGIN_HEADER

typedef struct STexCoord {
    float x;
    float y;
} TTexCoord;

typedef struct TCharMetrics {
    int advanceX;
    int advanceY;
    int bitmapWidth;
    int bitmapHeight;
    int bitmapTop;
    int bitmapLeft;
    TTexCoord texCoords[4];
} TCharMetrics;

typedef struct {
    TRGBAPixel * pixels;
    int width;
    int height;
} TGlyph;

typedef struct SFont {
    TCharMetrics charMetrics[256];
    TTexture * atlas;
    int size;
} TFont;

TFont * Font_LoadFromFile( const char * file, int size );

OLDTECH_END_HEADER

#endif