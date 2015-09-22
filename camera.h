#ifndef _CAMERA_
#define _CAMERA_

#include "matrix4.h"
#include "vector3.h"
#include "renderer.h"

OLDTECH_BEGIN_HEADER

typedef struct TCamera {
    struct TEntity * owner;
    float halfFov;
    float zNear;
    float zFar;
    TVec3 clearColor;
    TMatrix4 viewMatrix;
    TMatrix4 projectionMatrix;
    TMatrix4 tempProjectionMatrix;
    bool inDepthHack;
} TCamera;

extern TCamera * pActiveCamera;

void Camera_Create( TCamera * pCam );
void Camera_MakeCurrent( TCamera * pCam );
void Camera_BuildMatrices( TCamera * cam );
void Camera_EnterDepthHack( TCamera * cam, float depthHack );
void Camera_LeaveDepthHack( TCamera * cam );

OLDTECH_END_HEADER

#endif