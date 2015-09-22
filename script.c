#include "script.h"
#include "renderer.h"
#include <external/lua/include/lua.h>
#include <external/lua/include/lauxlib.h>
#include <external/lua/include/lualib.h>

lua_State * gLua = NULL;

void ScriptSystem_Initialize() {
    gLua = luaL_newstate( );
    luaL_openlibs( gLua );
    ScriptSystem_InitializeAPI();
}

void ScriptSystem_Shutdown() {
    lua_close( gLua );
}

void Script_ExecuteFile( const char * file ) {
    int ret = luaL_dofile( gLua, file );
    if( ret != 0 ) {
        Util_RaiseError( "Unable to load script from file %s.\nErrors: %s", file, lua_tostring( gLua, -1 ));
    }
}

float Script_GetGlobalNumber( const char * name ) {
    lua_getglobal( gLua, name );
    
    if( lua_isnumber( gLua, -1 )) {
        return lua_tonumber( gLua, -1 );
    } else {
         Util_RaiseError( "%s is not a number!", name );
    }
    
    return 0.0f;
}

const char * Script_GetGlobalString( const char * name ) {    
    lua_getglobal( gLua, name );
    
    if( lua_isstring( gLua, -1 )) {
        return lua_tostring( gLua, -1 );
    } else {
        Util_RaiseError( "%s is not a string!", name );
    }
    
    return NULL;
}

bool Script_GetGlobalBoolean( const char * name ) {
    lua_getglobal( gLua, name );
    
    if( lua_isboolean( gLua, -1 )) {
        return lua_toboolean( gLua, -1 );
    } else {
        Util_RaiseError( "%s is not a boolean value!", name );
    }
    
    return false; 
}

// original function was written by Roberto Ierusalimschy
// common syntax: Script_CallFunction( func_name, "[d][i][s][>][d][i][s]" );
void Script_CallFunction( const char * name, const char * argList, ... ) {    
    va_list vl;
    int narg, nres;  // number of arguments and results 

    va_start( vl, argList );
    lua_getglobal( gLua, name );  // get function 

    // push arguments 
    narg = 0;
    while (*argList) {  // push arguments 
        switch (*argList++) {

          case 'd':  // double argument 
            lua_pushnumber( gLua, va_arg(vl, double));
            break;

          case 'i':  // int argument 
            lua_pushnumber( gLua, va_arg(vl, int));
            break;

          case 's':  // string argument 
            lua_pushstring( gLua, va_arg(vl, char *));
            break;

          case '>':
            goto endwhile;

          default:
            Util_RaiseError( "Invalid option (%c)", *(argList - 1));
        }
        narg++;
        luaL_checkstack( gLua, 1, "too many arguments");
    } 
    
endwhile:

    // do the call 
    nres = strlen(argList);  // number of expected results 
    // do the call
    if (lua_pcall( gLua, narg, nres, 0) != 0) {
        Util_RaiseError( "Error running function `%s': %s", name, lua_tostring( gLua, -1));
    }

    // retrieve results 
    nres = -nres;  // stack index of first result 
    while (*argList) {  // get results 
        switch (*argList++) {

          case 'd':  // double result 
            if (!lua_isnumber( gLua, nres)) {
                Util_RaiseError( "Wrong result type" );
            }
            *va_arg(vl, double *) = lua_tonumber( gLua, nres);
            break;

          case 'i':  // int result 
            if (!lua_isnumber( gLua, nres)) {
                Util_RaiseError( "Wrong result type");          
            }
            *va_arg(vl, int *) = (int)lua_tonumber( gLua, nres);
            break;

          case 's':  // string result 
            if (!lua_isstring( gLua, nres)) {
                Util_RaiseError( "Wrong result type");            
            }
            *va_arg(vl, const char **) = lua_tostring( gLua, nres);
            break;

          default:
            Util_RaiseError( "Invalid option (%c)", *(argList - 1));
        }
        nres++;
    }
    va_end(vl);
}


// API

int API_GetResolutionWidth( lua_State * state ) {
    lua_pushnumber( state, Renderer_GetScreenResolutionWidth() );
    return 1;
}

int API_GetResolutionHeight( lua_State * state ) {
    lua_pushnumber( state, Renderer_GetScreenResolutionHeight() );
    return 1;
}

int API_SetResolution( lua_State * state ) {
    float width = lua_tonumber( state, 1 );
    float height = lua_tonumber( state, 2 );
    float bpp = lua_tonumber( state, 3 );
    Renderer_SetResolution( width, height, bpp );
    return 0;
}

void ScriptSystem_InitializeAPI( void ) {
    lua_register( gLua, "GetScreenResolutionWidth", API_GetResolutionWidth );
    lua_register( gLua, "GetScreenResolutionHeight", API_GetResolutionHeight );
    lua_register( gLua, "SetResolution", API_SetResolution );
}