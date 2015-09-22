#ifndef _BUFFER_
#define _BUFFER_

#include "common.h"
#include "vector3.h"
#include "quaternion.h"
#include "vector2.h"
#include "face.h"

OLDTECH_BEGIN_HEADER

// binary buffer
typedef struct {
    char * data;
    int size;
    int pointer;
    FILE * file;
} TBuffer;

// read
bool Buffer_LoadFile( TBuffer * buf, const char * fileName, unsigned int * crc32 );
void Buffer_Free( TBuffer * buf );
float Buffer_ReadFloat( TBuffer * buf );
int Buffer_ReadInteger( TBuffer * buf );
void Buffer_ReadVector3( TBuffer * buf, TVec3 * v );
void Buffer_ReadVector2( TBuffer * buf, TVector2 * v );
void Buffer_ReadQuaternion( TBuffer * buf, TQuaternion * q );
char Buffer_ReadByte( TBuffer * buf );
TIndex16 Buffer_ReadIndex16( TBuffer * buf );
void Buffer_ReadString( TBuffer * buf, char * buffer );
void Buffer_ReadData( TBuffer * buf, void * out, int size );
// write
bool Buffer_WriteFile( TBuffer * buf, const char * fileName );
void Buffer_WriteFloat( TBuffer * buf, float num );
void Buffer_WriteInteger( TBuffer * buf, int num );
void Buffer_WriteVector3( TBuffer * buf, const TVec3 * vec );
void Buffer_WriteVector2( TBuffer * buf, const TVector2 * vec );
void Buffer_WriteQuaternion( TBuffer * buf, const TQuaternion * quat );
void Buffer_WriteByte( TBuffer * buf, char b );
void Buffer_WriteString( TBuffer * buf, char * str );
void Buffer_WriteData( TBuffer * buf, void * data, int size );

OLDTECH_END_HEADER

#endif