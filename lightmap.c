// This file contains main lightmap generation algorithms, and some very useful 
// optimizations, such as multithreaded generation and precalculation of barycentric
// coefficients. Multithreading speedup generation at least 3 times on 2-core CPU 
// (tested on Intel Atom N570 @ 1.66 GHz). 
//
// Currently few bugs are present: 
//      1) Black seams appearing on a face edge due to bilinear filtration on GPU
//      2) Data-race between threads can cause very rare bugs, such as crash of 
//         generation on a heavy maps (>60k polygons), this bugs is hard to 
//         reproduce, so any help would be appreciated
//      3) Shadows generating using raytracing from collision detection engine
//      4) Alpha channel of diffuse texture is ignored
//
// Further improvements:
//      1) Ambient occlusion can be added very simply
//      2) Bump-mapping 
//      3) Move generation on GPU (OpenCL, DirectCompute)
//  
// mrDIMAS 2015
//

#include "Lightmap.h"
#include "Entity.h"
#include "Buffer.h"
#include "Thread.h"
#include "Timer.h"

TList gLightmapAtlasList = { NULL, NULL, 0 };
TList gLightProbeList = { NULL, NULL, 0 };

void Barycentric_Calculate2DFast( const TUVTriangle * precalcBary, const TVector2 * p, const TVector2 * a, TBarycentricCoord * out ) {
    TVector2 v2;
    Vector2_Subtract( &v2, p, a );        
    float d20 = Vector2_Dot(&v2, &precalcBary->v0);
    float d21 = Vector2_Dot(&v2, &precalcBary->v1);
    out->v = (precalcBary->d11 * d20 - precalcBary->d01 * d21) / precalcBary->denom;
    out->w = (precalcBary->d00 * d21 - precalcBary->d01 * d20) / precalcBary->denom;
    out->u = 1.0f - out->v - out->w;
}

void Barycentric_Calculate3D( const TVec3 * p, const TVec3 * a, const TVec3 * b, const TVec3 * c, TBarycentricCoord * out ) {
    TVec3 v0 = Vec3_Sub( *b, *a );    
    TVec3 v1 = Vec3_Sub( *c, *a );    
    TVec3 v2 = Vec3_Sub( *p, *a );
    
    float d00 = Vec3_Dot(v0, v0);
    float d01 = Vec3_Dot(v0, v1);
    float d11 = Vec3_Dot(v1, v1);
    float d20 = Vec3_Dot(v2, v0);
    float d21 = Vec3_Dot(v2, v1);
    float denom = d00 * d11 - d01 * d01;
    out->v = (d11 * d20 - d01 * d21) / denom;
    out->w = (d00 * d21 - d01 * d20) / denom;
    out->u = 1.0f - out->v - out->w;
}

void Barycentric_MapToWorld( TVec3 * out, const TVec3 * a, const TVec3 * b, const TVec3 * c, const TBarycentricCoord * bary ) {            
    out->x = a->x * bary->u + b->x * bary->v + c->x * bary->w;
    out->y = a->y * bary->u + b->y * bary->v + c->y * bary->w;
    out->z = a->z * bary->u + b->z * bary->v + c->z * bary->w; 
}

void Triangle_CalculateNormalUnnormalized( TVec3 * normal, const TVec3 * a, const TVec3 * b, const TVec3 * c ) {
    *normal = Vec3_Cross( Vec3_Sub( *b, *a ), Vec3_Sub( *c, *a ) );
}

void ProjectPointOntoPlane( TVec3 * projected, const TVec3 * point, const TVec3 * planePoint, const TVec3 * planeNormal ) {
    float d = planePoint->x * planeNormal->x + planePoint->y * planeNormal->y + planePoint->z * planeNormal->z;
    float a = point->x * planeNormal->x + point->y * planeNormal->y + point->z * planeNormal->z - d;
    projected->x = point->x - a * planeNormal->x;
    projected->y = point->y - a * planeNormal->y;
    projected->z = point->z - a * planeNormal->z;
}

unsigned long UpperPow2(unsigned long v) {
    int power = 2;
    while (v >>= 1) {
        power <<= 1;
    }
    return power;
}

int GetPlaneWithLongestNormal( const TVec3 * triangleNormal ) {   
    float longest = 0.0f;

    int plane = PLANE_ARBITRARY;
    
    if( fabsf( triangleNormal->x ) > longest ) {
        longest = fabsf( triangleNormal->x );
        plane = PLANE_OYZ;       
    }
  
    if( fabsf( triangleNormal->y ) > longest ) {
        longest = fabsf( triangleNormal->y );
        plane = PLANE_OXZ;           
    }
 
    if( fabsf( triangleNormal->z ) > longest ) {
        plane = PLANE_OXY;                
    }
    
    return plane;
}

static const TVec3 planePoint = { 0.0f, 0.0f, 0.0f };
static const TVec3 oXYNormal = { 0.0f, 0.0f, 1.0f };
static const TVec3 oXZNormal = { 0.0f, 1.0f, 0.0f };
static const TVec3 oYZNormal = { 1.0f, 0.0f, 0.0f };    
int ProjectPointOntoOrthoPlane( int plane, TVec3 * projected, const TVec3 * point ) {    
    if( plane == PLANE_OXY ) {
        ProjectPointOntoPlane( projected, point, &planePoint, &oXYNormal );
    } else if( plane == PLANE_OXZ ) {
        ProjectPointOntoPlane( projected, point, &planePoint, &oXZNormal );
    } else if( plane == PLANE_OYZ ) {
        ProjectPointOntoPlane( projected, point, &planePoint, &oYZNormal );
    } else {
        *projected = Vec3_Zero();
    }    
    return plane;
}

