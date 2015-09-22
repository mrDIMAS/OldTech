#include "collision.h"
#include <float.h>

float g_gravityAccel = 9.81;
TDynamicsWorld g_dynamicsWorld = { { NULL, NULL, 0 }, { NULL, NULL, 0 }, NULL, NULL };

TRay Ray_Set( TVec3 begin, TVec3 end ) {
    return (TRay) { .begin = begin, .end = end, .dir = Vec3_Sub( end, begin ) };
}

TRay Ray_SetDirection( TVec3 begin, TVec3 direction ) {
    return (TRay) { .begin = begin, .end = direction, .dir = direction };
}

void Plane_SetTriangle( TPlane * plane, const TTriangle * triangle ) {
    // plane equation is Ax + By + Cz + d = 0 
    plane->normal = triangle->normal;
    plane->dist = triangle->distance;
}

void Plane_Set( TPlane * plane, const TVec3 * normal, float d ) {
    plane->normal = *normal;
    plane->dist = d;
}

TSphereShape SphereShape_Set( TVec3 position, float radius ) {
    return (TSphereShape) { .position = position, .radius = radius };
}

void Triangle_Set( TTriangle * triangle, const TVec3 * a, const TVec3 * b, const TVec3 * c ) {
    triangle->a = *a;
    triangle->b = *b;
    triangle->c = *c;

    // find vectors from triangle vertices 
    TVec3 ba = Vec3_Sub( *b, *a );
    TVec3 ca = Vec3_Sub( *c, *a );

    // normal of triangle is a cross product of above vectors
    triangle->normal = Vec3_Normalize( Vec3_Cross( ba, ca ));

    // find triangle edges 
    triangle->ab = Vec3_Sub( *a, *b );
    triangle->bc = Vec3_Sub( *b, *c );
    triangle->ca = Vec3_Sub( *c, *a );    
    triangle->ba = ba;
    
    triangle->abRay = Ray_Set( *a, *b );
    triangle->bcRay = Ray_Set( *b, *c );
    triangle->caRay = Ray_Set( *c, *a );
    
    // precalculate dot-products for barycentric method 
    triangle->caDotca = Vec3_Dot( ca, ca );
    triangle->caDotba = Vec3_Dot( ca, ba );
    triangle->baDotba = Vec3_Dot( ba, ba );
    triangle->invDenom = 1.0f / (triangle->caDotca * triangle->baDotba - triangle->caDotba * triangle->caDotba);

    triangle->distance = -Vec3_Dot( triangle->a, triangle->normal );
    
    // it can be set in proper functions 
    triangle->material = NULL;
}

char Triangle_CheckPoint( const TVec3 * point, const TTriangle * triangle ) {
    // fast barycentric coordinates method 
    TVec3 vp = Vec3_Sub( *point, triangle->a );

    float dot02 = Vec3_Dot( triangle->ca, vp);
    float dot12 = Vec3_Dot( triangle->ba, vp);

    float u = (triangle->baDotba * dot02 - triangle->caDotba * dot12) * triangle->invDenom;
    float v = (triangle->caDotca * dot12 - triangle->caDotba * dot02) * triangle->invDenom;

    return (u >= 0.0f) && (v >= 0.0f) && (u + v < 1.0f);
}

void Dynamics_AddConstraint( TConstraint * constraint ) {
    List_Add( &g_dynamicsWorld.constraints, constraint );
}

bool Intersection_RayPlane( const TRay * ray, const TPlane * plane, TVec3 * outIntersectPoint, ERayType rayType ) {
    // solve plane equation 
    float u = -( Vec3_Dot( ray->begin, plane->normal ) + plane->dist );
    float v = Vec3_Dot( ray->dir, plane->normal );
    float t = u / v;

    // ray miss 
    if (t < 0.0f) {
        return false;
    }

    // find intersection point 
    *outIntersectPoint = Vec3_Add( ray->begin, Vec3_Scale( ray->dir, t ));

    if( rayType == RAY_INFINITE ) {
        return true;
    }

    if( rayType == RAY_LINE_SEGMENT ) {
        return ((t >= 0.0f) && (t <= 1.0f));
    }

    // bad ray type 
    return false;
}


