#include "renderer.h"
#include "vector3.h"
#include "surface.h"
#include "entity.h"
#include "list.h"
#include "camera.h"
#include "input.h"
#include "player.h"
#include "sound.h"
#include "timer.h"
#include "parser.h"
#include "str.h"
#include "lightmap.h"
#include "map.h"
#include "script.h"
#include "array.h"
#include "gui.h"
#include "font.h"
#include "mainmenu.h"
#include "monster.h"
TGUINode * testRect = 0;

void TestHandler( void ) {
    testRect->visible = !testRect->visible;
}


int main( int argc, char * argv[] ) {
    Test_Array();
    
    UNUSED_VARIABLE( argc );
    UNUSED_VARIABLE( argv );

    ScriptSystem_Initialize();
    Script_ExecuteFile( "config.lua" );
        
    TRenderSettings rs;  
    rs.fullscreen = Script_GetGlobalBoolean( "modeFullscreen" );
    rs.bpp = Script_GetGlobalNumber( "modeColorDepth" );
    rs.width = Script_GetGlobalNumber( "modeWidth" );
    rs.height = Script_GetGlobalNumber( "modeHeight" );
    
    Renderer_InitializeFull( &rs );
    
    TSoundSystem soundSystem;
    SoundSystem_Initialize( &soundSystem );
    
    Dynamics_CreateWorld();

    TTimer perfTimer;
    Timer_Create( &perfTimer );

    TTimer fpsTimer;
    Timer_Create( &fpsTimer );

    float gameLogicTime = 0;
    int fps = 0, fpsCounter = 0;

    TTimer gameTimer;
    Timer_Create( &gameTimer );

    double fixedFPS = 60.0;
    double fixedTimeStep = 1.0 / fixedFPS;
    double gameClock = Timer_GetSeconds( &gameTimer );

    Input_SetPointerVisible( true );    
    Script_ExecuteFile( "autoexec.lua" );        
    
    TFont * font = Font_LoadFromFile( "data/fonts/font3.ttf", 14 );
    TGUINode * fpsText = GUI_CreateText( 0, 0, 200, 200, "FPS Counter", font );
    
    MainMenu_Create();
    
    while( true ) {
        if( !Renderer_IsRunning()) {
            break;
        }     
        
        if( Input_IsKeyHit( KEY_Esc ) ) {
            MainMenu_SetVisible( !menu->visible );
        }
           
        Renderer_BeginRender();
		Renderer_RenderWorld( );
		
        double dt = Timer_GetSeconds( &gameTimer ) - gameClock;
        
        Timer_Restart( &perfTimer );
        while( dt >= fixedTimeStep ) {
            dt -= fixedTimeStep;
            gameClock += fixedTimeStep;
            
            Input_Flush(); 
            Renderer_PollWindowMessages( ); 
            
            if( !menu->visible ) {
                Dynamics_StepSimulation();                                                 
                Player_Update( fixedTimeStep );
                Monster_ThinkAll();
                World_Update();
            }
            
            if( Input_IsKeyHit( KEY_1 )) {
                Renderer_SetTextureFiltration( TF_LINEAR );
            } 
            if( Input_IsKeyHit( KEY_2 )) {
                Renderer_SetTextureFiltration( TF_ANISOTROPIC );
            }
            Script_CallFunction( "Main_FixedTimeUpdate", "" );
            
            gameLogicTime += Timer_GetElapsedMilliseconds( &perfTimer );
        }
        GUI_Render();
        Renderer_EndRender( );
 
        if( Timer_GetElapsedSeconds( &fpsTimer ) >= 1.0 ) {
            fps = fpsCounter;
            GUI_SetNodeText( fpsText, Std_Format( "FPS: %d\nPRT:%.2f ms", fps, gameLogicTime / fixedFPS ));
            Timer_Restart( &fpsTimer );
            gameLogicTime = 0;
            fpsCounter = 0;
        }

        fpsCounter++;
    };
    ScriptSystem_Shutdown();
    Monster_FreeAll( );
    Player_Free( );
    Entity_FreeAll( );
    SoundSystem_Free( &soundSystem );
    Renderer_Shutdown();
    Log_Write( "Dynamic memory still allocated: %d bytes... Collecting garbage...", Memory_GetAllocated() );
    // memory cleanup 
	Log_Close( &g_log );
    Memory_CollectGarbage();	
    printf( " Done!" );
    return 0;
}
