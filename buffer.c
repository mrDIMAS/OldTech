#include "buffer.h"

bool Buffer_LoadFile( TBuffer * buf, const char * fileName, unsigned int * crc32 ) {
    FILE * file = fopen( fileName, "rb" );
    if( !file ) {
        return false;
    };
    fseek( file, 0, SEEK_END );
    buf->size = ftell( file );
    fseek( file, 0, SEEK_SET );
    buf->data = Memory_Allocate( buf->size );
    fread( buf->data, buf->size, 1, file );
    if( crc32 ) {
        *crc32 = CRC32( 0, buf->data, buf->size );
    }
    buf->pointer = 0;
    buf->file = NULL;
    fclose( file );
    return true;
}

void Buffer_Free( TBuffer * buf ) {
    if( buf->data ) {
        Memory_Free( buf->data );
    }
    if( buf->file ) {
        fclose( buf->file );
    }
}

float Buffer_ReadFloat( TBuffer * buf ) {
    if( buf->pointer < buf->size ) {
        float * floats = (float*)( buf->data + buf->pointer );
        buf->pointer += sizeof( float );
        return floats[0];
    }
    return 0.0f;
}

void Buffer_ReadData( TBuffer * buf, void * out, int size ) {
    if( buf->pointer < buf->size ) {
        memcpy( out, buf->data + buf->pointer, size );
        buf->pointer += size;
    }
}

TIndex16 Buffer_ReadIndex16( TBuffer * buf ) {
    if( buf->pointer < buf->size ) {
        TIndex16 * ind = (TIndex16*)( buf->data + buf->pointer );
        buf->pointer += sizeof( TIndex16 );
        return ind[0];
    }
    return 0.0f;
}

char Buffer_ReadByte( TBuffer * buf ) {
    if( buf->pointer < buf->size ) {
        char * chars = (char*)( buf->data + buf->pointer );
        buf->pointer += sizeof( char );
        return chars[0];
    }
    return 0;
}

void Buffer_ReadString( TBuffer * buf, char * buffer ) {
    char symbol;
    int i = 0;
    while( 1 ) {
        symbol = Buffer_ReadByte( buf );
        if( symbol == '\0' ) {
            buffer[i] = '\0';
            break;
        }
        buffer[i] = symbol;
        i++;
    }
}

int Buffer_ReadInteger( TBuffer * buf ) {
    if( buf->pointer < buf->size ) {
        int * ints = (int*)( buf->data + buf->pointer );
        buf->pointer += sizeof( int );
        return ints[0];
    }
    return 0;
}

void Buffer_ReadVector3( TBuffer * buf, TVec3 * v ) {
    v->x = Buffer_ReadFloat( buf );
    v->y = Buffer_ReadFloat( buf );
    v->z = Buffer_ReadFloat( buf );
}

void Buffer_ReadVector2( TBuffer * buf, TVector2 * v ) {
    v->x = Buffer_ReadFloat( buf );
    v->y = Buffer_ReadFloat( buf );
}

void Buffer_ReadQuaternion( TBuffer * buf, TQuaternion * q ) {
    q->x = Buffer_ReadFloat( buf );
    q->y = Buffer_ReadFloat( buf );
    q->z = Buffer_ReadFloat( buf );
    q->w = Buffer_ReadFloat( buf );
}

bool Buffer_WriteFile( TBuffer * buf, const char * fileName ) {
    buf->file = fopen( fileName, "wb" );
    buf->data = NULL;
    buf->pointer = 0;
    buf->size = 0;
    if( !buf->file ) {
        return false;
    }
    return true;
}

void Buffer_WriteFloat( TBuffer * buf, float num ) {
    if( buf->file ) {
        fwrite( &num, sizeof( num ), 1, buf->file );
    }
}

void Buffer_WriteInteger( TBuffer * buf, int num ) {
    if( buf->file ) {
        fwrite( &num, sizeof( num ), 1, buf->file );
    }    
}

void Buffer_WriteVector3( TBuffer * buf, const TVec3 * vec ) {
    Buffer_WriteFloat( buf, vec->x );
    Buffer_WriteFloat( buf, vec->y );
    Buffer_WriteFloat( buf, vec->z );
}

void Buffer_WriteVector2( TBuffer * buf, const TVector2 * vec ) {
    Buffer_WriteFloat( buf, vec->x );
    Buffer_WriteFloat( buf, vec->y );  
}

void Buffer_WriteQuaternion( TBuffer * buf, const TQuaternion * quat ) {
    Buffer_WriteFloat( buf, quat->x );
    Buffer_WriteFloat( buf, quat->y );
    Buffer_WriteFloat( buf, quat->z );
    Buffer_WriteFloat( buf, quat->w );
}

void Buffer_WriteByte( TBuffer * buf, char b ) {
    if( buf->file ) {
        fwrite( &b, sizeof( b ), 1, buf->file );
    }        
}

void Buffer_WriteString( TBuffer * buf, char * str ) {
    if( buf->file ) {
        while( *str ) {
            fwrite( str, sizeof( char ), 1, buf->file );
            str++;
        }
        char zero = 0;
        fwrite( &zero, sizeof( char ), 1, buf->file );
    }
}

void Buffer_WriteData( TBuffer * buf, void * data, int size ) {
    if( buf->file ) {
        fwrite( data, size, 1, buf->file );
    }
}