bool Intersection_RayTriangle( const TRay * ray, const TTriangle * triangle, TVec3 * outIntersectPoint, ERayType rayType ) {
    // build plane 
    TPlane plane;
    Plane_SetTriangle( &plane, triangle );

    if( !Intersection_RayPlane( ray, &plane, outIntersectPoint, rayType )) {
        return false;
    }

    return Triangle_CheckPoint( outIntersectPoint, triangle );
}

float Plane_Distance( const TPlane * plane, const TVec3 * point ) {
    return fabs( Vec3_Dot( plane->normal, *point ) + plane->dist );
}

char Geometry_PointOnLineSegment( TVec3 * point, const TVec3 * a, const TVec3 * b ) {
    // simply check, if point lies in bounding box (a,b), means that point is on line 
    TVec3 min = *a;
    TVec3 max = *b;

    // swap coordinates if needed 
    float temp;
    if( min.x > max.x ) {
        temp = min.x;
        min.x = max.x;
        max.x = temp;
    }
    if( min.y > max.y ) {
        temp = min.y;
        min.y = max.y;
        max.y = temp;
    }
    if( min.z > max.z ) {
        temp = min.z;
        min.z = max.z;
        max.z = temp;
    }

    if( (point->x > max.x) || (point->y > max.y) || (point->z > max.z) ) {
        return 0;
    }
    if( (point->x < min.x) || (point->y < min.y) || (point->z < min.z) ) {
        return 0;
    }
    return 1;
}

char Intersection_RaySphere( const TRay * ray, const TSphereShape * sphere, TVec3 * intPoint1, TVec3 * intPoint2, ERayType rayType ) {
    TVec3 d = Vec3_Sub( ray->begin, sphere->position );

    float a = Vec3_Dot( ray->dir, ray->dir );
    float b = 2.0f * Vec3_Dot( ray->dir, d );
    float c = Vec3_Dot( d, d ) - sphere->radius * sphere->radius;

    float discriminant = b * b - 4 * a * c;

    if( discriminant < 0.0f ) {
        return 0;
    }

    float discrRoot = sqrt( discriminant );
    // find roots of quadratic equation 
    float r1 = (-b + discrRoot) / 2.0f;
    float r2 = (-b - discrRoot) / 2.0f;

    // write points out if needed 
    if( intPoint1 ) {        
        *intPoint1 = Vec3_Add( ray->begin, Vec3_Scale( ray->dir, r1 ));
    }

    if( intPoint2 ) {        
        *intPoint2 = Vec3_Add( ray->begin, Vec3_Scale( ray->dir, r2 ));
    }

    if( rayType == RAY_INFINITE ) {
        return 1;
    }

    if( rayType == RAY_LINE_SEGMENT ) {
        return ((r1 >= 0.0f) && (r1 <= 1.0f)) || ((r2 >= 0.0f) && (r2 <= 1.0f));
    }

    // bad ray type 
    return 0;
}

char Intersection_SphereSphere( const TSphereShape * sphere1, const TSphereShape * sphere2, float * penetrationDepth ) {
    TVec3 delta = Vec3_Sub( sphere1->position, sphere2->position );

    float sqrDistance = Vec3_Dot( delta, delta );
    float sqrRadius = (sphere1->radius + sphere2->radius) * (sphere1->radius + sphere2->radius);

    char intersection = sqrDistance <= sqrRadius;

    if( penetrationDepth ) {
        if( intersection ) {
            *penetrationDepth = ( sqrRadius - sqrDistance ) / 2.0f;
        } else {
            *penetrationDepth = 0.0f;
        }
    }

    return intersection;
}

