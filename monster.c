#include "monster.h"
#include "player.h"

TList gMonsterList;

TMonster * Monster_Create( void ) {
    TMonster * monster = Memory_New( TMonster );
    monster->model = Entity_LoadFromFile( "data/models/monsters/test.scene" );
    monster->runAnim = Animation_Create( 0, 20, 1.75 ); 
    monster->attackAnim = Animation_Create( 21, 26, 1 );    
    Entity_SetAnimation( monster->model, monster->runAnim );
    Entity_SetAnimationEnabled( monster->model, true );
    List_Add( &gMonsterList, monster );
    
    monster->debug = GUI_CreateText( 100, 100, 200, 200, " ", Font_LoadFromFile( "data/fonts/font3.ttf", 12 ));
    return monster;
}

void Monster_Free( TMonster * monster ) {
    List_Remove( &gMonsterList, monster );
    Memory_Free( monster );
}

void Monster_Think( TMonster * monster ) {
    TVec3 dir = Vec3_Sub( player->body.position, Entity_GetGlobalPosition( monster->model ));
    dir.y = 0.00001;
    float angle = Vec3_Angle( dir, Vec3_Set( 0.0f, 0.0f, 1.0f ));
 
    float distance = Vec3_Distance( player->body.position, Entity_GetGlobalPosition( monster->model ));
    if( distance < 0.05f ) {
        Entity_SetAnimation( monster->model, monster->attackAnim );
    } else {
        Entity_SetAnimation( monster->model, monster->runAnim );
    }
    monster->model->localRotation = Quaternion_SetAxisAngle( Vec3_Set( 0.0f, 1.0f, 0.0f ), 0 );
    GUI_SetNodeText( monster->debug, Std_Format( "%f", angle * 180.0f / M_PI));
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