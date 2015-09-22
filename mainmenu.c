#include "mainmenu.h"
#include "Renderer.h"
#include "Sound.h"
#include "map.h"
#include "player.h"

TMainMenu * menu = NULL;

void MainMenu_StartGame( void ) {
    MainMenu_SetVisible( false );
    MainMenu_HideModalWindow();
    Player_Create( );
    Map_LoadFromFile( "data/maps/cave/cave.scene" );
}

void MainMenu_ExitGame( void ) {
    Renderer_Stop();
}

void Handler_OnStartGameClick( void ) {
    MainMenu_ShowModalWindow( "Do you want to start a new game?", MainMenu_StartGame );    
}

void Handler_OnOptionsClick( void ) {
    GUI_SetNodeVisible( menu->optionsWindow->background, true );
    GUI_SetNodeVisible( menu->authorsWindow->background, false );
}

void Handler_OnAuthorsClick( void ) {
    GUI_SetNodeVisible( menu->authorsWindow->background, true );
}

void Handler_OnExitGameClick( void ) {
    MainMenu_ShowModalWindow( "Are you sure you want to quit?", MainMenu_ExitGame ); 
}

void Handler_OnModalWindowNoClick( void ) {
    GUI_SetNodeVisible( menu->modalWindow->background, false );
}

void MainMenu_HideModalWindow( void ) {
    GUI_SetNodeVisible( menu->modalWindow->background, false );
}

void Handler_OnSoundVolumeChanged( float newVolume ) {
    SoundListener_SetVolume( newVolume );
}

void Handler_OnMusicVolumeChanged( float newVolume ) {
    UNUSED_VARIABLE( newVolume );
}

void MainMenu_SetVisible( bool state ) {
    menu->visible = state;
    GUI_SetNodeVisible( menu->mainBackground, menu->visible );
}

