#ifndef _ANIM_
#define _ANIM_

#include "common.h"

OLDTECH_BEGIN_HEADER

typedef struct {
    float interp;
    int curFrame;
    int begFrame;
    int endFrame;
    int nextFrame;
    float frameDelta;
    float duration;
} TAnimation;

TAnimation * Animation_Create( int begFrame, int endFrame, float duration );
void Animation_Free( TAnimation * anim );
void Animation_UpdateAll( void );

OLDTECH_END_HEADER

#endif