// simple planar-mapping algorithm
void Lightmap_Map3DPointTo2DByPlane( int plane, const TVec3 * point, TVector2 * mapped ) {
    if( plane == PLANE_OXY ) {
        mapped->x = point->x;
        mapped->y = point->y;
    } else if( plane == PLANE_OXZ ) {
        mapped->x = point->x;
        mapped->y = point->z;              
    } else if( plane == PLANE_OYZ ) {
        mapped->x = point->z;
        mapped->y = point->y;
    } else {
        mapped->x = 0.0f;
        mapped->y = 0.0f;
    }    
}

void Lightmap_Blur( TLightmap * lm, const int borderSize ) {
    const int radius = 1;
    const int halfBorder = borderSize / 2;
    for_each( TLightmapLayer, layer, lm->layers ) {
        for( int y = halfBorder; y < lm->height - halfBorder; ++y ) {
            for( int x = halfBorder; x < lm->width - halfBorder; ++x ) {
                int totalRed = 0, totalGreen = 0, totalBlue = 0;
                float denom = 0;
                for( int ky = -radius; ky <= radius; ++ky) {
                    for( int kx = -radius; kx <= radius; ++kx) {
                        if( (y + ky < (lm->height - halfBorder)) && 
                            (x + kx < (lm->width - halfBorder)) &&
                            (y + ky > halfBorder ) && ( x + kx > halfBorder )) 
                        {
                            int index = ( y + ky ) * lm->width + ( x + kx );
                            totalRed += layer->pixels[index].r;                 
                            totalGreen += layer->pixels[index].g;
                            totalBlue += layer->pixels[index].b;   
                            denom += 1.0f;
                        }
                    }
                }                          
                totalRed /= denom;
                totalGreen /= denom;
                totalBlue /= denom;
                if( totalRed > 255 ) totalRed = 255;
                if( totalGreen > 255 ) totalGreen = 255;
                if( totalBlue > 255 ) totalBlue = 255;
                int index = y * lm->width + x;
                layer->pixels[index].r = totalRed;                 
                layer->pixels[index].g = totalGreen;
                layer->pixels[index].b = totalBlue;
            }
        }
    }
}

float Lightmap_CalculateAttenuation( const TVec3 * src, const TVec3 * dst, float radius, TVec3 * outDir ) {
    float distance = 0.0f; 
    TVec3 direction = Vec3_NormalizeEx( Vec3_Sub( *src, *dst ), &distance );                                 
    float linearCoeff = (2.0f * distance) / radius;
    float quadCoeff = distance * distance / ( radius * radius);
    if( outDir ) {
        (*outDir) = direction;
    }
    return 1.0f / ( 1.0f + linearCoeff + quadCoeff );
}

