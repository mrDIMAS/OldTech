/*
 * Note, that only this file use OpenGL functionality, so you can add new renderer by 
 * replacing content of this file
 */
 
void Util_CheckGLErrorFunc( const char * file, int line, const char * func );

#ifdef _DEBUG_GL_
#   define Debug_CheckGLError( glFunc ) ( glFunc ); Util_CheckGLErrorFunc( __FILE__, __LINE__, __func__ )
#else
#   define Debug_CheckGLError( glFunc ) ( glFunc )
#endif

#ifdef _WIN32
#   define _WIN32_WINNT 0x0501 // win xp
#   include <windows.h>
#   include <winuser.h>
#endif

#include "renderer.h"
#include "camera.h"
#include "entity.h"
#include "timer.h"
#include "log.h"
#include "lightmap.h"
#include "billboard.h"
#include "input.h"
#include "lightmap.h"
#include "shader.h"
#include "font.h"

// OpenGL Functionality
#include <gl/gl.h>
#include <gl/glu.h>
#include "glext.h"
#include "wglext.h"

// Multitexturing
PFNGLACTIVETEXTUREARBPROC glActiveTextureARB = NULL;
PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB = NULL;
PFNGLMULTITEXCOORD1FARBPROC glMultiTexCoord1f = NULL;
PFNGLMULTITEXCOORD1FVARBPROC glMultiTexCoord1fv = NULL;
PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2f = NULL;
PFNGLMULTITEXCOORD2FVARBPROC glMultiTexCoord2fv = NULL;

// ARB Program
PFNGLGETPROGRAMIVARBPROC glGetProgramivARB = NULL;
PFNGLPROGRAMSTRINGARBPROC glProgramStringARB = NULL;
PFNGLBINDPROGRAMARBPROC glBindProgramARB = NULL;
PFNGLDELETEPROGRAMSARBPROC glDeleteProgramsARB = NULL;
PFNGLGENPROGRAMSARBPROC glGenProgramsARB = NULL;
PFNGLPROGRAMENVPARAMETER4FARBPROC glProgramEnvParameter4fARB = NULL;
PFNGLPROGRAMENVPARAMETER4FVARBPROC glProgramEnvParameter4fvARB = NULL;
PFNGLPROGRAMLOCALPARAMETER4FARBPROC glProgramLocalParameter4fARB = NULL;
PFNGLPROGRAMLOCALPARAMETER4FVARBPROC glProgramLocalParameter4fvARB = NULL;
PFNGLGETPROGRAMENVPARAMETERFVARBPROC glGetProgramEnvParameterfvARB = NULL;
PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC glGetProgramLocalParameterfvARB = NULL;
PFNGLGETPROGRAMSTRINGARBPROC glGetProgramStringARB = NULL;
PFNGLISPROGRAMARBPROC glIsProgramARB = NULL;

// VBO
PFNGLGENBUFFERSARBPROC glGenBuffersARB = NULL;
PFNGLBINDBUFFERARBPROC glBindBufferARB = NULL;
PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB = NULL;
PFNGLBUFFERDATAARBPROC glBufferDataARB = NULL;
PFNGLBUFFERSUBDATAARBPROC glBufferSubDataARB = NULL;

// Win32 Window Procedure
LRESULT CALLBACK Renderer_WindowProc( HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam );

typedef struct TRenderer {
    HWND handle;
    TRenderSettings settings;
    HGLRC glContext;
    HDC deviceContext;
    float aspect;
    bool running;
    
    bool anisotropicSupport;
    int maxAnisotropy;
    ETextureFilter texFilter;
    
    TShader vertexShader;
    TShader fragmentShader;
    
    TShader guiVertexShader;
    TShader guiFragmentShader;
    
    GLuint whitePixelTexture;
} TRenderer;

TRenderer * gRenderer = NULL;

float Renderer_GetWindowAspectRatio() {
    return gRenderer->aspect;
}

void Renderer_Shutdown() {
    Texture2D_FreeAll();
    Renderer_FreeOpenGL();
    Renderer_FreeWindow();
}

bool Renderer_IsRunning() {
    return gRenderer->running;
}

void Renderer_Stop() {
    gRenderer->running = false;
}

void Renderer_DeleteSurfaceBuffers( TSurface * surf ) {
    if( surf->buffersReady ) {
        glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );
        glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );
        glDeleteBuffersARB( 1, &surf->vertexBuffer );
        for( int i = 0; i < SURF_MAXLIGHTMAPS; i++ ) {
            glDeleteBuffersARB( 1, &surf->indexBuffers[i] );            
        }
        surf->buffersReady = false;
    }
}

void Renderer_RenderRect( float x, float y, float x2, float y2, TVec3 color ) {
    Renderer_SetShaderLocal( &gRenderer->guiVertexShader, 0, color.x, color.y, color.z, 1.0f );
    
    glBegin( GL_QUADS );
    
    glMultiTexCoord2f( GL_TEXTURE0_ARB, 0.0f, 0.0f );
    glVertex3f( x, y, 0.0f );
    
    glMultiTexCoord2f( GL_TEXTURE0_ARB, 1.0f, 0.0f );
    glVertex3f( x2, y, 0.0f ); 
  
    glMultiTexCoord2f( GL_TEXTURE0_ARB, 1.0f, 1.0f );
    glVertex3f( x2, y2, 0.0f ); 
  
    glMultiTexCoord2f( GL_TEXTURE0_ARB, 0.0f, 1.0f );
    glVertex3f( x, y2, 0.0f ); 
    
    glEnd();
}

