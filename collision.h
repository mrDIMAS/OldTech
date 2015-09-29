#ifndef _COLLISION_
#define _COLLISION_

#include "common.h"
#include "Vector3.h"
#include "surface.h"
#include "list.h"
#include "octree.h"

OLDTECH_BEGIN_HEADER

typedef struct TRay {
    TVec3 begin;
    TVec3 end;
    TVec3 dir;
} TRay;

typedef enum ERayType {
    RAY_INFINITE,
    RAY_LINE_SEGMENT,
} ERayType;

typedef struct TPlane {
    TVec3 normal;
    float dist;
} TPlane;

typedef struct TSphereShape {
    TVec3 position;
    float radius;
} TSphereShape;

// implementation of non-moving triangle
typedef struct TTriangle {
    TVec3 normal;
    // vertices 
    TVec3 a;
    TVec3 b;
    TVec3 c;
    // edges 
    TVec3 ab;
    TVec3 bc;
    TVec3 ca;
    // precalculated, for barycentric method 
    TVec3 ba;
    float caDotca;
    float caDotba;
    float baDotba;
    float invDenom;    
    // precalculated edge rays
    TRay abRay;
    TRay bcRay;
    TRay caRay;    
    // precalculated distance from center of coordinates
    float distance;    
    // additional info 
    // 'material' very useful in sound emitting on collision, it
    // provides to use proper sound according to kind of material
    TTexture * material;
} TTriangle;

typedef struct TBoxShape {
    TVec3 min, max;
    TVec3 vertices[8];
    TRay edges[12];
} TBoxShape;

typedef struct TCapsuleShape {
    TVec3 a; // begin of generatrix
    TVec3 b; // end of generatrix
    float radius; // radius of a capsule
} TCapsuleShape;

typedef enum EShapeType {
    // dynamic shapes 
    SHAPE_AABB,
    SHAPE_SPHERE,
    SHAPE_CAPSULE,
    // static shapes 
    SHAPE_POLYGON,
} EShapeType;

typedef struct TCollisionShape {
    EShapeType type;
    // polygon shape 
    TTriangle * triangles;
    TOctree octree;
    int triangleCount;
    // sphere 
    float sphereRadius;
    // box 
    TVec3 min;
    TVec3 max;    
    TCapsuleShape * capsule;
} TCollisionShape;

#define MAX_CONTACTS (16)

typedef struct TContact {
    TVec3 normal;
    TTriangle * triangle;
    struct TBody * body;
    TVec3 position;
} TContact;

typedef struct TBody {
    TVec3 position;
    TVec3 linearVelocity;
    TCollisionShape * shape;
    int contactCount;
    TContact contacts[ MAX_CONTACTS ];
    float elasticity;
} TBody;

typedef struct TDynamicsWorld {
    TList bodies;
    TList constraints;
    void (*SphereSphereCollisionCallback)( TBody * sph1, TBody * sph2 );
    void (*SphereTriangleCollisionCallback)( TBody * sph1, TBody * polygon, TTriangle * triangle );
} TDynamicsWorld;

typedef struct TRayTraceResult {
    TBody * body;
    TVec3 position;
    TVec3 normal;
    TTriangle * triangle;
} TRayTraceResult;

typedef struct SConstraint {
    TBody * body1;
    TBody * body2;
    float length;
    float stiffness;
} TConstraint;

typedef struct TCapsuleTriangleIntersectionInfo {
    TVec3 intersectionPoint;
    TVec3 applicableOffset;
    TVec3 normal;
    bool intersects;
} TCapsuleTriangleIntersectionInfo;

extern float g_gravityAccel;
extern TDynamicsWorld g_dynamicsWorld;

#define C_EPSILON (0.000001f)

bool EdgeSphereIntersection( const TRay * edgeRay, const TSphereShape * sphere, TVec3 * intersectionPoint );