void Lightmap_Build( const TVec3 * offset, TLightmap * lm, TVertex * a, TVertex * b, TVertex * c, int faceID, int threadNum ) {
    lm->a = a;
    lm->b = b;
    lm->c = c;

    lm->faceID = faceID;
    
    // generate lightmap texture coordinates by projecting triangle on a proper plane, aligned on axes ( such oXY and so on )
    TVec3 triangleNormal;
    Triangle_CalculateNormalUnnormalized( &triangleNormal, &a->p, &b->p, &c->p );     
    
    int plane = GetPlaneWithLongestNormal( &triangleNormal );
    
    TVec3 aProjected, bProjected, cProjected;
    ProjectPointOntoOrthoPlane( plane, &aProjected, &a->p );
    ProjectPointOntoOrthoPlane( plane, &bProjected, &b->p );
    ProjectPointOntoOrthoPlane( plane, &cProjected, &c->p );
    
    TVector2 uvA, uvB, uvC;
    Lightmap_Map3DPointTo2DByPlane( plane, &aProjected, &uvA );
    Lightmap_Map3DPointTo2DByPlane( plane, &bProjected, &uvB );
    Lightmap_Map3DPointTo2DByPlane( plane, &cProjected, &uvC );
       
    float uMax = -999999.0f;
    float uMin = 999999.0f;
    float vMax = -999999.0f;
    float vMin = 999999.0f;    
   
    // select uMax
    if( uvA.x > uMax ) uMax = uvA.x;
    if( uvB.x > uMax ) uMax = uvB.x;
    if( uvC.x > uMax ) uMax = uvC.x;    
    // select vMax
    if( uvA.y > vMax ) vMax = uvA.y;
    if( uvB.y > vMax ) vMax = uvB.y;
    if( uvC.y > vMax ) vMax = uvC.y;    
    // select uMin
    if( uvA.x < uMin ) uMin = uvA.x;
    if( uvB.x < uMin ) uMin = uvB.x;
    if( uvC.x < uMin ) uMin = uvC.x;    
    // select vMin
    if( uvA.y < vMin ) vMin = uvA.y;
    if( uvB.y < vMin ) vMin = uvB.y;
    if( uvC.y < vMin ) vMin = uvC.y;
    
    // next, remap texture coordinates to [0;1] 
    float faceWidth = fabsf( uMax - uMin );
    float faceHeight = fabsf( vMax - vMin );

    // and write it to the second texture coordinates of the triangle
    a->t2.x = fabsf(( uvA.x - uMin ) / faceWidth );
    a->t2.y = fabsf(( uvA.y - vMin ) / faceHeight );
    
    b->t2.x = fabsf(( uvB.x - uMin ) / faceWidth );
    b->t2.y = fabsf(( uvB.y - vMin ) / faceHeight );
    
    c->t2.x = fabsf(( uvC.x - uMin ) / faceWidth );
    c->t2.y = fabsf(( uvC.y - vMin ) / faceHeight );
    // now we've got scaled texture coordinates, nicely fits in [0;1]

    // compute texture size
    lm->width = faceWidth * 8;
    lm->height = faceHeight * 8;
    
    const int borderSize = 3; 
    
    // lightmap texture must be at least 4x4 pixels size + border size
    const int lowerLimit = 8 + borderSize;
    // upper limit, there must be balance between quality and speed
    // better resolution cause slower generation 
    const int upperLimit = 127;    
    
    if( lm->width < lowerLimit ) lm->width = lowerLimit;
    if( lm->height < lowerLimit ) lm->height = lowerLimit;    
    
    if( lm->width > upperLimit ) { 
        // clamp size with saving aspect ratio
        lm->height *= (float)upperLimit / (float)lm->width;
        lm->width = upperLimit;        
    }
    if( lm->height > upperLimit ) {
        // clamp size with saving aspect ratio
        lm->width *= (float)upperLimit / (float)lm->height;
        lm->height = upperLimit;
    }
    
    // calculate push coefficients    
    float scaleCoeffX = (float)lm->width / (float)( lm->width + borderSize );
    float scaleCoeffY = (float)lm->height / (float)( lm->height + borderSize  );
    
    float offsetX = (float)borderSize / ( 2.0f * (float)lm->width );
    float offsetY = (float)borderSize / ( 2.0f * (float)lm->height );
    
    lm->width += borderSize;
    lm->height += borderSize;
    
    // push uv's towards center, to remove possible seams due to bilinear filtration on GPU
    a->t2.x = ( a->t2.x + offsetX ) * scaleCoeffX;
    a->t2.y = ( a->t2.y + offsetY ) * scaleCoeffY;

    b->t2.x = ( b->t2.x + offsetX ) * scaleCoeffX;
    b->t2.y = ( b->t2.y + offsetY ) * scaleCoeffY;
    
    c->t2.x = ( c->t2.x + offsetX ) * scaleCoeffX;
    c->t2.y = ( c->t2.y + offsetY ) * scaleCoeffY;   
   
    //triangleNormal = Vec3_Normalize( triangleNormal );
    
    List_Create( &lm->layers );
    
    TLightmapLayer * tempLayer = (TLightmapLayer*)malloc( sizeof( TLightmapLayer ) );
    tempLayer->pixels = (TRGBAPixel*)malloc( lm->width * lm->height * sizeof( TRGBAPixel ) );    
    
    // precalculate coefficients for barycentric method for speedup (~2% ffs!)
    TUVTriangle precalcBary;    
    Vector2_Subtract( &precalcBary.v0, &b->t2, &a->t2 );
    Vector2_Subtract( &precalcBary.v1, &c->t2, &a->t2 );        
    precalcBary.d00 = Vector2_Dot( &precalcBary.v0, &precalcBary.v0 );
    precalcBary.d01 = Vector2_Dot( &precalcBary.v0, &precalcBary.v1 );
    precalcBary.d11 = Vector2_Dot( &precalcBary.v1, &precalcBary.v1 );
    precalcBary.denom = precalcBary.d00 * precalcBary.d11 - precalcBary.d01 * precalcBary.d01;
        
    const float constantBrightnessMultiplier = 3.0f;
    const float blackThreshold = (6.0f / 255.0f) / constantBrightnessMultiplier;
    // for each light we must create a new layer
    for_each( TLight, light, g_lights ) {
        float averageAttenuation = 0;    
        for( int i = 0; i < lm->height; i++ ) {
            for( int j = 0; j < lm->width; j++ ) {
                int index = i * lm->width + j;
                
                TVector2 uv;
                uv.x = (float)j / (float)lm->width;
                uv.y = (float)i / (float)lm->height;
                          
                // next, using barycentric coordinates calculate lighting for every point of the triangle   
                TBarycentricCoord barycentricCoord;
                Barycentric_Calculate2DFast( &precalcBary, &uv, &a->t2, &barycentricCoord );
        
                // using barycentric coordinates, we can find position of the pixel in world space
                TVec3 worldPosition; 
                Barycentric_MapToWorld( &worldPosition, &a->p, &b->p, &c->p, &barycentricCoord );
                
                // add offset 
                worldPosition = Vec3_Add( worldPosition, *offset );
                
                // diffuse lighting
                TVec3 direction;
                float attenuation = Lightmap_CalculateAttenuation( &light->owner->globalPosition, &worldPosition, light->radius, &direction );
                
                TVec3 interpolatedNormal;
                Barycentric_MapToWorld( &interpolatedNormal, &a->n, &b->n, &c->n, &barycentricCoord );
                attenuation *= Vec3_Dot( interpolatedNormal, direction );
                
                // TODO: there should be added bump-mapping
                //
                /////////////////////////////////////////
                
                averageAttenuation += attenuation;
                
                // shadows, most heavyweighted part, eats a lot of time (~85%) of all gen time
                // so this part of code can be multithreaded and it is
                
                // if pixel is bright enough, do shadows. this optimization gives 30% boost
                if( attenuation > blackThreshold ) {
                    TRayTraceResult rtResult;
                    TRay ray = Ray_SetDirection( light->owner->globalPosition, Vec3_Scale( direction, -99999.0f ));
                    // ray tracing using collision detection engine to detect intersections, so objects
                    // with collision body can cast shadows, otherwise no shadow will be generated
                    // also this raytracing could not able to detect alpha channel in texture
                    // so if you want to generate proper shadow, i.e. from a mesh, you need to create
                    // it with polygons, not texture with alpha channel, this bug must be fixed somehow
                   
                    if( threadNum >= 0 ) {
                        Ray_TraceWorldStaticMultithreaded( &ray, &rtResult, threadNum ); 
                    } else {
                        Ray_TraceWorldStatic( &ray, &rtResult );  
                    }       
                    const float shadowBias = 0.75f;
                    if( rtResult.body ) {
                        if( fabsf( Vec3_SqrDistance( light->owner->globalPosition, rtResult.position ) - 
                                   Vec3_SqrDistance( light->owner->globalPosition, worldPosition )) > shadowBias ){
                            attenuation = 0.0f;
                        } 
                    }     
                }

                TVec3 diffuseColor = Vec3_Scale( light->color, attenuation * constantBrightnessMultiplier );
                diffuseColor = Vec3_Clamp( diffuseColor, 0.0f, 1.0f );

                tempLayer->pixels[ index ].r = diffuseColor.x * 255.0f ;
                tempLayer->pixels[ index ].g = diffuseColor.y * 255.0f ;
                tempLayer->pixels[ index ].b = diffuseColor.z * 255.0f ; 
            }
        }    
        
        averageAttenuation /= (float)( lm->width * lm->height );
        
        // add new layer only if it is bright enough
        if( averageAttenuation > blackThreshold ) {
            int size = lm->width * lm->height * sizeof( TRGBAPixel );
            TLightmapLayer * layer = Memory_New( TLightmapLayer );
            layer->pixels = Memory_Allocate( size );
            memcpy( layer->pixels, tempLayer->pixels, size );
            layer->light = light;
            List_Add( &lm->layers, layer );
        }      
    }
    
    free( tempLayer->pixels );
    free( tempLayer );
    
    // blur final lightmap 
    Lightmap_Blur( lm, borderSize );
}

