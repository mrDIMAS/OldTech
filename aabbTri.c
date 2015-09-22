#include "aabbTri.h"

int planeBoxOverlap(float normal[3], float vert[3], float maxbox[3]) {
    int q;
    float vmin[3],vmax[3],v;
    for(q=X; q<=Z; q++) {
        v=vert[q];
        if(normal[q]>0.0f) {
            vmin[q]=-maxbox[q] - v;
            vmax[q]= maxbox[q] - v;
        } else {
            vmin[q]= maxbox[q] - v;
            vmax[q]=-maxbox[q] - v;
        }
    }
    if(DOT(normal,vmin)>0.0f) {
        return 0;
    }
    if(DOT(normal,vmax)>=0.0f) {
        return 1;
    }
    return 0;
}

int triBoxOverlap(float boxcenter[3],float boxhalfsize[3],float triverts[3][3]) {
    float v0[3],v1[3],v2[3];
    float min,max,p0,p1,p2,rad,fex,fey,fez;
    float normal[3],e0[3],e1[3],e2[3];

    SUB(v0,triverts[0],boxcenter);
    SUB(v1,triverts[1],boxcenter);
    SUB(v2,triverts[2],boxcenter);

    /* compute triangle edges */
    SUB(e0,v1,v0);
    SUB(e1,v2,v1);
    SUB(e2,v0,v2);

    /*  test the 9 tests first */
    fex = fabsf(e0[X]);
    fey = fabsf(e0[Y]);
    fez = fabsf(e0[Z]);

    AXISTEST_X01(e0[Z], e0[Y], fez, fey);
    AXISTEST_Y02(e0[Z], e0[X], fez, fex);
    AXISTEST_Z12(e0[Y], e0[X], fey, fex);

    fex = fabsf(e1[X]);
    fey = fabsf(e1[Y]);
    fez = fabsf(e1[Z]);

    AXISTEST_X01(e1[Z], e1[Y], fez, fey);
    AXISTEST_Y02(e1[Z], e1[X], fez, fex);
    AXISTEST_Z0(e1[Y], e1[X], fey, fex);

    fex = fabsf(e2[X]);
    fey = fabsf(e2[Y]);
    fez = fabsf(e2[Z]);

    AXISTEST_X2(e2[Z], e2[Y], fez, fey);
    AXISTEST_Y1(e2[Z], e2[X], fez, fex);
    AXISTEST_Z12(e2[Y], e2[X], fey, fex);


    /* test in X-direction */
    FINDMINMAX(v0[X],v1[X],v2[X],min,max);
    if(min>boxhalfsize[X] || max<-boxhalfsize[X]) {
        return 0;
    }

    /* test in Y-direction */
    FINDMINMAX(v0[Y],v1[Y],v2[Y],min,max);
    if(min>boxhalfsize[Y] || max<-boxhalfsize[Y]) {
        return 0;
    }

    /* test in Z-direction */
    FINDMINMAX(v0[Z],v1[Z],v2[Z],min,max);
    if(min>boxhalfsize[Z] || max<-boxhalfsize[Z]) {
        return 0;
    }

    /*  test if the box intersects the plane of the triangle */
    /*  compute plane equation of triangle: normal*x+d=0 */
    CROSS(normal,e0,e1);

    if(!planeBoxOverlap(normal,v0,boxhalfsize)) {
        return 0;
    }

    return 1;   /* box and triangle overlaps */
}