void Renderer_RenderGlyph( const TGlyphRenderInfo * ri ) {
    Renderer_SetShaderLocal( &gRenderer->guiVertexShader, 0, ri->color.x, ri->color.y, ri->color.z, 1.0f );
    
    glBegin( GL_QUADS );
    
    glMultiTexCoord2f( GL_TEXTURE0_ARB, ri->texCoords[0].x, ri->texCoords[0].y );
    glVertex3f( ri->x, ri->y, 0.0f );
    
    glMultiTexCoord2f( GL_TEXTURE0_ARB, ri->texCoords[1].x, ri->texCoords[1].y );
    glVertex3f( ri->x + ri->w, ri->y, 0.0f ); 
  
    glMultiTexCoord2f( GL_TEXTURE0_ARB, ri->texCoords[2].x, ri->texCoords[2].y );
    glVertex3f( ri->x + ri->w, ri->y + ri->h, 0.0f ); 
  
    glMultiTexCoord2f( GL_TEXTURE0_ARB, ri->texCoords[3].x, ri->texCoords[3].y );
    glVertex3f( ri->x, ri->y + ri->h, 0.0f ); 
    
    glEnd();
}

void Renderer_LoadTextureFromMemory( TTexture * texture, int width, int height, int bytePerPixel, void * data, bool generateMIPS ) {
    Debug_CheckGLError( glGenTextures( 1, &texture->glTexture ));
    Debug_CheckGLError( glBindTexture( GL_TEXTURE_2D, texture->glTexture ));
    Debug_CheckGLError( glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, generateMIPS ? GL_TRUE : GL_FALSE ));    
    // auto compress textures
    if( bytePerPixel == 3 ){
        Debug_CheckGLError( glTexImage2D( GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data ));
    } else {
        Debug_CheckGLError( glTexImage2D( GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data ));        
    }    
    Debug_CheckGLError( glBindTexture( GL_TEXTURE_2D, 0 ));
    // now texture loaded to GPU
    texture->width = width;
    texture->height = height;
    texture->mips = generateMIPS;
    texture->bpp = bytePerPixel * 8;
    texture->bytesPerPixel = bytePerPixel;
    texture->bytesCount = width * height * bytePerPixel;
}

void Renderer_CompileARBShader( TShader * shader, char * program, int size ) {
    if( strncmp( program, "!!ARBvp", 7 ) == 0 ) {
        shader->type = SHADER_VERTEX;
    } else if ( strncmp( program, "!!ARBfp", 7 ) == 0 ) {
        shader->type = SHADER_FRAGMENT;
    } else {
        Util_RaiseError( "Unable to load shader! Unsupported format!" );
    }
    
	shader->target = (shader->type == SHADER_VERTEX) ? GL_VERTEX_PROGRAM_ARB : GL_FRAGMENT_PROGRAM_ARB;
    
	glEnable( shader->target );
	glGenProgramsARB( 1, &shader->id );
	glBindProgramARB( shader->target, shader->id );
	glProgramStringARB( shader->target, GL_PROGRAM_FORMAT_ASCII_ARB, size, program );
	const GLubyte* errorString = glGetString( GL_PROGRAM_ERROR_STRING_ARB );
	if( glGetError() != GL_NO_ERROR ) {
		Util_RaiseError( "Unable to compile shader! Compiler output:\n%s", errorString );
	} else {
        Log_Write( "Shader successfully compiled!" );
    }    
}

void Renderer_FreeTexture( TTexture * texture ) {
    glDeleteTextures( 1, &texture->glTexture );
}

typedef struct {
    unsigned int a, b, c;
} TRawIndex;

void Renderer_BuildSurfaceBuffers( TSurface * surf ) {       
    // vertex buffer
    if( !surf->vertexBuffer ) {
        // create vertex buffer
        Debug_CheckGLError( glGenBuffersARB( 1, &surf->vertexBuffer ));
        Debug_CheckGLError( glBindBufferARB( GL_ARRAY_BUFFER_ARB, surf->vertexBuffer ));
        if( surf->skinned ) {
            Debug_CheckGLError( glBufferDataARB( GL_ARRAY_BUFFER_ARB, surf->vertexCount * sizeof( TVertex ), surf->vertices, GL_DYNAMIC_DRAW_ARB ));
        } else {
            Debug_CheckGLError( glBufferDataARB( GL_ARRAY_BUFFER_ARB, surf->vertexCount * sizeof( TVertex ), surf->vertices, GL_STATIC_DRAW_ARB ));
        }
        Debug_CheckGLError( glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 ));
    } else {
        // modify vertex buffer (i.e. for animation)
        Debug_CheckGLError( glBindBufferARB( GL_ARRAY_BUFFER_ARB, surf->vertexBuffer ));
        if( surf->skinned ) {
            Debug_CheckGLError( glBufferSubDataARB( GL_ARRAY_BUFFER_ARB, 0, surf->vertexCount * sizeof( TVertex ), surf->skinVertices ));
        } else {            
            Debug_CheckGLError( glBufferSubDataARB( GL_ARRAY_BUFFER_ARB, 0, surf->vertexCount * sizeof( TVertex ), surf->vertices ));
        }
        Debug_CheckGLError( glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 ));
    }
    // index buffers 
    TRawIndex * tempIndices = Memory_NewCount( surf->faceCount, TRawIndex );
    if( surf->lightmapCount > 0 ) {
        int index = 0;
        while( true ) {
            int pointer = 0;
            for( int i = 0; i < surf->faceCount; i++ ) {
                TFace * face = &surf->faces[i];                    
                tempIndices[pointer].a = face->index[0];
                tempIndices[pointer].b = face->index[1];
                tempIndices[pointer].c = face->index[2];                    
                if( face->lightmapIndex > index ) {                        
                    index = face->lightmapIndex;
                    break;
                }
                pointer++;
            }
            // for lightmapped surface create index buffer per lightmap
            Debug_CheckGLError( glGenBuffersARB( 1, &surf->indexBuffers[ index ] ));
            Debug_CheckGLError( glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, surf->indexBuffers[ index ] ));
            Debug_CheckGLError( glBufferDataARB( GL_ELEMENT_ARRAY_BUFFER_ARB, surf->lightmapFaceCount[ index ] * sizeof( TRawIndex ), tempIndices, GL_STATIC_DRAW_ARB ));
            Debug_CheckGLError( glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 ));  
            if( pointer == surf->faceCount ) {
                break;
            }
        }
    } else { 
        // for non-lightmapped surface create single index buffer
        for( int i = 0; i < surf->faceCount; i++ ) {
            TFace * face = &surf->faces[i];
            tempIndices[i].a = face->index[0];
            tempIndices[i].b = face->index[1];
            tempIndices[i].c = face->index[2];
        }
        Debug_CheckGLError( glGenBuffersARB( 1, &surf->indexBuffers[0] ));
        Debug_CheckGLError( glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, surf->indexBuffers[0] ));
        Debug_CheckGLError( glBufferDataARB( GL_ELEMENT_ARRAY_BUFFER_ARB, surf->faceCount * sizeof( TRawIndex ), tempIndices, GL_STATIC_DRAW_ARB ));
        Debug_CheckGLError( glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 ));
    }
    Memory_Free( tempIndices );        
    surf->buffersReady = true;
}

