#ifndef _UTILS_
#define _UTILS_

#define DEG2RAD( a ) ( a * ( 3.14159f / 180.0f ) )

void Util_Message( const char * format, ... );
void Util_RaiseError( const char * format, ... );
int Util_RandomInt( int min, int max );
float Util_RandomFloat( float min, float max );
void Util_BeginPerformanceMeasure( void );
float Util_EndPerformanceMeasure( void );
#endif