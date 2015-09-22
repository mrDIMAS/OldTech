#ifndef _COMMON_
#define _COMMON_

#ifdef _DEBUG 
#   define _DEBUG_GL_
#endif

#ifdef _cplusplus
#   define OLDTECH_BEGIN_HEADER extern "C" {
#   define OLDTECH_END_HEADER   }
#else
#   define OLDTECH_BEGIN_HEADER // nothing
#   define OLDTECH_END_HEADER   // nothing
#endif

OLDTECH_BEGIN_HEADER

#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>

#include "memory.h"
#include "str.h"
#include "utils.h"
#include "log.h"
#include "crc32.h"
#include "array.h"
#include "variable.h"

char * Std_Format( const char * format, ... );

#define UNUSED_VARIABLE( var ) (void)(var)

#ifndef M_PI
#   define M_PI 3.14159265358979323846
#endif

OLDTECH_END_HEADER



#endif