#include <memory>
#include <yavin>

#include "cube_shader.h"
#include "fullscreen_quad.h"
#include "linalg.h"
#include "unit_cube.h"
#include "util.h"
#include "volume_shader.h"
//==============================================================================
struct dvr : yavin::window {
  GLsizei m_width = 500, m_height = 500;

  cube_shader_t m_cube_shader;
  volume_shader_t m_volume_shader;

  yavin::tex2rgb<float> m_cube_back_tex, m_cube_front_tex;
  yavin::tex3r<float> m_volume_data_tex;

  yavin::indexeddata<vec3, vec3> m_cube_data;
  yavin::indexeddata<vec2> m_fullscreen_quad_data;

  float m_theta = M_PI, m_phi = M_PI / 2, m_radius = 3;
  float m_old_mouse_x, m_old_mouse_y;
  float m_cur_mouse_x, m_cur_mouse_y;
  bool m_mouse_down = false;
  float m_fov = 60;
  float m_nearplane = 0.001;
  float m_farplane = 100;
  //==============================================================================
  dvr()
      : yavin::window{"dvr", 500, 500},
        m_cube_back_tex{std::max<size_t>(m_width, 1), std::max<size_t>(m_height, 1)},
        m_cube_front_tex{std::max<size_t>(m_width, 1), std::max<size_t>(m_height, 1)},
        m_cube_data{unit_cube()},
        m_fullscreen_quad_data{fullscreen_quad()} {
    create_volume_data();
    m_cube_shader.set_projection_matrix(perspective_matrix(
        m_fov, (float)m_width / (float)m_height, m_nearplane, m_farplane));

    update_modelview_matrix();
    render_loop();
  }
  //==============================================================================
  void update_modelview_matrix() {
    vec3 eye{m_radius * std::sin(m_phi) * std::sin(m_theta),
             m_radius * std::cos(m_phi),
             m_radius * std::sin(m_phi) * std::cos(m_theta)};
    vec3 to{0, 0, 0};
    vec3 up{0, 1, 0};

    m_cube_shader.set_modelview_matrix(look_at_matrix(eye, to, up));
  }
  //------------------------------------------------------------------------------
  void render_loop() {
    bool run = true;
    while (run) {
      refresh();
      yavin::gl::clear_color(255, 255, 255, 255);
      yavin::clear_color_buffer();
      yavin::gl::viewport(0, 0, m_width, m_height);

      render_cube();
      render_volume();

      render_imgui();
      swap_buffers();
    }
  }
  //------------------------------------------------------------------------------
  void create_volume_data() {
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
  void render_cube() {
    m_cube_shader.bind();
    yavin::enable_face_culling();

    yavin::set_back_face_culling();
    yavin::framebuffer fbo_front{m_cube_front_tex};
    fbo_front.bind();
    yavin::gl::clear_color(-1, -1, -1, -1);
    yavin::clear_color_buffer();
    m_cube_data.draw_triangles();

    yavin::set_front_face_culling();
    yavin::framebuffer fbo_back{m_cube_back_tex};
    fbo_back.bind();
    yavin::clear_color_buffer();
    m_cube_data.draw_triangles();

    yavin::disable_face_culling();
  }
  //----------------------------------------------------------------------------
  void render_volume() {
    m_cube_front_tex.bind(0);
    m_cube_back_tex.bind(1);
    m_volume_data_tex.bind(2);
    m_volume_shader.bind();
    m_fullscreen_quad_data.draw_triangles();
  }
  //------------------------------------------------------------------------------
  // EVENTS
  //------------------------------------------------------------------------------
  void on_resize(int new_width, int new_height) override {
    m_width = new_width;
    m_height = new_height;
    yavin::window::on_resize(m_width, m_height);
    m_cube_back_tex.resize(std::max<size_t>(m_width, 1), std::max<size_t>(m_height, 1));
    m_cube_front_tex.resize(std::max<size_t>(m_width, 1), std::max<size_t>(m_height, 1));
    m_cube_shader.set_projection_matrix(perspective_matrix(
        m_fov, (float)m_width / (float)m_height, m_nearplane, m_farplane));
  }
  //------------------------------------------------------------------------------
  void on_key_pressed(yavin::key k) override {
    yavin::window::on_key_pressed(k);
    if (k == yavin::KEY_1) {
      m_volume_shader.set_mode(1);
    } else if (k == yavin::KEY_2) {
      m_volume_shader.set_mode(2);
    } else if (k == yavin::KEY_3) {
      m_volume_shader.set_mode(3);
    } else if (k == yavin::KEY_4) {
      m_volume_shader.set_mode(4);
    }
  }
  //------------------------------------------------------------------------------
  void on_mouse_motion(int x, int y) override {
    yavin::window::on_mouse_motion(x, y);
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
  }
  //------------------------------------------------------------------------------
  void on_button_pressed(yavin::button b) override {
    yavin::window::on_button_pressed(b);
    m_old_mouse_x = m_cur_mouse_x;
    m_old_mouse_y = m_cur_mouse_y;
    m_mouse_down = true;
  }
  //------------------------------------------------------------------------------
  void on_button_released(yavin::button b) override {
    yavin::window::on_button_released(b);
    m_mouse_down = false;
  }
  //------------------------------------------------------------------------------
  void on_wheel_up() override {
    yavin::window::on_wheel_up();
    m_radius -= 0.05;

    m_radius = std::max<float>(m_radius, 0.001f);
    update_modelview_matrix();
  }
  //------------------------------------------------------------------------------
  void on_wheel_down() override {
    yavin::window::on_wheel_down();
    m_radius += 0.05;
    update_modelview_matrix();
  }
};
//==============================================================================
auto main() -> int { dvr win; }
