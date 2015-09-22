#ifndef _TEXTURE_
#define _TEXTURE_

#include "common.h"
#include "list.h"

typedef struct SRGBAPixel {
    unsigned char b;
    unsigned char g;
    unsigned char r;
    unsigned char a;
} TRGBAPixel;

typedef struct SRGBAPixelInt32 {
    unsigned int b;
    unsigned int g;
    unsigned int r;
    unsigned int a;
} TRGBAPixelInt32;

typedef struct {
    unsigned int glTexture;
    unsigned char * pixels;
    bool mips;
    int width;
    int height;
    int bpp;
    int bytesPerPixel;
    unsigned int bytesCount;
    char fileName[256];
} TTexture;

// global storage of all textures
extern TList g_textures;

// to get texture from file use this function, it also cache loaded textures. note that
// this function create texture on heap and it's must be freed after use
TTexture * Texture2D_LoadFromFile( const char * file );
TTexture * Texture2D_Create( int width, int height, int bytePerPixel, void * data );
void Texture2D_Free( TTexture * tex );
void Texture2D_FreeAll( void );

#endif