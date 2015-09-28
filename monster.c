#include "monster.h"
#include "player.h"

TList gMonsterList;

TMonster * Monster_Create( void ) {
    TMonster * monster = Memory_New( TMonster );
    monster->model = Entity_LoadFromFile( "data/models/monsters/test.scene" );
    
    Shape_CreateSphere( &monster->shape, 0.45f );
    Body_Create( &monster->body, &monster->shape );
    Dynamics_AddBody( &monster->body );
    //monster->body.position = player->body.position;
    monster->body.position = Vec3_Set( 0, 3, 0 );
    monster->runAnim = Animation_Create( 0, 20, 1.75 ); 
    monster->attackAnim = Animation_Create( 21, 26, 1 );    
    Entity_SetAnimation( monster->model, monster->runAnim );
    List_Add( &gMonsterList, monster );
    return monster;
}

void Monster_Free( TMonster * monster ) {
    List_Remove( &gMonsterList, monster );
    Memory_Free( monster );
}

void Monster_Think( TMonster * monster ) {
    TVec3 dir = Vec3_Sub( player->body.position, Entity_GetGlobalPosition( monster->model ));
    TVec3 dirNorm = Vec3_Normalize( dir );
    float angle = atan2f( dir.x, dir.z );    
    float distance = Vec3_Distance( player->body.position, Entity_GetGlobalPosition( monster->model ));
    if( distance < 1.5f ) {
        Entity_SetAnimation( monster->model, monster->attackAnim );
        monster->attackAnim->enabled = true;
        monster->runAnim->enabled = false;
    } else {
        Entity_SetAnimation( monster->model, monster->runAnim );
        monster->runAnim->enabled = true;
        monster->attackAnim->enabled = false;
        monster->body.linearVelocity.x = Vec3_Scale( dirNorm, 0.02 ).x;
        monster->body.linearVelocity.z = Vec3_Scale( dirNorm, 0.02 ).z;
    }
    monster->model->localRotation = Quaternion_SetAxisAngle( Vec3_Set( 0.0f, 1.0f, 0.0f ), angle );  
    monster->model->localPosition = monster->body.position;
}

void Monster_ThinkAll( void ) {
    for_each( TMonster, monster, gMonsterList ) {
        Monster_Think( monster );
    }
}

void Monster_FreeAll( void ) {
    while( gMonsterList.head ) {
        Monster_Free( gMonsterList.head->data );
    }
}