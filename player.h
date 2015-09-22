#ifndef _PLAYER_
#define _PLAYER_

#include "camera.h"
#include "entity.h"
#include "collision.h"
#include "sound.h"
#include "weapon.h"
#include "gui.h"

OLDTECH_BEGIN_HEADER

#define GROUP_MAX_SOUNDS (4)
#define GROUP_MAX_MATERIALS (16)

typedef struct {
    TSoundBuffer buffers[GROUP_MAX_SOUNDS];
    TSound sounds[GROUP_MAX_SOUNDS];
    int count;
    // material need to identify step sound
    TTexture * material[GROUP_MAX_MATERIALS];
    int materialCount;
} TSoundGroup;


typedef enum {
    STEP_SOIL = 0,
    STEP_ROCK,
    STEP_WOOD,
    STEP_METAL,
    STEP_GROUP_COUNT,
} EStepSoundType;

typedef struct TPlayerInterface {
    TGUINode * ammo;
    TGUINode * health;
    TFont * font;
} TPlayerInterface;

typedef struct {
    TEntity * cameraPivot;
    TEntity * pivot;
    float yaw;
    float pitch;
    TCollisionShape shape;
    TBody body;
    float speed;
    float jumpStrength;
    float mouseSensivity;
    float maxPitch;
    bool crouch;
    bool move;
    bool run;
    bool onGround;
    float pathLength;
    float crouchRadius;
    float standRadius;
    float speedMultiplier;
    TVec3 velocity;
    TVec3 destVelocity;
    TVec3 cameraShakeOffset;
    TVec3 cameraOffset;
    float cameraShakeCoeff;
    TSoundGroup stepSounds[STEP_GROUP_COUNT];
    TSoundGroup * stepSoundGroup;
    TWeapon * weapon;
    TPlayerInterface interface;
} TPlayer;

extern TPlayer * player;

void Player_SetSoundGroup( const char * baseName, EStepSoundType group, const char * materialFile );
void Player_Create( void );
void Player_Update( float timeStep );
void Player_CameraShake( void );
void Player_Free( void );

OLDTECH_END_HEADER

#endif