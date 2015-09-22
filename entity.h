#ifndef _ENTITY_
#define _ENTITY_

#include "common.h"
#include "vector3.h"
#include "quaternion.h"
#include "matrix4.h"
#include "surface.h"
#include "list.h"
#include "renderer.h"
#include "light.h"
#include "animation.h"
#include "collision.h"
#include "ValueArray.h"
#include "Parser.h"
#include "billboard.h"

OLDTECH_BEGIN_HEADER

struct TBillboard;

typedef enum {
    EFX_BLEND_ADD = 1,
    EFX_BLEND_MULTIPLY = 2,
} EEntityFX;

typedef struct SKeyFrame {
    TVec3 pos;
    TQuaternion rot;
} TKeyFrame;

typedef struct TEntity {
    TVec3 localPosition; // read\write
	TVec3 localScale; // read\write
    TVec3 globalPosition; // read only
    TQuaternion localRotation; // read\write
    TMatrix4 globalTransform; // read only
    TMatrix4 localTransform; // read\write
    TMatrix4 invBindTransform; // read only
    struct TEntity * parent;
    TList surfaces;
    TList childs;
    bool skinned;
    char * name;
    TList keyFrames;
    int totalFrames;
    TAnimation * anim;
    bool visible;
    bool animated;
    bool animationEnabled;
	bool globalTransformCalculated;
	float depthHack;
    TValueArray properties;
    int fxFlags;
    unsigned int sourceCRC32;

    TList allSurfaces;
    float alpha;
    TVec3 color;
    TBody * dynBody;
    // components
    struct TCamera * componentCamera;
    TLight * componentLight;    
    struct TBillboard * componentBillboard;
    struct TEntity *instanceOf;
} TEntity;

extern TList g_entities;

TEntity * Entity_Create( void );
TEntity * Entity_CreateInstance( TEntity * source );
TVec3 Entity_GetLookVector( TEntity * ent );
TVec3 Entity_GetRightVector( TEntity * ent );
TVec3 Entity_GetUpVector( TEntity * ent );
TVec3 Entity_GetGlobalPosition( TEntity * ent );
void Entity_Free( TEntity * ent );
struct TCamera * Entity_MakeCamera( TEntity * ent );
struct TBillboard * Entity_MakeBillboard( TEntity * ent );
void Entity_CalculateGlobalTransform( TEntity * ent );
void Entity_Attach( TEntity * ent, TEntity * parent );
void Entity_AddSurface( TEntity * ent, TSurface * surf );
void Entity_AddChild( TEntity * ent, TEntity * child );
void Entity_SetColor( TEntity * ent, const TVec3 * color, bool affectChilds );
TEntity * Entity_GetChild( TEntity * ent, int childNum );
void Entity_FreeAll( void );
void Entity_SetBody( TEntity * ent, TBody * body );
void Entity_SetLocalPosition( TEntity * ent, const TVec3 * pos );
TEntity * Entity_LoadFromFile( const char * fileName );
TEntity * Entity_GetChildByName( TEntity * parent, const char * name );
void Entity_GetGlobalRotation( TEntity * ent, TQuaternion * globRot );
void Entity_SetDepthHack( TEntity * ent, float depthHack );
void Entity_ApplyProperties( TEntity * ent );
void Entity_SetAnimationEnabled( TEntity * ent, bool state );
void Entity_SetAnimation( TEntity * ent, TAnimation * anim );
void Entity_Animate( TEntity * ent );

// also performs animation of each entity
void World_Update( void );

OLDTECH_END_HEADER

#endif