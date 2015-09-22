#ifndef _VARIABLE_
#define _VARIABLE_

typedef struct TVariable {
    const char * name;
    float num;
    char * str;
    char * help;
} TVariable;

void Variable_InitSubSystem( void );
void Variable_Register( TVariable * var );
TVariable * Variable_Find( const char * name );

#endif