void Renderer_BindSurface( TSurface * surf ) {
    Debug_CheckGLError( glEnableClientState( GL_INDEX_ARRAY ));
    Debug_CheckGLError( glEnableClientState( GL_VERTEX_ARRAY ));
    
    Debug_CheckGLError( glBindBufferARB( GL_ARRAY_BUFFER_ARB, surf->vertexBuffer ));
    Debug_CheckGLError( glVertexPointer( 3, GL_FLOAT, sizeof( TVertex ), (void*)0 ));
    
    int texCoord0Stride = ((int)&surf->vertices[0].t) - ((int)&surf->vertices[0]);
    int texCoord1Stride = ((int)&surf->vertices[0].t2) - ((int)&surf->vertices[0]);
    
    Debug_CheckGLError( glClientActiveTextureARB( GL_TEXTURE0_ARB ));
    Debug_CheckGLError( glEnableClientState( GL_TEXTURE_COORD_ARRAY ));
    Debug_CheckGLError( glTexCoordPointer( 2, GL_FLOAT, sizeof( TVertex ), (void*)texCoord0Stride ));

    Debug_CheckGLError( glClientActiveTextureARB( GL_TEXTURE1_ARB ));
    Debug_CheckGLError( glEnableClientState( GL_TEXTURE_COORD_ARRAY ));
    Debug_CheckGLError( glTexCoordPointer( 2, GL_FLOAT, sizeof( TVertex ), (void*)texCoord1Stride ));
}

void Renderer_RenderSurface( TSurface * surf, const TVec3 * color ) {
    Renderer_SetShaderLocal( &gRenderer->vertexShader, 0, color->x, color->y, color->z, 1.0f );    
    
    Renderer_BindTexture( surf->texture, 0 );    
    
    if( !surf->lightmapped ) {     
        // set 'white-pixel' texture as active to lightmap sampler
        Debug_CheckGLError( glActiveTextureARB( GL_TEXTURE1_ARB ));
        Debug_CheckGLError( glBindTexture( GL_TEXTURE_2D, gRenderer->whitePixelTexture ));
    }
    
    if( !surf->facesSorted ) {
        Surface_SortFaces( surf );
    }

    if( !surf->buffersReady ) { 
        Renderer_BuildSurfaceBuffers( surf );
    }
    
    Renderer_BindSurface( surf );
    
    if( surf->lightmapCount > 0 ){         
        for( int i = 0; i < surf->lightmapCount; i++ ) {
            TLightmapAtlas * atlas = surf->lightmaps[ i ];              
            if( atlas->texture.glTexture == 0 ) {
                Debug_CheckGLError( glGenTextures( 1, &atlas->texture.glTexture ));   
                Debug_CheckGLError( glBindTexture( GL_TEXTURE_2D, atlas->texture.glTexture ));     
                Debug_CheckGLError( glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, atlas->texture.width, atlas->texture.height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, atlas->texture.pixels ));    
                atlas->modified = false;
            } else { 
                Renderer_BindTexture( &atlas->texture, 1 );     
                if( atlas->modified ) {                     
                    Debug_CheckGLError( glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, atlas->texture.width, atlas->texture.height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, atlas->texture.pixels ));            
                    atlas->modified = false;
                }
            }    
                
            Debug_CheckGLError( glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, surf->indexBuffers[i] ));
            Debug_CheckGLError( glIndexPointer( GL_INT, 0, NULL ));
            Debug_CheckGLError( glDrawElements( GL_TRIANGLES, 3 * surf->lightmapFaceCount[i], GL_UNSIGNED_INT, NULL ));
        }
    } else {
        Debug_CheckGLError( glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, surf->indexBuffers[0] ));
        Debug_CheckGLError( glIndexPointer( GL_INT, 0, NULL ));        
        Debug_CheckGLError( glDrawElements( GL_TRIANGLES, 3 * surf->faceCount, GL_UNSIGNED_INT, NULL ));
    }
}

