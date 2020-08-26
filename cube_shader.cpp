#include "cube_shader.h"
cube_shader_t::cube_shader_t() {
  add_stage<yavin::vertexshader>("cube.vert");
  add_stage<yavin::fragmentshader>("cube.frag");
  create();
}
void cube_shader_t::set_modelview_matrix(mat4 const& data) {
  set_uniform_mat4("modelview_mat", data.data());
}void cube_shader_t::set_projection_matrix(mat4 const& data) {
  set_uniform_mat4("projection_mat", data.data());
}
