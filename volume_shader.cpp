#include "volume_shader.h"

#include "util.h"
volume_shader_t::volume_shader_t() {
  add_stage<yavin::vertexshader>("volume.vert");
  add_stage<yavin::fragmentshader>("volume.frag");
  create();
  set_cube_front_tex_binding(0);
  set_cube_back_tex_binding(1);
  set_volume_data_tex_binding(2);
  set_alpha_tex_binding(3);
  set_mode(1);
  set_projection_matrix(orthographic_matrix(0, 1, 0, 1, -1, 1));
}
void volume_shader_t::set_projection_matrix(mat4 const& data) {
  set_uniform_mat4("projection_mat", data.data());
}
void volume_shader_t::set_cube_front_tex_binding(int b) {
  set_uniform("cube_front_tex", b);
}
void volume_shader_t::set_cube_back_tex_binding(int b) {
  set_uniform("cube_front_tex", b);
}
void volume_shader_t::set_volume_data_tex_binding(int b) {
  set_uniform("volume_data_tex", b);
}
void volume_shader_t::set_alpha_tex_binding(int b) {
  set_uniform("alpha_tex", b);
}
void volume_shader_t::set_sample_distance(float d) {
  set_uniform("sample_distance", d);
}
void volume_shader_t::set_mode(int m) { set_uniform("mode", m); }
