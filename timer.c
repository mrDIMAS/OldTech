#include "timer.h"
#include <windows.h>

LARGE_INTEGER freq;

void Timer_Create( TTimer * timer ) {
    QueryPerformanceFrequency ( &freq );
    Timer_Restart( timer );
}

double Timer_GetElapsedSeconds( TTimer * timer ) {
    LARGE_INTEGER time;
    QueryPerformanceCounter ( &time );
    return ( double ) ( time.QuadPart ) / ( double ) ( freq.QuadPart ) - timer->lastTime / 1000000.0;
}

double Timer_GetElapsedMilliseconds( TTimer * timer ) {
    LARGE_INTEGER time;
    QueryPerformanceCounter ( &time );
    return ( double ) ( time.QuadPart * 1000.0 ) / ( double ) ( freq.QuadPart ) - timer->lastTime / 1000.0;
}

double Timer_GetElapsedMicroseconds( TTimer * timer ) {
    LARGE_INTEGER time;
    QueryPerformanceCounter ( &time );
    return ( double ) ( time.QuadPart * 1000000.0 ) / ( double ) ( freq.QuadPart ) - timer->lastTime;
}

double Timer_GetSeconds( TTimer * timer ) {
    UNUSED_VARIABLE( timer );
    LARGE_INTEGER time;
    QueryPerformanceCounter ( &time );
    return ( double ) ( time.QuadPart ) / ( double ) ( freq.QuadPart );
}

double Timer_GetMilliseconds( TTimer * timer ) {
    UNUSED_VARIABLE( timer );
    LARGE_INTEGER time;
    QueryPerformanceCounter ( &time );
    return ( double ) ( time.QuadPart * 1000.0 ) / ( double ) ( freq.QuadPart );
}

double Timer_GetMicroseconds( TTimer * timer ) {
    UNUSED_VARIABLE( timer );
    LARGE_INTEGER time;
    QueryPerformanceCounter ( &time );
    return ( double ) ( time.QuadPart * 1000000.0 ) / ( double ) ( freq.QuadPart );
}

void Timer_Restart( TTimer * timer ) {
    LARGE_INTEGER time;
    QueryPerformanceCounter ( &time );
    timer->lastTime = ( double ) ( time.QuadPart * 1000000.0 ) / ( double ) ( freq.QuadPart );
}

void Time_Sleep( int milliseconds ) {
    Sleep( milliseconds );
}