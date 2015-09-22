#include "log.h"

TLog g_log;
char g_logOpened = 0;

void Log_Open( TLog * log, const char * fn ) {
    log->file = fopen( fn, "w" );
    g_logOpened = 1;
}

void Log_Write( const char * str, ... ) {
    if( g_logOpened ) {
        va_list	argumentList;
        char buffer[4096];
        va_start(argumentList, str );
        vsprintf( buffer, str, argumentList);
        va_end(argumentList);
        fprintf( g_log.file, "%s\n", buffer );
        printf(  "%s\n", buffer );
    };
}

void Log_Close( TLog * log ) {
    fclose( log->file );
    g_logOpened = 0;
}