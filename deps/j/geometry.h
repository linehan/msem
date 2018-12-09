#ifndef _LIBJDL_MATH_GEOMETRY_H
#define _LIBJDL_MATH_GEOMETRY_H


extern float Q_rsqrt(float number);
#define rsqrt Q_rsqrt

extern void   fast_normalize(float *a, float *b, float *c);
extern float  fast_norm     (float a, float b, float c);
extern float  euclidean_norm(float a, float b, float c);


#endif

