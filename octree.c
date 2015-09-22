#include "octree.h"
#include "collision.h"
#include "renderer.h"
#include <float.h>

int IndexCmpFunc( const void * a, const void * b );

void Octree_Build( TOctree * octree, TTriangle * triangles, int triangleCount, int maxTrianglesPerNode ) {
    // compute metrics of vertices( min, max ) and build root node 
    octree->root = Memory_New( TOctreeNode );
    octree->root->indexCount = 0;
    octree->root->indices = 0;
    octree->root->split = 0;

    octree->root->min = Vec3_Set( FLT_MAX, FLT_MAX, FLT_MAX );
    octree->root->max = Vec3_Set( -FLT_MAX, -FLT_MAX, -FLT_MAX );

    int i;
    for( i = 0; i < triangleCount; i++ ) {
        const TTriangle * triangle = triangles + i;

        // so much wow, so copypaste wow 
        if( triangle->a.x < octree->root->min.x ) octree->root->min.x = triangle->a.x;
        if( triangle->b.x < octree->root->min.x ) octree->root->min.x = triangle->b.x;
        if( triangle->c.x < octree->root->min.x ) octree->root->min.x = triangle->c.x;

        if( triangle->a.y < octree->root->min.y ) octree->root->min.y = triangle->a.y;
        if( triangle->b.y < octree->root->min.y ) octree->root->min.y = triangle->b.y;
        if( triangle->c.y < octree->root->min.y ) octree->root->min.y = triangle->c.y;

        if( triangle->a.z < octree->root->min.z ) octree->root->min.z = triangle->a.z;
        if( triangle->b.z < octree->root->min.z ) octree->root->min.z = triangle->b.z;
        if( triangle->c.z < octree->root->min.z ) octree->root->min.z = triangle->c.z;

        if( triangle->a.x > octree->root->max.x ) octree->root->max.x = triangle->a.x;
        if( triangle->b.x > octree->root->max.x ) octree->root->max.x = triangle->b.x;
        if( triangle->c.x > octree->root->max.x ) octree->root->max.x = triangle->c.x;

        if( triangle->a.y > octree->root->max.y ) octree->root->max.y = triangle->a.y;
        if( triangle->b.y > octree->root->max.y ) octree->root->max.y = triangle->b.y;
        if( triangle->c.y > octree->root->max.y ) octree->root->max.y = triangle->c.y;

        if( triangle->a.z > octree->root->max.z ) octree->root->max.z = triangle->a.z;
        if( triangle->b.z > octree->root->max.z ) octree->root->max.z = triangle->b.z;
        if( triangle->c.z > octree->root->max.z ) octree->root->max.z = triangle->c.z;
    }

    int * indices = Memory_NewCount( triangleCount, int );
    for( int i = 0; i < triangleCount; i++ ) {
        indices[i] = i;
    }

    int conIndSize = 10 * triangleCount;
    octree->containIndices = Memory_NewCount( conIndSize, int );
    octree->containIndexCount = 0;
    
    // multithreaded version
    for( int i = 0; i < OCTREE_MAX_SIMULTANEOUS_THREADS; i++ ) {
        octree->containIndicesMT[i] = Memory_NewCount( conIndSize, int );
        octree->containIndexCountMT[i] = 0;
    }

    Octree_BuildRecursiveInternal( octree->root, triangles, triangleCount, indices, triangleCount, maxTrianglesPerNode );
}

int IndexCmpFunc( const void * a, const void * b ) {
    return (*(int*)a) - (*(int*)b);
}

void Octree_TraceRay( TOctree * octree, const TRay * ray ) {
    octree->containIndexCount = 0;
    Octree_TraceRayRecursiveInternal( octree, octree->root, ray );
}

