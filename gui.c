// Code of GUI written on pure C very fast becomes complicated

#include "gui.h"
#include "renderer.h"
#include "input.h"

TList gGUINodeList;

TGUINode * GUI_CreateNode( void ) {
    TGUINode * node = Memory_New( TGUINode );
    node->visible = true;
    node->color = Vec3_Set( 1.0f, 1.0f, 1.0f );
    List_Add( &gGUINodeList, node );
    return node;
}

TGUINode * GUI_CreateRect( float x, float y, float w, float h, TTexture * tex ) { 
    TGUINode * node = GUI_CreateNode();
    node->rect = Memory_New( TGUIRect );
    node->rect->tex = tex; 
    node->rect->x = x;
    node->rect->y = y;
    node->rect->w = w;
    node->rect->h = h;    
    return node;
}

TGUINode * GUI_CreateText( float x, float y, float fieldWidth, float fieldHeight, const char * text, TFont * font ) {
    TGUINode * node = GUI_CreateNode();
    node->text = Memory_New( TGUIText );
    node->text->text = String_Duplicate( text );
    node->text->fieldWidth = fieldWidth;
    node->text->fieldHeight = fieldHeight;
    node->text->x = x;
    node->text->y = y;
    node->text->font = font;
    node->text->width = 0;
    node->text->height = font->size;
    int length = strlen( text );
    for( int i = 0; i < length; i++ ) {
        node->text->width += font->charMetrics[(unsigned int)text[i]].advanceX;
    }
    return node;
}

TGUINode * GUI_CreateButton( float x, float y, float width, float height, TTexture * backgroundTex, TFont * font, const char * text ) {
    TGUINode * node = GUI_CreateNode();
    node->button = Memory_New( TGUIButton );
    node->button->background = GUI_CreateRect( x, y, width, height, backgroundTex );
    node->button->text = GUI_CreateText( x, y, width, height, text, font );
    GUI_SetTextAlign( node, GUITA_MIDDLE );
    node->button->normalColor = Vec3_Set( 1.0f, 1.0f, 1.0f );
    node->button->pressedColor = Vec3_Set( 0.8f, 0.8f, 0.8f );
    node->button->pickedColor = Vec3_Set( 0.9f, 0.9f, 0.9f );
    node->color = node->button->normalColor;
    return node;
}

TGUINode * GUI_CreateSlider( float x, float y, float width, float height, TTexture * backgroundTex, TTexture * sliderTex, float minValue, float maxValue, TFont * font, const char * text ) {
    TGUINode * node = GUI_CreateNode();
    node->slider = Memory_New( TGUISlider );
    node->slider->minValue = minValue;
    node->slider->maxValue = maxValue;
    node->slider->value = maxValue;
    node->slider->background = GUI_CreateRect( x, y, width, height, backgroundTex );
    node->slider->slider = GUI_CreateRect( width - 10, -4, 10, height + 8, sliderTex );
    GUI_Attach( node->slider->slider, node->slider->background );
    node->slider->text = GUI_CreateText( width + 10, 0, width, height, text, font );
    GUI_Attach( node->slider->text, node->slider->background );
    return node;
}

void GUI_FreeNode( TGUINode * node ) {
    if( node->button ) {
        GUI_FreeNode( node->button->text );
        GUI_FreeNode( node->button->background );
        Memory_Free( node->button );
    }
    if( node->text ) {
        Memory_Free( node->text );
    }
    if( node->rect ) {
        Memory_Free( node->rect );
    }
    Memory_Free( node );
}

void GUI_SetNodeText( TGUINode * node, const char * text ) {
    if( node->text ) {
        Memory_Free( node->text->text );
        node->text->text = String_Duplicate( text );
        node->text->width = 0;
        int length = strlen( text );
        for( int i = 0; i < length; i++ ) {
            node->text->width += node->text->font->charMetrics[(unsigned int)text[i]].advanceX;
        }
    } else if( node->button ) {
        GUI_SetNodeText( node->button->text, text );
    }
}

