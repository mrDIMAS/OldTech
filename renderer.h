#ifndef _RENDERER_
#define _RENDERER_

#include "common.h"
#include "surface.h"
#include "matrix4.h"
#include "texture.h"
#include "collision.h"
#include "font.h"
#include "shader.h"
#include "entity.h"

typedef struct {
    short width;
    short height;
    char bpp;
    bool fullscreen;
} TRenderSettings;

typedef enum {
    TF_LINEAR,
    TF_ANISOTROPIC,
} ETextureFilter;

typedef struct TGlyphRenderInfo {
    float x;
    float y;
    float w;
    float h;
    TTexCoord texCoords[4];
    TVec3 color;
} TGlyphRenderInfo;

void Renderer_RenderSurface( TSurface * surf, TMatrix4 mvp, struct TEntity * owner );
void Renderer_SetResolution( int width, int height, int bpp );
void Renderer_SetModelViewTransform( TMatrix4 modelView );
void Renderer_SetProjectionTransform( TMatrix4 proj );
int Renderer_GetScreenResolutionWidth( void );
int Renderer_GetScreenResolutionHeight( void );
float Renderer_GetWindowAspectRatio( void );
void Renderer_CreateWindow( const TRenderSettings * settings );
void Renderer_InitializeFull( const TRenderSettings * settings );
void Renderer_Shutdown( void );
void Renderer_FreeWindow( void );
void Renderer_PollWindowMessages( void );
void Renderer_InitializeOpenGL( void );
bool Renderer_IsRunning( void );
void Renderer_FreeOpenGL( void );
void Renderer_BeginRender( void );
void Renderer_EndRender( void );
void Renderer_BindTexture( TTexture * tex, int level );
void Renderer_RenderWorld( void );
void Renderer_Stop( void );
void Renderer_SetTextureFiltration( ETextureFilter texFilter );
void Renderer_DeleteSurfaceBuffers( TSurface * surf );
void Renderer_BuildSurfaceBuffers( TSurface * surf );
void Renderer_BindSurface( TSurface * surf );
void Renderer_RenderRect( float x, float y, float x2, float y2, TVec3 color );
void Renderer_RenderGlyph( const TGlyphRenderInfo * ri );
void Renderer_LoadTextureFromMemory( TTexture * texture, int width, int height, int bytePerPixel, void * data, bool generateMIPS );
void Renderer_FreeTexture( TTexture * texture );
int Renderer_GetWindowWidth( void );
int Renderer_GetWindowHeight( void );

#endif