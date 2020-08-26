#ifndef VOLUME_SHADER_H
#define VOLUME_SHADER_H
#include <yavin>
#include "typedefs.h"
struct volume_shader_t : yavin::shader{
  volume_shader_t();
  void set_projection_matrix(mat4 const& data);
  void set_cube_front_tex_binding(int b);
  void set_cube_back_tex_binding(int b);
  void set_volume_data_tex_binding(int b);
  void set_mode(int m);
};
#endif
