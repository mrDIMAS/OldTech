#include "matrix4.h"

TMatrix4 Matrix4_Identity( void ) {
    return (TMatrix4) { .f[0] = 1.0f, .f[5] = 1.0f, .f[10] = 1.0f, .f[15] = 1.0f };
}

TMatrix4 Matrix4_Transpose( TMatrix4 in ) {
    TMatrix4 out;
    for(int n = 0; n < 16; n++) {
        int i = n/4;
        int j = n%4;
        out.f[n] = in.f[ 4*j + i];
    }
    return out;
}


TMatrix4 Matrix4_SetRotationOrigin( TQuaternion q, TVec3 v ) {
    float s = 2.0f / Quaternion_SqrLength( q );
    float xs = q.x * s,   ys = q.y * s,   zs = q.z * s;
    float wx = q.w * xs,  wy = q.w * ys,  wz = q.w * zs;
    float xx = q.x * xs,  xy = q.x * ys,  xz = q.x * zs;
    float yy = q.y * ys,  yz = q.y * zs,  zz = q.z * zs;
    return (TMatrix4) { .f[0]  = 1.0f - (yy + zz), .f[1]  = xy + wz,          .f[2] = xz - wy,           .f[3] = 0.0f, 			
                        .f[4]  = xy - wz,          .f[5]  = 1.0f - (xx + zz), .f[6] = yz + wx,           .f[7] = 0.0f,
                        .f[8]  = xz + wy,          .f[9]  = yz - wx,          .f[10] = 1.0f - (xx + yy), .f[11] = 0.0f,
                        .f[12] = v.x,              .f[13] = v.y,              .f[14] = v.z,              .f[15] = 1.0f }; 
}

TMatrix4 Matrix4_SetRotation( TQuaternion q ) {
    return Matrix4_SetRotationOrigin( q, Vec3_Set( 0.0f, 0.0f, 0.0f ));
}

TMatrix4 Matrix4_Scale( TVec3 v ) {
    return (TMatrix4) { .f[0] = v.x, .f[5] = v.y, .f[10] = v.z, .f[15] = 1.0f };
}

TMatrix4 Matrix4_Multiply( TMatrix4 a, TMatrix4 b ) {
    TMatrix4 temp = { {0.0f} };
    for( int i = 0; i < 4; i++ ) {
        for( int j = 0; j < 4; j++ ) {            
            for( int k = 0; k < 4; k++ ) {                
                temp.f[i * 4 + j] += a.f[i * 4 + k] * b.f[k * 4 + j];
            }
        }
    }
    return temp;
}

TMatrix4 Matrix4_PerspectiveFov( float fov, float aspect, float zNear, float zFar ) {
    float yMax = zNear * tan( fov );
    float xMax = yMax * aspect;
    return Matrix4_Frustum( -xMax, xMax, -yMax, yMax, zNear, zFar );
}

TMatrix4 Matrix4_Frustum( float left, float right, float bottom, float top, float zNear, float zFar ) {
    float temp = 2.0f * zNear;
    float temp2 = right - left;
    float temp3 = top - bottom;
    float temp4 = zFar - zNear;
    return (TMatrix4) { .f[0] = temp / temp2,             .f[1] = 0.0f,                      .f[2] = 0.0f,                      .f[3] = 0.0f,
                        .f[4] = 0.0f,                     .f[5] = temp / temp3,              .f[6] = 0.0f,                      .f[7] = 0.0f,
                        .f[8] = ( right + left ) / temp2, .f[9] = ( top + bottom ) / temp3,  .f[10] = (-zFar - zNear ) / temp4, .f[11] = -1.0f,
                        .f[12] = 0.0f,                    .f[13] = 0.0f,                     .f[14] = ( -temp * zFar ) / temp4, .f[15] = 0.0f };
}

TMatrix4 Matrix4_Ortho2D( float left, float right, float bottom, float top, float zNear, float zFar ) {
    return (TMatrix4) { .f[0]  = 2.0f / ( right - left ),             .f[1] = 0.0f,                                 .f[2] = 0.0f,                      .f[3] = 0.0f,
                        .f[4]  = 0.0f,                                .f[5] = 2.0f / ( top - bottom ),              .f[6] = 0.0f,                      .f[7] = 0.0f,
                        .f[8]  = 0.0f,                                .f[9] = 0.0f,                                 .f[10] = 1.0f / ( zFar - zNear ),  .f[11] = 0.0f,
                        .f[12] = ( left + right ) / ( left - right ), .f[13] = ( top + bottom ) / ( bottom - top ), .f[14] = zNear / ( zNear - zFar ), .f[15] = 1.0f };
}

TMatrix4 Matrix4_Translation( TVec3 v ) {
    return (TMatrix4) { .f[0] = 1.0f, .f[5] = 1.0f, .f[10] = 1.0f, .f[12] = v.x, .f[13] = v.y, .f[14] = v.z, .f[15] = 1.0f };
}

TMatrix4 Matrix4_LookAt( TVec3 eye, TVec3 look, TVec3 up ) {
    TMatrix4 out = Matrix4_Identity();
    
    TVec3 forwVec = Vec3_Normalize( Vec3_Sub( look, eye ));    
    TVec3 rightVec = Vec3_Normalize( Vec3_Cross( forwVec, up ));
	TVec3 upNorm = Vec3_Normalize( Vec3_Cross( rightVec, forwVec ));
    
    out.f[0] = rightVec.x;
    out.f[4] = rightVec.y;
    out.f[8] = rightVec.z;
	
    out.f[1] = upNorm.x;
    out.f[5] = upNorm.y;
    out.f[9] = upNorm.z;
	
    out.f[2] = -forwVec.x;
    out.f[6] = -forwVec.y;
    out.f[10] = -forwVec.z;

    TMatrix4 translation = Matrix4_Translation( Vec3_Negate( eye ) );
    return Matrix4_Multiply( translation, out );
}


