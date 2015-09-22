#ifndef _GRAPH_OCTREE_
#define _GRAPH_OCTREE_

#include "common.h"
#include "face.h"
#include "vertex.h"

OLDTECH_BEGIN_HEADER

typedef struct TGraphOctreeNode {
    int * indices;
    int indexCount;
    bool split;
    TVec3 min;
    TVec3 max;
    struct TGraphOctreeNode * childs[8];
} TGraphOctreeNode;

typedef struct TGraphOctree {
    int * containIndides;
    int containIndexCount;
    TGraphOctreeNode * root;
} TGraphOctree;

/*
void GraphOctree_Build( TGraphOctree * octree, struct TTriangle * triangles, int triangleCount, int maxTrianglesPerNode );
void GraphOctree_SplitNode( TGraphOctreeNode * node );
void GraphOctree_GetContainIndex( TGraphOctree * octree, struct TSphereShape * sphere );
bool GraphOctree_IsPointInsideNode( TGraphOctreeNode * node, TVector3 * point );

// these functions for internal use only
bool GraphOctree_TraceRayRecursiveInternal( TGraphOctree * octree, TGraphOctreeNode * node, const struct TRay * ray );
void GraphOctree_BuildRecursiveInternal( TGraphOctreeNode * node, struct TTriangle * triangles, int triangleCount, int * indices, int indexCount, int maxTrianglesPerNode );
void GraphOctree_GetContainIndexListRecursiveInternal( TGraphOctreeNode * octree, TGraphOctreeNode * node, struct TSphereShape * sphere );
*/

OLDTECH_END_HEADER

#endif