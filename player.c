#include "player.h"
#include "input.h"
#include "sound.h"
#include "parser.h"
#include "lightmap.h"

TPlayer * player = NULL;

void Player_Create( ) {
    if( player ) {
        Player_Free( );
        Memory_Free( player );        
    }
    
    player = Memory_New( TPlayer );
    // create main pivot
    player->pivot = Entity_Create( );
	
	// create camera
    player->cameraPivot = Entity_Create();
    Entity_MakeCamera( player->cameraPivot );
    Entity_Attach( player->cameraPivot, player->pivot );
    player->cameraPivot->localPosition = Vec3_Set( 0, 0.4, 0 );

    // add physical body
    Shape_CreateSphere( &player->shape, 1.0f );
    Body_Create( &player->body, &player->shape );
    Dynamics_AddBody( &player->body );

    player->body.position.y = 10;
    player->body.elasticity = 0.05f;

    player->cameraOffset = Vec3_Set( 0, 0.4, 0 );
    player->cameraShakeOffset = Vec3_Zero();    
    player->velocity = Vec3_Zero();
    player->destVelocity = Vec3_Zero(); 
    
    // set parameters
    player->speed = 4.5f;
    player->speedMultiplier = 1.8f;
    player->jumpStrength = 0.1f;
    player->mouseSensivity = 0.2f;
    player->yaw = 0.0f;
    player->pitch = 0.0f;
    player->maxPitch = M_PI / 2.0;
    player->pathLength = 0.0f;
    player->crouchRadius = 0.5f;
    player->standRadius = 1.0f;
    player->crouch = false;
    player->move = false;
    player->run = false;
    player->onGround = false;
	player->cameraShakeCoeff = 0.0f;
    
    
    // ==================
    // step sounds
    Player_SetSoundGroup( "data/sounds/dirt", STEP_SOIL, "data/materials/soil.mtl" );
    Player_SetSoundGroup( "data/sounds/stonestep", STEP_ROCK, "data/materials/stone.mtl" );
    Player_SetSoundGroup( "data/sounds/wood", STEP_WOOD, "data/materials/wood.mtl" );
    Player_SetSoundGroup( "data/sounds/metal", STEP_METAL, "data/materials/metal.mtl" );
    // set default step sounds
    player->stepSoundGroup = &player->stepSounds[STEP_SOIL];

	player->weapon = Weapon_Create( WEAPON_BLASTER );
	Entity_Attach( player->weapon->model, player->cameraPivot );
    
    player->interface.font = Font_LoadFromFile( "data/fonts/font3.ttf", 16 );
    player->interface.ammo = GUI_CreateText( Renderer_GetScreenResolutionWidth() - 200, Renderer_GetScreenResolutionHeight() - 200, 190, 190, "Ammo:\n", player->interface.font );
}

void Player_Free( void ) {
    if( player ){
        Entity_Free( player->cameraPivot );
        Entity_Free( player->pivot );
    }
}

void Player_SetSoundGroup( const char * baseName, EStepSoundType group, const char * materialFile ) {
    if( player ) {
        int soundNum = 0;
        for( int i = 0; i < STEP_GROUP_COUNT; i++ ) {
            char * fileName = Std_Format( "%s%d.ogg", baseName, i + 1 );
            // check file existing
            FILE * file = fopen( fileName, "rb" );
            if( file ) {
                fclose( file );
                SoundBuffer_LoadFile( fileName, &player->stepSounds[group].buffers[soundNum], 0 );
                SoundSource_Create( &player->stepSounds[group].sounds[soundNum], &player->stepSounds[group].buffers[soundNum] );
                soundNum++;
            }
        }
        TValueArray values;
        Parser_LoadFile( materialFile, &values );
        player->stepSounds[group].count = soundNum;
        for( int i = 0; i < values.count; i++ ) {
            player->stepSounds[group].material[i] = Texture2D_LoadFromFile( values.values[i].string );
        }
        player->stepSounds[group].materialCount = values.count;
    }
}