TMatrix4 Matrix4_Inverse( TMatrix4 mat ) {    
    TMatrix4 out;
    const float* a = mat.f;
    float* r = out.f;
    r[0]  =  a[5]*a[10]*a[15] - a[5]*a[14]*a[11] - a[6]*a[9]*a[15] + a[6]*a[13]*a[11] + a[7]*a[9]*a[14] - a[7]*a[13]*a[10];
    r[1]  = -a[1]*a[10]*a[15] + a[1]*a[14]*a[11] + a[2]*a[9]*a[15] - a[2]*a[13]*a[11] - a[3]*a[9]*a[14] + a[3]*a[13]*a[10];
    r[2]  =  a[1]*a[6]*a[15]  - a[1]*a[14]*a[7]  - a[2]*a[5]*a[15] + a[2]*a[13]*a[7]  + a[3]*a[5]*a[14] - a[3]*a[13]*a[6];
    r[3]  = -a[1]*a[6]*a[11]  + a[1]*a[10]*a[7]  + a[2]*a[5]*a[11] - a[2]*a[9]*a[7]   - a[3]*a[5]*a[10] + a[3]*a[9]*a[6];
    r[4]  = -a[4]*a[10]*a[15] + a[4]*a[14]*a[11] + a[6]*a[8]*a[15] - a[6]*a[12]*a[11] - a[7]*a[8]*a[14] + a[7]*a[12]*a[10];
    r[5]  =  a[0]*a[10]*a[15] - a[0]*a[14]*a[11] - a[2]*a[8]*a[15] + a[2]*a[12]*a[11] + a[3]*a[8]*a[14] - a[3]*a[12]*a[10];
    r[6]  = -a[0]*a[6]*a[15]  + a[0]*a[14]*a[7]  + a[2]*a[4]*a[15] - a[2]*a[12]*a[7]  - a[3]*a[4]*a[14] + a[3]*a[12]*a[6];
    r[7]  =  a[0]*a[6]*a[11]  - a[0]*a[10]*a[7]  - a[2]*a[4]*a[11] + a[2]*a[8]*a[7]   + a[3]*a[4]*a[10] - a[3]*a[8]*a[6];
    r[8]  =  a[4]*a[9]*a[15]  - a[4]*a[13]*a[11] - a[5]*a[8]*a[15] + a[5]*a[12]*a[11] + a[7]*a[8]*a[13] - a[7]*a[12]*a[9];
    r[9]  = -a[0]*a[9]*a[15]  + a[0]*a[13]*a[11] + a[1]*a[8]*a[15] - a[1]*a[12]*a[11] - a[3]*a[8]*a[13] + a[3]*a[12]*a[9];
    r[10] =  a[0]*a[5]*a[15]  - a[0]*a[13]*a[7]  - a[1]*a[4]*a[15] + a[1]*a[12]*a[7]  + a[3]*a[4]*a[13] - a[3]*a[12]*a[5];
    r[11] = -a[0]*a[5]*a[11]  + a[0]*a[9]*a[7]   + a[1]*a[4]*a[11] - a[1]*a[8]*a[7]   - a[3]*a[4]*a[9]  + a[3]*a[8]*a[5];
    r[12] = -a[4]*a[9]*a[14]  + a[4]*a[13]*a[10] + a[5]*a[8]*a[14] - a[5]*a[12]*a[10] - a[6]*a[8]*a[13] + a[6]*a[12]*a[9];
    r[13] =  a[0]*a[9]*a[14]  - a[0]*a[13]*a[10] - a[1]*a[8]*a[14] + a[1]*a[12]*a[10] + a[2]*a[8]*a[13] - a[2]*a[12]*a[9];
    r[14] = -a[0]*a[5]*a[14]  + a[0]*a[13]*a[6]  + a[1]*a[4]*a[14] - a[1]*a[12]*a[6]  - a[2]*a[4]*a[13] + a[2]*a[12]*a[5];
    r[15] =  a[0]*a[5]*a[10]  - a[0]*a[9]*a[6]   - a[1]*a[4]*a[10] + a[1]*a[8]*a[6]   + a[2]*a[4]*a[9]  - a[2]*a[8]*a[5];
    float det = a[0]*r[0] + a[4]*r[1] + a[8]*r[2] + a[12]*r[3];
    if( fabsf( det ) > 0.00001f ) {
        det = 1.0f / det;
    }
    for( int i = 0; i < 16; i++ ) {
        r[i] *= det;
    }
    return out;
}

TVec3 Matrix4_TransformVector( TMatrix4 mat, TVec3 vec ) {
    return (TVec3) { .x = vec.x * mat.f[0] + vec.y * mat.f[4] + vec.z * mat.f[8]  + mat.f[12],
                     .y = vec.x * mat.f[1] + vec.y * mat.f[5] + vec.z * mat.f[9]  + mat.f[13],
                     .z = vec.x * mat.f[2] + vec.y * mat.f[6] + vec.z * mat.f[10] + mat.f[14] };
}
