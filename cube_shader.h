#ifndef CUBE_SHADER_H
#define CUBE_SHADER_H
#include <yavin>
#include "typedefs.h"
struct cube_shader_t : yavin::shader{
  cube_shader_t();
  void set_modelview_matrix(mat4 const& data);
  void set_projection_matrix(mat4 const& data);
};
#endif
