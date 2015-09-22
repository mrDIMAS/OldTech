#ifndef _MAT4_
#define _MAT4_

#include "quaternion.h"
#include "vector3.h"

typedef struct TMatrix4 {
    float f[16];   
} TMatrix4;

TMatrix4 Matrix4_Identity( void );
TMatrix4 Matrix4_SetRotationOrigin( TQuaternion q, TVec3 v );
TMatrix4 Matrix4_SetRotation( TQuaternion q );
TMatrix4 Matrix4_Translation( TVec3 v );
TMatrix4 Matrix4_Scale( TVec3 v );
TMatrix4 Matrix4_Multiply( TMatrix4 a, TMatrix4 b );
TMatrix4 Matrix4_PerspectiveFov( float fov, float aspect, float zNear, float zFar );
TMatrix4 Matrix4_LookAt( TVec3 eye, TVec3 look, TVec3 up );
TMatrix4 Matrix4_Frustum( float left, float right, float bottom, float top, float zNear, float zFar );
TMatrix4 Matrix4_Inverse( TMatrix4 mat );
TMatrix4 Matrix4_Transpose( TMatrix4 in );
TMatrix4 Matrix4_Ortho2D( float left, float right, float bottom, float top, float zNear, float zFar );
TVec3 Matrix4_TransformVector( TMatrix4 mat, TVec3 vec );

#endif