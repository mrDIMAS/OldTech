#ifndef _LIGHTPROBE_
#define _LIGHTPROBE_

#include "Light.h"
#include "Vertex.h"

typedef struct TLightProbe {
    TVec3 position;
    TVec3 color;
} TLightProbe;

extern TList gLightProbeList;
    
void LightProbe_BuildRegularArray( struct TEntity * volumeEntity, float density );
void LightProbe_Calculate( void );
void LightProbe_Clear( void );
void Lightprobe_LightEntity( struct TEntity * entity );
TLightProbe * LightProbe_Trace( const TVec3 * point );

#endif