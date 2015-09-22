#include "map.h"
#include "entity.h"
#include "Timer.h"
#include "Player.h"
#include "lightmap.h"
#include "monster.h"

TMap * Map_LoadFromFile( const char * fileName ) {
    TMap * map = Memory_New( TMap );
    
    map->root = Entity_LoadFromFile( fileName );
    
    if( !map->root ){
        Util_RaiseError( "Unable to load %s map", fileName );
    }
    
    map->body = Entity_GetChildByName( map->root, "Polygon" );
    
    if( !map->body ) {
        Util_RaiseError( "Unable to find 'Polygon' entity in map '%s'. It must be in every map loaded in this engine!", fileName );
    }
    
    TCollisionShape * polygonShape = Memory_New( TCollisionShape );
    Shape_PolygonFromSurfaces( polygonShape, &map->body->surfaces );

    TBody * polygonBody = Memory_New( TBody );
    Body_Create( polygonBody, polygonShape );
    Dynamics_AddBody( polygonBody );
    
    TTimer timer; 
    Timer_Create( &timer );
    
    // try to load lightmap from cache    
    bool loadingFailed = false;   
    int surfaceNum = 0;   
    for_each( TSurface, surf, map->root->allSurfaces ) {
        const char * atlasName = Std_Format( "%s/%s_surface%d.lmp", "data/lightmaps/", surf->sourceName, surfaceNum );
        if( !LightmapAtlas_LoadSurfaceAtlases( map->root, surf, atlasName )) {
            loadingFailed = true;
            break;
        }
        surfaceNum++;
    } 
    // generate lightmap
    if( loadingFailed ) {
        int surfaceNum = 0;
        for_each( TSurface, surf, map->root->allSurfaces ) {    
            const char * atlasName = Std_Format( "%s/%s_surface%d.lmp", "data/lightmaps/", surf->sourceName, surfaceNum );
            //Lightmap_BuildForSurface( surf, &surf->onLoadOwner->globalPosition );     
            Lightmap_BuildForSurfaceMultithreaded( surf, &surf->onLoadOwner->globalPosition, surfaceNum, map->root->allSurfaces.size );       
            LightmapAtlas_SaveSurfaceAtlases( surf, atlasName );            
            surfaceNum++;
        }
        Log_Write( "Lightmap generation time: %.2f seconds", Timer_GetElapsedSeconds( &timer ));
    } else {
        Log_Write( "Lightmap loading time: %.2f seconds", Timer_GetElapsedSeconds( &timer ));
    }
    
    //LightProbe_BuildRegularArray( map->root, 3.0f );
    //LightProbe_Calculate();
    TEntity * playerPos = Entity_GetChildByName( map->root, "PlayerPosition" );
    if( playerPos ) {
        player->body.position = playerPos->globalPosition;
    } else {
        player->body.position = Vec3_Zero();
        player->body.position.y = 10;
    }
    
    Monster_Create();

    return map;
}