char Intersection_SpherePoint( const TSphereShape * sphere, const TVec3 * point ) {
    TVec3 delta = Vec3_Sub( sphere->position, *point );
    return Vec3_Dot( delta, delta ) <= (sphere->radius * sphere->radius);
}

TVec3 Geometry_ProjectPointOnLine( TVec3 point, TVec3 a, TVec3 b ) {
    // A + dot(AP,AB) / dot(AB,AB) * AB	 
    TVec3 ap = Vec3_Sub( point, a );
    TVec3 ab = Vec3_Sub( b, a );    
    float div = Vec3_Dot( ab, ab );
    // degenerated vector can produce NAN result, prevent it 
    if( div < 0.000001f ) {
        return Vec3_Set( 0.0f, 0.0f, 0.0f );
    }
    return Vec3_Add( a, Vec3_Scale( ab, Vec3_Dot( ap, ab ) / div ) );
}

void Ray_TraceWorld( TRay * ray, TRayTraceResult * out ) {
    TTriangle * nearestTriangle = 0;
    TVec3 nearestPosition;
    TVec3 nearestNormal;
    TBody * nearestBody = 0;
    float distanceToNearest = FLT_MAX;

    for_each( TBody, body, g_dynamicsWorld.bodies ) {
        // trace ray through polygon to find nearest triangle 
        if( body->shape->type == SHAPE_POLYGON ) {
            // trace ray through polygon's octree 
            Octree_TraceRay( &body->shape->octree, ray );
            for( int i = 0; i < body->shape->octree.containIndexCount; i++ ) {
                int currentTriangleIndex = body->shape->octree.containIndices[i];
                TTriangle * triangle = &body->shape->triangles[ currentTriangleIndex ];
                TVec3 intersectionPoint;
                const float sqrDistanceThreshold = 0.025f;
                if( Intersection_RayTriangle( ray, triangle, &intersectionPoint, RAY_INFINITE ) ) {
                    float sqrDist = Vec3_SqrDistance( intersectionPoint, ray->begin );
                    if( sqrDist < distanceToNearest ) {
                        distanceToNearest = sqrDist;
                        nearestTriangle = triangle;
                        nearestPosition = intersectionPoint;
                        nearestNormal = triangle->normal;
                        nearestBody = body;                            
                        if( sqrDist < sqrDistanceThreshold ) {
                            break;
                        }
                    }
                }                
            }
        }
        // trace ray through spheres 
        if( body->shape->type == SHAPE_SPHERE ) {
            TSphereShape sph = SphereShape_Set( body->position, body->shape->sphereRadius );
            TVec3 ip1, ip2;
            Intersection_RaySphere( ray, &sph, &ip1, &ip2, RAY_INFINITE );
        }
    }

    out->body = nearestBody;
    out->position = nearestPosition;
    out->triangle = nearestTriangle;
    out->normal = nearestNormal;
}

void Ray_TraceWorldMultithreaded( TRay * ray, TRayTraceResult * out, int threadNum ) {
    TTriangle * nearestTriangle = 0;
    TVec3 nearestPosition;
    TVec3 nearestNormal;
    TBody * nearestBody = 0;
    float distanceToNearest = FLT_MAX;

    for_each( TBody, body, g_dynamicsWorld.bodies ) {
        // trace ray through polygon to find nearest triangle 
        if( body->shape->type == SHAPE_POLYGON ) {
            // trace ray through polygon's octree 
            Octree_TraceRayMultithreaded( &body->shape->octree, ray, threadNum );
            for( int i = 0; i < body->shape->octree.containIndexCountMT[threadNum]; i++ ) {
                int * ptr = body->shape->octree.containIndicesMT[threadNum];
                int currentTriangleIndex = *(ptr + i);               
                TTriangle * triangle = &body->shape->triangles[ currentTriangleIndex ];
                TVec3 intersectionPoint;
                const float sqrDistanceThreshold = 0.025f;
                if( Intersection_RayTriangle( ray, triangle, &intersectionPoint, RAY_INFINITE ) ) {
                    float sqrDist = Vec3_SqrDistance( intersectionPoint, ray->begin );
                    if( sqrDist < distanceToNearest ) {
                        distanceToNearest = sqrDist;
                        nearestTriangle = triangle;
                        nearestPosition = intersectionPoint;
                        nearestNormal = triangle->normal;
                        nearestBody = body;                            
                        if( sqrDist < sqrDistanceThreshold ) {
                            break;
                        }
                    }
                }  
            }
        }
    }

    out->body = nearestBody;
    out->position = nearestPosition;
    out->triangle = nearestTriangle;
    out->normal = nearestNormal;
}