void LightmapAtlas_Free( TLightmapAtlas * atlas ) {
    Memory_Free( atlas->texture.pixels );
    Memory_Free( atlas );
}

TPackerNode * LightmapPacker_CreateNode( TPackerNode * parent ) {
    TPackerNode * node = Memory_New( TPackerNode );
    node->faceIndex = -1;
    node->parent = parent;
    return node;
}

TPackerNode * LightmapPacker_FindPlaceToInsert( TPackerNode * node, TLightmap * lm ) {
    if( node->split ) {
        TPackerNode * newNode = LightmapPacker_FindPlaceToInsert( node->childs[0], lm );
        if( newNode ) {
            return newNode;
        } else {
            return LightmapPacker_FindPlaceToInsert( node->childs[1], lm );
        }
    } else {
        if( node->lm ) {
            return NULL;
        }
        
        // too small to fit
        if( node->rect.w < lm->width || node->rect.h < lm->height ) {
            return NULL;
        } 
        
        // fits perfectly
        if( node->rect.w == lm->width && node->rect.h == lm->height ) {
            node->lm = lm;
            return node;
        }        
        
        node->childs[0] = LightmapPacker_CreateNode( node );
        node->childs[1] = LightmapPacker_CreateNode( node );
        
        float dw = node->rect.w - lm->width;
        float dh = node->rect.h - lm->height;
        
        if( dw > dh ) {
            node->childs[0]->rect.x = node->rect.x;
            node->childs[0]->rect.y = node->rect.y;            
            node->childs[0]->rect.w = lm->width;
            node->childs[0]->rect.h = node->rect.h;
            
            node->childs[1]->rect.x = node->rect.x + lm->width;
            node->childs[1]->rect.y = node->rect.y;            
            node->childs[1]->rect.w = node->rect.w - lm->width;
            node->childs[1]->rect.h = node->rect.h;
        } else {
            node->childs[0]->rect.x = node->rect.x;
            node->childs[0]->rect.y = node->rect.y;            
            node->childs[0]->rect.w = node->rect.w;
            node->childs[0]->rect.h = lm->height;
            
            node->childs[1]->rect.x = node->rect.x;
            node->childs[1]->rect.y = node->rect.y + lm->height;            
            node->childs[1]->rect.w = node->rect.w;
            node->childs[1]->rect.h = node->rect.h - lm->height;
        }
        
        node->split = true;
        
        return LightmapPacker_FindPlaceToInsert( node->childs[0], lm );
    }
    
    return NULL;
}

// this function sometimes returns wrong value, we need to find a better way
// to compute atlas size. size of atlas must be at least 2 * lightmap_face_max_size 
int LightmapPacker_ComputeAtlasSize( TSurface * surface, TLightmap * lightmaps, int faceOffset ) {
    // compute total area of all lightmaps
    int totalArea = 0;
    for( int i = faceOffset; i < surface->faceCount; i++ ) {
        TLightmap * lm = &lightmaps[i];
        totalArea += lm->width * lm->height;
    }

    // fix coefficient
    totalArea *= 1.5f;
    
    // get nearest pow2 side length
    int sideLength = 1;
    while( sideLength * sideLength < totalArea ) {
        sideLength *= 2;
    }

    return sideLength;
}

