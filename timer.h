#ifndef _TIMER_
#define _TIMER_

#include "common.h"

typedef struct {
    double lastTime;
} TTimer;

void Timer_Create( TTimer * timer );
double Timer_GetElapsedSeconds( TTimer * timer );
double Timer_GetElapsedMilliseconds( TTimer * timer );
double Timer_GetElapsedMicroseconds( TTimer * timer );
double Timer_GetSeconds( TTimer * timer );
double Timer_GetMilliseconds( TTimer * timer );
double Timer_GetMicroseconds( TTimer * timer );
void Timer_Restart( TTimer * timer );

void Time_Sleep( int milliseconds );
#endif