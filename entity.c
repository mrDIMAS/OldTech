#include "entity.h"
#include "camera.h"
#include "light.h"
#include "buffer.h"
#include "billboard.h"
#include "lightmap.h"

TList g_entities = { NULL, NULL, 0 };
int gEntityCounter = 0;

TEntity * Entity_Create( ) {
    TEntity * ent = Memory_New( TEntity );
    ent->localPosition = Vec3_Set( 0.0f, 0.0f, 0.0f );
	ent->localScale = Vec3_Set( 1.0f, 1.0f, 1.0f );
    ent->globalPosition = Vec3_Set( 0.0f, 0.0f, 0.0f );
    ent->color = Vec3_Set( 1.0f, 1.0f, 1.0f );
    ent->localRotation = Quaternion_SetEulerAngles( 0.0f, 0.0f, 0.0f );
    ent->parent = NULL;
    ent->componentCamera = NULL;
    ent->visible = true;
    ent->animated = false;
	ent->globalTransformCalculated = false;
    ent->globalTransform = Matrix4_Identity();
    ent->localTransform = Matrix4_Identity();
    ent->invBindTransform = Matrix4_Identity();
    List_Create( &ent->surfaces );
    List_Create( &ent->childs );
    List_Create( &ent->keyFrames );
    List_Create( &ent->allSurfaces );
    ent->skinned = false;
    ent->dynBody = NULL;
    ent->totalFrames = 0;
    ent->componentLight = NULL;
    ent->anim = NULL;
	ent->depthHack = 0.0f;
    ent->instanceOf = NULL;
    ent->alpha = 1.0f;
    ent->sourceCRC32 = 0; // computed when entity loading from file
    ent->name = String_Format( "UnnamedEntity%d", gEntityCounter++ );
    List_Add( &g_entities, ent );
    return ent;
}

TEntity * Entity_CreateInstance( TEntity * source ) {
    TEntity * ent = Memory_New( TEntity );
    ent->localPosition = source->localPosition;
    ent->localRotation = source->localRotation;
	ent->localScale = source->localScale;
    ent->parent = NULL;
    ent->visible = true;
    ent->alpha = source->alpha;
	ent->globalTransformCalculated = false;
	ent->depthHack = source->depthHack;
    ent->animated = source->animated;
    ent->globalTransform = source->globalTransform;
    ent->localTransform = source->localTransform;
    ent->invBindTransform = source->invBindTransform;
    ent->color = source->color;
    ent->sourceCRC32 = source->sourceCRC32;
    for_each( TSurface, surf, source->allSurfaces ) {
        List_Add( &ent->allSurfaces, surf );
    }
    // copy pointers to surfaces, surfaces can be shared
    List_Create( &ent->surfaces );
    for_each( TSurface, surface, source->surfaces ) {
        surface->shareCount++;
        List_Add( &ent->surfaces, surface );
    }
    // instantiate childs
    List_Create( &ent->childs );
    for_each( TEntity, sourceChild, source->childs ) {
        TEntity * child = Entity_CreateInstance( sourceChild );
        List_Add( &ent->childs, child );
        child->parent = ent;
    }
    // copy keyframes
    List_Create( &ent->keyFrames );
    for_each( TKeyFrame, sourceKF, source->keyFrames ) {
        TKeyFrame * kf = Memory_New( TKeyFrame );
        memcpy( kf, sourceKF, sizeof( TKeyFrame ));
        List_Add( &ent->keyFrames, kf );
    }
    ent->skinned = source->skinned;
    // todo: fix body copying
    ent->dynBody = source->dynBody;
    ent->totalFrames = source->totalFrames;
    ent->anim = source->anim;
    ent->instanceOf = source;    
    ent->name = String_Duplicate( source->name );
    List_Add( &g_entities, ent );

    if( source->componentLight ) {
        ent->componentLight = Memory_New( TLight );
        (*ent->componentLight) = (*source->componentLight);
        ent->componentLight->owner = ent;
        List_Create( &ent->componentLight->affectedAtlasList );
    } else {
        ent->componentLight = NULL;
    }
    
    if( source->componentBillboard ) {
        ent->componentBillboard = Memory_New( TBillboard );
        (*ent->componentBillboard) = (*source->componentBillboard);
        ent->componentBillboard->owner = ent;
    } else {
        ent->componentBillboard = NULL;
    }
    
    ent->componentCamera = source->componentCamera;
    return ent;
}