bool EdgeSphereIntersection( const TRay * edgeRay, const TSphereShape * sphere, TVec3 * intersectionPoint ) {
    if( Intersection_RaySphere( edgeRay, sphere, 0, 0, RAY_INFINITE ) ) {
        *intersectionPoint = Geometry_ProjectPointOnLine( sphere->position, edgeRay->begin, edgeRay->end );
        if( Geometry_PointOnLineSegment( intersectionPoint, &edgeRay->begin, &edgeRay->end ) ) {
            return true;
        }
    }
    return false;
}

bool Intersection_SphereTriangle( const TSphereShape * sphere, const TTriangle * triangle, TVec3 * intersectionPoint ) {
    // build plane from triangle 
    TPlane plane;
    Plane_SetTriangle( &plane, triangle );

    // find distance from center of the sphere to the plane 
    float distance = Plane_Distance( &plane, &sphere->position );

    // check distance 
    if( distance <= sphere->radius ) {
        // project center of the sphere to the plane 
        // plane's normal vector is normalized! 
        *intersectionPoint = Vec3_Sub( sphere->position, Vec3_Scale( plane.normal, distance ));

        // check, that projected point lies in triangle 
        if( Triangle_CheckPoint( intersectionPoint, triangle ) ) {
            return true;
        } else {       
            //check each triangle edge intersection with the sphere                 
            if( EdgeSphereIntersection( &triangle->abRay, sphere, intersectionPoint )) {
                return true;
            }
            if( EdgeSphereIntersection( &triangle->bcRay, sphere, intersectionPoint )) {
                return true;
            }
            if(	EdgeSphereIntersection( &triangle->caRay, sphere, intersectionPoint )) {
                return true;
            }
        }
    }
    // no intersection 
    return false;
}
// build plane for specified box face, box is axis-aligned 
void Plane_SetBoxFace( TPlane * plane, const TVec3 * min, const TVec3 * max, int faceNum ) {
    switch( faceNum ) {
    // positive X face
    case 0: {
        plane->normal = Vec3_Set( 1.0f, 0.0f, 0.0f );
        plane->dist = -Vec3_Dot( *max, plane->normal );
        break;
    }
    // negative X face 
    case 1: {
        plane->normal = Vec3_Set( -1.0f, 0.0f, 0.0f );
        plane->dist = -Vec3_Dot( *min, plane->normal );
        break;
    }
    // positive Y face 
    case 2: {
        plane->normal = Vec3_Set( 0.0f, 1.0f, 0.0f );
        plane->dist = -Vec3_Dot( *max, plane->normal );
        break;
    }
    // negative Y face 
    case 3: {
        plane->normal = Vec3_Set( 0.0f, -1.0f, 0.0f );
        plane->dist = -Vec3_Dot( *min, plane->normal );
        break;
    }
    // positive Z face 
    case 4: {
        plane->normal = Vec3_Set( 0.0f, 0.0f, 1.0f );
        plane->dist = -Vec3_Dot( *max, plane->normal );
        break;
    }
    // negative Z face
    case 5: {
        plane->normal = Vec3_Set( 0.0f, 0.0f, -1.0f );
        plane->dist = -Vec3_Dot( *min, plane->normal );
        break;
    }
    }
}

