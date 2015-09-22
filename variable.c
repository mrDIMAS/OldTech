#include "variable.h"
#include "list.h"
#include "common.h"

TList gVariables;

TVariable v_countVariables = { .name = "v_countVariables", .num = 0, .str = NULL, .help = "Count of all registered variables" };

void Variable_InitSubSystem( void ) {
    Variable_Register( &v_countVariables );
}

void Variable_Register( TVariable * var ) {
    if( var->str ) {
        var->num = atof( var->str );
    } else {        
        char * fmt = Std_Format( "%f", var->num );
        int len = strlen( fmt );
        var->str = Memory_AllocateClean( len + 1 );
        memcpy( var->str, fmt, len );
    }
    Log_Write( "Variable %s registered. Defaults: %f, '%s' - %s", var->name, var->num, var->str, var->help );
    List_Add( &gVariables, var );
}

TVariable * Variable_Find( const char * name ) {
    for_each( TVariable, v, gVariables ) {
        if( !strcmp( v->name, name )) {
            return v;
        }
    }
    Log_Write( "%s variable not found!", name );
    return NULL;
}