void Entity_SetColor( TEntity * ent, const TVec3 * color, bool affectChilds ) {
    ent->color = *color;
    if( affectChilds ) {
        for_each( TEntity, child, ent->childs ) {
            Entity_SetColor( child, color, affectChilds );
        }
    }
}

void Entity_SetDepthHack( TEntity * ent, float depthHack ) {
    ent->depthHack = depthHack;
    for_each( TEntity, child, ent->childs ) {
        Entity_SetDepthHack( child, depthHack );
    }
}

void Entity_Free( TEntity * ent ) {
    String_Free( ent->name );    
    for_each( TEntity, entity, g_entities ) {
        // if this entity is a child of other entity, remove it from child list
        for_each( TEntity, child, entity->childs ) {
            if( child == ent ) {
                List_Remove( &entity->childs, ent );
                break;
            }
        }
        // if this entity is parent for other entity, orphan it
        if( entity->parent == ent ) {
            entity->parent = NULL;
        }
    }
    
    List_Remove( &g_entities, ent );
    
    // free surfaces
    if( !ent->instanceOf ) {
        for_each( TSurface, surface, ent->surfaces ) {
            surface->shareCount--;
            if( surface->shareCount <= 0 ) {
                Surface_Free( surface );
            }
        }
    }
    List_Free( &ent->surfaces );

    // free keyframes
    for_each( TKeyFrame, kf, ent->keyFrames ) {
        Memory_Free( kf );
    }
    List_Free( &ent->keyFrames );

    // free childs
    for_each( TEntity, child, ent->childs ) {
        Entity_Free( child );
    }
    List_Free( &ent->childs );

    Memory_Free( ent );
}

TBillboard * Entity_MakeBillboard( TEntity * ent ) {    
    ent->componentBillboard = Billboard_Create( 1.0f, 1.0f );
    ent->componentBillboard->owner = ent;
    return ent->componentBillboard;
}

void Entity_GetGlobalRotation( TEntity * ent, TQuaternion * globRot ) {
	if( ent->parent ) {
		TQuaternion parentGlobRot;
		Entity_GetGlobalRotation( ent->parent, &parentGlobRot );
		*globRot = Quaternion_Multiply( ent->localRotation, parentGlobRot );
	} else {
		*globRot = ent->localRotation;
	}
}

void Entity_FreeAll() {
    List_Clear( &g_entities, 1 );
}

void Entity_AddSurface( TEntity * ent, TSurface * surf ) {
    List_Add( &ent->surfaces, surf );
}

TVec3 Entity_GetLookVector( TEntity * ent ) {
    return (TVec3) { .x = ent->globalTransform.f[8], .y = ent->globalTransform.f[9], .z = ent->globalTransform.f[10] };
}

TVec3 Entity_GetRightVector( TEntity * ent ) {
    return (TVec3) { .x = ent->globalTransform.f[0], .y = ent->globalTransform.f[1], .z = ent->globalTransform.f[2] };
}

TVec3 Entity_GetUpVector( TEntity * ent ) {
    return (TVec3) { .x = ent->globalTransform.f[4], .y = ent->globalTransform.f[5], .z = ent->globalTransform.f[6] };
}

void Entity_AddChild( TEntity * ent, TEntity * child ) {
    List_Add( &ent->childs, child );
}

TEntity * Entity_GetChild( TEntity * ent, int childNum ) {
    return List_GetNodeData( &ent->childs, childNum );
}

TCamera * Entity_MakeCamera( TEntity * ent ) {
    ent->componentCamera = Memory_New( TCamera );
    Camera_Create( ent->componentCamera );
    ent->componentCamera->owner = ent;
    return ent->componentCamera;
}

void Entity_CalculateGlobalTransform( TEntity * ent ) {
    if( ent->dynBody ) {
        ent->localPosition = ent->dynBody->position;
    }
    TMatrix4 scale = Matrix4_Scale( ent->localScale );
    ent->localTransform = Matrix4_SetRotationOrigin( ent->localRotation, ent->localPosition );	    
    ent->localTransform = Matrix4_Multiply( scale, ent->localTransform );
    if( ent->parent ) {
        Entity_CalculateGlobalTransform( ent->parent );
        ent->globalTransform = Matrix4_Multiply( ent->localTransform, ent->parent->globalTransform );
    } else {
        ent->globalTransform = ent->localTransform;
    }
    
    ent->globalPosition.x = ent->globalTransform.f[12];
    ent->globalPosition.y = ent->globalTransform.f[13];
    ent->globalPosition.z = ent->globalTransform.f[14];
}