// check that the point lies on the specified box's face 
char Box_PointOnFace( const TVec3 * point, const TBoxShape * box ) {
        return  (point->x >= box->min.x) && (point->x <= box->max.x ) &&
                (point->z >= box->min.z) && (point->z <= box->max.z ) && 
                (point->y >= box->min.y) && (point->y <= box->max.y ) ;
}

bool Intersection_RayBox( const TRay * ray, const TBoxShape * box, TVec3 * outIntersectPoint ) {
    for( int i = 0; i < 6; i++ ) {
        TPlane boxFacePlane;
        Plane_SetBoxFace( &boxFacePlane, &box->min, &box->max, i );
        if( Intersection_RayPlane( ray, &boxFacePlane, outIntersectPoint, RAY_LINE_SEGMENT )) {
            if( Box_PointOnFace( outIntersectPoint, box )) {
                return true;
            }
        }         
    }
    return false;
}

// algorithm: represent triangle edges as rays and check that rays intersects with box face and vice versa:
// represent box edges as rays and check that rays intersects with the triangle plane 
bool Intersection_BoxTriangle( const TBoxShape * box, const TTriangle * triangle, TVec3 * intersectionPoint ) {
    // stage 1 
    // test triangle-box: test intersection of each edge with each face of the box 
    if( Intersection_RayBox( &triangle->abRay, box, intersectionPoint )) {
        return true;
    }
    if( Intersection_RayBox( &triangle->bcRay, box, intersectionPoint )) {
        return true;
    }
    if( Intersection_RayBox( &triangle->caRay, box, intersectionPoint )) {
        return true;
    }

    // if still no intersection, check box edge intersection with triangle 
    TRay ray = Ray_Set( box->vertices[0], box->vertices[1] );
    Intersection_RayTriangle( &ray, triangle, intersectionPoint, RAY_LINE_SEGMENT );

    return false;
}

void Shape_GetSurfacesExtents( const TList * surfaces, TVec3 * min, TVec3 * max ) {
    *min = Vec3_Set( FLT_MAX, FLT_MAX, FLT_MAX );
    *max = Vec3_Set( -FLT_MAX, -FLT_MAX, -FLT_MAX );
    for_each( TSurface, surface, *surfaces ) {
        for( int i = 0; i < surface->vertexCount; i++ ) {
            const TVec3 * point = &surface->vertices[ i ].p;
            if( point->x > max->x ) max->x = point->x;
            if( point->y > max->y ) max->y = point->y;
            if( point->z > max->z ) max->z = point->z;
            if( point->x < min->x ) min->x = point->x;
            if( point->y < min->y ) min->y = point->y;
            if( point->z < min->z ) min->z = point->z;
        }
    }
}

void Shape_BoxFromSurfaces( TCollisionShape * shape, const TList * surfaces ) {
    Shape_GetSurfacesExtents( surfaces, &shape->min, &shape->max );
    shape->type = SHAPE_AABB;
    shape->triangleCount = 0;
    shape->triangles = 0;
    shape->octree.root = 0;
    shape->sphereRadius = 0;
}

void Shape_SphereFromSurfaces( TCollisionShape * shape, const TList * surfaces ) {
    Shape_GetSurfacesExtents( surfaces, &shape->min, &shape->max );

    shape->type = SHAPE_SPHERE;
    shape->triangleCount = 0;
    shape->triangles = 0;
    shape->octree.root = 0;
    shape->sphereRadius = Vec3_Length( Vec3_Sub( shape->max, shape->min )) / 4.0f;
}

