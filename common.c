#include "common.h"

char formatBuffer[8192];

char * Std_Format( const char * format, ... ) {
    va_list	argumentList;
    va_start(argumentList, format);
    vsprintf(formatBuffer, format, argumentList);
    va_end(argumentList);
    return formatBuffer;
}