void Renderer_InitializeFull( const TRenderSettings * settings ) {
    gRenderer = Memory_New( TRenderer );
    Log_Open( &g_log, "OldTech.log" );
    Variable_InitSubSystem();
    Renderer_CreateWindow( settings );
    Renderer_InitializeOpenGL();
    gRenderer->running = true;
}

void Renderer_RenderWorld() { 
    if( pActiveCamera ) {
        Renderer_BindShader( &gRenderer->vertexShader );
        Renderer_BindShader( &gRenderer->fragmentShader );
        
        for_each( TEntity, entity, g_entities ) {
            Entity_CalculateGlobalTransform( entity );

            bool visible = entity->visible;            
            if( entity->parent ) {
                visible &= entity->parent->visible;
            }
            
            if( entity->alpha < 0.99f ) {
                glEnable( GL_BLEND );
                glBlendFunc( GL_SRC_ALPHA, GL_ONE );
            }  
            

            if( visible ) {
                if( entity->componentBillboard ) {      
                    TBillboard * billboard = entity->componentBillboard;
                    
                    float halfWidth = billboard->width * 0.5f * entity->localScale.x;
                    float halfHeight = billboard->height * 0.5f * entity->localScale.y;
                    
                    TVec3 right = { pActiveCamera->viewMatrix.f[0] * halfWidth, pActiveCamera->viewMatrix.f[4] * halfWidth, pActiveCamera->viewMatrix.f[8] * halfWidth };
                    TVec3 up    = { pActiveCamera->viewMatrix.f[1] * halfHeight, pActiveCamera->viewMatrix.f[5] * halfHeight, pActiveCamera->viewMatrix.f[9] * halfHeight };
                    TVec3 pos   = billboard->owner->globalPosition;
                    
                    Renderer_BindTexture( billboard->texture, 0 );
                    Renderer_SetModelViewTransform( pActiveCamera->viewMatrix );
                    
                    glBegin( GL_QUADS );
                    
                    glColor4f( 1.0f, 1.0f, 1.0f, entity->alpha );
                    
                    glMultiTexCoord2f( GL_TEXTURE0_ARB, 0.0f, 0.0f );
                    glVertex3f( pos.x - right.x - up.x, pos.y - right.y - up.y, pos.z - right.z - up.z );
                    
                    glMultiTexCoord2f( GL_TEXTURE0_ARB, 1.0f, 0.0f );
                    glVertex3f( pos.x + right.x - up.x, pos.y + right.y - up.y, pos.z + right.z - up.z );                    
 
                    glMultiTexCoord2f( GL_TEXTURE0_ARB, 1.0f, 1.0f );
                    glVertex3f( pos.x + right.x + up.x, pos.y + right.y + up.y, pos.z + right.z + up.z ); 

                    glMultiTexCoord2f( GL_TEXTURE0_ARB, 0.0f, 1.0f );
                    glVertex3f( pos.x - right.x + up.x, pos.y - right.y + up.y, pos.z - right.z + up.z ); 
                   
                    glEnd();
                } else if ( entity->componentCamera ) {
                    // there is no special code to draw this
                } else if ( entity->componentLight ) {
                    // there is no special code to draw this
                } else {
                    TMatrix4 modelView;
                    if( entity->skinned ) {
                        modelView = pActiveCamera->viewMatrix;
                    } else {
                        modelView = Matrix4_Multiply( entity->globalTransform, pActiveCamera->viewMatrix );
                    };

                    Renderer_SetModelViewTransform( modelView );
                    
                    bool depthHack = fabsf( entity->depthHack ) > 0.0f ;
                    if( depthHack ) {
                        Camera_EnterDepthHack( pActiveCamera, entity->depthHack );
                        Renderer_SetProjectionTransform( pActiveCamera->projectionMatrix );
                    }
                    for_each( TSurface, surface, entity->surfaces ) {
                        Renderer_RenderSurface( surface, &entity->color );
                    }  
                    if( depthHack ) {
                        Camera_LeaveDepthHack( pActiveCamera );
                        Renderer_SetProjectionTransform( pActiveCamera->projectionMatrix );
                    }
                }                
            }
            if( entity->alpha < 0.99f ) {
                glDisable( GL_BLEND );
            }
        }
    }
}

void Renderer_Enable2DMode( bool state ) {
    if( state ) {
        glDisable( GL_DEPTH_TEST );
        glDisable( GL_CULL_FACE );
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        Renderer_BindShader( &gRenderer->guiVertexShader );
        Renderer_BindShader( &gRenderer->guiFragmentShader );
        Renderer_SetModelViewTransform( Matrix4_Identity() );    
        Renderer_SetProjectionTransform( Matrix4_Ortho2D( 0, Renderer_GetWindowWidth(), Renderer_GetWindowHeight(), 0.0f, 0.0, 1.0f ));
    } else {
        Renderer_BindShader( &gRenderer->vertexShader );
        Renderer_BindShader( &gRenderer->fragmentShader );
        glClearDepth( 1.0f );
        glEnable( GL_DEPTH_TEST );
        glDisable( GL_BLEND );
        glDepthFunc( GL_LEQUAL );
        glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
        glEnable( GL_TEXTURE_2D );
        glShadeModel( GL_SMOOTH );
        glEnable( GL_CULL_FACE );
        glDisable( GL_STENCIL_TEST );
        glCullFace( GL_BACK );    
        glEnable( GL_ALPHA_TEST );
        glAlphaFunc( GL_GREATER, 0.025f );
    }
}