void Shape_PolygonFromSurfaces( TCollisionShape * shape, const TList * surfaces ) {
    shape->triangleCount = 0;
    // count triangles in all surfaces 
    for_each( TSurface, surface, *surfaces ) {
        shape->triangleCount += surface->faceCount;
    }    
    shape->type = SHAPE_POLYGON;
    shape->triangles = Memory_NewCount( shape->triangleCount, TTriangle );
    shape->sphereRadius = 0;
    // copy triangles 
    int triangleNum = 0, faceNum;
    for_each( TSurface, surf, *surfaces ) {
        for( faceNum = 0; faceNum < surf->faceCount; faceNum++ ) {
            TFace * face = &surf->faces[ faceNum ];
            Triangle_Set( &shape->triangles[triangleNum], &surf->vertices[face->index[0]].p, &surf->vertices[face->index[1]].p, &surf->vertices[face->index[2]].p );
            // set material of triangle according to surface texture 
            shape->triangles[triangleNum].material = surf->texture;
            triangleNum++;
        }
    }
    shape->octree.containIndexCount = 0;
    shape->octree.root = 0;
    // build octree 
    Octree_Build( &shape->octree, shape->triangles, shape->triangleCount, 64 );
}

void Dynamics_SphereSphereCollision( TBody * sphere1, TBody * sphere2 ) {
    TSphereShape sph1 = SphereShape_Set( sphere1->position, sphere1->shape->sphereRadius );
    TSphereShape sph2 = SphereShape_Set( sphere2->position, sphere2->shape->sphereRadius );

    float penetrationDepth;
    char intersection = Intersection_SphereSphere( &sph1, &sph2, &penetrationDepth );

    if( intersection ) {
        TVec3 middle = Vec3_Sub( sphere1->position, sphere2->position );
        TVec3 direction = Vec3_Normalize( middle );
        TVec3 offset = Vec3_Scale( direction, penetrationDepth / 2.0f );
        sphere1->position = Vec3_Add( sphere1->position, offset );
        sphere2->position = Vec3_Sub( sphere2->position, offset );
        // project linear velocities on a fake plane 
        Plane_ProjectVector( &sphere1->linearVelocity, &sphere1->linearVelocity, &direction );
        Plane_ProjectVector( &sphere2->linearVelocity, &sphere2->linearVelocity, &direction );
        // fill contact information 
        sphere1->contacts[ sphere1->contactCount ].body = sphere2;
        sphere1->contacts[ sphere1->contactCount ].normal = direction;
        sphere1->contacts[ sphere1->contactCount ].triangle = 0;
        sphere1->contacts[ sphere1->contactCount ].position = middle;
        if( sphere1->contactCount < MAX_CONTACTS ) {
            sphere1->contactCount++;
        }
        sphere2->contacts[ sphere2->contactCount ].body = sphere1;
        sphere2->contacts[ sphere2->contactCount ].normal = direction;
        sphere2->contacts[ sphere2->contactCount ].triangle = 0;
        sphere2->contacts[ sphere2->contactCount ].position = middle;
        if( sphere2->contactCount < MAX_CONTACTS ) {
            sphere2->contactCount++;
        }
        if( g_dynamicsWorld.SphereSphereCollisionCallback ) {
            g_dynamicsWorld.SphereSphereCollisionCallback( sphere1, sphere2 );
        }
    }
}

// project vector onto plane
void Plane_ProjectVector( TVec3 * out, const TVec3 * a, const TVec3 * planeNormal ) {
    // proj-plane(a) = a - planeNormal * ((a * planeNormal) / |planeNormal|^2) 
    float nlen = Vec3_SqrLength( *planeNormal );
    // normal vector is degenerated 
    if( nlen < C_EPSILON ) {
        *out = Vec3_Zero();
        return;
    }
    float t = Vec3_Dot( *a, *planeNormal ) / nlen;   
    *out = Vec3_Sub( *a, Vec3_Scale( *planeNormal, t ));
}

