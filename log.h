#ifndef _LOG_
#define _LOG_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

typedef struct {
    FILE * file;
} TLog;

extern TLog g_log;

void Log_Open( TLog * log, const char * fn );
void Log_Write( const char * str, ... );
void Log_Close( TLog * log );

#endif