void Renderer_SetModelViewTransform( TMatrix4 modelView ) {
    Debug_CheckGLError( glMatrixMode( GL_MODELVIEW ));
    Debug_CheckGLError( glLoadMatrixf( modelView.f ));
}

void Renderer_SetProjectionTransform( TMatrix4 proj ) {
    Debug_CheckGLError( glMatrixMode( GL_PROJECTION ));
    Debug_CheckGLError( glLoadMatrixf( proj.f ));
}

int Renderer_GetScreenResolutionWidth( ) {
#ifdef _WIN32
    return GetSystemMetrics( SM_CXSCREEN );
#endif
}

int Renderer_GetScreenResolutionHeight( ) {
#ifdef _WIN32
    return GetSystemMetrics( SM_CYSCREEN );
#endif
}

#ifdef _WIN32
LRESULT CALLBACK Renderer_WindowProc( HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
    switch ( msg ) {
        case WM_DESTROY: {
            PostQuitMessage ( 0 );
            gRenderer->running = false;
            break;
        };
        case WM_CLOSE: {
            if (MessageBox ( 0, "Are you sure you want to quit?", "Confirm Exit", MB_YESNO | MB_SETFOREGROUND | MB_ICONQUESTION) == IDYES) {
                gRenderer->running = false;
            }
            return 1;
            break;
        };
    }    
    Input_HandleMessage( wnd, msg, lParam, wParam );        
    return DefWindowProc ( wnd, msg, wParam, lParam );
}
#endif

void Renderer_SetResolution( int width, int height, int bpp ) {    
    gRenderer->settings.width = width;
    gRenderer->settings.height = height;
    gRenderer->settings.bpp = bpp;
    
    if( gRenderer->settings.fullscreen ) {
        DEVMODE screenSettings;
        memset( &screenSettings, 0, sizeof( screenSettings ));
        screenSettings.dmSize = sizeof( screenSettings );
        screenSettings.dmBitsPerPel = gRenderer->settings.bpp;
        screenSettings.dmPelsHeight = gRenderer->settings.height;
        screenSettings.dmPelsWidth = gRenderer->settings.width;
        screenSettings.dmFields = DM_BITSPERPEL | DM_PELSHEIGHT | DM_PELSWIDTH;
        if( ChangeDisplaySettings( &screenSettings, CDS_FULLSCREEN ) != DISP_CHANGE_SUCCESSFUL ) {
            Util_RaiseError( "Unable to set %dx%dx%d fullscreen mode!", gRenderer->settings.width, gRenderer->settings.height, gRenderer->settings.bpp );
        } else {
            glViewport( 0, 0, width, height );
        }
    } 
}

void Renderer_CreateWindow( const TRenderSettings * settings ) {
#ifdef _WIN32
    Log_Write( "CreateRenderWindow: %dx%dx%dx%d", settings->width, settings->height, settings->bpp, settings->fullscreen );
    gRenderer->settings = *settings;
    const char * className = "OldTech Render Window";
    WNDCLASSEX wcx = { 0 };
    wcx.cbSize = sizeof ( wcx );
    wcx.hCursor = LoadCursor ( NULL, IDC_ARROW );
    wcx.hbrBackground = 0;
    wcx.hInstance = GetModuleHandle ( 0 );
    wcx.lpfnWndProc = Renderer_WindowProc;
    wcx.lpszClassName = className;
    wcx.style = CS_HREDRAW | CS_VREDRAW;
    RegisterClassEx ( &wcx );
    DWORD style = WS_POPUP;
    if ( !gRenderer->settings.fullscreen ) {
        style = WS_SYSMENU | WS_BORDER | WS_CAPTION | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
    }
    RECT wRect;
    wRect.left = 0;
    wRect.top = 0;
    wRect.right = gRenderer->settings.width;
    wRect.bottom = gRenderer->settings.height;
    AdjustWindowRect ( &wRect, style, 0 );
    gRenderer->handle = CreateWindow( className, className, style, 0, 0, wRect.right - wRect.left, wRect.bottom - wRect.top, 0, 0, wcx.hInstance, 0 );
    ShowWindow ( gRenderer->handle, SW_SHOW );
    UpdateWindow ( gRenderer->handle );
    SetActiveWindow ( gRenderer->handle );
    SetForegroundWindow ( gRenderer->handle );
    gRenderer->deviceContext = GetDC( gRenderer->handle );
    if( gRenderer->settings.fullscreen ) {
        DEVMODE screenSettings;
        memset( &screenSettings, 0, sizeof( screenSettings ));
        screenSettings.dmSize = sizeof( screenSettings );
        screenSettings.dmBitsPerPel = gRenderer->settings.bpp;
        screenSettings.dmPelsHeight = gRenderer->settings.height;
        screenSettings.dmPelsWidth = gRenderer->settings.width;
        screenSettings.dmFields = DM_BITSPERPEL | DM_PELSHEIGHT | DM_PELSWIDTH;
        if( ChangeDisplaySettings( &screenSettings, CDS_FULLSCREEN ) != DISP_CHANGE_SUCCESSFUL ) {
            Util_RaiseError( "Unable to set %dx%dx%d fullscreen mode!", gRenderer->settings.width, gRenderer->settings.height, gRenderer->settings.bpp );
        }
    }
    
    RAWINPUTDEVICE rawMouse;
    rawMouse.usUsagePage = 0x01; 
    rawMouse.usUsage = 0x02; 
    rawMouse.dwFlags = 0;
    rawMouse.hwndTarget = 0;
    if( RegisterRawInputDevices( &rawMouse, 1, sizeof( RAWINPUTDEVICE )) == FALSE ) {
        Util_RaiseError( "Raw input failed!" );
    }
#endif
    Log_Write( "CreateRenderWindow: success" );
}