// creates binary tree to pack all lightmaps
// hierarchy is available in atlas->root
// full list of nodes available in atlas->nodes
TTexture * LightmapPacker_PackLightmaps( TSurface * surface, TLightmap * lightmaps, int faceOffset ) {
    TLightmapAtlas * atlas = Memory_New( TLightmapAtlas );
    List_Add( &gLightmapAtlasList, atlas );
    List_Create( &atlas->nodes );
    // create atlas texture
    int fixedSize = LightmapPacker_ComputeAtlasSize( surface, lightmaps, faceOffset );
    if( fixedSize > 2048 ) { // works well even on GMA3150, but can fail on older hardware
        fixedSize = 2048;
    }
    atlas->surface = surface;
    atlas->texture.width = fixedSize;
    atlas->texture.height = fixedSize;
    atlas->texture.bytesPerPixel = sizeof( TRGBAPixel );
    atlas->texture.bpp = sizeof( TRGBAPixel ) * 8;
    atlas->texture.glTexture = 0;
    atlas->texture.mips = 0;    
    atlas->texture.bytesCount = atlas->texture.width * atlas->texture.height * atlas->texture.bytesPerPixel;
    atlas->texture.pixels = Memory_Allocate( atlas->texture.bytesCount );
    strcpy( atlas->texture.fileName, "LightmapAtlas" );
    Log_Write( "Lightmapper: - Creating atlas: %dx%d", fixedSize, fixedSize );
    
    // create root node
    atlas->root.rect.x = 0;
    atlas->root.rect.y = 0;
    atlas->root.rect.w = atlas->texture.width;
    atlas->root.rect.h = atlas->texture.height;
    atlas->root.faceIndex = -1;
    atlas->root.split = false;
    atlas->root.lm = NULL;
    atlas->root.childs[0] = NULL;
    atlas->root.childs[1] = NULL;
    atlas->root.parent = NULL;   

    int currentLightmapIndex = surface->lightmapCount;
    surface->lightmaps[ currentLightmapIndex ] = atlas;
    surface->lightmapCount++;
    
    TRGBAPixel * pixels = (TRGBAPixel *)atlas->texture.pixels;
    
    // set background color to ambient
    int pixelCount = atlas->texture.width * atlas->texture.height;
    TRGBAPixel ambColor = { (unsigned char)(gAmbientLight.x * 255.0f), 
                            (unsigned char)(gAmbientLight.y * 255.0f), 
                            (unsigned char)(gAmbientLight.z * 255.0f), 255 };
    for( int i = 0; i < pixelCount; i++ ) {
        pixels[i] = ambColor;
    }
    
    for( int i = faceOffset; i < surface->faceCount; i++ ) {
        TFace * face = &surface->faces[i];
        TVertex * a = &surface->vertices[ face->index[0] ];
        TVertex * b = &surface->vertices[ face->index[1] ];
        TVertex * c = &surface->vertices[ face->index[2] ];
        
        TLightmap * lm = &lightmaps[i];
        
        face->lightmapIndex = currentLightmapIndex;
        
        // add pointer to this lightmap to the light
        for_each( TLightmapLayer, layer, lm->layers ) {
            TLight * light = layer->light;
            if( List_Find( &light->affectedAtlasList, atlas ) == NULL ) {
                List_Add( &light->affectedAtlasList, atlas );
            }
        }
        
        TPackerNode * node = LightmapPacker_FindPlaceToInsert( &atlas->root, lm );
        // if enough space in atlas, add current face's lightmap to the atlas
        if( node ) {   
            surface->lightmapFaceCount[currentLightmapIndex]++;
            
            List_Add( &atlas->nodes, node );
            
            // remap vertex second texture coordinates to atlas
            float uScale = (float)node->rect.w / (float)atlas->texture.width;
            float vScale = (float)node->rect.h / (float)atlas->texture.height;
            float uOffset = (float)node->rect.x / (float)atlas->texture.width;
            float vOffset = (float)node->rect.y / (float)atlas->texture.height;
            
            a->t2.x = a->t2.x * uScale + uOffset;
            a->t2.y = a->t2.y * vScale + vOffset;
            
            b->t2.x = b->t2.x * uScale + uOffset;
            b->t2.y = b->t2.y * vScale + vOffset;
            
            c->t2.x = c->t2.x * uScale + uOffset;
            c->t2.y = c->t2.y * vScale + vOffset;             
             
            TLightmap * lightmap = node->lm; 

            int rowEnd = node->rect.y + node->rect.h;
            int colEnd = node->rect.x + node->rect.w;
            for_each( TLightmapLayer, layer, lightmap->layers ) {
                TRGBAPixel * srcPixels = (TRGBAPixel*)layer->pixels;            
                // copy pixels only if light is enabled
                if( layer->light->enabled ) {
                    for( int row = node->rect.y, srcRow = 0; row < rowEnd; row++, srcRow++ )  {            
                        for( int col = node->rect.x, srcCol = 0; col < colEnd; col++, srcCol++ )  {
                            int srcIndex = srcRow * node->rect.w + srcCol;
                            int dstIndex = row * atlas->texture.width + col;

                            int r = pixels[ dstIndex ].r;
                            int g = pixels[ dstIndex ].g;
                            int b = pixels[ dstIndex ].b;
                            
                            r += srcPixels[ srcIndex ].r;
                            g += srcPixels[ srcIndex ].g;
                            b += srcPixels[ srcIndex ].b;
                            
                            if( r > 255 ) r = 255;                                               
                            if( g > 255 ) g = 255;                            
                            if( b > 255 ) b = 255;
                            if( r < 0 ) r = 0;
                            if( g < 0 ) g = 0;
                            if( b < 0 ) b = 0;
                                                        
                            pixels[dstIndex].r = r;
                            pixels[dstIndex].g = g;
                            pixels[dstIndex].b = b;       
                        }                
                    }                
                }
            }  
        // if not enough space in atlas, create new one and pack into it
        } else {
            LightmapPacker_PackLightmaps( surface, lightmaps, i );
            break;
        }
    }
    atlas->modified = true;    
    LightmapAtlas_Update( atlas, NULL );    
    return &atlas->texture;
}

