#ifndef _MAIN_MENU_
#define _MAIN_MENU_

#include "font.h"
#include "gui.h"

typedef struct TModalWindow {
    TGUINode * background;
    TGUINode * text;
    TGUINode * yesButton;
    TGUINode * noButton;
} TModalWindow;

typedef struct TOptionsWindow {
    TGUINode * background;
    TGUINode * musicVolumeSlider;
    TGUINode * soundVolumeSlider;
} TOptionsWindow;

typedef struct TAuthorsWindow {
    TGUINode * background;
    TGUINode * text;
} TAuthorsWindow;

typedef struct TMainMenu {
    bool visible;
    TFont * smallFont;
    TFont * largeFont;
    TGUINode * mainBackground;
    TGUINode * startButton;
    TGUINode * optionsButton;
    TGUINode * authorsButton;
    TGUINode * exitButton;
    TModalWindow * modalWindow;
    TOptionsWindow * optionsWindow;
    TAuthorsWindow * authorsWindow;
} TMainMenu;

extern TMainMenu * menu;

void MainMenu_Create( void );
void MainMenu_Free( void );
void MainMenu_SetVisible( bool state );
void MainMenu_ShowModalWindow( const char * question, TGUICallback yesAction );
void MainMenu_HideModalWindow( void );

#endif