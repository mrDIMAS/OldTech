#include "Weapon.h"

typedef struct {
    bool initialized;
    TEntity * blasterModel;
    TEntity * rocketLauncherModel;
    TSoundBuffer sndBufWeaponShoot;
} TWeaponBase;

TWeaponBase gWeaponBase;

void Weapon_LoadBase( void ) {
    gWeaponBase.blasterModel = Entity_LoadFromFile( "data/models/weapons/blaster.scene" );
    gWeaponBase.blasterModel->visible = false;
    
    SoundBuffer_LoadFile( "data/sounds/weapon_fire.ogg", &gWeaponBase.sndBufWeaponShoot, false );
    gWeaponBase.initialized = true;
}

TWeapon * Weapon_Create( EWeaponType type ) {
    if( !gWeaponBase.initialized ) {
        Weapon_LoadBase();
    }
    
	TWeapon * wpn = Memory_New( TWeapon );
    wpn->type = type;
    wpn->wait = 0;
    switch( type ) {
    case WEAPON_BLASTER:
        wpn->shootInterval = 6;
		wpn->model = Entity_CreateInstance( gWeaponBase.blasterModel );       
        wpn->projectileModel = Entity_Create();
        TBillboard * billboard = Entity_MakeBillboard( wpn->projectileModel );
        billboard->texture = Texture2D_LoadFromFile( "data/textures/flare.tga" );
        wpn->projectileModel->visible = false;
        
        SoundSource_Create( &wpn->sndShot, &gWeaponBase.sndBufWeaponShoot );
		
		wpn->shootPoint = Entity_GetChildByName( wpn->model, "ShootPoint" );
        break;
    case WEAPON_ROCKETLAUNCHER:
        wpn->shootInterval = 20;
		wpn->model = Entity_LoadFromFile( "data/models/weapons/blaster.scene" );
        wpn->projectileModel = Entity_LoadFromFile( "data/models/projectiles/plasma.scene" );
		wpn->shootPoint = Entity_GetChildByName( wpn->model, "ShootPoint" );
        SoundSource_Create( &wpn->sndShot, &gWeaponBase.sndBufWeaponShoot );
        break;		
    }
    List_Create( &wpn->projectiles );
    //Entity_SetDepthHack( wpn->model, 0.175f );
	return wpn;
}

void Weapon_Shoot( TWeapon * wpn ) {
    if( wpn->wait <= 0 ) {
		TProjectile * proj = Projectile_Create( wpn, PROJECTILE_PLASMA, wpn->projectileModel );
		proj->model->localPosition = Entity_GetGlobalPosition( wpn->shootPoint );
		proj->model->localRotation = Quaternion_SetMatrix( wpn->shootPoint->globalTransform );
        Entity_CalculateGlobalTransform( proj->model );
        proj->direction = Entity_GetLookVector( proj->model );        
        proj->direction = Vec3_Normalize( proj->direction );
        Projectile_DoPrecalculations( proj );
        List_Add( &wpn->projectiles, proj );
        wpn->wait = wpn->shootInterval;       
        SoundSource_Play( &wpn->sndShot );
    }
}

void Weapon_Update( TWeapon * wpn ) {
	wpn->wait--;
	TListNode * node = wpn->projectiles.head;
    
    TVec3 globalPos = Entity_GetGlobalPosition( wpn->model );
    SoundSource_SetPosition( &wpn->sndShot, &globalPos );
    
	while( node ) {
		TProjectile * proj = node->data;
		Projectile_Update( proj );
		if( proj->lifeTime <= 0 ) {
            if( !SoundSource_IsPlaying( &proj->hitSound )) {                
                node = List_Remove( &wpn->projectiles, proj );
                Projectile_Free( proj );
            } else {
                proj->model->visible = false;
                node = node->next;
            }
		} else {
			node = node->next;
		}
	}
}