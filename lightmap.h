#ifndef _LIGHTMAP_
#define _LIGHTMAP_

#include "Light.h"
#include "Vertex.h"
#include "Texture.h"
#include "lightprobe.h"


typedef enum {
    PLANE_ARBITRARY = 0,
    PLANE_OXY = 1,
    PLANE_OXZ = 2,
    PLANE_OYZ = 3,
} EPlaneClass;

typedef struct {
    int x;
    int y;
    int w;
    int h;
} TRect;

typedef struct SPackerNode {
    struct SPackerNode * parent;
    struct SPackerNode * childs[2];
    TRect rect;
    int faceIndex;
    bool split;
    struct TLightmap * lm;    
} TPackerNode;

typedef struct {
    float v;
    float w;
    float u;
} TBarycentricCoord;

typedef struct SLightmapLayer {
    TRGBAPixel * pixels;
    TLight * light;
} TLightmapLayer;
 
typedef struct TLightmap {
    TVertex * a;
    TVertex * b;
    TVertex * c;
    int width;
    int height;
    TList layers;
    int faceID;
} TLightmap;

typedef struct TLightmapAtlas {
    TPackerNode root; // main atlas hierarchy
    TList nodes;
    TTexture texture;
    struct TSurface * surface;
    bool modified;
} TLightmapAtlas;

// contains data for fast calculation of barycentric coordinates using UV's of the triangle
typedef struct TUVTriangle {
    TVector2 v0;
    TVector2 v1;
    float d00;
    float d01;
    float d11;
    float denom;
} TUVTriangle;

typedef struct TMTGenInfo {
    TLightmap * lm;
    TVertex * a;
    TVertex * b;
    TVertex * c;
    int faceNum;
    TVec3 * offset;
} TMTGenInfo;

extern TList gLightmapAtlasList;

void Barycentric_Calculate2D( const TVector2 * p, const TVector2 * a, const TVector2 * b, const TVector2 * c, TBarycentricCoord * out );
void Barycentric_Calculate3D( const TVec3 * p, const TVec3 * a, const TVec3 * b, const TVec3 * c, TBarycentricCoord * out );
void Barycentric_MapToWorld( TVec3 * out, const TVec3 * a, const TVec3 * b, const TVec3 * c, const TBarycentricCoord * bary );
void Triangle_CalculateNormalUnnormalized( TVec3 * normal, const TVec3 * a, const TVec3 * b, const TVec3 * c );
void ProjectPointOntoPlane( TVec3 * projected, const TVec3 * point, const TVec3 * planePoint, const TVec3 * planeNormal );
unsigned long UpperPow2( unsigned long v );
int GetPlaneWithLongestNormal( const TVec3 * triangleNormal );

int ProjectPointOntoOrthoPlane( int plane, TVec3 * projected, const TVec3 * point );
void Lightmap_Map3DPointTo2DByPlane( int plane, const TVec3 * point, TVector2 * mapped );   
void Lightmap_Blur( TLightmap * lm, const int borderSize );
float Lightmap_CalculateAttenuation( const TVec3 * src, const TVec3 * dst, float radius, TVec3 * outDir );

// threadNum >= 0 - use MT version, other - use singlethreaded
void Lightmap_Build( const TVec3 * offset, TLightmap * lm, TVertex * a, TVertex * b, TVertex * c, int faceID, int threadNum );

// basic function to generate lightmap for surface
void Lightmap_BuildForSurface( struct TSurface * surf,  const TVec3 * offset );
void Lightmap_BuildForSurfaceMultithreaded( struct TSurface * surf, TVec3 * offset, int surfNum, int totalSurfaces );

void LightmapAtlas_SaveSurfaceAtlases( struct TSurface * surf, const char * path );
bool LightmapAtlas_LoadSurfaceAtlases( struct TEntity * root, struct TSurface * surface, const char * path );

void LightmapAtlas_Update( TLightmapAtlas * atlas, TLight * light );
void LightmapAtlas_Free( TLightmapAtlas * atlas );



#endif