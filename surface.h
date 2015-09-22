#ifndef _SURFACE_
#define _SURFACE_

#include "common.h"
#include "vertex.h"
#include "face.h"
#include "texture.h"
#include "matrix4.h"

typedef struct {
    TVec3 min;
    TVec3 max;
    TVec3 center;
    float radius;
} TAABB;

typedef struct {
    // skeletal anim bone weight 
    float weight;
    // bone number in a scene 
    int boneId;
    // bone entity ptr 
    struct TEntity * boneEnt;
    // transformation of this bone
    TMatrix4 transform;
} TBone;

// allow you to define four bones per vertex, this is usually enough 
typedef struct {
    TBone bones[4];
    int boneCount;
} TBoneGroup;

#define SURF_MAXLIGHTMAPS (32)

// basic type used to represent geometry in this engine. surfaces are sorted by texture
typedef struct TSurface {
    // =====================
    // generic geometry info
    TVertex * vertices;
    TFace * faces;
    int faceCount;
    int vertexCount;
        
    // =====================
    // underlying members are used for animation    
    TVertex * skinVertices; // array to store transformed vertices    
    TBoneGroup * vertexBones;
    bool skinned;
    
    // =====================
    // used for instancing
    int shareCount;    
    
    // =====================
    // list of bones affecting this surface
    TList bones; 
     
    // =====================
    // underlying members are used to manage lightmaps
    struct TEntity * onLoadOwner;  // entity, that owns this surface in the scene file    
    unsigned int sourceCRC32; // crc32 of scene file
    int lightmapCount;
    bool lightmapped;
    struct TLightmapAtlas * lightmaps[ SURF_MAXLIGHTMAPS ];
    
    // =====================
    // identifiers to opengl's buffers
    unsigned int vertexBuffer;
    unsigned int indexBuffers[ SURF_MAXLIGHTMAPS ];
    unsigned int lightmapFaceCount[ SURF_MAXLIGHTMAPS ]; // face count per lightmap
    
    // =====================
    // some info, that is needed for rendering
    bool facesSorted;
    bool buffersReady;
    TAABB aabb;
    
    // =====================
    // members for the materials
    TTexture * texture;    
    char diffuseTexName[64];
    char normalTexName[64];
    char sourceName[64]; // scene file name from which this surface was loaded
    float opacity;
} TSurface;

void Surface_Free( TSurface * surf );
void Surface_SortFaces( TSurface * surf );

#endif