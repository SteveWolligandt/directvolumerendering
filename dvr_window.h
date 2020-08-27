#ifndef DVR_WINDOW_H
#define DVR_WINDOW_H
#include <yavin>
#include "cube_shader.h"
#include "volume_shader.h"
//==============================================================================
struct dvr_window : yavin::window {
  GLsizei m_width = 500, m_height = 500;

  cube_shader_t m_cube_shader;
  volume_shader_t m_volume_shader;

  yavin::tex1r<float> m_alpha_tex;
  std::vector<float> m_alpha_data;
  yavin::tex2rgb<float> m_cube_back_tex, m_cube_front_tex;
  yavin::tex3r<float> m_volume_data_tex;
  size_t const m_num_alpha_samples = 1000;

  yavin::indexeddata<vec3, vec3> m_cube_data;
  yavin::indexeddata<vec2> m_fullscreen_quad_data;

  float m_theta = M_PI, m_phi = M_PI / 2, m_radius = 3;
  float m_old_mouse_x, m_old_mouse_y;
  float m_cur_mouse_x, m_cur_mouse_y;
  bool m_mouse_down = false;
  float m_fov = 60;
  float m_nearplane = 0.001;
  float m_farplane = 100;
  float m_sample_distance = 0.01;
  //==============================================================================
  dvr_window();
  //==============================================================================
  void update_modelview_matrix();
  //------------------------------------------------------------------------------
  void render_loop();
  //------------------------------------------------------------------------------
  void create_volume_data();
  //------------------------------------------------------------------------------
  void render_volume();
  //------------------------------------------------------------------------------
  void render_cube();
};
#endif