void Renderer_FreeWindow(  ) {
#ifdef _WIN32
    CloseWindow( gRenderer->handle );
    DestroyWindow( gRenderer->handle );
#endif
    Log_Write( "FreeRenderWindow: success" );
}

void Renderer_PollWindowMessages( ) {
#ifdef _WIN32
    MSG message;
    while ( PeekMessage ( &message, NULL, 0, 0, PM_REMOVE ) ) {
        TranslateMessage( &message );
        DispatchMessage ( &message );
    }
#endif
}

void Renderer_SetTextureFiltration( ETextureFilter texFilter ) {
    gRenderer->texFilter = texFilter;
}

void Renderer_BindTexture( TTexture * tex, int level ) {
    Debug_CheckGLError( glActiveTextureARB( GL_TEXTURE0_ARB + level ));   
    if( tex ) {
        if( gRenderer->texFilter == TF_ANISOTROPIC ) {
            Debug_CheckGLError( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, gRenderer->maxAnisotropy ));
        } else {
            Debug_CheckGLError( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1 ));
        }
        Debug_CheckGLError( glEnable( GL_TEXTURE_2D ));
        Debug_CheckGLError( glBindTexture( GL_TEXTURE_2D, tex->glTexture ));        
        Debug_CheckGLError( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ));
        // mipmap management
        if( tex->mips ) {
            Debug_CheckGLError( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR ));
        } else {
            Debug_CheckGLError( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ));
        }
    } else {
        Debug_CheckGLError( glDisable( GL_TEXTURE_2D ));
        Debug_CheckGLError( glBindTexture( GL_TEXTURE_2D, 0 ));
    }
}

