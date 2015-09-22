#include "lightprobe.h"
#include "entity.h"
#include "lightmap.h"
#include <float.h>

void LightProbe_BuildRegularArray( TEntity * volumeEntity, float density ) {
    TVec3 min = { FLT_MAX, FLT_MAX, FLT_MAX };
    TVec3 max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
    for_each( TEntity, child, volumeEntity->childs ) {
        if( child->surfaces.size ) {
            for_each( TSurface, surface, child->surfaces ) {
                for( int i = 0; i < surface->vertexCount; i++ ) {
                    TVec3 * vp = &surface->vertices[i].p;
                    if( vp->x < min.x ) min.x = vp->x;                
                    if( vp->y < min.y ) min.y = vp->y;                
                    if( vp->z < min.z ) min.z = vp->z;
                    if( vp->x > max.x ) max.x = vp->x;                
                    if( vp->y > max.y ) max.y = vp->y;
                    if( vp->z > max.z ) max.z = vp->z;
                }
            }
        }
    }
    TVec3 bounds = Vec3_Sub( max, min );
    int xSteps = bounds.x / density;
    int ySteps = bounds.y / density;
    int zSteps = bounds.z / density;

    for( int stack = 0; stack < ySteps; stack++ ) {
        float y = stack * density;
        for( int row = 0; row < zSteps; row++ ) {
            float z = row * density;
            for( int col = 0; col < xSteps; col++ ) {
                float x = col * density;
                TLightProbe * lightprobe = Memory_New( TLightProbe );
                lightprobe->position = Vec3_Set( x, y, z );
                lightprobe->color = Vec3_Zero( );
                List_Add( &gLightProbeList, lightprobe );
            }
        }
    }
}

void Lightprobe_LightEntity( TEntity * entity ) {
    TLightProbe * lp = LightProbe_Trace( &entity->globalPosition );    
    if( lp ) {    
        Entity_SetColor( entity, &lp->color, true );
    }
}


void LightProbe_Calculate() {
    for_each( TLightProbe, lp, gLightProbeList ) {
        for_each( TLight, light, g_lights ) {          
            TVec3 direction;
            float attenuation = Lightmap_CalculateAttenuation( &light->owner->globalPosition, &lp->position, light->radius, &direction );
            lp->color = Vec3_Add( lp->color, Vec3_Scale( light->color, attenuation ) );            
        }
        lp->color = Vec3_Clamp( lp->color, 0.0f, 1.0f );
    }
}

TLightProbe probe;
TLightProbe * LightProbe_Trace( const TVec3 * point ) {
    /*
    TLightProbe * nearest = NULL;
    float nearestDist = FLT_MAX;    
    for_each( TLightProbe, probe, gLightProbeList ) {
        float sqrDist = Vector3_SqrDistance( &probe->position, point );
        if( sqrDist < nearestDist ) {
            nearest = probe;
            nearestDist = sqrDist;
        }
    }
    return nearest;
    */
    
    probe.color = Vec3_Zero();
    for_each( TLight, light, g_lights ) {
        TVec3 dir;
        float attenuation = Lightmap_CalculateAttenuation( point, &light->owner->globalPosition, light->radius, &dir );
        probe.color = Vec3_Add( probe.color, Vec3_Scale( light->color, attenuation ));
    }
    probe.color = Vec3_Clamp( probe.color, 0.0, 1.0 );
    return &probe;
}

void LightProbe_Clear() {
    List_Clear( &gLightProbeList, true );
}
