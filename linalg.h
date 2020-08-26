#ifndef LINALG_H
#define LINALG_H
#include "typedefs.h"
vec3 cross(vec3 const& a, vec3 const& b);
float dot(vec3 const& a, vec3 const& b);
void normalize(vec3& vec);
void scale(vec3& v, float s);
#endif
