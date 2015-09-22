#ifndef _PROJECTILE_
#define _PROJECTILE_

#include "Entity.h"
#include "Sound.h"

OLDTECH_BEGIN_HEADER

typedef enum {
    PROJECTILE_PLASMA,
    PROJECTILE_ROCKET,
} EProjectileType;

typedef struct SProjectile {
    TEntity * model;
    TVec3 direction;
    struct TWeapon * owner;
    TSound hitSound;
    float damage;
    float velocity;
    int lifeTime;
    // After projectile created, we must do ray cast into it's direction and check intersection 
    // with static bodies just only once and store it's status, but intersection with dynamic 
    // bodies are checked every frame. This is sort of optimisation, which let us prevent 
    // redundant ray-casts, but this incompatible with projectiles with controlled speed 
    // vector, like self-aiming rockets and so on
    bool futureHit;
    TRayTraceResult futureHitResult;
    float futureHitDistance;
    float doneDistance;
} TProjectile;

typedef enum EImpactSoundType {
    IST_CONCRETE    = 0,
    IST_METAL       = 1,
    IST_WOOD        = 2,
    IST_SOIL        = 3,
    IST_COUNT,
} EImpactSoundType;

void Projectile_Update( TProjectile * proj );
TProjectile * Projectile_Create( struct TWeapon * owner, EProjectileType projType, TEntity * projectileModel );
void Projectile_DoPrecalculations( TProjectile * proj );
void Projectile_Free( TProjectile * proj );

OLDTECH_END_HEADER

#endif