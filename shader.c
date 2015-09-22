#include "shader.h"
#include "renderer.h"

void Shader_LoadFromFile( TShader * shader, const char * filename ) {
    // read file
	FILE * file = fopen( filename, "rb" );
	if( !file ) {
		Util_RaiseError( "Unable to load ARB program from %s file", filename );
	} else {
        Log_Write( "Loading shader from %s", filename );
    }
    
	fseek( file, 0, SEEK_END );
	int size = ftell( file );
	rewind( file );
	char * buffer = Memory_AllocateClean( size );
	fread( buffer, 1, size, file );
	fclose( file );

    Renderer_CompileARBShader( shader, buffer, size );
    
    Memory_Free( buffer );
}