char Octree_TraceRayRecursiveInternal( TOctree * octree, TOctreeNode * node, const TRay * ray ) {
    // ray-box intersection check 
    float tmin, tmax, tymin, tymax, tzmin, tzmax;
    if( ray->dir.x >= 0 ) {
        tmin = (node->min.x - ray->begin.x) / ray->dir.x;
        tmax = (node->max.x - ray->begin.x) / ray->dir.x;
    } else {
        tmin = (node->max.x - ray->begin.x) / ray->dir.x;
        tmax = (node->min.x - ray->begin.x) / ray->dir.x;
    }
    if( ray->dir.y >= 0 ) {
        tymin = (node->min.y - ray->begin.y) / ray->dir.y;
        tymax = (node->max.y - ray->begin.y) / ray->dir.y;
    } else {
        tymin = (node->max.y - ray->begin.y) / ray->dir.y;
        tymax = (node->min.y - ray->begin.y) / ray->dir.y;
    }
    if( (tmin > tymax) || (tymin > tmax) ) {
        return 0;
    }
    if( tymin > tmin ) {
        tmin = tymin;
    }
    if( tymax < tmax ) {
        tmax = tymax;
    }
    if( ray->dir.z >= 0 ) {
        tzmin = (node->min.z - ray->begin.z) / ray->dir.z;
        tzmax = (node->max.z - ray->begin.z) / ray->dir.z;
    } else {
        tzmin = (node->max.z - ray->begin.z) / ray->dir.z;
        tzmax = (node->min.z - ray->begin.z) / ray->dir.z;
    }
    if( (tmin > tzmax) || (tzmin > tmax) ) {
        return 0;
    }
    if( tzmin > tmin ) {
        tmin = tzmin;
    }
    if( tzmax < tmax ) {
        tmax = tzmax;
    }
    if( (tmin < 1.0f) && (tmax > 0.0f)) {
        if( node->split ) {
            for( int i = 0; i < 8; i++ ) {
                Octree_TraceRayRecursiveInternal( octree, node->childs[i], ray );
            }
        } else {
            for( int i = 0; i < node->indexCount; i++ ) {
                octree->containIndices[ octree->containIndexCount ] = node->indices[i];
                octree->containIndexCount++;
            }
        }
    }
    return 1;
}

void Octree_TraceRayMultithreaded( TOctree * octree, const struct TRay * ray, int threadNum ) {
    octree->containIndexCountMT[threadNum] = 0;
    Octree_TraceRayRecursiveInternalMultithreaded( octree, octree->root, ray, threadNum );
}

char Octree_TraceRayRecursiveInternalMultithreaded( TOctree * octree, TOctreeNode * node, const struct TRay * ray, int threadNum ) {
    // ray-box intersection check 
    float tmin, tmax, tymin, tymax, tzmin, tzmax;
    if( ray->dir.x >= 0 ) {
        tmin = (node->min.x - ray->begin.x) / ray->dir.x;
        tmax = (node->max.x - ray->begin.x) / ray->dir.x;
    } else {
        tmin = (node->max.x - ray->begin.x) / ray->dir.x;
        tmax = (node->min.x - ray->begin.x) / ray->dir.x;
    }
    if( ray->dir.y >= 0 ) {
        tymin = (node->min.y - ray->begin.y) / ray->dir.y;
        tymax = (node->max.y - ray->begin.y) / ray->dir.y;
    } else {
        tymin = (node->max.y - ray->begin.y) / ray->dir.y;
        tymax = (node->min.y - ray->begin.y) / ray->dir.y;
    }
    if( (tmin > tymax) || (tymin > tmax) ) {
        return 0;
    }
    if( tymin > tmin ) {
        tmin = tymin;
    }
    if( tymax < tmax ) {
        tmax = tymax;
    }
    if( ray->dir.z >= 0 ) {
        tzmin = (node->min.z - ray->begin.z) / ray->dir.z;
        tzmax = (node->max.z - ray->begin.z) / ray->dir.z;
    } else {
        tzmin = (node->max.z - ray->begin.z) / ray->dir.z;
        tzmax = (node->min.z - ray->begin.z) / ray->dir.z;
    }
    if( (tmin > tzmax) || (tzmin > tmax) ) {
        return 0;
    }
    if( tzmin > tmin ) {
        tmin = tzmin;
    }
    if( tzmax < tmax ) {
        tmax = tzmax;
    }
    if( (tmin < 1.0f) && (tmax > 0.0f)) {
        if( node->split ) {
            for( int i = 0; i < 8; i++ ) {
                Octree_TraceRayRecursiveInternalMultithreaded( octree, node->childs[i], ray, threadNum );
            }
        } else {
            for( int i = 0; i < node->indexCount; i++ ) {
                *(octree->containIndicesMT[threadNum] + octree->containIndexCountMT[threadNum]) = node->indices[i];
                octree->containIndexCountMT[threadNum]++;
            }
        }
    }
    return 1;
}

