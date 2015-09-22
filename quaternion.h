#ifndef _QUAT_
#define _QUAT_

#include "vector3.h"

struct TMatrix4;

typedef struct {
    float x;
    float y;
    float z;
    float w;
} TQuaternion;

TQuaternion Quaternion_Set( float x, float y, float z, float w );
TQuaternion Quaternion_SetEulerAngles( float yaw, float pitch, float roll );
TQuaternion Quaternion_SetAxisAngle( TVec3 axis, float angle );
TQuaternion Quaternion_Multiply( TQuaternion a, TQuaternion b );
TQuaternion Quaternion_Slerp( TQuaternion a, TQuaternion b, float t );
float Quaternion_Angle( TQuaternion a, TQuaternion b );
float Quaternion_SqrLength( TQuaternion q );
float Quaternion_Dot( TQuaternion a, TQuaternion b );
TQuaternion Quaternion_SetMatrix( struct TMatrix4 mat );

#endif