void Dynamics_SpherePolygonCollision( TBody * sphere, TBody * polygon ) {
    TSphereShape sph = SphereShape_Set( sphere->position, sphere->shape->sphereRadius );
    // now find list of triangles that are close enough to our sphere 
    Octree_GetContainIndex( &polygon->shape->octree, &sph );
    int lastTriangleIndex = -1;
    // iterate over it and do collision detection 
    for( int i = 0; i < polygon->shape->octree.containIndexCount; i++ ) {
        TTriangle * triangle = polygon->shape->triangles + polygon->shape->octree.containIndices[i];
        int currentTriangleIndex = polygon->shape->octree.containIndices[i];
        // remove intersection check with duplicated triangle indices
        if( currentTriangleIndex != lastTriangleIndex ) {
            TVec3 intersectionPoint;
            if( Intersection_SphereTriangle( &sph, triangle, &intersectionPoint) ) {
                float length = 0.0f;
                TVec3 middle = Vec3_Sub( sphere->position, intersectionPoint );
                TVec3 direction = Vec3_NormalizeEx( middle, &length );
                float penetrationDepth = sphere->shape->sphereRadius - length;
                // degenerated case, ignore 
                if( penetrationDepth < 0 ) continue;                
                sphere->position = Vec3_Add( sphere->position, Vec3_Scale( direction, penetrationDepth * 1.01f ));
                // perform sliding by projecting velocity vector on triangle plane 
                Plane_ProjectVector( &sphere->linearVelocity, &sphere->linearVelocity, &triangle->normal );
                // write contact info 
                if( sphere->contactCount < MAX_CONTACTS ) {
                    sphere->contacts[ sphere->contactCount ].normal = direction;
                    sphere->contacts[ sphere->contactCount ].triangle = triangle;
                    sphere->contacts[ sphere->contactCount ].body = polygon;
                    sphere->contacts[ sphere->contactCount ].position = intersectionPoint;
                    sphere->contactCount++;
                }
                // contact count of polygon body can be exteremely high, so skip polygon's contact info 
                if( polygon->contactCount < MAX_CONTACTS ) {
                    polygon->contactCount++;
                }
                // use callback if defined 
                if( g_dynamicsWorld.SphereTriangleCollisionCallback ) {
                    g_dynamicsWorld.SphereTriangleCollisionCallback( sphere, polygon, triangle );
                }
            }
            lastTriangleIndex = currentTriangleIndex;
        }
    }
}

void BoxShape_Set( TBoxShape * box, const TVec3 * min, const TVec3 * max, const TVec3 * position ) {
    box->max = *max;
    box->min = *min;

    box->max = Vec3_Add( box->max, *position );
    box->min = Vec3_Add( box->min, *position );

    box->vertices[0] = Vec3_Set( box->min.x, box->min.y, box->min.z );
    box->vertices[1] = Vec3_Set( box->min.x, box->min.y, box->max.z );
    box->vertices[2] = Vec3_Set( box->max.x, box->min.y, box->max.z );
    box->vertices[3] = Vec3_Set( box->max.x, box->min.y, box->min.z );
    
    box->vertices[4] = Vec3_Set( box->min.x, box->max.y, box->min.z );
    box->vertices[5] = Vec3_Set( box->min.x, box->max.y, box->max.z );
    box->vertices[6] = Vec3_Set( box->max.x, box->max.y, box->max.z );
    box->vertices[7] = Vec3_Set( box->max.x, box->max.y, box->min.z );
    
    box->edges[0] = Ray_Set( box->vertices[0], box->vertices[1] );
    box->edges[1] = Ray_Set( box->vertices[1], box->vertices[2] );
    box->edges[2] = Ray_Set( box->vertices[0], box->vertices[3] );
    box->edges[3] = Ray_Set( box->vertices[2], box->vertices[3] );
    
    box->edges[4] = Ray_Set( box->vertices[4], box->vertices[5] );
    box->edges[5] = Ray_Set( box->vertices[5], box->vertices[6] );
    box->edges[6] = Ray_Set( box->vertices[4], box->vertices[7] );
    box->edges[7] = Ray_Set( box->vertices[6], box->vertices[7] );
    
    box->edges[8] = Ray_Set( box->vertices[0], box->vertices[4] );
    box->edges[9] = Ray_Set( box->vertices[1], box->vertices[5] );
    box->edges[10] = Ray_Set( box->vertices[2], box->vertices[6] );
    box->edges[11] = Ray_Set( box->vertices[3], box->vertices[7] );
}