void Renderer_InitializeOpenGL( ) {    
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR), 1, PFD_SWAP_COPY | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 8, 0, PFD_MAIN_PLANE, 0, 0, 0, 0
    };
    int pixelFormat = ChoosePixelFormat( gRenderer->deviceContext, &pfd );
    if( !SetPixelFormat( gRenderer->deviceContext, pixelFormat, &pfd )) {
        Util_RaiseError( "Unable to SetPixelFormat. Initialization failed!" );
    }    
    gRenderer->deviceContext = GetDC( gRenderer->handle );        
    gRenderer->glContext = wglCreateContext( gRenderer->deviceContext );
    if( !gRenderer->glContext ) {
        Util_RaiseError( "Unable to create OpenGL render. Initialization failed!" );
    }
    if( !wglMakeCurrent( gRenderer->deviceContext, gRenderer->glContext )) {
        Util_RaiseError( "Unable to wglMakeCurrent. Initialization failed!" );
    }  

    const unsigned char * version = glGetString( GL_VERSION );
    
    int minor = version[0];
    int major = version[2];
    
    if( minor == 1 && major < 4 ){
        Util_RaiseError( "Your video card doesn't support OpenGL 1.4! Initialization failed!" );
    }
    
    Log_Write( "Create OpenGL context.\nOpenGL Version: %s\nOpenGL Vendor: %s\nOpenGL Renderer: %s",
               version, glGetString( GL_VENDOR ), glGetString( GL_RENDERER ) );
    
    glClearDepth( 1.0f );
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
    glEnable( GL_TEXTURE_2D );
    glShadeModel( GL_SMOOTH );
    glEnable( GL_CULL_FACE );
    glDisable( GL_STENCIL_TEST );
    glCullFace( GL_BACK );    
    glEnable( GL_ALPHA_TEST );
    glAlphaFunc( GL_GREATER, 0.025f );
    
    const char * exts = (const char *)glGetString(GL_EXTENSIONS);
    int size = strlen( exts ) + 1;
    char * buffer = Memory_AllocateClean( size );
    memcpy( buffer, exts, size );
    strtok( buffer, " " );
    while( true ) {
        char * ext = strtok( NULL, " " );
        if( ext ) {
            Log_Write( ext );
        } else {
            break;
        }
    }
    Memory_Free( buffer );

    // Multitexturing
    if( !strstr( exts, "GL_ARB_multitexture" )) {
        Util_RaiseError( "GL_ARB_multitexture not supported! Initialization failed!" );
    } else {
        glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress ( "glActiveTextureARB" );
        glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC)wglGetProcAddress ( "glClientActiveTextureARB" );
        glMultiTexCoord1f = (PFNGLMULTITEXCOORD1FARBPROC)wglGetProcAddress ( "glMultiTexCoord1fARB" );
        glMultiTexCoord1fv = (PFNGLMULTITEXCOORD1FVARBPROC)wglGetProcAddress ( "glMultiTexCoord1fvARB" );
        glMultiTexCoord2f = (PFNGLMULTITEXCOORD2FARBPROC)wglGetProcAddress ( "glMultiTexCoord2fARB" );
        glMultiTexCoord2fv = (PFNGLMULTITEXCOORD2FVARBPROC)wglGetProcAddress ( "glMultiTexCoord2fvARB" );
    }   
    
    if( !strstr( exts, "GL_ARB_fragment_program" ) || !strstr( exts, "GL_ARB_vertex_program")) {
        Util_RaiseError( "GL_ARB_fragment_program and GL_ARB_vertex_program are not supported! Initialization failed!" );
    } else {
        glGetProgramivARB = (PFNGLGETPROGRAMIVARBPROC)wglGetProcAddress( "glGetProgramivARB" );
        glProgramStringARB = (PFNGLPROGRAMSTRINGARBPROC)wglGetProcAddress( "glProgramStringARB" );
        glBindProgramARB = (PFNGLBINDPROGRAMARBPROC)wglGetProcAddress( "glBindProgramARB" );
        glDeleteProgramsARB = (PFNGLDELETEPROGRAMSARBPROC)wglGetProcAddress( "glDeleteProgramsARB" );
        glGenProgramsARB = (PFNGLGENPROGRAMSARBPROC)wglGetProcAddress( "glGenProgramsARB" );
        glProgramEnvParameter4fARB = (PFNGLPROGRAMENVPARAMETER4FARBPROC)wglGetProcAddress( "glProgramEnvParameter4fARB" );
        glProgramEnvParameter4fvARB = (PFNGLPROGRAMENVPARAMETER4FVARBPROC)wglGetProcAddress( "glProgramEnvParameter4fvARB" );
        glProgramLocalParameter4fARB = (PFNGLPROGRAMLOCALPARAMETER4FARBPROC)wglGetProcAddress( "glProgramLocalParameter4fARB" );
        glProgramLocalParameter4fvARB = (PFNGLPROGRAMLOCALPARAMETER4FVARBPROC)wglGetProcAddress( "glProgramLocalParameter4fvARB" );
        glGetProgramEnvParameterfvARB = (PFNGLGETPROGRAMENVPARAMETERFVARBPROC)wglGetProcAddress( "glGetProgramEnvParameterfvARB" );
        glGetProgramLocalParameterfvARB = (PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC)wglGetProcAddress( "glGetProgramLocalParameterfvARB" );
        glGetProgramStringARB = (PFNGLGETPROGRAMSTRINGARBPROC)wglGetProcAddress( "glGetProgramStringARB" );
        glIsProgramARB = (PFNGLISPROGRAMARBPROC)wglGetProcAddress( "glIsProgramARB" );
    }
    
    if( !strstr( exts, "GL_ARB_texture_compression" )) {
        Util_RaiseError( "GL_ARB_texture_compression not supported! Initialization failed!" );
    }
    if( !strstr( exts, "GL_EXT_texture_compression_s3tc" )) {
        Util_RaiseError( "GL_EXT_texture_compression_s3tc not supported! Initialization failed!" );
    }
    
    if( strstr( exts, "GL_EXT_texture_filter_anisotropic" )) {
        gRenderer->anisotropicSupport = true;
        glGetIntegerv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &gRenderer->maxAnisotropy );
        Log_Write( "Anisotropic texture filtering supported! Max anisotropy: %d", gRenderer->maxAnisotropy );
        gRenderer->texFilter = TF_ANISOTROPIC;
    } else {
        gRenderer->anisotropicSupport = false;
        gRenderer->maxAnisotropy = 0;
        gRenderer->texFilter = TF_LINEAR;
        Log_Write( "Anisotropic texture filtering is not supported!" );
    }
   
    if( !strstr( exts, "GL_ARB_vertex_buffer_object" )) {
        Util_RaiseError( "GL_ARB_vertex_buffer_object not supported! Initialization failed!" );
    } else {
        glGenBuffersARB = (PFNGLGENBUFFERSARBPROC)wglGetProcAddress( "glGenBuffersARB" );
        glBindBufferARB = (PFNGLBINDBUFFERARBPROC)wglGetProcAddress( "glBindBufferARB" );
        glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC)wglGetProcAddress( "glDeleteBuffersARB" ); 
        glBufferDataARB = (PFNGLBUFFERDATAARBPROC)wglGetProcAddress( "glBufferDataARB" );
        glBufferSubDataARB = (PFNGLBUFFERSUBDATAARBPROC)wglGetProcAddress( "glBufferSubDataARB" );
    }
    gRenderer->aspect = (float)gRenderer->settings.width / (float)gRenderer->settings.height;

    Log_Write( "InitializeOpenGL: success" );
    Log_Write( "OpenGL version: %s", glGetString( GL_VERSION ) );
    
    
    Shader_LoadFromFile( &gRenderer->vertexShader, "data/shaders/vertex.shr" );
    Shader_LoadFromFile( &gRenderer->fragmentShader, "data/shaders/fragment.shr" );
    
    Shader_LoadFromFile( &gRenderer->guiVertexShader, "data/shaders/gui_vertex.shr" );
    Shader_LoadFromFile( &gRenderer->guiFragmentShader, "data/shaders/gui_fragment.shr" );
  
    unsigned char whitePixel[4] = { 255, 255, 255, 255 };
    Debug_CheckGLError( glGenTextures( 1, &gRenderer->whitePixelTexture ));
    Debug_CheckGLError( glBindTexture( GL_TEXTURE_2D, gRenderer->whitePixelTexture ));
    Debug_CheckGLError( glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, whitePixel ));
    Debug_CheckGLError( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ));
    Debug_CheckGLError( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ));
    Debug_CheckGLError( glBindTexture( GL_TEXTURE_2D, 0 ));
    
    int maxAttribs;
    int maxLocalParameters;
    int maxEnvParameters;
    int maxMatrices;
    int maxTemporaries;
    int maxParams;
    int maxAddressRegisters;
    
    Debug_CheckGLError( glGetIntegerv( GL_MAX_PROGRAM_MATRICES_ARB, &maxMatrices ));
    
    // vertex program
    Debug_CheckGLError( glGetProgramivARB( GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_ATTRIBS_ARB, &maxAttribs ));
    Debug_CheckGLError( glGetProgramivARB( GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB, &maxLocalParameters ));
    Debug_CheckGLError( glGetProgramivARB( GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_ENV_PARAMETERS_ARB, &maxEnvParameters ));    
    Debug_CheckGLError( glGetProgramivARB( GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_TEMPORARIES_ARB, &maxTemporaries ));
    Debug_CheckGLError( glGetProgramivARB( GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_PARAMETERS_ARB, &maxParams ));
    Debug_CheckGLError( glGetProgramivARB( GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB, &maxAddressRegisters ));
    
    Log_Write( "Vertex program limits:            " );
    Log_Write( "    attributes:                 %d", maxAttribs );
    Log_Write( "    local parameters:           %d", maxLocalParameters );
    Log_Write( "    env parameters:             %d", maxEnvParameters );
    Log_Write( "    program matrices:           %d", maxMatrices );
    Log_Write( "    program temporaries:        %d", maxTemporaries );
    Log_Write( "    program parameters:         %d", maxParams );
    Log_Write( "    program address registers:  %d", maxAddressRegisters );
    
    // fragment program
    Debug_CheckGLError( glGetProgramivARB( GL_FRAGMENT_PROGRAM_ARB, GL_MAX_PROGRAM_ATTRIBS_ARB, &maxAttribs ));
    Debug_CheckGLError( glGetProgramivARB( GL_FRAGMENT_PROGRAM_ARB, GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB, &maxLocalParameters ));
    Debug_CheckGLError( glGetProgramivARB( GL_FRAGMENT_PROGRAM_ARB, GL_MAX_PROGRAM_ENV_PARAMETERS_ARB, &maxEnvParameters ));
    Debug_CheckGLError( glGetProgramivARB( GL_FRAGMENT_PROGRAM_ARB, GL_MAX_PROGRAM_TEMPORARIES_ARB, &maxTemporaries ));
    Debug_CheckGLError( glGetProgramivARB( GL_FRAGMENT_PROGRAM_ARB, GL_MAX_PROGRAM_PARAMETERS_ARB, &maxParams ));

    Log_Write( "Fragment program limits:            " );
    Log_Write( "    attributes:                 %d", maxAttribs );
    Log_Write( "    local parameters:           %d", maxLocalParameters );
    Log_Write( "    env parameters:             %d", maxEnvParameters );
    Log_Write( "    program matrices:           %d", maxMatrices );
    Log_Write( "    program temporaries:        %d", maxTemporaries );
    Log_Write( "    program parameters:         %d", maxParams );
}


