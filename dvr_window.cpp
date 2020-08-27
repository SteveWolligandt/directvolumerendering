#include "dvr_window.h"
#include "bezier_widget.h"

#include <chrono>

#include "fullscreen_quad.h"
#include "linalg.h"
#include "unit_cube.h"
#include "util.h"
dvr_window::dvr_window()
    : yavin::window{"Direct Volume Rendering", 500, 500},
      m_alpha_tex{yavin::LINEAR, yavin::CLAMP_TO_BORDER, 100},
      m_cube_back_tex{std::max<size_t>(m_width, 1),
                      std::max<size_t>(m_height, 1)},
      m_cube_front_tex{std::max<size_t>(m_width, 1),
                       std::max<size_t>(m_height, 1)},
      m_cube_data{unit_cube()},
      m_fullscreen_quad_data{fullscreen_quad()} {
  create_volume_data();
  m_cube_shader.set_projection_matrix(perspective_matrix(
      m_fov, (float)m_width / (float)m_height, m_nearplane, m_farplane));

  add_resize_event([this](int new_width, int new_height) {
    m_width = new_width;
    m_height = new_height;
    m_cube_back_tex.resize(std::max<size_t>(m_width, 1),
                           std::max<size_t>(m_height, 1));
    m_cube_front_tex.resize(std::max<size_t>(m_width, 1),
                            std::max<size_t>(m_height, 1));
    m_cube_shader.set_projection_matrix(perspective_matrix(
        m_fov, (float)m_width / (float)m_height, m_nearplane, m_farplane));
  });

  add_mouse_motion_event([this](int x, int y) {
    m_cur_mouse_x = x;
    m_cur_mouse_y = y;

    if (m_mouse_down) {
      auto off_x = m_cur_mouse_x - m_old_mouse_x;
      auto off_y = m_cur_mouse_y - m_old_mouse_y;
      m_old_mouse_x = m_cur_mouse_x;
      m_old_mouse_y = m_cur_mouse_y;

      m_theta -= off_x * 0.005;
      m_phi -= off_y * 0.005;
      m_phi = std::min<float>(M_PI, m_phi);
      m_phi = std::max<float>(0.0001, m_phi);
      update_modelview_matrix();
    }
  });

  add_key_pressed_event([this](yavin::key k) {
    if (k == yavin::KEY_1) {
      m_volume_shader.set_mode(1);
    } else if (k == yavin::KEY_2) {
      m_volume_shader.set_mode(2);
    } else if (k == yavin::KEY_3) {
      m_volume_shader.set_mode(3);
    } else if (k == yavin::KEY_4) {
      m_volume_shader.set_mode(4);
    }
  });
  //------------------------------------------------------------------------------
  add_button_pressed_event([this](yavin::button /*b*/) {
    m_old_mouse_x = m_cur_mouse_x;
    m_old_mouse_y = m_cur_mouse_y;
    m_mouse_down = true;
  });
  //------------------------------------------------------------------------------
  add_button_released_event(
      [this](yavin::button /*b*/) { m_mouse_down = false; });
  //------------------------------------------------------------------------------
  add_wheel_up_event([this] {
    m_radius -= 0.05;

    m_radius = std::max<float>(m_radius, 0.001f);
    update_modelview_matrix();
  });
  //------------------------------------------------------------------------------
  add_wheel_down_event([this]() {
    m_radius += 0.05;
    update_modelview_matrix();
  });

  update_modelview_matrix();
  render_loop();
}
//==============================================================================
void dvr_window::update_modelview_matrix() {
  vec3 eye{m_radius * std::sin(m_phi) * std::sin(m_theta),
           m_radius * std::cos(m_phi),
           m_radius * std::sin(m_phi) * std::cos(m_theta)};
  vec3 to{0, 0, 0};
  vec3 up{0, 1, 0};
  m_cube_shader.set_modelview_matrix(look_at_matrix(eye, to, up));
}
//------------------------------------------------------------------------------
void dvr_window::render_loop() {
  bool run = true;
  while (run) {
    refresh();
    static float v[] = {0.000f, 0.000f, 1.000f, 1.000f};
    ImGui::SliderFloat("sample distance", &m_sample_distance, 0.001f, 0.1f);
    ImGui::Bezier("alpha", v);
    m_alpha_data.resize(m_num_alpha_samples);
    for (size_t i = 0; i < m_num_alpha_samples; ++i) {
      float const pos = i / (float)(m_num_alpha_samples - 1);
      m_alpha_data[i] = ImGui::BezierValue(pos, v);
    }
    render_volume();
    render_imgui();
    swap_buffers();
    std::this_thread::sleep_for(std::chrono::milliseconds{10});
  }
}
//------------------------------------------------------------------------------
void dvr_window::create_volume_data() {
  size_t const width = 100, height = 100, depth = 100;
  std::vector<float> data(width * height * depth);
  for (size_t w = 0; w < depth; ++w) {
    for (size_t v = 0; v < height; ++v) {
      for (size_t u = 0; u < width; ++u) {
        float norm_u = u / float(width - 1);
        float norm_v = v / float(height - 1);
        float norm_w = w / float(depth - 1);
        data[u + v * width + w * width * height] =
            (std::cos(norm_u * 2 * M_PI * 3 + M_PI) * 0.5 + 0.5) *
            (std::cos(norm_v * 2 * M_PI * 3 + M_PI) * 0.5 + 0.5) *
            (std::cos(norm_w * 2 * M_PI * 3 + M_PI) * 0.5 + 0.5);
      }
    }
  }
  m_volume_data_tex.upload_data(data, width, height, depth);
}
//------------------------------------------------------------------------------
void dvr_window::render_volume() {
  m_alpha_tex.upload_data(m_alpha_data, m_num_alpha_samples);
  yavin::gl::clear_color(255, 255, 255, 255);
  yavin::clear_color_buffer();
  yavin::gl::viewport(0, 0, m_width, m_height);
  render_cube();
  m_cube_front_tex.bind(0);
  m_cube_back_tex.bind(1);
  m_volume_data_tex.bind(2);
  m_alpha_tex.bind(3);
  m_volume_shader.bind();
  m_volume_shader.set_sample_distance(m_sample_distance);
  m_fullscreen_quad_data.draw_triangles();
}
//------------------------------------------------------------------------------
void dvr_window::render_cube() {
  m_cube_shader.bind();
  yavin::enable_face_culling();

  // render front
  yavin::set_back_face_culling();
  yavin::framebuffer fbo_front{m_cube_front_tex};
  fbo_front.bind();
  yavin::gl::clear_color(-1, -1, -1, -1);
  yavin::clear_color_buffer();
  m_cube_data.draw_triangles();

  // render back
  yavin::set_front_face_culling();
  yavin::framebuffer fbo_back{m_cube_back_tex};
  fbo_back.bind();
  yavin::clear_color_buffer();
  m_cube_data.draw_triangles();

  yavin::disable_face_culling();
}
//------------------------------------------------------------------------------
// EVENTS
//------------------------------------------------------------------------------
