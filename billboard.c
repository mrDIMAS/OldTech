#include "billboard.h"


TBillboard * Billboard_Create( float w, float h ) {
    TBillboard * bb = Memory_New( TBillboard );
    bb->width = w;
    bb->height = h;
    bb->texture = NULL;
    return bb;
}