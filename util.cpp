#include "util.h"
#include "linalg.h"
#include<cmath>
mat4 look_at_matrix(vec3 const& eye, vec3 const& to, vec3 const& up) {
  vec3 Z{eye[0] - to[0], eye[1] - to[1], eye[2] - to[2]};
  normalize(Z);
  auto X = cross(up, Z);
  normalize(X);
  auto const Y = cross(Z, X);
  return {  X[0],   X[1],   X[2], 0.0f,
            Y[0],   Y[1],   Y[2], 0.0f,
            Z[0],   Z[1],   Z[2], 0.0f,
          eye[0], eye[1], eye[2], 1.0f};
}
mat4 perspective_matrix(float const l, float const r, float const b,
                        float const t, float const n, float const f) {
  float const z = 0, no = -1;
  return {
    2 * n / (r - l), z, z, z,
    z, 2 * n / (t - b), z,z,
    (r + l) / (r - l), (t + b) / (t - b), -(f + n) / (f - n), no,
    z, z, -2 * f * n / (f - n), z
  };
}
mat4 perspective_matrix(float const angle_of_view,
                        float const image_aspect_ratio, float const near,
                        float const far) {
  float const scale = std::tan(angle_of_view * 0.5 * M_PI / 180) * near;
  float const r = image_aspect_ratio * scale;
  float const l = -r;
  float const t = scale;
  float const b = -t;
  return perspective_matrix(l, r, b, t, near, far);
}
mat4 orthographic_matrix(float const l, float const r,
                         float const b, float const t,
                         float const n, float const f) {
  return {
      2 / (r - l),        0,         0,         0,
      0,         2 / (t - b),        0,         0,
      0,         0,         -2 / (f - n),       0,
      -(r + l) / (r - l), -(t + b) / (t - b), -(f + n) / (f - n), 1};
}
