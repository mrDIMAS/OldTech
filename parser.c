#include "parser.h"

// read entire file into temporary buffer and parse it 
void Parser_LoadFile( const char * fileName, TValueArray * array ) {
    FILE * file = fopen( fileName, "rb" );
    if( !file ) {
        Util_RaiseError( "ParseFile: Unable to open file '%s' for parsing!", fileName );
    }
    fseek( file, 0, SEEK_END );
    int size = ftell( file );
    fseek( file, 0, SEEK_SET );
    char * buffer = Memory_AllocateClean( size + 1 );
    fread( buffer, 1, size, file );
    buffer[ size ] = '\0';
    fclose( file );
    Parser_LoadString( buffer, array );
    // free temporary buffer 
    Memory_Free( buffer );
}



// parse string with format: valueName = "value"; 
void Parser_LoadString( const char * str, TValueArray * array ) {
    assert( str );
    assert( array );

    // copy string to temporary buffer 
    unsigned int bufferLength = strlen( str );
    char * buffer = Memory_AllocateClean( bufferLength );
    memcpy( buffer, str, bufferLength);

    char equalFound = 0;
    char quoteLF = 0;
    char quoteRF = 0;
    unsigned int n = 0;

    int invalidIndex = -1;
    int valueBegin = invalidIndex, valueEnd = invalidIndex;
    int valueNameBegin = invalidIndex, valueNameEnd = invalidIndex;

    ValueArray_Create( array );

    TValue * pCurrentValue = 0;

    while( true ) {
        unsigned char symbol = buffer[n];

        // end of name-value pair 
        if( symbol == ';' ) {
            // clear state 
            quoteLF = 0;
            quoteRF = 0;
            equalFound = 0;
            valueBegin = 0;
            valueEnd = 0;
            valueNameBegin = invalidIndex;
            valueNameEnd = invalidIndex;
        };

        if( isalpha(symbol) || isdigit (symbol) || symbol == '_' ) {
            // this is 'name' symbol 
            if ( !equalFound ) {
                if( valueNameBegin == invalidIndex ) {
                    valueNameBegin = n;
                }
            }
        } else {
            if( symbol != '\n' && symbol != '\r' && symbol != ';' ) {
                if( valueNameEnd == invalidIndex ) {
                    valueNameEnd = n;
                }
            }

            // 'equals' found, so next going the value 
            if ( symbol == '=' ) {
                equalFound = 1;
            }

            if ( symbol == '"' ) {
                if ( quoteLF == 0 ) {
                    quoteLF = 1;
                    // next symbol after quote is the first symbol of the value, so store 'n+1' 
                    valueBegin = n + 1;
                } else {
                    // closing quote 
                    quoteRF = 1;
                    valueEnd = n - 1;
                }
            }
        };

        if ( quoteLF ) {
            // closing quote, extract value 
            if ( quoteRF ) {
                if( array->values ) {
                    // got new value, reallocate array 
                    int newLength = ( array->count + 1 ) * sizeof( TValue );
                    array->values = Memory_Reallocate( array->values, newLength );
                    pCurrentValue = array->values + array->count;
                } else {
                    // allocate memory for the first element 
                    array->values = Memory_AllocateClean( sizeof( TValue ) );
                    pCurrentValue = array->values;
                }
                array->count++;
                valueEnd++;
                // allocate memory for the valueName, and value 
                int valueLength = valueEnd - valueBegin;
                char * value = Memory_AllocateClean( valueLength + 1 );
                int valueNameLength = valueNameEnd - valueNameBegin;
                char * valueName = Memory_AllocateClean( valueNameLength + 1 );
                // copy value and valueName 
                memcpy( value, buffer + valueBegin, valueLength );
                value[valueLength] = '\0';
                memcpy( valueName, buffer + valueNameBegin, valueNameLength );
                valueName[valueNameLength] = '\0';
                pCurrentValue->name = valueName;
                pCurrentValue->string = value;
                pCurrentValue->number = atof( value );
            }
        };

        n++;

        if ( n >= bufferLength ) {
            break;
        }
    };

    // free temporary buffer 
    Memory_Free( buffer );
}