static inline float squared(float v) {
    return v * v;
}

char OctreeNodeIntersectSphere( TOctreeNode * node, TSphereShape * sphere ) {
    float r2 = sphere->radius * sphere->radius;
    float dmin = 0;

    if( sphere->position.x < node->min.x ) {
        dmin += squared( sphere->position.x - node->min.x );
    } else if( sphere->position.x > node->max.x ) {
        dmin += squared( sphere->position.x - node->max.x );
    }

    if( sphere->position.y < node->min.y ) {
        dmin += squared( sphere->position.y - node->min.y );
    } else if( sphere->position.y > node->max.y ) {
        dmin += squared( sphere->position.y - node->max.y );
    }

    if( sphere->position.z < node->min.z ) {
        dmin += squared( sphere->position.z - node->min.z );
    } else if( sphere->position.z > node->max.z ) {
        dmin += squared( sphere->position.z - node->max.z );
    }

    char sphereInside = (sphere->position.x >= node->min.x) && (sphere->position.x <= node->max.x) &&
                        (sphere->position.y >= node->min.y) && (sphere->position.y <= node->max.y) &&
                        (sphere->position.z >= node->min.z) && (sphere->position.z <= node->max.z);

    return dmin <= r2 || sphereInside;
}

void Octree_GetContainIndexListRecursiveInternal( TOctree * octree, TOctreeNode * node, TSphereShape * sphere ) {
    if( OctreeNodeIntersectSphere( node, sphere ) ) {
        if( node->split ) {
            for( int i = 0; i < 8; i++ ) {
                Octree_GetContainIndexListRecursiveInternal( octree, node->childs[i], sphere );
            }
        } else {
            for( int i = 0; i < node->indexCount; i++ ) {
                octree->containIndices[ octree->containIndexCount ] = node->indices[i];
                octree->containIndexCount++;
            }
        }
    }
}

char Octree_IsPointInsideNode( TOctreeNode * node, TVec3 * point ) {
    return	point->x >= node->min.x && point->x <= node->max.x &&
            point->y >= node->min.y && point->y <= node->max.y &&
            point->z >= node->min.z && point->z <= node->max.z;
}

void Octree_GetContainIndex( TOctree * octree, TSphereShape * sphere ) {
    octree->containIndexCount = 0;

    Octree_GetContainIndexListRecursiveInternal( octree, octree->root, sphere );
}