void Player_Update( float timeStep ) {
    if( player ) {
        TVec3 vAxis = Vec3_Set( 0.0f, 1.0f, 0.0f );
        
        // get correct vectors for moving
        TVec3 look = Entity_GetLookVector( player->pivot );
        TVec3 right = Entity_GetRightVector( player->pivot );
        TVec3 listenerPosition = Entity_GetGlobalPosition( player->cameraPivot );
        
        Lightprobe_LightEntity( player->weapon->model );

        player->onGround = false;
        for( int i = 0; i < player->body.contactCount; i++ ) {
            TContact * contact = &player->body.contacts[i];
            // check angle between contact normal and vertical axis, it must be less than 60 degrees
            if( Vec3_Angle( vAxis, contact->normal ) < M_PI / 3.0 ) {
                player->onGround = true;
            }
        }
        
        // ====================
        // moving
        player->destVelocity = Vec3_Zero();
        
        player->move = false;
        if( Input_IsKeyDown( KEY_W )) {
            player->destVelocity = Vec3_Add( player->destVelocity, look );
            player->move = true;
        }
        if( Input_IsKeyDown( KEY_S )) {
            player->destVelocity = Vec3_Sub( player->destVelocity, look );
            player->move = true;
        }
        if( Input_IsKeyDown( KEY_A )) {
            player->destVelocity = Vec3_Add( player->destVelocity, right );
            player->move = true;
        }
        if( Input_IsKeyDown( KEY_D )) {
            player->destVelocity = Vec3_Sub( player->destVelocity, right );
            player->move = true;
        }

        if( Input_IsKeyDown( KEY_C ) ) {
            player->crouch = true;
            if( player->body.shape->sphereRadius > player->crouchRadius ) {
                player->body.shape->sphereRadius -= 0.05f;
            } else {
                player->body.shape->sphereRadius = player->crouchRadius;
            }
        } else {
            player->crouch = false;
            char standUp = 1;
            // before stand up, we must trace ray up on player's head, to ensure, that
            // there enough space to stand up.
            TVec3 end = Vec3_Set( player->body.position.x, player->body.position.y + 1.0f, player->body.position.z );
            TRay headRay = Ray_Set( player->body.position, end );
            TRayTraceResult out;
            Ray_TraceWorld( &headRay, &out );
            if( out.body ) {
                float maxRadius = player->shape.sphereRadius + 0.4f;
                if( Vec3_SqrDistance( out.position, player->body.position ) < maxRadius * maxRadius ) {
                    standUp = 0;
                    player->crouch = true;
                }
            }
            if( standUp ) {
                if( player->body.shape->sphereRadius < player->standRadius ) {
                    player->body.shape->sphereRadius += 0.04f;
                } else {
                    player->body.shape->sphereRadius = player->standRadius;
                }
            }
        }
        // sync step sound position with player position
        for( int i = 0; i < player->stepSoundGroup->count; i++ ) {
            SoundSource_SetPosition( &player->stepSoundGroup->sounds[i], &listenerPosition );
            if( player->crouch ) {
                SoundSource_SetVolume( &player->stepSoundGroup->sounds[i], 0.5f );
            } else {
                SoundSource_SetVolume( &player->stepSoundGroup->sounds[i], 1.0f );
            }
        }
        // emit step sound only if got contact with the ground
        if( player->body.contactCount > 0 ) {
            TContact * lowestContact = &player->body.contacts[0];
            for( int i = 0; i < player->body.contactCount; i++ ) {
                if( player->body.contacts[i].position.y < lowestContact->position.y ) {
                    lowestContact = &player->body.contacts[i];
                }
            }
            // select emittable sounds
            if( lowestContact->triangle ) {
                // iterate over all possible step sound groups
                for( int i = 0; i < STEP_GROUP_COUNT; i++ ) {
                    // iterate over each material in group
                    for( int j = 0; j < player->stepSounds[i].materialCount; j++ ) {
                        if( lowestContact->triangle->material == player->stepSounds[i].material[j] ) {
                            player->stepSoundGroup = &player->stepSounds[i];
                        }
                    }
                }
            }

            if( player->pathLength > 15 * player->speed ) {
                int randomSound = rand() % player->stepSoundGroup->count;
                if( randomSound >= 4 ) {
                    randomSound = 3;
                }
                SoundSource_Play( &player->stepSoundGroup->sounds[ randomSound ] );
                player->pathLength = 0.0f;
            }
        }

        float realSpeed = player->speed;

        player->run = false;
        if( Input_IsKeyDown( KEY_LeftShift ) && !player->crouch ) {
            realSpeed *= player->speedMultiplier;
            player->run = true;
        }
        
        if( player->crouch ) {
            realSpeed *= 0.65f;
        }

        if( player->move ) {
            // prevent divide by zero
            player->destVelocity = Vec3_Normalize( player->destVelocity );
            player->pathLength += realSpeed * 0.5f;
        }

        float interpCoeff = 0.25f;
        if( !player->onGround ) {
            interpCoeff = 0.0035f;
        }
        
        player->destVelocity.x *= realSpeed;
        player->destVelocity.z *= realSpeed;
        
        player->velocity.x += ( player->destVelocity.x - player->velocity.x ) * interpCoeff;
        player->velocity.z += ( player->destVelocity.z - player->velocity.z ) * interpCoeff;
        
        // do not overwrite gravity component (y), apply only x and z
        player->body.linearVelocity.x = player->velocity.x * timeStep;
        player->body.linearVelocity.z = player->velocity.z * timeStep;

        // ====================
        // jump   
        if( Input_IsKeyHit( KEY_Space ) ) {
            if( player->onGround ) {
                player->body.linearVelocity.y += player->jumpStrength;
            }
        }

        // sync pivot's position with physical body
        player->pivot->localPosition = player->body.position;

        // ====================
        // mouse look
        player->yaw -= Input_GetMouseXSpeed() * player->mouseSensivity * timeStep;
        player->pitch += Input_GetMouseYSpeed() * player->mouseSensivity * timeStep;

        // clamp pitch
        if( player->pitch > player->maxPitch ) {
            player->pitch = player->maxPitch;
        }
        if( player->pitch < -player->maxPitch ) {
            player->pitch = -player->maxPitch;
        }

        right = Vec3_Set( 1, 0, 0 );
        vAxis = Vec3_Set( 0, 1, 0 );
        player->pivot->localRotation = Quaternion_SetAxisAngle( vAxis, player->yaw );
        player->cameraPivot->localRotation = Quaternion_SetAxisAngle( right, player->pitch );

        // sound
        TVec3 up = Entity_GetUpVector( player->cameraPivot );
        SoundListener_SetPosition( &listenerPosition );        
        SoundListener_SetOrientation( &look, &up );
        
        // weapon
        Weapon_Update( player->weapon );
        if( Input_IsMouseDown( MB_Left )) {
            Weapon_Shoot( player->weapon );
        }
        player->weapon->moveSpeed = player->body.linearVelocity;
        
        Player_CameraShake( );
    }
}

void Player_CameraShake( ) {
    if( player ) {
        if( player->move && player->onGround ) {
            if( player->run ) {
                player->cameraShakeCoeff += 0.35f;
            } else {
                player->cameraShakeCoeff += 0.25f;
            }
            
            float yOffset = 0.05f * sinf( player->cameraShakeCoeff );
            float xOffset = 0.05f * cosf( player->cameraShakeCoeff / 2 );
            
            if( player->run ) {
                xOffset *= 1.75f;
                yOffset *= 2.0f;
            }
            
            player->cameraShakeOffset = Vec3_Set( xOffset, yOffset, 0.0f );
        }

        player->cameraPivot->localPosition = Vec3_Add( player->cameraOffset, player->cameraShakeOffset );    
    }
}