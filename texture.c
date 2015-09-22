#include "texture.h"
#include "renderer.h"

void LoadTextureFromTGA( TTexture * tex, const char * file );
    
TList g_textures = { 0 };

void LoadTextureFromTGA( TTexture * tex, const char * file ) {
    unsigned char tgaHeader[12], tgaInfo[6];
    FILE * tgaFile = fopen( file, "rb" );
    if( !tgaFile ) {
        Util_RaiseError( "Unable to load %s", file );
    }
    if( !fread( &tgaHeader, sizeof(tgaHeader), 1, tgaFile )) {
        Util_RaiseError( "Unable to load %s", file );
    }
    if( !fread( tgaInfo, sizeof( tgaInfo ), 1, tgaFile )) {
        Util_RaiseError( "Unable to load %s", file );
    }
    tex->width = tgaInfo[1] * 256 + tgaInfo[0];
    tex->height = tgaInfo[3] * 256 + tgaInfo[2];
    tex->bpp = tgaInfo[4];
    if( ( tex->width <= 0) || ( tex->height <= 0 ) || (( tex->bpp != 24 ) && ( tex->bpp != 32))) {
        Util_RaiseError( "Unable to load %s! Bad metrics!", file );
    }

    tex->bytesPerPixel = tex->bpp / 8;
    tex->bytesCount = tex->bytesPerPixel * tex->width * tex->height;

    tex->pixels = Memory_Allocate( tex->bytesCount );
    strcpy( tex->fileName, file );
    if( fread( tex->pixels, 1, tex->bytesCount, tgaFile ) != tex->bytesCount ) {
        Util_RaiseError( "Unable to load %s. Corrupted data!", file );
    }

    // Red and Blue swap loop 
    for( unsigned int i = 0; i < tex->bytesCount; i += tex->bytesPerPixel ) {
        unsigned char temp = tex->pixels[i];
        tex->pixels[i] = tex->pixels[i+2];
        tex->pixels[i+2] = temp;
    }

    fclose( tgaFile );

    Renderer_LoadTextureFromMemory( tex, tex->width, tex->height, tex->bytesPerPixel, tex->pixels, true );

    Memory_Free( tex->pixels );
    tex->pixels = 0;
    
    Log_Write( "Texture '%s' loaded successfully!", file );
    List_Add( &g_textures, tex );
}



TTexture * Texture2D_LoadFromFile( const char * file ) {
    // find existing 
    for( TListNode * current = g_textures.head; current; current = current->next ) {
        TTexture * tex = (TTexture*)current->data;
        if( strcmp( file, tex->fileName ) == 0 ) {
            return tex;
        }
    }
    // no existing texture, so load new one 
    TTexture * newTex = Memory_New( TTexture );
    LoadTextureFromTGA( newTex, file );
    return newTex;
}

TTexture * Texture2D_Create( int width, int height, int bytePerPixel, void * data ) {
    TTexture * texture = Memory_New( TTexture );
    Renderer_LoadTextureFromMemory( texture, width, height, bytePerPixel, data, false );
    strcpy( texture->fileName, "[Procedure texture]" );    
    return texture;
}

void Texture2D_Free( TTexture * tex ) {
    if( tex ) {
        Renderer_FreeTexture( tex );
        if( tex->pixels ) {
            Memory_Free( tex->pixels );
        }
    }
}

void Texture2D_FreeAll() {
    for_each( TTexture, texture, g_textures ){
        Texture2D_Free( texture );
        Memory_Free( texture );
    }
}