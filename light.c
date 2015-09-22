#include "light.h"
#include "lightmap.h"

TList g_lights = { NULL, NULL, 0 };
TVec3 gAmbientLight = { 0.075f, 0.075f, 0.075f };

void Light_CreatePoint( TLight * light, struct TEntity * owner, TVec3 * color, float radius ) {
    light->owner = owner;
    light->type = LT_POINT;
    light->brightness = 1.0f;
    light->radius = radius;
    light->innerAngle = 0.0f;
    light->outerAngle = 1.0f;
    light->color = *color;
    List_Create( &light->affectedAtlasList );
    List_Add( &g_lights, light );
}

void Light_SetEnabled( TLight * light, bool state ) {
    if( light->enabled != state ) {
        light->enabled = state;
        for_each( TLightmapAtlas, atlas, light->affectedAtlasList ) {      
            LightmapAtlas_Update( atlas, light );
        }
    }
}