#ifndef _WEAPON_
#define _WEAPON_

#include "Projectile.h"

OLDTECH_BEGIN_HEADER

typedef enum {
    WEAPON_BLASTER,
    WEAPON_ROCKETLAUNCHER,
} EWeaponType;

typedef struct TWeapon {
    TEntity * model;
    TEntity * projectileModel;
	TEntity * shootPoint;
    TList projectiles;
    TSound sndShot;
    EWeaponType type;
    int shootInterval;
    int wait;
    TVec3 moveSpeed;
} TWeapon;

TWeapon * Weapon_Create( EWeaponType type );
void Weapon_Shoot( TWeapon * wpn );
void Weapon_Update( TWeapon * wpn );
void Weapon_LoadSoundBufferBase( void );

OLDTECH_END_HEADER

#endif