void Lightmap_PrepareSurface( TSurface * surf ) {
    // unpack surface to delete shared vertices between faces, to get unique second texcoords
    int newVertexCount = surf->faceCount * 3;
    TVertex * newVertices = Memory_NewCount( newVertexCount, TVertex );
    
    for( int i = 0; i < surf->faceCount; i++ ) {
        TFace * face = &surf->faces[i];
        
        TFace newFace;
        newFace.index[0] = i * 3;
        newFace.index[1] = i * 3 + 1;
        newFace.index[2] = i * 3 + 2;
        
        newVertices[ newFace.index[0] ] = surf->vertices[ face->index[0] ];
        newVertices[ newFace.index[1] ] = surf->vertices[ face->index[1] ];
        newVertices[ newFace.index[2] ] = surf->vertices[ face->index[2] ];
        
        (*face) = newFace;
    }
    
    Memory_Free( surf->vertices );
    
    surf->vertices = newVertices;
    surf->vertexCount = newVertexCount;
}

// WARNING! Multithreaded lightmap generation contains tricky code, that can damage your brain :D

// common status var of all threads
// this vars must be volatile, otherwise compiler (with -On) gonna "optimize" code with it, and ruins everything
volatile int gThreadsStopped = false;
volatile TMTGenInfo lmMTGenInfo[OCTREE_MAX_SIMULTANEOUS_THREADS] = { {0} };
TEvent gDataReadyEvent[OCTREE_MAX_SIMULTANEOUS_THREADS] = {0};
TEvent gGenerationDoneEvent[OCTREE_MAX_SIMULTANEOUS_THREADS] = {0};

// main function to generate lightmaps in multithreaded mode
// ptr points on the thread number
int __stdcall Lightmap_Thread_BuildLightmap( void * ptr ) {
    int threadNum = *((int*)ptr);    
    while( !gThreadsStopped ) {
        Event_WaitSingle( gDataReadyEvent[threadNum] );        
        if( !gThreadsStopped ) {
            Event_Reset( gGenerationDoneEvent[threadNum] );
            Lightmap_Build( lmMTGenInfo[threadNum].offset, lmMTGenInfo[threadNum].lm, lmMTGenInfo[threadNum].a, 
                            lmMTGenInfo[threadNum].b, lmMTGenInfo[threadNum].c, lmMTGenInfo[threadNum].faceNum, threadNum ); 
            Event_Reset( gDataReadyEvent[ threadNum ] );            
        }
        Event_Set( gGenerationDoneEvent[ threadNum ] );
    }
    Event_Set( gGenerationDoneEvent[ threadNum ] );
    return 0;
}

