#ifndef _SCRIPT_
#define _SCRIPT_

#include "common.h"

void ScriptSystem_Initialize( void );
void ScriptSystem_Shutdown( void );
void ScriptSystem_InitializeAPI( void );

void Script_ExecuteFile( const char * file );
float Script_GetGlobalNumber( const char * name );
const char * Script_GetGlobalString( const char * name );
bool Script_GetGlobalBoolean( const char * name );

void Script_CallFunction( const char * name, const char * argList, ... );


#endif