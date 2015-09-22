#ifndef _PARSER_
#define _PARSER_

#include "ValueArray.h"

// parse string with format: valueName = "value";
void Parser_LoadFile( const char * fileName, TValueArray * array );
void Parser_LoadString( const char * str, TValueArray * array );

#endif