void Lightmap_BuildForSurfaceMultithreaded( TSurface * surf, TVec3 * offset, int surfNum, int totalSurfaces ) {    
    Lightmap_PrepareSurface( surf );

    Log_Write( "Lightmapper: - Generating lightmap for surface %d of %d in multithreaded mode...", surfNum, totalSurfaces );
    
    int threadNums[ OCTREE_MAX_SIMULTANEOUS_THREADS ] = { 0 };
    // start threads
    gThreadsStopped = false;
    for( int i = 0; i < OCTREE_MAX_SIMULTANEOUS_THREADS; i++ ) {
        // create events first
        gDataReadyEvent[i] = Event_Create();
        gGenerationDoneEvent[i] = Event_Create();
        Event_Set( gGenerationDoneEvent[i] );
        // start threads
        threadNums[ i ] = i;
        if( Thread_Start( Lightmap_Thread_BuildLightmap, &threadNums[ i ] )) {
            Log_Write( "Lightmapper: - Thread %d started successfully!", i );
        }
    }
    
    TLightmap * lightmaps = Memory_NewCount( surf->faceCount, TLightmap );    
    for( int i = 0; i < surf->faceCount; i++ ) {
        TFace * face = &surf->faces[i];
        
        TVertex * a = &surf->vertices[ face->index[ 0 ]];
        TVertex * b = &surf->vertices[ face->index[ 1 ]];
        TVertex * c = &surf->vertices[ face->index[ 2 ]];
        
        // wait until we get free thread for generation
        int freeThreadNum = Event_WaitMultiple( OCTREE_MAX_SIMULTANEOUS_THREADS, gGenerationDoneEvent );  
        
        // prepare data for the thread
        lmMTGenInfo[freeThreadNum].lm = &lightmaps[i];
        lmMTGenInfo[freeThreadNum].a = a;
        lmMTGenInfo[freeThreadNum].b = b;
        lmMTGenInfo[freeThreadNum].c = c;
        lmMTGenInfo[freeThreadNum].offset = offset;
        lmMTGenInfo[freeThreadNum].faceNum = i;
        
        // enable thread
        Event_Set( gDataReadyEvent[freeThreadNum] );
    }
            
    // wait for gen done
    for( int i = 0; i < OCTREE_MAX_SIMULTANEOUS_THREADS; i++ ) {
        Event_WaitSingle( gGenerationDoneEvent[i] );
    }    
    // stop threads 
    gThreadsStopped = true;
    for( int i = 0; i < OCTREE_MAX_SIMULTANEOUS_THREADS; i++ ) {
        Event_Set( gDataReadyEvent[i] );
    } 
    // wait for exiting threads
    for( int i = 0; i < OCTREE_MAX_SIMULTANEOUS_THREADS; i++ ) {
        Event_WaitSingle( gGenerationDoneEvent[i] );
        Log_Write( "Lightmapper: - Thread %d exited successfully!", i );
    }
    // destroy events
    for( int i = 0; i < OCTREE_MAX_SIMULTANEOUS_THREADS; i++ ) {
        Event_Destroy( gDataReadyEvent[i] );
        Event_Destroy( gGenerationDoneEvent[i] );
    } 
    
    surf->lightmapped = true;

    Log_Write( "Lightmapper: - Packing lightmaps into atlas...!" );  
    LightmapPacker_PackLightmaps( surf, lightmaps, 0 );  
    Log_Write( "Lightmapper: - Generation done!" );  
}

void LightmapAtlas_Update( TLightmapAtlas * atlas, TLight * light ) {
    TRGBAPixel * pixels = (TRGBAPixel *)atlas->texture.pixels;
    
    TRGBAPixel ambColor = { 
        (unsigned char)(gAmbientLight.x * 255.0f), 
        (unsigned char)(gAmbientLight.y * 255.0f), 
        (unsigned char)(gAmbientLight.z * 255.0f), 
        255
    };
    
    if( light ) {
        for_each( TPackerNode, node, atlas->nodes ) {
            TLightmap * lightmap = node->lm; 
            int rowEnd = node->rect.y + node->rect.h;
            int colEnd = node->rect.x + node->rect.w;
            for_each( TLightmapLayer, layer, lightmap->layers ) {
                TRGBAPixel * srcPixels = (TRGBAPixel*)layer->pixels;                    
                if( layer->light == light ) {
                    for( int row = node->rect.y, srcRow = 0; row < rowEnd; row++, srcRow++ )  {            
                        for( int col = node->rect.x, srcCol = 0; col < colEnd; col++, srcCol++ )  {
                            int srcIndex = srcRow * node->rect.w + srcCol;
                            int dstIndex = row * atlas->texture.width + col;  
                          
                            int r = (int)pixels[ dstIndex ].r; 
                            int g = (int)pixels[ dstIndex ].g;
                            int b = (int)pixels[ dstIndex ].b;
                            
                            if( layer->light->enabled ) {
                                r += srcPixels[ srcIndex ].r;
                                g += srcPixels[ srcIndex ].g;
                                b += srcPixels[ srcIndex ].b;
                            } else {
                                r -= srcPixels[ srcIndex ].r;
                                g -= srcPixels[ srcIndex ].g;
                                b -= srcPixels[ srcIndex ].b;  
                            }
                            
                            if( r > 255 ) r = 255;                                                    
                            if( g > 255 ) g = 255;
                            if( b > 255 ) b = 255;
                            if( r < ambColor.r ) r = ambColor.r;                                                         
                            if( g < ambColor.g ) g = ambColor.g;
                            if( b < ambColor.b ) b = ambColor.b;                              
                            
                            pixels[dstIndex].r = r;
                            pixels[dstIndex].g = g;
                            pixels[dstIndex].b = b;                              
                        }                
                    }                
                }
            }        
        }        
        atlas->modified = true;
    }     
}

