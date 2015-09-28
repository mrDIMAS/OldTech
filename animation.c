#include "animation.h"
#include "list.h"

TList gAnimations = { .size = 0, .head = NULL, .tail = NULL };

TAnimation * Animation_Create( int begFrame, int endFrame, float duration ) {
    TAnimation * anim = Memory_New( TAnimation );
    if( endFrame < begFrame ) {
        int temp = begFrame;
        begFrame = endFrame;
        endFrame = temp;
    }
    anim->begFrame = begFrame;
    anim->endFrame = endFrame;
    anim->curFrame = begFrame;
    anim->nextFrame = begFrame + 1;
    anim->duration = duration;
    anim->frameDelta = endFrame - begFrame;
    anim->interp = 0.0f;
    anim->enabled = false;
    List_Add( &gAnimations, anim );
    return anim;
}

void Animation_Free( TAnimation * anim ) {
    List_Remove( &gAnimations, anim );
    Memory_Free( anim );
}

void Animation_Update( TAnimation * anim, float dt ) {      
    anim->interp += dt * (anim->frameDelta / anim->duration);
    if( anim->interp >= 1.0f ) {
        anim->curFrame++;
        if( anim->curFrame >= anim->endFrame ) {
            anim->curFrame = anim->begFrame;
        }
        anim->nextFrame = anim->curFrame + 1;
        anim->interp = 0.0f;
    } 
}

void Animation_UpdateAll( void ) {
    for_each( TAnimation, anim, gAnimations ) {
        if( anim->enabled ) {
            Animation_Update( anim, 1.0f / 60.0f );
        }
    }
}