TRay Ray_Set( TVec3 begin, TVec3 end );
TRay Ray_SetDirection( TVec3 begin, TVec3 direction );
void Ray_TraceWorldStatic( TRay * ray, TRayTraceResult * out );
// traces ray only through dynamic object, such as spheres and boxes
void Ray_TraceWorldDynamic( TRay * ray, TRayTraceResult * out ); 
// special multithreaded version, mostly used in lightmap generation
// threadNum can be 0 to 4, keep in mind that this function traces ray
// only through static geometry
void Ray_TraceWorldStaticMultithreaded( TRay * ray, TRayTraceResult * out, int threadNum );

TVec3 Geometry_ProjectPointOnLine( TVec3 point, TVec3 a, TVec3 b );
bool Geometry_PointOnLineSegment( TVec3 * point, const TVec3 * a, const TVec3 * b );

TPlane Plane_SetTriangle( const TTriangle * triangle );
void Plane_SetBoxFace( TPlane * plane, const TVec3 * min, const TVec3 * max, int faceNum );
TVec3 Plane_ProjectVector( TVec3 a, TVec3 planeNormal );
float Plane_Distance( const TPlane * plane, const TVec3 * point );

void Shape_GetSurfacesExtents( const TList * surfaces, TVec3 * min, TVec3 * max );
    
TSphereShape SphereShape_Set( TVec3 position, float radius );

TCollisionShape * CapsuleShape_Create( TVec3 a, TVec3 b, float radius );

void BoxShape_Set( TBoxShape * box, const TVec3 * min, const TVec3 * max, const TVec3 * position );
char Box_PointOnFace( const TVec3 * point, const TBoxShape * box );

void Triangle_Set( TTriangle * triangle, const TVec3 * a, const TVec3 * b, const TVec3 * c );
char Triangle_CheckPoint( const TVec3 * point, const TTriangle * triangle );

bool Intersection_RayPlane( const TRay * ray, const TPlane * plane, TVec3 * outIntersectPoint, ERayType rayType );
bool Intersection_RayTriangle( const TRay * ray, const TTriangle * triangle, TVec3 * outIntersectPoint, ERayType rayType  );
bool Intersection_RaySphere( const TRay * ray, const TSphereShape * sphere, TVec3 * intPoint1, TVec3 * intPoint2, ERayType rayType  );
char Intersection_SphereSphere( const TSphereShape * sphere1, const TSphereShape * sphere2, float * penetrationDepth );
bool Intersection_SphereTriangle( const TSphereShape * sphere, const TTriangle * triangle, TVec3 * intersectionPoint );
char Intersection_SpherePoint( const TSphereShape * sphere, const TVec3 * point );
bool Intersection_BoxTriangle( const TBoxShape * box, const TTriangle * triangle, TVec3 * intersectionPoint );
bool Intersection_RayBox( const TRay * ray, const TBoxShape * box, TVec3 * outIntersectPoint );
TCapsuleTriangleIntersectionInfo Intersection_CapsuleTriangle( const TCapsuleShape * capsule, const TTriangle * triangle );

void CollisionDebugDrawPoint( const TVec3 * point );

void Constraint_Create( TConstraint * constraint, TBody * body1, TBody * body2, float linkLength, float stiffness );

void Dynamics_AddConstraint( TConstraint * constraint );
void Dynamics_SphereSphereCollision( TBody * sphere1, TBody * sphere2 );
void Dynamics_SpherePolygonCollision( TBody * sphere, TBody * polygon );
void Dynamics_CapsulePolygonCollision( TBody * capsuleBody, TBody * polygon );
void Dynamics_BoxPolygonCollision( TBody * box, TBody * polygon );
void Dynamics_CreateWorld( void );
void Dynamics_AddBody( TBody * body );
void Dynamics_StepSimulation( void );
void Dynamics_SolveConstraints( void );

void Shape_SphereFromSurfaces( TCollisionShape * shape, const TList * surfaces );
void Shape_PolygonFromSurfaces( TCollisionShape * shape, const TList * surfaces );
void Shape_CreateSphere( TCollisionShape * shape, float radius );
void Shape_BoxFromSurfaces( TCollisionShape * shape, const TList * surfaces );

void Body_Create( TBody * body, TCollisionShape * shape );
void Body_ApplyGravity( TBody * body );

OLDTECH_END_HEADER

#endif