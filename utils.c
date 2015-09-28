#include "utils.h"
#include "common.h"
#include "log.h"
#include "timer.h"
#include <windows.h>

TTimer gPerformanceTimer;

int Util_RandomInt( int min, int max ) {
	return min + ( max - min ) * ( rand() / RAND_MAX );
}

float Util_RandomFloat( float min, float max ) {
	return min + ( max - min ) * ( rand() / RAND_MAX );
}

void Util_Message( const char * format, ... ) {
    va_list	argumentList;
    char buffer[4096];
    va_start(argumentList, format);
    vsprintf( buffer, format, argumentList);
    va_end(argumentList);
#ifdef _WIN32
    MessageBoxA( 0, buffer, "SYS_Message", MB_OK );
#endif
    Log_Write( buffer );
}


void Util_RaiseError( const char * format, ... ) {
    //__builtin_trap(); // raise breakpoint
    
    va_list	argumentList;
    char buffer[4096];
    va_start(argumentList, format );
    vsprintf( buffer, format, argumentList);
    va_end(argumentList);
#ifdef _WIN32
    MessageBoxA( 0, buffer, "Error", MB_OK | MB_ICONERROR );
#endif
    Log_Write( buffer );
    /* free memory allocated in this process */
    Memory_CollectGarbage();
    exit( -1 );
}

void Util_BeginPerformanceMeasure() {
    Timer_Create( &gPerformanceTimer );
}

float Util_EndPerformanceMeasure() {
    return Timer_GetElapsedMilliseconds( &gPerformanceTimer );
}

