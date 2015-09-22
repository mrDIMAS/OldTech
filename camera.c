#include "camera.h"
#include "entity.h"

TCamera * pActiveCamera = 0;

void Camera_Create( TCamera * pCam ) {
    pCam->halfFov = 40.0f * M_PI / 180.0f;
    pCam->zFar = 4096.0f;
    pCam->zNear = 0.025f;
    pCam->inDepthHack = false;
    pCam->owner = NULL;
    pCam->viewMatrix = Matrix4_Identity();
    pCam->projectionMatrix = Matrix4_Identity();
    pCam->clearColor = Vec3_Set( 0.25f, 0.25f, 0.255f );
    Camera_MakeCurrent( pCam );
}

void Camera_MakeCurrent( TCamera * pCam ) {
    pActiveCamera = pCam;
}

void Camera_BuildMatrices( TCamera * cam ) {
	TEntity * owner = cam->owner;
    if( owner ) {		
        Entity_CalculateGlobalTransform( owner );
        cam->projectionMatrix = Matrix4_PerspectiveFov( cam->halfFov, Renderer_GetWindowAspectRatio(), cam->zNear, cam->zFar );
        TVec3 eye = Entity_GetGlobalPosition( owner );
        TVec3 up = Entity_GetUpVector( owner );
        TVec3 lookAt = Vec3_Add( eye, Entity_GetLookVector( owner ));
        cam->viewMatrix = Matrix4_LookAt( eye, lookAt, up );
    }
}

void Camera_EnterDepthHack( TCamera * cam, float depthHack ) {    
    if( !cam->inDepthHack ) {
        // save current projection matrix
        cam->tempProjectionMatrix = cam->projectionMatrix;        
        // modify projection matrix
        cam->projectionMatrix.f[14] -= depthHack;        
        cam->inDepthHack = true;
    }
}

void Camera_LeaveDepthHack( TCamera * cam ) {
    cam->inDepthHack = false;
    // restore projection matrix
    cam->projectionMatrix = cam->tempProjectionMatrix;
}