#ifndef _SHADER_
#define _SHADER_

#include "common.h"

typedef enum {
    SHADER_VERTEX,
    SHADER_FRAGMENT,
} EShaderType;

typedef struct {
    unsigned int target;
    unsigned int id;
    EShaderType type;
} TShader;

//void Shader_LoadFromFile( TShader * shader, const char * filename );

#endif