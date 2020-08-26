#include "linalg.h"
#include<cmath>
vec3 cross(vec3 const& a, vec3 const& b) {
  return {a[1] * b[2] - a[2] * b[1],
          a[2] * b[0] - a[0] * b[2],
          a[0] * b[1] - a[1] * b[0]};
}
float dot(vec3 const& a, vec3 const& b) {
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}
void normalize(vec3& v) {
  float const inv_len = 1 / std::sqrt(dot(v, v));
  v[0] *= inv_len;
  v[1] *= inv_len;
  v[2] *= inv_len;
}
void scale(vec3& v, float s) {
  v[0] *= s;
  v[1] *= s;
  v[2] *= s;
}
