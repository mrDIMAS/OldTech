#ifndef _LIGHT_
#define _LIGHT_

#include "vector3.h"
#include "list.h"

typedef enum {
    LT_POINT,
    LT_SPOT,
} ELightType;

typedef struct SLight {
    struct TEntity * owner;
    TVec3 color;
    int type;
    bool enabled;
    float radius;
    float brightness;
    float innerAngle;
    float outerAngle;
    TList affectedAtlasList; // list of lightmap atlases
} TLight;

void Light_CreatePoint( TLight * light, struct TEntity * owner, TVec3 * color, float radius );
void Light_SetEnabled( TLight * light, bool state ); // use this to enable\disable light, do not use 'enabled' field of TLight

extern TList g_lights;
extern TVec3 gAmbientLight;
#endif