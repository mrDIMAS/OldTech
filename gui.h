#ifndef _GUI_
#define _GUI_

#include "Texture.h"
#include "Font.h"
#include "Vector3.h"
#include "vector2.h"

OLDTECH_BEGIN_HEADER

typedef void (*TGUICallback)(void);
typedef void (*TGUICallback1F)(float);

typedef struct TGUIRect {
    float x;
    float y;
    float w;
    float h;
    TTexture * tex;
} TGUIRect;

typedef enum ETextAlign {
    GUITA_LEFT = 0,
    GUITA_MIDDLE
} ETextAlign;

typedef struct TGUIText {
    float x;
    float y;
    float width;
    float height;
    float fieldWidth;
    float fieldHeight;
    char * text;
    TFont * font;
    int textAlign; // any of ETextAlign
    bool wordBreak;
} TGUIText;

// this is compound object
typedef struct TGUIButton {
    TVec3 normalColor;
    TVec3 pickedColor;
    TVec3 pressedColor;
    TGUICallback OnClick;
    TGUICallback OnMouseEnter;
    TGUICallback OnMouseLeave;
    struct TGUINode * text;
    struct TGUINode * background;
} TGUIButton;


// this is compound object
typedef struct TGUISlider {
    struct TGUINode * background;
    struct TGUINode * slider;
    struct TGUINode * text;
    float minValue;
    float maxValue;
    float value;
    float step;
    TGUICallback1F OnValueChanged;
} TGUISlider;

typedef struct TGUINode {
    bool visible; // do not use this field, use GUI_SetNodeVisible
    TVec3 color; // this field can be controlled from other runtime features of the engine, so do not use this field
    TGUIRect * rect; // not null, if this is rect
    TGUIText * text; // not null, if this is text
    TGUIButton * button; // not null, is this is button
    TGUISlider * slider;
    TList childs;
    struct TGUINode * parent;
} TGUINode;

extern TList gGUINodeList;

TGUINode * GUI_CreateNode( void );
TGUINode * GUI_CreateRect( float x, float y, float w, float h, TTexture * tex );
TGUINode * GUI_CreateText( float x, float y, float fieldWidth, float fieldHeight, const char * text, TFont * font );
TGUINode * GUI_CreateButton( float x, float y, float width, float height, TTexture * backgroundTex, TFont * font, const char * text );
TGUINode * GUI_CreateSlider( float x, float y, float width, float height, TTexture * backgroundTex, TTexture * sliderTex, float minValue, float maxValue, TFont * font, const char * text );
void GUI_Attach( TGUINode * node, TGUINode * parent );
void GUI_FreeNode( TGUINode * node );
void GUI_SetNodeText( TGUINode * node, const char * text );
void GUI_SetTextAlign( TGUINode * node, int textAlign );
void GUI_SetNodeVisible( TGUINode * node, bool visible );
void GUI_SetButtonOnClick( TGUINode * node, TGUICallback newOnClick );
void GUI_Render( void );
TVector2 GUI_GetNodeAbsolutePosition( TGUINode * node );
bool GUI_IsNodeVisible( TGUINode * node );

OLDTECH_END_HEADER

#endif