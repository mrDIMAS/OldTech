#include "surface.h"
#include "vector3.h"
#include "face.h"
#include "renderer.h"

int FaceCmpFunc( const void * a, const void * b );

void Surface_Free( TSurface * surf ) {
    Renderer_DeleteSurfaceBuffers( surf );
    Memory_Free( surf->faces );
    Memory_Free( surf->vertices );
    if( surf->skinned ) {
        Memory_Free( surf->skinVertices );
        Memory_Free( surf->vertexBones );
    }
    Memory_Free( surf );
}

int FaceCmpFunc( const void * a, const void * b ) {
    TFace * faceA = (TFace*)a;
    TFace * faceB = (TFace*)b;
    return faceA->lightmapIndex - faceB->lightmapIndex;
}

void Surface_SortFaces( TSurface * surf ) {    
    qsort( surf->faces, surf->faceCount, sizeof( TFace ), FaceCmpFunc );
    surf->facesSorted = true;
}