void GUI_SetTextAlign( TGUINode * node, int textAlign ) {
    if( node->text ) {
        node->text->textAlign = textAlign;
    } else if( node->button ) {
        node->button->text->text->textAlign = textAlign;
    }
}

void GUI_SetButtonOnClick( TGUINode * node, TGUICallback newOnClick ) {
    if( node->button ) {
        node->button->OnClick = newOnClick;
    }
}

void GUI_SetNodeVisible( TGUINode * node, bool visible ) {
    if( node ) {
        node->visible = visible;
        if( node->button ) {
            node->button->text->visible = visible;
            node->button->background->visible = visible;
        } else if( node->slider ){
            node->slider->background->visible = visible;
            node->slider->slider->visible = visible;
        }
    }
}

void GUI_Attach( TGUINode * node, TGUINode * parent ) {    
    if( parent ) {
        node->parent = parent;
        List_Add( &parent->childs, node );
        // special attachment for compound objects
        if( node->button ){
            GUI_Attach( node->button->background, parent );
            GUI_Attach( node->button->text, parent );
        } else if( node->slider ){
            GUI_Attach( node->slider->background, parent );
        }
    } else { // detach
        if( node->parent ) {
            List_Remove( &node->parent->childs, node );
        }
        node->parent = 0;
    }
}

TVector2 GUI_GetNodeAbsolutePosition( TGUINode * node ) {
    TVector2 position = { 0, 0 };
    if( node->rect ) {
        position.x = node->rect->x;
        position.y = node->rect->y;
    } else if( node->text ){
        position.x = node->text->x;
        position.y = node->text->y;
    } else if( node->button ) {
        position.x = node->button->background->rect->x;
        position.y = node->button->background->rect->y;
    } else if( node->slider ){
        position.x = node->slider->background->rect->x;
        position.y = node->slider->background->rect->y;
    }
    if( node->parent ) {
        TVector2 parentPosition = GUI_GetNodeAbsolutePosition( node->parent );
        position.x += parentPosition.x;
        position.y += parentPosition.y;
    }
    return position;
}

bool GUI_IsNodeVisible( TGUINode * node ) {
    bool visible = node->visible;
    if( node->parent ) {
        visible &= GUI_IsNodeVisible( node->parent );
    }
    return visible;
}

bool GUI_MouseInsideRectangle( int x, int y, int w, int h ){
    int mouseX = Input_GetMouseX();
    int mouseY = Input_GetMouseY();    
    return (mouseX >= x) && (mouseX <= (x + w)) &&
           (mouseY >= y) && (mouseY <= (y + h));
}

