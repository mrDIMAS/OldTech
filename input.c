#include "input.h"

//#define _WIN32_WINNT 0x0501 // win xp
#include <windows.h>
#include <winuser.h>

#define KEYS_COUNT (256)

typedef struct {
    bool keys[ KEYS_COUNT ];
    bool lastKeys[ KEYS_COUNT ];
    bool keysHit[ KEYS_COUNT ];
    bool keysUp[ KEYS_COUNT ];
} TKeyboard;

typedef struct {
    bool leftButtonUp;
    bool leftButtonHit;
    bool leftButtonPressed;
    bool leftButtonLastState;    
    
    bool middleButtonUp;
    bool middleButtonHit;
    bool middleButtonPressed;
    bool middleButtonLastState;
    
    bool rightButtonUp;
    bool rightButtonHit;
    bool rightButtonPressed;
    bool rightButtonLastState;

    int x;
    int y;
    int xSpeed;
    int ySpeed;
    
    int wheel;
    int wheelSpeed;
} TMouse;

TKeyboard gKeyboard = { {0}, {0}, {0}, {0} };
TMouse gMouse = { 0 };

void Input_FlushMouse( void ) {
    gMouse.leftButtonUp = false;
    gMouse.leftButtonHit = false;
    gMouse.leftButtonLastState = false;
    gMouse.leftButtonPressed = false;
  
    gMouse.rightButtonUp = false;
    gMouse.rightButtonHit = false;
    gMouse.rightButtonLastState = false;
    gMouse.rightButtonPressed = false;

    gMouse.middleButtonUp = false;
    gMouse.middleButtonHit = false;
    gMouse.middleButtonLastState = false;  
    gMouse.middleButtonPressed = false;
}

void Input_HandleMessage( void * window, int message, int lParam, int wParam ) {
#ifdef _WIN32
    switch( message ) {
        case WM_KEYDOWN: {
            gKeyboard.keys[ (lParam & 0x01FF0000) >> 16 ] = true;
            break;
        }
        case WM_KEYUP: {
            gKeyboard.keys[ (lParam & 0x01FF0000) >> 16  ] = false;            
            break;
        }
        case WM_MOUSEMOVE: {
            gMouse.x = LOWORD( lParam );
            gMouse.y = HIWORD( lParam );
            RECT clientRect;
            GetClientRect( window, &clientRect );
            ClipCursor( &clientRect );
            break;
        }        
        case WM_LBUTTONDOWN: {
            gMouse.leftButtonPressed = true;            
            break;
        }
        case WM_LBUTTONUP: {
            gMouse.leftButtonPressed = false;
            break;
        }
        case WM_RBUTTONDOWN: {
            gMouse.rightButtonPressed = true;  
            break;
        }
        case WM_RBUTTONUP: {
            gMouse.rightButtonPressed = false;
            break;
        }
        case WM_MBUTTONDOWN: {
            gMouse.middleButtonPressed = true;
            break;
        }
        case WM_MBUTTONUP: {
            gMouse.middleButtonPressed = false;
            break;
        }
        case WM_MOUSEWHEEL: {
            gMouse.wheelSpeed = ((short)HIWORD( wParam )) / WHEEL_DELTA;
            gMouse.wheel += gMouse.wheelSpeed;
            break;
        }
        case WM_NCMOUSEMOVE:
        case WM_NCMOUSELEAVE:
        case WM_MOUSELEAVE: {
            Input_FlushMouse();
            break;
        }
        case WM_INPUT:    {   
            char inputBuffer[ sizeof( RAWINPUT ) ] = { 0 };
            UINT dwSize = sizeof( RAWINPUT );        
            GetRawInputData((HRAWINPUT)lParam, RID_INPUT, inputBuffer, &dwSize, sizeof(RAWINPUTHEADER));
            RAWINPUT* raw = (RAWINPUT*)inputBuffer;
            if (raw->header.dwType == RIM_TYPEMOUSE) {
                gMouse.xSpeed += raw->data.mouse.lLastX;
                gMouse.ySpeed += raw->data.mouse.lLastY;
            }
            break;
        }
    }
#endif
}

void Input_SetPointerVisible( bool state ) {
    ShowCursor( state );
}

void Input_Flush( ) {    
    gMouse.leftButtonUp = gMouse.leftButtonPressed & ~gMouse.leftButtonLastState;
    gMouse.leftButtonHit = ~gMouse.leftButtonPressed & gMouse.leftButtonLastState;
    gMouse.leftButtonLastState = gMouse.leftButtonPressed;
  
    gMouse.rightButtonUp = gMouse.rightButtonPressed & ~gMouse.rightButtonLastState;
    gMouse.rightButtonHit = ~gMouse.rightButtonPressed & gMouse.rightButtonLastState;
    gMouse.rightButtonLastState = gMouse.rightButtonPressed;

    gMouse.middleButtonUp = gMouse.middleButtonPressed & ~gMouse.middleButtonLastState;
    gMouse.middleButtonHit = ~gMouse.middleButtonPressed & gMouse.middleButtonLastState;
    gMouse.middleButtonLastState = gMouse.middleButtonPressed;  
    
    for( int i = 0; i < KEYS_COUNT; ++i ) {
        gKeyboard.keysHit[ i ] = gKeyboard.keys[ i ] & ~gKeyboard.lastKeys[ i ];
        gKeyboard.keysUp[ i ] = ~gKeyboard.keys[ i ] & gKeyboard.lastKeys[ i ];
    }
    
    memcpy( gKeyboard.lastKeys, gKeyboard.keys, KEYS_COUNT );
    
    gMouse.xSpeed = 0;
    gMouse.ySpeed = 0;
    gMouse.wheelSpeed = 0;
}

int Input_IsKeyDown( int key ) {
    return gKeyboard.keys[ key ];
}

int Input_IsKeyHit( int key ) {
    return gKeyboard.keysHit[ key ];
}

int	Input_IsKeyUp( int key ) {
    return gKeyboard.keysUp[ key ];
}

int Input_IsMouseDown( int button ) {
    switch( button ) {
        case MB_Left: {
            return gMouse.leftButtonPressed;
            break;
        }
        case MB_Middle: {
            return gMouse.middleButtonPressed;
            break;
        }        
        case MB_Right: {
            return gMouse.rightButtonPressed;
            break;
        }
        default: {
            return 0;
        }
    } 
}

int Input_IsMouseHit( int button ) {
    switch( button ) {
        case MB_Left: {
            return gMouse.leftButtonHit;
            break;
        }
        case MB_Middle: {
            return gMouse.middleButtonHit;
            break;
        }        
        case MB_Right: {
            return gMouse.rightButtonHit;
            break;
        }
        default: {
            return 0;
        }
    }    
}

int Input_GetMouseX( ) {
    return gMouse.x;
}

int Input_GetMouseY( ) {
    return gMouse.y;
}

int Input_GetMouseWheel( ) {
    return gMouse.wheel;
}

int Input_GetMouseXSpeed( ) {
    return gMouse.xSpeed;
}

int Input_GetMouseYSpeed( ) {
    return gMouse.ySpeed;
}

int Input_GetMouseWheelSpeed( ) {
    return 0;
}