void Octree_BuildRecursiveInternal( TOctreeNode * node, TTriangle * triangles, int triangleCount, int * indices, int indexCount, int maxTrianglesPerNode ) {
    if( indexCount < maxTrianglesPerNode ) {
        int sizeBytes = sizeof( int ) * indexCount;
        node->indexCount = indexCount;
        node->indices = Memory_AllocateClean( sizeBytes );
        memcpy( node->indices, indices, sizeBytes );
        return;
    }

    Octree_SplitNode( node );

    for( int childNum = 0; childNum < 8; childNum++ ) {
        TOctreeNode * child = node->childs[childNum];

        TVec3 middle = Vec3_Middle( child->min, child->max );
        TVec3 halfSize = Vec3_Scale( Vec3_Sub( child->max, child->min ), 0.5f );

        float center[3] = {middle.x, middle.y, middle.z};
        float half[3] = {halfSize.x, halfSize.y, halfSize.z};

        // count triangles in each leaf 
        int leafTriangleCount = 0;

        for( int i = 0; i < triangleCount; i++ ) {
            TTriangle * triangle = triangles + i;
            float triverts[3][3] = { 	{triangle->a.x,triangle->a.y, triangle->a.z},
                {triangle->b.x,triangle->b.y, triangle->b.z},
                {triangle->c.x,triangle->c.y, triangle->c.z}
            };
            if( triBoxOverlap( center, half, triverts ) ||
                    Octree_IsPointInsideNode( child, &triangle->a ) ||
                    Octree_IsPointInsideNode( child, &triangle->b ) ||
                    Octree_IsPointInsideNode( child, &triangle->c ) ) {
                leafTriangleCount++;
            }
        }

        // allocate memory for temporary leaf and fill it with data 
        int * leafIndices = Memory_NewCount( leafTriangleCount, int );
        int triangleNum = 0;
        for( int i = 0; i < triangleCount; i++ ) {
            TTriangle * triangle = triangles + i;

            float triverts[3][3] = { 	{triangle->a.x,triangle->a.y, triangle->a.z},
                {triangle->b.x,triangle->b.y, triangle->b.z},
                {triangle->c.x,triangle->c.y, triangle->c.z}
            };
            if( triBoxOverlap( center, half, triverts )||
                    Octree_IsPointInsideNode( child, &triangle->a ) ||
                    Octree_IsPointInsideNode( child, &triangle->b ) ||
                    Octree_IsPointInsideNode( child, &triangle->c ) ) {
                *(leafIndices + triangleNum) = i;
                triangleNum++;
            }
        }

        // recursively process childs of this node 
        Octree_BuildRecursiveInternal( child, triangles, triangleCount, leafIndices, triangleNum, maxTrianglesPerNode );

        // leafFaces data already copied to another node, so we can free temporary array 
        Memory_Free( leafIndices );
    }
}

void Octree_SplitNode( TOctreeNode * node ) {
    TVec3 center = Vec3_Middle( node->min, node->max );

    for(int i = 0; i < 8; i++) {
        node->childs[i] = Memory_New( TOctreeNode );
        node->childs[i]->split = 0;
        node->childs[i]->indices = 0;
        node->childs[i]->indexCount = 0;
    }

    node->childs[0]->min = Vec3_Set( node->min.x, node->min.y, node->min.z );
    node->childs[0]->max = Vec3_Set( center.x, center.y, center.z );

    node->childs[1]->min = Vec3_Set( center.x, node->min.y, node->min.z );
    node->childs[1]->max = Vec3_Set( node->max.x, center.y, center.z );

    node->childs[2]->min = Vec3_Set( node->min.x, node->min.y, center.z );
    node->childs[2]->max = Vec3_Set( center.x, center.y, node->max.z );

    node->childs[3]->min = Vec3_Set( center.x, node->min.y, center.z );
    node->childs[3]->max = Vec3_Set( node->max.x, center.y, node->max.z );

    node->childs[4]->min = Vec3_Set( node->min.x, center.y, node->min.z );
    node->childs[4]->max = Vec3_Set( center.x, node->max.y, center.z );

    node->childs[5]->min = Vec3_Set( center.x, center.y, node->min.z );
    node->childs[5]->max = Vec3_Set( node->max.x, node->max.y, center.z );

    node->childs[6]->min = Vec3_Set( node->min.x, center.y, center.z );
    node->childs[6]->max = Vec3_Set( center.x, node->max.y, node->max.z );

    node->childs[7]->min = Vec3_Set( center.x, center.y, center.z );
    node->childs[7]->max = Vec3_Set( node->max.x, node->max.y, node->max.z );

    node->split = true;
}