void Entity_Attach( TEntity * ent, TEntity * parent ) {
    ent->parent = parent;
    List_Add( &parent->childs, ent );
}

TVec3 Entity_GetGlobalPosition( TEntity * ent ) {
    return (TVec3) { .x = ent->globalTransform.f[12], .y = ent->globalTransform.f[13], .z = ent->globalTransform.f[14] };
}

void Entity_SetBody( TEntity * ent, TBody * body ) {
    ent->dynBody = body;
}

void Entity_SetLocalPosition( TEntity * ent, const TVec3 * pos ) {
    ent->localPosition = *pos;
}

//=================
// Animation
//=================
void Entity_SetAnimation( TEntity * ent, TAnimation * anim ) {
    ent->anim = anim;
    // also process childs
    for_each( TEntity, child, ent->childs ) {
        child->anim = anim;
    }
}

void Entity_SetAnimationEnabled( TEntity * ent, bool state ) {
    ent->animationEnabled = state;
    // also process childs
    for_each( TEntity, child, ent->childs ) {
        child->animationEnabled = state;
    }
}

void Entity_Animate( TEntity * ent ) {
    if( ent->animationEnabled ) {
        // skeletal animation 
        if( ent->skinned ) {
            for_each( TSurface, surface, ent->surfaces ) {
                // firstly, compute transformation of the each bone affecting this surface
                for_each( TBone, bone, surface->bones ) {
                    bone->transform = Matrix4_Multiply( bone->boneEnt->invBindTransform, bone->boneEnt->globalTransform  );
                    //Matrix4_Multiply( &bone->transform, &bone->transform , &ent->globalTransform );                    
                    bone->transform = Matrix4_Multiply( ent->globalTransform, bone->transform );                    
                }
                // mark buffers as 'not processed' for the renderer
                surface->buffersReady = false;
                // finally, do skeletal animation                 
                for( int i = 0; i < surface->vertexCount; i++ ) {
                    TVertex * vertex = surface->vertices + i;
                    TVertex * skinVertex = surface->skinVertices + i;
                    TBoneGroup * bg = surface->vertexBones + i;
                    skinVertex->p = Vec3_Zero();
                    for( int k = 0; k < bg->boneCount; k++ ) {
                        TBone * bone = &bg->bones[k];
                        // transform vertex 
                        TVec3 transformed = Matrix4_TransformVector( bone->transform, vertex->p );
                        skinVertex->p = Vec3_Add( skinVertex->p, Vec3_Scale( transformed, bone->weight ));
                    }
                }
            }
        } else {
            // keyframe animation 
            if( ent->anim ) {
                if( ent->keyFrames.size ) {
                    TKeyFrame * currentFrame = List_GetNodeData( &ent->keyFrames, ent->anim->curFrame );
                    TKeyFrame * nextFrame = List_GetNodeData( &ent->keyFrames, ent->anim->nextFrame );

                    ent->localRotation = Quaternion_Slerp( currentFrame->rot, nextFrame->rot, ent->anim->interp );
                    ent->localPosition = Vec3_Lerp( currentFrame->pos, nextFrame->pos, ent->anim->interp );
                }
            } 
        }
    }
}

void Entity_ApplyProperties( TEntity * ent ) {
    // apply properties passed from an editor
    for( int i = 0; i < ent->properties.count; i++ ) {
        char * name = ent->properties.values[i].name;
        float value = ent->properties.values[i].number;
        
        if( !strcmp( name, "visible" )) {
            ent->visible = (bool)value;
        }
    }
}

//=================
// World
//=================
void World_Update( ) {
    Animation_UpdateAll();
	for_each( TEntity, entity, g_entities ) {
		Entity_CalculateGlobalTransform( entity );        
        Entity_Animate( entity );
	}
}