void GUI_Render( void ) {     
    for_each( TGUINode, node, gGUINodeList ) { 
        if( GUI_IsNodeVisible( node ) ) {
            TVector2 absPos = GUI_GetNodeAbsolutePosition( node );
            // render and update rectangles
            if( node->rect ) {                
                TGUIRect * pRect = node->rect;
                Renderer_BindTexture( pRect->tex, 0 );
                Renderer_RenderRect( absPos.x, absPos.y, absPos.x + pRect->w, absPos.y + pRect->h, node->color );
            // render and update texts
            } else if( node->text ) {    
                TGUIText * pText = node->text;
                Renderer_BindTexture( pText->font->atlas, 0 );                
                int caretX = absPos.x;
                int caretY = absPos.y;
                int length = strlen( pText->text );
                if( pText->textAlign == GUITA_MIDDLE ) {
                    caretX += (pText->fieldWidth - pText->width) / 2;
                    caretY += (pText->fieldHeight - pText->font->size ) / 4;
                }
                int nextWordPosition = -1; // for wordbreak
                for( int i = 0; i < length; i++ ) {
                    unsigned char symbol = pText->text[i];
                    TCharMetrics * symbolMetrics = &pText->font->charMetrics[symbol];
                    // handle wordbreak
                    int wordWidth = 0;
                    if( pText->wordBreak ) {
                        if( i >= nextWordPosition ) {
                            int wordBegin = -1;
                            int wordEnd = -1;
                            bool wordFound = false;
                            int j = i;
                            for( ; j < length; j++ ) {
                                if( isalpha( (unsigned int)pText->text[j] )) {
                                    if( wordBegin < 0 ) {
                                        wordBegin = j;
                                        wordFound = true;
                                    }
                                } else {
                                    if( wordFound ) {
                                        wordEnd = j;
                                        break;
                                    }
                                }
                            }
                            if( wordFound ) {
                                if( wordEnd < 0 ) {
                                    wordEnd = j;
                                }
                                nextWordPosition = wordEnd;
                            }
                            if( wordFound && (wordBegin >= 0) && (wordEnd > 0)) {
                                for( int k = wordBegin; k < wordEnd; k++ ){
                                    wordWidth += pText->font->charMetrics[(unsigned int)pText->text[k]].advanceX;
                                }
                                wordFound = -1;
                                wordEnd = -1;
                                wordFound = false;
                                if( caretX + wordWidth > absPos.x + pText->fieldWidth ) {
                                    caretX = absPos.x;
                                    caretY += pText->font->size;
                                }
                            }
                        }
                    }
                    if( symbol != '\n' && symbol != '\t' ) {
                        TGlyphRenderInfo ri;
                        ri.x = caretX + symbolMetrics->bitmapLeft;
                        ri.y = caretY - symbolMetrics->bitmapTop + pText->font->size;
                        ri.w = pText->font->size;
                        ri.h = pText->font->size;
                        memcpy( ri.texCoords, symbolMetrics->texCoords, sizeof( TTexCoord ) * 4 );
                        ri.color = node->color;
                        Renderer_RenderGlyph( &ri );
                        caretX += symbolMetrics->advanceX;
                    } else if( symbol == '\t' ) { // four space indentation
                        caretX += pText->font->charMetrics[(unsigned int)' '].advanceX * 4;
                    }
                    if( (caretX > absPos.x + pText->fieldWidth) || symbol == '\n' ) {
                        caretX = absPos.x;
                        caretY += pText->font->size;
                    }
                }         
            // render and update buttons
            } else if( node->button ) {
                TGUIButton * button = node->button;
                TGUIRect * background = button->background->rect;    
                button->background->color = button->normalColor;
                if( GUI_MouseInsideRectangle( absPos.x, absPos.y, background->w, background->h )) { 
                    button->background->color = button->pickedColor;
                    if( Input_IsMouseDown( MB_Left )) {
                        button->background->color = button->pressedColor;
                        // execute events
                        if( button->OnClick ){                
                            button->OnClick();                            
                        }
                    }
                }
            // update slider
            } else if( node->slider ) {  
                TGUISlider * slider = node->slider;
                TGUIRect * sliderRect = slider->slider->rect;
                TVector2 sliderRectAbsPos = GUI_GetNodeAbsolutePosition( slider->slider );
                if( GUI_MouseInsideRectangle( sliderRectAbsPos.x , sliderRectAbsPos.y, sliderRect->w, sliderRect->h )) {
                    if( Input_IsMouseDown( MB_Left )) {
                        sliderRect->x = Input_GetMouseX() - absPos.x - sliderRect->w / 2;
                        if( sliderRect->x < 0 ){
                            sliderRect->x = 0;
                        }
                        if( sliderRect->x > slider->background->rect->w - sliderRect->w ) {
                            sliderRect->x = slider->background->rect->w - sliderRect->w;
                        }
                        slider->value = ((float)sliderRect->x / (float)slider->background->rect->w) * ( slider->maxValue - slider->minValue );
                        if( slider->OnValueChanged ) {
                            slider->OnValueChanged( slider->value );
                        }
                    }
                }             
                
            }
        }
    }      
}