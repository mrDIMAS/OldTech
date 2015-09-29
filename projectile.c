#include "Projectile.h"
#include "weapon.h"
#include "player.h"


typedef struct TProjectileSoundBase {
    bool initialized;
    TSoundBuffer bulletImpact[ IST_COUNT ];
    TValueArray materials[ IST_COUNT ];
} TProjectileSoundBase;

TProjectileSoundBase gProjectileSndBase;

void Projectile_EmitHitSound( TProjectile * proj, TTexture * texture ) {    
    // select proper sound by texture
    for( int storageNum = 0; storageNum < IST_COUNT; storageNum++ ) {
        for( int i = 0; i < gProjectileSndBase.materials[storageNum].count; i++ ) {
            TValue * value = &gProjectileSndBase.materials[storageNum].values[i];
            if( strcmp( value->string, texture->fileName ) == 0 ) {
                SoundSource_Create( &proj->hitSound, &gProjectileSndBase.bulletImpact[storageNum] );
                SoundSource_Play( &proj->hitSound );
            }
        }
    }
}

void Projectile_LoadSoundBufferBase( void ) {
    gProjectileSndBase.initialized = true;
    SoundBuffer_LoadFile( "data/sounds/bullet_metal_impact.ogg", &gProjectileSndBase.bulletImpact[ IST_METAL ], false );
    SoundBuffer_LoadFile( "data/sounds/bullet_concrete_impact.ogg", &gProjectileSndBase.bulletImpact[ IST_CONCRETE ], false );
    SoundBuffer_LoadFile( "data/sounds/bullet_wood_impact.ogg", &gProjectileSndBase.bulletImpact[ IST_WOOD ], false );
    SoundBuffer_LoadFile( "data/sounds/bullet_soil_impact.ogg", &gProjectileSndBase.bulletImpact[ IST_SOIL ], false );
    Parser_LoadFile( "data/materials/metal.mtl", &gProjectileSndBase.materials[ IST_METAL ] );
    Parser_LoadFile( "data/materials/stone.mtl", &gProjectileSndBase.materials[ IST_CONCRETE ] );
    Parser_LoadFile( "data/materials/wood.mtl", &gProjectileSndBase.materials[ IST_WOOD ] );
    Parser_LoadFile( "data/materials/soil.mtl", &gProjectileSndBase.materials[ IST_SOIL ] );
}

void Projectile_Update( TProjectile * proj ) {
    if( proj->lifeTime > 0 ) {
        proj->lifeTime--;
            
        TRayTraceResult result;
        TRay ray = Ray_SetDirection( proj->model->globalPosition, proj->direction );
        Ray_TraceWorldDynamic( &ray, &result );
        
        if( result.body ) {
            if( result.body != &player->body ) {
                if( Vec3_Distance( result.position, proj->model->globalPosition ) < 1.0f ) {
                    proj->lifeTime = 0;
                }
            }
        };
        if( proj->futureHit ) {
            if( proj->doneDistance > proj->futureHitDistance ) {
                if( proj->futureHitResult.triangle ) {
                    Projectile_EmitHitSound( proj, proj->futureHitResult.triangle->material );
                    SoundSource_SetPosition( &proj->hitSound, &proj->futureHitResult.position );                   
                }
                proj->lifeTime = 0;
            }
        }
        
    
        
        proj->model->localPosition = Vec3_Add( proj->model->localPosition, Vec3_Scale( proj->direction, proj->velocity ));
        
        proj->doneDistance += proj->velocity;
    }
}

void Projectile_Free( TProjectile * proj ) {
    SoundSource_Free( &proj->hitSound );
    Entity_Free( proj->model );
    Memory_Free( proj );
}

TProjectile * Projectile_Create( TWeapon * owner, EProjectileType projType, TEntity * projectileModel ) {
    if( !gProjectileSndBase.initialized ) {
        Projectile_LoadSoundBufferBase();        
    }
    
    TProjectile * proj = Memory_New( TProjectile );
    proj->model = Entity_CreateInstance( projectileModel );
    proj->owner = owner;
    switch( projType ) {
        case PROJECTILE_PLASMA: {
            proj->damage = 10.0f;
            proj->lifeTime = 200;
            proj->velocity = 1.5f;
            proj->model->localScale = Vec3_Set( 0.02f, 0.02f, 0.02f );
            break;
        }
        case PROJECTILE_ROCKET: {
            proj->damage = 25.0f;
            proj->lifeTime = 200;
            proj->velocity = 0.25;
            proj->model->localScale = Vec3_Set( 0.1f, 0.1f, 0.1f );
            break;
        }       
        default: {
            Util_RaiseError( "Unable to create projectile! Unknown type!" );
            break;
        }
    }
        
	return proj;
}

void Projectile_DoPrecalculations( TProjectile * proj ) {
    TRay ray = Ray_SetDirection( proj->model->localPosition, Vec3_Scale( proj->direction, 10000.0f ));    
    Ray_TraceWorldStatic( &ray, &proj->futureHitResult );	    
    if( proj->futureHitResult.body ) {
        if( proj->futureHitResult.body != &player->body ) {
            proj->futureHitDistance = Vec3_Distance( proj->model->localPosition, proj->futureHitResult.position );
            proj->futureHit = true;
        } else {
            proj->futureHit = false;
        }
    } else {
        proj->futureHit = false;
    }    
}