void MainMenu_Create( void ) {
    menu = Memory_New( TMainMenu );    
    menu->smallFont = Font_LoadFromFile( "data/fonts/font3.ttf", 12 );
    menu->largeFont = Font_LoadFromFile( "data/fonts/font3.ttf", 16 );
    menu->mainBackground = GUI_CreateRect( 10, Renderer_GetWindowHeight() - 160, 130, 140, Texture2D_LoadFromFile( "data/textures/gui/window.tga" ) );
    // start game button
    menu->startButton = GUI_CreateButton( 0, 0, 128, 32, Texture2D_LoadFromFile( "data/textures/gui/button.tga" ), menu->smallFont, "Start" );
    GUI_SetButtonOnClick( menu->startButton, Handler_OnStartGameClick );
    GUI_Attach( menu->startButton, menu->mainBackground );
    // options button
    menu->optionsButton = GUI_CreateButton( 0, 36, 128, 32, Texture2D_LoadFromFile( "data/textures/gui/button.tga" ), menu->smallFont, "Options" );
    GUI_SetButtonOnClick( menu->optionsButton, Handler_OnOptionsClick );
    GUI_Attach( menu->optionsButton, menu->mainBackground );
    // authors button
    menu->authorsButton = GUI_CreateButton( 0, 72, 128, 32, Texture2D_LoadFromFile( "data/textures/gui/button.tga" ), menu->smallFont, "Authors" );
    GUI_SetButtonOnClick( menu->authorsButton, Handler_OnAuthorsClick );
    GUI_Attach( menu->authorsButton, menu->mainBackground );
    // exit game button
    menu->exitButton = GUI_CreateButton( 0, 108, 128, 32, Texture2D_LoadFromFile( "data/textures/gui/button.tga" ), menu->smallFont, "Exit" );
    GUI_SetButtonOnClick( menu->exitButton, Handler_OnExitGameClick );
    GUI_Attach( menu->exitButton, menu->mainBackground );
    
    // create modal window 
    int modalWidth = 350;
    int modalHeight = 200;
    menu->modalWindow = Memory_New( TModalWindow );
    menu->modalWindow->background = GUI_CreateRect( (Renderer_GetWindowWidth() - modalWidth) / 2, (Renderer_GetWindowHeight() - modalHeight) / 2, modalWidth, modalHeight, Texture2D_LoadFromFile( "data/textures/gui/window.tga" ));
    //GUI_Attach( menu->modalWindow->background, menu->mainBackground );
    GUI_SetNodeVisible( menu->modalWindow->background, false );
    // yes button
    menu->modalWindow->yesButton = GUI_CreateButton( 10, modalHeight - 36, 128, 32, Texture2D_LoadFromFile( "data/textures/gui/button.tga" ), menu->smallFont, "Yes" );
    GUI_Attach( menu->modalWindow->yesButton, menu->modalWindow->background );
    // no button
    menu->modalWindow->noButton = GUI_CreateButton( modalWidth - 138, modalHeight - 36, 128, 32, Texture2D_LoadFromFile( "data/textures/gui/button.tga" ), menu->smallFont, "No" );
    GUI_Attach( menu->modalWindow->noButton, menu->modalWindow->background );
    GUI_SetButtonOnClick( menu->modalWindow->noButton, Handler_OnModalWindowNoClick );
    // text
    menu->modalWindow->text = GUI_CreateText( 20, 40, modalWidth - 40, modalHeight - 120, "Question", menu->smallFont ); 
    GUI_Attach( menu->modalWindow->text, menu->modalWindow->background ); 

    // create options window
    int optWidth = 400;
    int optHeight = 300;
    menu->optionsWindow = Memory_New( TOptionsWindow );
    menu->optionsWindow->background = GUI_CreateRect( (Renderer_GetWindowWidth() - optWidth) / 2, (Renderer_GetWindowHeight() - optHeight) / 2, optWidth, optHeight, Texture2D_LoadFromFile( "data/textures/gui/window.tga" ) );
    //GUI_Attach( menu->optionsWindow->background, menu->mainBackground );
    GUI_SetNodeVisible( menu->optionsWindow->background, false );
    menu->optionsWindow->soundVolumeSlider = GUI_CreateSlider( 20, 20, 128, 24, Texture2D_LoadFromFile( "data/textures/gui/button.tga" ), Texture2D_LoadFromFile( "data/textures/gui/slider.tga" ), 0, 1, menu->smallFont, "Music volume" );   
    menu->optionsWindow->soundVolumeSlider->slider->OnValueChanged = Handler_OnSoundVolumeChanged;
    GUI_Attach( menu->optionsWindow->soundVolumeSlider, menu->optionsWindow->background );    
    menu->optionsWindow->musicVolumeSlider = GUI_CreateSlider( 20, 56, 128, 24, Texture2D_LoadFromFile( "data/textures/gui/button.tga" ), Texture2D_LoadFromFile( "data/textures/gui/slider.tga" ), 0, 1, menu->smallFont, "Sound volume"  );   
    menu->optionsWindow->musicVolumeSlider->slider->OnValueChanged = Handler_OnMusicVolumeChanged;
    GUI_Attach( menu->optionsWindow->musicVolumeSlider, menu->optionsWindow->background );
    
    // create authors window
    int authWidth = 400;
    int authHeight = 300;
    const char * authText = "OldTech Engine Programming: mrDIMAS\nLevel design: mrDIMAS\nGame design: mrDIMAS\n";
    menu->authorsWindow = Memory_New( TAuthorsWindow );
    menu->authorsWindow->background = GUI_CreateRect( (Renderer_GetWindowWidth() - authWidth) / 2, (Renderer_GetWindowHeight() - authHeight) / 2, authWidth, authHeight, Texture2D_LoadFromFile( "data/textures/gui/window.tga" ) );
    GUI_SetNodeVisible( menu->authorsWindow->background, false );
    menu->authorsWindow->text = GUI_CreateText( 10, 10, authWidth - 20, authHeight - 20, authText, menu->smallFont );
    GUI_Attach( menu->authorsWindow->text, menu->authorsWindow->background );
}

void MainMenu_ShowModalWindow( const char * question, TGUICallback yesAction ) {
    GUI_SetNodeText( menu->modalWindow->text, question );
    GUI_SetButtonOnClick( menu->modalWindow->yesButton, yesAction );
    GUI_SetNodeVisible( menu->modalWindow->background, true );
    GUI_SetNodeVisible( menu->optionsWindow->background, false );
    GUI_SetNodeVisible( menu->authorsWindow->background, false );
}

void MainMenu_Free( void ) {
    GUI_FreeNode( menu->startButton );
    GUI_FreeNode( menu->exitButton );
    GUI_FreeNode( menu->authorsButton );
    GUI_FreeNode( menu->optionsButton );
    GUI_FreeNode( menu->mainBackground );
    Memory_Free( menu );
}