void LightmapAtlas_SaveSurfaceAtlases( TSurface * surf, const char * path ) {
    TBuffer output;   
    if( !Buffer_WriteFile( &output, path )) {
        Util_RaiseError( "Unable to create file to save lightmap atlas!" );
    }
    
    Buffer_WriteData( &output, &surf->sourceCRC32, sizeof( surf->sourceCRC32 ));
    
    // save face's lightmap indices
    Buffer_WriteInteger( &output, surf->faceCount );
    for( int i = 0; i < surf->faceCount; i++ ) {
        Buffer_WriteInteger( &output, surf->faces[i].lightmapIndex );
    }
    
    Buffer_WriteInteger( &output, surf->lightmapCount );
    for( int i = 0; i < surf->lightmapCount; i++ ) {
        TLightmapAtlas * atlas = surf->lightmaps[i];
    
        // write atlas
        Buffer_WriteInteger( &output, atlas->texture.width );
        Buffer_WriteInteger( &output, atlas->texture.height );
        
        int byteCount = atlas->texture.width * atlas->texture.height * sizeof( TRGBAPixel );
        Buffer_WriteData( &output, atlas->texture.pixels, byteCount );
        
        // write node count
        Buffer_WriteInteger( &output, atlas->nodes.size );
        
        // write nodes without hierarchy
        for_each( TPackerNode, node, atlas->nodes ) {
            TLightmap * lightmap = node->lm;

            // write node metrics
            Buffer_WriteInteger( &output, node->rect.x );
            Buffer_WriteInteger( &output, node->rect.y );
            Buffer_WriteInteger( &output, node->rect.w );
            Buffer_WriteInteger( &output, node->rect.h );
                   
            Buffer_WriteInteger( &output, lightmap->faceID );
            
            // write vertices second texture coordinates
            Buffer_WriteVector2( &output, &lightmap->a->t2 );
            Buffer_WriteVector2( &output, &lightmap->b->t2 );
            Buffer_WriteVector2( &output, &lightmap->c->t2 );
            
            // write layer count
            Buffer_WriteInteger( &output, lightmap->layers.size );

            // write layers
            for_each( TLightmapLayer, layer, lightmap->layers ) {            
                // write layer light for identification on load
                TLight * light = layer->light;
                Buffer_WriteString( &output, light->owner->name );
                
                // write layer pixels
                int size = node->rect.w * node->rect.h * sizeof( TRGBAPixel );
                Buffer_WriteData( &output, layer->pixels, size ); 
            }   
        }
    }
    Buffer_Free( &output );
}

bool LightmapAtlas_LoadSurfaceAtlases( TEntity * root, TSurface * surface, const char * path ) {      
    TBuffer input; 
    if( !Buffer_LoadFile( &input, path, 0 ) ) {
        return false;
    }     
    
    // check crc
    unsigned int sourceCRC32;
    Buffer_ReadData( &input, &sourceCRC32, sizeof( sourceCRC32 ) );
    
    // surface has been changed, so return false, to signal, that lightmap must be generated again
    if( sourceCRC32 != root->sourceCRC32 ) {
        return false;
    }
    
    // break surface into unique faces
    Lightmap_PrepareSurface( surface );
    
    // read face lightmap indices
    int surfCount = Buffer_ReadInteger( &input );
    if( surfCount != surface->faceCount ) {
        return false;
    }
    for( int i = 0; i < surface->faceCount; i++ ) {
        surface->faces[i].lightmapIndex = Buffer_ReadInteger( &input );
    }
    
    surface->lightmapCount = Buffer_ReadInteger( &input );    

    for( int i = 0; i < surface->lightmapCount; i++ ) {        
        // create and load atlas 
        TLightmapAtlas * atlas = Memory_AllocateClean( sizeof( TLightmapAtlas ));
        
        atlas->texture.width = Buffer_ReadInteger( &input );
        atlas->texture.height = Buffer_ReadInteger( &input );
        
        int byteCount = atlas->texture.width * atlas->texture.height * sizeof( TRGBAPixel );
        atlas->texture.pixels = Memory_AllocateClean( byteCount );
        Buffer_ReadData( &input, atlas->texture.pixels, byteCount );
        
        // read atlas nodes
        int nodeCount = Buffer_ReadInteger( &input );
        
        surface->lightmapFaceCount[i] = nodeCount;
        
        for( int i = 0; i < nodeCount; i++ ) {
            TPackerNode * node = Memory_New( TPackerNode );
            
            // read node metrics
            node->rect.x = Buffer_ReadInteger( &input );
            node->rect.y = Buffer_ReadInteger( &input );
            node->rect.w = Buffer_ReadInteger( &input );
            node->rect.h = Buffer_ReadInteger( &input );
                   
            TLightmap * lightmap = Memory_New( TLightmap );
            node->lm = lightmap;
            
            int faceID = Buffer_ReadInteger( &input );
            
            // restore second texcoords
            TFace * face = &surface->faces[ faceID ];
            
            lightmap->a = &surface->vertices[ face->index[0] ];
            lightmap->b = &surface->vertices[ face->index[1] ];
            lightmap->c = &surface->vertices[ face->index[2] ];
            
            Buffer_ReadVector2( &input, &lightmap->a->t2 );
            Buffer_ReadVector2( &input, &lightmap->b->t2 );
            Buffer_ReadVector2( &input, &lightmap->c->t2 );
                        
            int layerCount = Buffer_ReadInteger( &input );
            
            List_Add( &atlas->nodes, node );
            
            for( int layerNum = 0; layerNum < layerCount; layerNum++ ) {            
                TLightmapLayer * layer = Memory_New( TLightmapLayer );
                
                char nameBuffer[256];
                Buffer_ReadString( &input, nameBuffer );
                TEntity * lightEntity = Entity_GetChildByName( root, nameBuffer );
                if( !lightEntity->componentLight ) {
                    return false;
                }
                layer->light = lightEntity->componentLight;
                if( List_Find( &layer->light->affectedAtlasList, atlas ) == NULL ) {
                    List_Add( &layer->light->affectedAtlasList, atlas );
                }                
                int layerSize = node->rect.w * node->rect.h * sizeof( TRGBAPixel );
                layer->pixels = Memory_AllocateClean( layerSize );
                Buffer_ReadData( &input, layer->pixels, layerSize ); 
                List_Add( &lightmap->layers, layer );
            }   
        }         
        surface->lightmaps[i] = atlas;
        LightmapAtlas_Update( atlas, NULL );
    }

    surface->lightmapped = true;
    
    Buffer_Free( &input );   
    
    return true;
}