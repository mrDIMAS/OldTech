#ifndef _OCTREE_
#define _OCTREE_

/* This octree used only for collision detection and lightmap generation
 * for graphics needs use TGraphOctree defined in graphoctree.h
 */

#include "common.h"
#include "face.h"
#include "vertex.h"
#include "aabbTri.h"

struct TTriangle;
struct TSphereShape;
struct TRay;

typedef struct SOctreeNode {
    int * indices;
    int indexCount;
    char split;
    TVec3 min;
    TVec3 max;
    struct SOctreeNode * childs[8];
} TOctreeNode;

#define OCTREE_MAX_SIMULTANEOUS_THREADS (8)

typedef struct SOctree {
    int * containIndices;
    int containIndexCount;
    
    // for multithreaded raytracing
    int * containIndicesMT[OCTREE_MAX_SIMULTANEOUS_THREADS];
    int containIndexCountMT[OCTREE_MAX_SIMULTANEOUS_THREADS];
    
    TOctreeNode * root;
} TOctree;

char OctreeNodeIntersectSphere( TOctreeNode * node, struct TSphereShape * sphere );
void Octree_Build( TOctree * octree, struct TTriangle * triangles, int triangleCount,int maxTrianglesPerNode );
void Octree_TraceRay( TOctree * octree, const struct TRay * ray );
void Octree_SplitNode( TOctreeNode * node );
void Octree_GetContainIndex( TOctree * octree, struct TSphereShape * sphere );
void Octree_GetContainIndexListRecursiveInternal( TOctree * octree, TOctreeNode * node, struct TSphereShape * sphere );
char Octree_IsPointInsideNode( TOctreeNode * node, TVec3 * point );
char Octree_TraceRayRecursiveInternal( TOctree * octree, TOctreeNode * node, const struct TRay * ray );
void Octree_BuildRecursiveInternal( TOctreeNode * node, struct TTriangle * triangles, int triangleCount, int * indices, int indexCount, int maxTrianglesPerNode );

void Octree_TraceRayMultithreaded( TOctree * octree, const struct TRay * ray, int threadNum );
char Octree_TraceRayRecursiveInternalMultithreaded( TOctree * octree, TOctreeNode * node, const struct TRay * ray, int threadNum );

#endif