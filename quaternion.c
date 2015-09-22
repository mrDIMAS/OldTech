#include "quaternion.h"
#include "vector3.h"
#include "matrix4.h"

TQuaternion Quaternion_Set( float x, float y, float z, float w ) {
    return (TQuaternion) { .x = x, .y = y, .z = z, .w = w };
}

TQuaternion Quaternion_SetAxisAngle( TVec3 axis, float angle ) {
    float halfAngle = angle * 0.5f;
    float d = Vec3_Length( axis );
    float s = sin( halfAngle ) / d;
    return (TQuaternion) { .x = axis.x * s, .y = axis.y * s, .z = axis.z * s, .w = cos( halfAngle ) };
}

float Quaternion_Dot( TQuaternion a, TQuaternion b ) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

float Quaternion_Angle( TQuaternion a, TQuaternion b ) {
    float s = sqrt( Quaternion_SqrLength( a ) * Quaternion_SqrLength( b ));
    return acos( Quaternion_Dot( a, b ) / s);
}

TQuaternion Quaternion_Slerp( TQuaternion a, TQuaternion b, float t ) {
    float theta = Quaternion_Angle( a, b );
    if( fabsf( theta ) > 0.00001f ) {
        float d = 1.0f / sin( theta );
        float s0 = sin(( 1.0f - t) * theta);
        float s1 = sin( t * theta );
        if ( Quaternion_Dot( a, b ) < 0) {
            return (TQuaternion) {  .x = ( a.x * s0 + -b.x * s1) * d, 
                                    .y = ( a.y * s0 + -b.y * s1) * d, 
                                    .z = ( a.z * s0 + -b.z * s1) * d, 
                                    .w = ( a.w * s0 + -b.w * s1) * d };
        } else {
            return (TQuaternion) {  .x = ( a.x * s0 + b.x * s1) * d, 
                                    .y = ( a.y * s0 + b.y * s1) * d, 
                                    .z = ( a.z * s0 + b.z * s1) * d, 
                                    .w = ( a.w * s0 + b.w * s1) * d };
        }
    }
    return a;
}

TQuaternion Quaternion_SetMatrix( TMatrix4 mat ) {
	mat = Matrix4_Transpose( mat );
	float trace = mat.f[0] + mat.f[5] + mat.f[10];
	if( trace > 0 ) {
		float s = 0.5f / sqrtf(trace+ 1.0f);
        return (TQuaternion) {  .w = 0.25f / s, 
                                .x = ( mat.f[9] - mat.f[6] ) * s, 
                                .y = ( mat.f[2] - mat.f[8] ) * s, 
                                .z = ( mat.f[4] - mat.f[1] ) * s };
	} else {
		if ( mat.f[0] > mat.f[5] && mat.f[0] > mat.f[10] ) {
			float s = 2.0f * sqrtf( 1.0f + mat.f[0] - mat.f[5] - mat.f[10] );
            return (TQuaternion) {  .w = (mat.f[9] - mat.f[6] ) / s,
                                    .x = 0.25f * s,
                                    .y = (mat.f[1] + mat.f[4] ) / s,
                                    .z = (mat.f[2] + mat.f[8] ) / s };
		} else if (mat.f[5] > mat.f[10]) {
			float s = 2.0f * sqrtf( 1.0f + mat.f[5] - mat.f[0] - mat.f[10]);
            return (TQuaternion) {  .w = (mat.f[2] - mat.f[8] ) / s,
                                    .x = (mat.f[1] + mat.f[4] ) / s,
                                    .y = 0.25f * s,
                                    .z = (mat.f[6] + mat.f[9] ) / s };
		} else {
			float s = 2.0f * sqrtf( 1.0f + mat.f[10] - mat.f[0] - mat.f[5] );
            return (TQuaternion) {  .w = (mat.f[4] - mat.f[1] ) / s,
                                    .x = (mat.f[2] + mat.f[8] ) / s,
                                    .y = (mat.f[6] + mat.f[9] ) / s,
                                    .z = 0.25f * s };
		}
	}
}

TQuaternion Quaternion_SetEulerAngles( float yaw, float pitch, float roll ) {
    float halfYaw = yaw * 0.5f;
    float halfPitch = pitch * 0.5f;
    float halfRoll = roll * 0.5f;
    float cosYaw = cos(halfYaw);
    float sinYaw = sin(halfYaw);
    float cosPitch = cos(halfPitch);
    float sinPitch = sin(halfPitch);
    float cosRoll = cos(halfRoll);
    float sinRoll = sin(halfRoll);
    return (TQuaternion) {  .x = cosRoll * sinPitch * cosYaw + sinRoll * cosPitch * sinYaw,
                            .y = cosRoll * cosPitch * sinYaw - sinRoll * sinPitch * cosYaw,
                            .z = sinRoll * cosPitch * cosYaw - cosRoll * sinPitch * sinYaw,
                            .w = cosRoll * cosPitch * cosYaw + sinRoll * sinPitch * sinYaw };
}

TQuaternion Quaternion_Multiply( TQuaternion a, TQuaternion b ) {
    return (TQuaternion) {  .x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
                            .y = a.w * b.y + a.y * b.w + a.z * b.x - a.x * b.z,
                            .z = a.w * b.z + a.z * b.w + a.x * b.y - a.y * b.x,
                            .w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z };
}

float Quaternion_SqrLength( TQuaternion q ) {
    return q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
}