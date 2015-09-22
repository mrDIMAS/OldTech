#include "font.h"
#include "common.h"
#include "gui.h"
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

unsigned long CeilPow2( unsigned long v ) {
    int power = 1;
    while (v >>= 1) {
        power <<= 1;
    }
    power <<= 1;
    return power;
}

TFont * Font_LoadFromFile( const char * file, int size ) {   
    FT_Library ftLibrary;
    FT_Face face;     
    TFont * font = Memory_New( TFont );
    font->size = size;
	if( FT_Init_FreeType( &ftLibrary ) ) {
		Util_RaiseError( "Unable to initialize FreeType" );
	}
    // load new font face
    if( FT_New_Face( ftLibrary, file, 0, &face ) ) {
        Util_RaiseError( "Failed to load %s font!", file );
    }
    if( FT_Set_Pixel_Sizes( face, 0, size )) {
        Util_RaiseError( "Failed to FT_Set_Pixel_Sizes!" );
    }
    if( FT_Select_Charmap( face, FT_ENCODING_UNICODE )) {
        Util_RaiseError( "Failed to FT_Select_Charmap!" );
    }    
    int textureSize = CeilPow2( size * 16 );
    TRGBAPixel * pixels = Memory_AllocateClean( textureSize * textureSize * sizeof( TRGBAPixel ) );
    int subRectRow = 0;
    int subRectCol = 0;
    float tcStep = (1.0f / 16.0f) * ((float)(size*16) / (float)textureSize); 
    float tcX = 0.0f;
    float tcY = 0.0f;
    int charIndexOffset = 0;
    for( int i = 0; i < 256; i++ ) {
        int charIndex = i;
        if( i >= 177 ) {
            charIndex = 1024;
            charIndexOffset++;
        }
        if( FT_Load_Glyph( face, FT_Get_Char_Index( face, charIndex + charIndexOffset ), FT_LOAD_DEFAULT )) {
            Util_RaiseError( "Failed to FT_Load_Glyph!" );
        }
        if( FT_Render_Glyph( face->glyph, FT_RENDER_MODE_NORMAL )) {
            Util_RaiseError( "Failed to FT_Render_Glyph!" );
        }
        FT_Bitmap * bitmap = &face->glyph->bitmap;
        font->charMetrics[i].advanceX = face->glyph->advance.x >> 6;
        font->charMetrics[i].advanceY = face->glyph->advance.y >> 6;
        font->charMetrics[i].texCoords[0].x = tcX; 
        font->charMetrics[i].texCoords[0].y = tcY;
        font->charMetrics[i].texCoords[1].x = tcX + tcStep;
        font->charMetrics[i].texCoords[1].y = tcY;
        font->charMetrics[i].texCoords[2].x = tcX + tcStep;
        font->charMetrics[i].texCoords[2].y = tcY + tcStep;
        font->charMetrics[i].texCoords[3].x = tcX; 
        font->charMetrics[i].texCoords[3].y = tcY + tcStep;
        font->charMetrics[i].bitmapTop = face->glyph->bitmap_top;
        font->charMetrics[i].bitmapLeft = face->glyph->bitmap_left;
        font->charMetrics[i].bitmapWidth = bitmap->width;
        font->charMetrics[i].bitmapHeight = bitmap->rows;
        // read glyph bitmap into the atlas
        int pixelOffset = subRectRow * textureSize * size + subRectCol * size;
        TRGBAPixel * subRectPixel = pixels + pixelOffset;        
        for( int row = 0; row < bitmap->rows; row++ ) {
            for( int col = 0; col < bitmap->width; col++ ) {
                TRGBAPixel * pixel = subRectPixel + row * textureSize + col ;
                pixel->a = bitmap->buffer[ row * bitmap->width + col ];
                pixel->r = 255;
                pixel->g = 255;
                pixel->b = 255;
            }
        }
        tcX += tcStep;
        subRectCol++;
        if( subRectCol >= 16 ) {
            tcX = 0.0f;
            tcY += tcStep;
            subRectCol = 0;
            subRectRow++;
        }
    }
    if( FT_Done_Face( face )) {
        Util_RaiseError( "Unable to FT_Done_Face" );
    }
	if( FT_Done_FreeType( ftLibrary )) {
        Util_RaiseError( "Unable to FT_Done_FreeType" );
    }
    font->atlas = Texture2D_Create( textureSize, textureSize, 4, pixels );
    Log_Write( "Font '%s' successfully generated with glyph size %d!", file, size );
    Memory_Free( pixels );
    return font;
}