void Util_CheckGLErrorFunc( const char * file, int line,  const char * func  ) {
    GLenum g = glGetError();
    switch (g) {
    case GL_INVALID_ENUM:
        Util_RaiseError( "OpenGL Error: GL_INVALID_ENUM in '%s' in line %d in function %s", file, line, func );
    case GL_INVALID_VALUE:
        Util_RaiseError( "OpenGL Error: GL_INVALID_VALUE in '%s' in line %d in function %s", file, line, func );
    case GL_INVALID_OPERATION:
        Util_RaiseError( "OpenGL Error: GL_INVALID_OPERATION in '%s' in line %d in function %s", file, line, func );
    case GL_STACK_OVERFLOW:
        Util_RaiseError( "OpenGL Error: GL_STACK_OVERFLOW in '%s' in line %d in function %s", file, line, func );
    case GL_STACK_UNDERFLOW:
        Util_RaiseError( "OpenGL Error: GL_STACK_UNDERFLOW in '%s' in line %d in function %s", file, line, func );
    case GL_OUT_OF_MEMORY:
        Util_RaiseError( "OpenGL Error: GL_OUT_OF_MEMORY in '%s' in line %d in function %s", file, line, func );
    };
}

void Renderer_FreeOpenGL() {
    wglMakeCurrent( 0, 0 );
    wglDeleteContext( gRenderer->glContext );
    Log_Write( "FreeOpenGL: success" );
}

void Renderer_BeginRender() {
    if( pActiveCamera ) {
        Debug_CheckGLError( glClearColor( pActiveCamera->clearColor.x, pActiveCamera->clearColor.y, pActiveCamera->clearColor.z, 1.0f ) );

        Camera_BuildMatrices( pActiveCamera );

        // load projection matrix 
        Renderer_SetProjectionTransform( pActiveCamera->projectionMatrix );
    } 
    
    Debug_CheckGLError( glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT ) );

}

int Renderer_GetWindowWidth( void ) {
    return gRenderer->settings.width;
}

int Renderer_GetWindowHeight( void ) {
    return gRenderer->settings.height;
}

void Renderer_SetShaderLocal( TShader * shader, int index, float x, float y, float z, float w ) {
    glProgramLocalParameter4fARB( shader->target, index, x, y, z, w );
}

void Renderer_BindShader( TShader * shader ) {
    if( shader ) {
        glBindProgramARB( shader->target, shader->id );        
    } else {
        Util_RaiseError( "Unable to set shader!" );
    }
}

void Renderer_EndRender() {
    if( gRenderer->running ) {
        glFlush();
#ifdef _WIN32
        if( !SwapBuffers( gRenderer->deviceContext )) {
            Util_RaiseError( "SwapBuffers failed!" );
        }
#endif
    }
    
}