#ifndef UTIL_H
#define UTIL_H
#include "typedefs.h"
mat4 orthographic_matrix(float const l, float const r, float const b,
                         float const t, float const n, float const f);
//------------------------------------------------------------------------------
mat4 perspective_matrix(float const l, float const r, float const b,
                        float const t, float const n, float const f);
//------------------------------------------------------------------------------
mat4 perspective_matrix(float const angle_of_view,
                        float const image_aspect_ratio, float const near,
                        float const far);
//------------------------------------------------------------------------------
mat4 look_at_matrix(vec3 const& eye, vec3 const& to, vec3 const& up);
#endif