//=================
// Loading
//=================
TEntity * Entity_LoadFromFile( const char * fileName ) {
    TBuffer buf;
    int meshObjectNum;
    int i, boneNum;
    int vertexNum;
    int faceNum;
    unsigned int crc32;
    if( !Buffer_LoadFile( &buf, fileName, &crc32 ) ) {
        Util_RaiseError( "Unable to load scene %s", fileName );
    }    
    int numObjects = Buffer_ReadInteger( &buf );
    int numMeshes = Buffer_ReadInteger( &buf );
    int numLights = Buffer_ReadInteger( &buf ); 
    int framesCount = Buffer_ReadInteger( &buf );   
    TEntity * root = Entity_Create();    
    root->sourceCRC32 = crc32;
    for( meshObjectNum = 0; meshObjectNum < numMeshes; meshObjectNum++ ) {
        TEntity * node;
        if( numObjects == 1 ) {
            node = root;
        } else {
            node = Entity_Create();            
        }
        node->sourceCRC32 = crc32;
        Buffer_ReadVector3( &buf, &node->localPosition );
        Buffer_ReadQuaternion( &buf, &node->localRotation );
        Entity_CalculateGlobalTransform( node );
        node->animated = Buffer_ReadInteger( &buf );
        int isSkinned = Buffer_ReadInteger( &buf );
        int meshCount = Buffer_ReadInteger( &buf );
        int keyframeCount = Buffer_ReadInteger( &buf );
        node->skinned = isSkinned;
        char tempBuffer[4096];
        Buffer_ReadString( &buf, tempBuffer );
        Parser_LoadString( tempBuffer, &node->properties );
        Buffer_ReadString( &buf, tempBuffer );
        node->name = String_Duplicate( tempBuffer );        
        for( i = 0; i < keyframeCount; i++ ) {
            TKeyFrame * keyframe = Memory_New( TKeyFrame );
            Buffer_ReadVector3( &buf, &keyframe->pos );
            Buffer_ReadQuaternion( &buf, &keyframe->rot );
            List_Add( &node->keyFrames, keyframe );
        }
        if( keyframeCount ) {
            TKeyFrame * keyFrame = List_GetNodeData( &node->keyFrames, 0 );
            node->localPosition = keyFrame->pos;
            node->localRotation = keyFrame->rot;
        }
        node->totalFrames = framesCount - 1;
        for( i = 0; i < meshCount; i++ ) {
            TSurface * surf = Memory_New( TSurface );
            List_Add( &root->allSurfaces, surf );
            surf->shareCount = 1;
            int vertexCount = Buffer_ReadInteger( &buf );
            int indexCount = Buffer_ReadInteger( &buf );
            surf->skinned = 0;
            surf->vertexCount = vertexCount;
            Buffer_ReadVector3( &buf, &surf->aabb.min );
            Buffer_ReadVector3( &buf, &surf->aabb.max );
            Buffer_ReadVector3( &buf, &surf->aabb.center );
            surf->aabb.radius = Buffer_ReadFloat( &buf );
            Buffer_ReadString( &buf, surf->diffuseTexName );
            Buffer_ReadString( &buf, surf->normalTexName );
            strcpy( surf->sourceName, strrchr( fileName, '/' ));
            surf->onLoadOwner = node;
            surf->opacity = Buffer_ReadFloat( &buf ) / 100.0f;
            surf->lightmapped = false;
            surf->sourceCRC32 = crc32;
            surf->facesSorted = false;
            // read vertices
            surf->vertices = Memory_NewCount( vertexCount, TVertex );
            for( vertexNum = 0; vertexNum < vertexCount; vertexNum++ ) {
                TVertex * v = surf->vertices + vertexNum;
                Buffer_ReadVector3( &buf, &v->p );
                Buffer_ReadVector3( &buf, &v->n );
                Buffer_ReadVector2( &buf, &v->t );
                Buffer_ReadVector2( &buf, &v->t2 );
                Buffer_ReadVector3( &buf, &v->tg );
            }            
            // read faces
            surf->faceCount = indexCount / 3;
            surf->faces = Memory_NewCount( surf->faceCount, TFace );
            for( faceNum = 0; faceNum < surf->faceCount; faceNum++ ) {
                TFace * face = surf->faces + faceNum;
                face->index[0] = Buffer_ReadIndex16( &buf );
                face->index[1] = Buffer_ReadIndex16( &buf );
                face->index[2] = Buffer_ReadIndex16( &buf );
                face->lightmapIndex = -1;
            }
            // try to load diffuse texture
            const char * pathToTexture = Std_Format( "data/textures/%s", surf->diffuseTexName );
            surf->texture = Texture2D_LoadFromFile( pathToTexture );
            // add new surface to surface list
            List_Add( &node->surfaces, surf );
            surf->skinVertices = 0;
            surf->vertexBones = 0;
            surf->lightmapCount = 0;
            if( node->skinned ) {
                // read bone's info
                surf->skinned = true;
                // allocate some memory for animation
                surf->skinVertices = Memory_NewCount( surf->vertexCount, TVertex );
                memcpy( surf->skinVertices, surf->vertices, surf->vertexCount * sizeof( TVertex ));
                surf->vertexBones = Memory_NewCount( surf->vertexCount, TBoneGroup );
                for( vertexNum = 0; vertexNum < vertexCount; vertexNum++ ) {
                    TBoneGroup * bg = surf->vertexBones + vertexNum;
                    bg->boneCount = Buffer_ReadInteger( &buf );
                    for( boneNum = 0; boneNum < bg->boneCount; boneNum++ ) {
                        // read entity identifier, that represents bone in the current scene
                        bg->bones[boneNum].boneId = Buffer_ReadInteger( &buf );
                        // read weight of that bone
                        bg->bones[boneNum].weight = Buffer_ReadFloat( &buf );
                    }
                }
            }
        }
        if( numObjects > 1 ) {
            Entity_Attach( node, root );
        }        
    }
    // load lights
    int lightObjectNum;
    for( lightObjectNum = 0; lightObjectNum < numLights; lightObjectNum++ ) {
        TLight * lit = Memory_New( TLight );
        TEntity * litEnt = Entity_Create();
        litEnt->componentLight = lit;
        Buffer_ReadString( &buf, litEnt->name );
        lit->type = Buffer_ReadInteger( &buf );
        Buffer_ReadVector3( &buf, &lit->color );
        lit->color = Vec3_Scale( lit->color, 1.0f / 255.0f );
        lit->owner = litEnt;
        lit->radius = Buffer_ReadFloat( &buf );
        lit->brightness = Buffer_ReadFloat( &buf );
        lit->enabled = true;
        List_Create( &lit->affectedAtlasList );
        Buffer_ReadVector3( &buf, &litEnt->localPosition );
        if( lit->type == LT_SPOT ) {
            lit->innerAngle = Buffer_ReadFloat( &buf );
            lit->outerAngle = Buffer_ReadFloat( &buf );
            Buffer_ReadQuaternion( &buf, &litEnt->localRotation );
        }
        Entity_Attach( litEnt, root );
        List_Add( &g_lights, lit );
    }
    // load hierarchy
    for_each( TEntity, child, root->childs ) {
        UNUSED_VARIABLE( child );
        char objName[128], parName[128];
        Buffer_ReadString( &buf, objName );
        Buffer_ReadString( &buf, parName );
        TEntity * object = Entity_GetChildByName( root, objName );
        TEntity * parent = Entity_GetChildByName( root, parName );
        if( parent ) {
            Entity_Attach( object, parent );
        }
    }
    // iterate over scene's entities
    for_each( TEntity, ent, root->childs ) {
        Entity_ApplyProperties( ent );
        Entity_CalculateGlobalTransform( ent );
        // calculate inverse bind transform of the entity
        ent->invBindTransform = Matrix4_Inverse( ent->globalTransform );
        if( ent->skinned ) {
            // find bones. we can't assing correct bone to each vertex in loading time, because
            // there is no confidence that needed bone does exist, assigning bones after load 
            // each entity removes this undefined behaviour
            for_each( TSurface, surf, ent->surfaces ) {
                for( vertexNum = 0; vertexNum < surf->vertexCount; vertexNum++ ) {
                    TBoneGroup * bg = surf->vertexBones + vertexNum;
                    for( boneNum = 0; boneNum < bg->boneCount; boneNum++ ) {
                        TBone * bone = &bg->bones[boneNum];
                        // acquire correct bone node
                        bone->boneEnt = List_GetNodeData( &root->childs, bone->boneId );
                        // check for duplicates
                        if( !List_Find( &surf->bones, bone )) { 
                            List_Add( &surf->bones, bone );
                        }
                    }
                }
            }
        }
    }
    Buffer_Free( &buf );
    return root;
}

TEntity * Entity_GetChildByName( TEntity * parent, const char * name ) {
    for_each( TEntity, child, parent->childs ) {
        if( !strcmp( child->name, name ) ) {
            return child;
        }
    }
    return NULL;
}

