#ifndef _MONSTER_
#define _MONSTER_

#include "entity.h"
#include "gui.h"

typedef struct TMonster {
    TEntity * model;
    float life;
    TAnimation * runAnim;
    TAnimation * attackAnim;
    TCollisionShape shape;
    TBody body;
} TMonster;

TMonster * Monster_Create( void );
void Monster_Free( TMonster * monster );
void Monster_ThinkAll( void );
void Monster_FreeAll( void );
#endif