void Dynamics_BoxPolygonCollision( TBody * box, TBody * polygon ) {
    TBoxShape aabb;
    BoxShape_Set( &aabb, &box->shape->min, &box->shape->max, &box->position );
    (void)polygon;
}

void Dynamics_CreateWorld() {
    g_dynamicsWorld.SphereSphereCollisionCallback = NULL;
    g_dynamicsWorld.SphereTriangleCollisionCallback = NULL;    
    List_Create( &g_dynamicsWorld.bodies );
    List_Create( &g_dynamicsWorld.constraints );
}

void Constraint_Create( TConstraint * constraint, TBody * body1, TBody * body2, float linkLength, float stiffness ) {
    constraint->body1 = body1;
    constraint->body2 = body2;
    constraint->length = linkLength;
    constraint->stiffness = stiffness;
}

void Dynamics_SolveConstraints( ) {
    for_each( TConstraint, constraint, g_dynamicsWorld.constraints ) {
        TBody * body1 = constraint->body1;
        TBody * body2 = constraint->body2;
        
        TVec3 delta = Vec3_Scale( Vec3_Normalize( Vec3_Sub( body2->position, body1->position )), constraint->length * 0.5f );
        TVec3 middle = Vec3_Middle( body1->position, body2->position );        
        TVec3 goal1 = Vec3_Sub( middle, delta );  
        TVec3 goal2 = Vec3_Add( middle, delta );  
        body1->position = Vec3_Add( body1->position, Vec3_Scale( Vec3_Sub( goal1, body1->position ), constraint->stiffness ));
        body2->position = Vec3_Add( body2->position, Vec3_Scale( Vec3_Sub( goal2, body2->position ), constraint->stiffness ));
    }
}

void Body_Create( TBody * body, TCollisionShape * shape ) {
    body->shape = shape;
    body->contactCount = 0;
    body->elasticity = 0.5f;
    body->linearVelocity = Vec3_Zero();
    body->position = Vec3_Zero();
}

void Body_ApplyGravity( TBody * body ) {
    static TVec3 gravity = { .x = 0.0f, .y = -0.00666f, .z = 0.0f };
    body->linearVelocity = Vec3_Add( body->linearVelocity, gravity );
}


void Dynamics_AddBody( TBody * body ) {
    List_Add( &g_dynamicsWorld.bodies, body );
}

void Shape_CreateSphere( TCollisionShape * shape, float radius ) {
    shape->sphereRadius = radius;
    shape->type = SHAPE_SPHERE;
    shape->triangles = 0;
    shape->triangleCount = 0;
}

void Dynamics_StepSimulation() {
    // solve constraints first
    Dynamics_SolveConstraints( );
    
    // iterate over bodies and update it 
    for_each( TBody, body, g_dynamicsWorld.bodies ) {
        Body_ApplyGravity( body );
        body->position = Vec3_Add( body->position, body->linearVelocity );
        body->contactCount = 0;        
        // check collisions with each other world's body 
        // collision pairs can be: sphere-sphere, sphere-polygon; polygon-sphere is not supported, 
        // because polygon must be static body, with identity transform 
        for_each( TBody, otherBody, g_dynamicsWorld.bodies ) {
            // prevent self-collision 
            if( otherBody != body ) {
                if( body->shape->type == SHAPE_SPHERE ) {
                    if( otherBody->shape->type == SHAPE_SPHERE ) {
                        Dynamics_SphereSphereCollision( body, otherBody );
                    } else if( otherBody->shape->type == SHAPE_POLYGON ) {
                        Dynamics_SpherePolygonCollision( body, otherBody );
                    }
                }
                if( body->shape->type == SHAPE_AABB ) {
                    if( otherBody->shape->type == SHAPE_POLYGON ) {
                        Dynamics_BoxPolygonCollision( body, otherBody );
                    }
                }
            }
        }
    }
}