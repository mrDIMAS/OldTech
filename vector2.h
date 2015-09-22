#ifndef _VECTOR2_
#define _VECTOR2_

typedef struct {
    float x;
    float y;
} TVector2;

static inline void Vector2_Zero( TVector2 * v ) {
    v->x = 0.0f;
    v->y = 0.0f;
}

static inline void Vector2_Set( TVector2 * v, float x, float y ) {
    v->x = x;
    v->y = y;
}

static inline float Vector2_Dot( const TVector2 * a, const TVector2 * b ) {
    return a->x * b->x + a->y * b->y;
}

static inline void Vector2_Subtract( TVector2 * out, const TVector2 * a, const TVector2 * b ) {
    out->x = a->x - b->x;
    out->y = a->y - b->y;
}

#endif