#include <chrono>
#include <memory>
#include <yavin>
//==============================================================================
GLsizei width = 500, height = 500;
using vec3 = std::array<GLfloat, 3>;
using mat4 = std::array<GLfloat, 16>;
yavin::window win{"dvr", width, height};
std::unique_ptr<yavin::tex2rgb<GLfloat>> cube_back_tex, cube_front_tex;
std::unique_ptr<yavin::shader> cube_shader;
std::unique_ptr<yavin::tex3r<GLfloat>> data_tex;
std::unique_ptr<yavin::indexeddata<vec3, vec3>> cube_data;

GLfloat theta = M_PI , phi = M_PI / 2, radius = 3;
GLfloat old_mouse_x, old_mouse_y;
GLfloat cur_mouse_x, cur_mouse_y;
bool mouse_down = false;
//==============================================================================
void on_window_resize(size_t width, size_t height);
void render_cube();
void init();
void init_cube();
void init_textures();
void init_shaders();
void render_loop();
constexpr mat4 orthographic_matrix(GLfloat const l, GLfloat const r,
                                         GLfloat const b, GLfloat const t,
                                         GLfloat const n, GLfloat const f) {
  return {
      2 / (r - l),        GLfloat(0),         GLfloat(0),         GLfloat(0),
      GLfloat(0),         2 / (t - b),        GLfloat(0),         GLfloat(0),
      GLfloat(0),         GLfloat(0),         -2 / (f - n),       GLfloat(0),
      -(r + l) / (r - l), -(t + b) / (t - b), -(f + n) / (f - n), GLfloat(1)};
}



constexpr auto perspective_matrix(GLfloat const l, GLfloat const r,
                                  GLfloat const b, GLfloat const t,
                                  GLfloat const n, GLfloat const f) {
  return mat4{2 * n / (r - l),   GLfloat(0),        GLfloat(0),
              GLfloat(0),

              GLfloat(0),        2 * n / (t - b),   GLfloat(0),
              GLfloat(0),

              (r + l) / (r - l), (t + b) / (t - b), -(f + n) / (f - n),
              GLfloat(-1),

              GLfloat(0),        GLfloat(0),        -2 * f * n / (f - n),
              GLfloat(0)

  };
}
//------------------------------------------------------------------------------
auto perspective_matrix(GLfloat const angle_of_view,
                        GLfloat const image_aspect_ratio, GLfloat const near,
                        GLfloat const far) {
  GLfloat const scale = std::tan(angle_of_view * 0.5 * M_PI / 180) * near;
  GLfloat const r = image_aspect_ratio * scale;
  GLfloat const l = -r;
  GLfloat const t = scale;
  GLfloat const b = -t;
  return perspective_matrix(l, r, b, t, near, far);
}

// Calculate the cross product and return it
constexpr auto cross(vec3 const& srcA, vec3 const& srcB) {
  return vec3{srcA[1] * srcB[2] - srcA[2] * srcB[1],
              srcA[2] * srcB[0] - srcA[0] * srcB[2],
              srcA[0] * srcB[1] - srcA[1] * srcB[0]};
}
// Calculate the cross product and return it
constexpr auto dot(vec3 const& srcA, vec3 const& srcB) {
  return srcA[0] * srcB[0] + srcA[1] * srcB[1] + srcA[2] * srcB[2];
}

// Normalize the input vector
void normalize(vec3& vec) {
  const GLfloat invLen =
      1.f / std::sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
  vec[0] *= invLen;
  vec[1] *= invLen;
  vec[2] *= invLen;
}
void scale(vec3& v, GLfloat s) {
  v[0] *= s;
  v[1] *= s;
  v[2] *= s;
}
//------------------------------------------------------------------------------
auto look_at_matrix(vec3 const& eye) {
  vec3 up{0.0f, 1.0f, 0.0f};
  vec3 D{-eye[0],- eye[1],- eye[2]};
  normalize(D);
  const auto R = cross(up, D);
  const auto U = cross(D, R);
  //return mat4{1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
  return mat4{  R[0],   R[1],   R[2], 0.0f,
                U[0],   U[1],   U[2], 0.0f,
                D[0],   D[1],   D[2], 0.0f,
              eye[0], eye[1], eye[2], 1.0f};
}

//==============================================================================
void update_modelview_matrices() {
  vec3 eye{
    radius * std::sin(phi) * std::sin(theta),
    radius * std::cos(phi),
    radius * std::sin(phi) * std::cos(theta)
  };
  std::cerr << "eye: [" << eye[0] << ", " << eye[1] << ", " << eye[2] << "]\n";
  std::cerr << theta << '\n';
  std::cerr << phi << '\n';

  auto const view_mat = look_at_matrix(eye);
  if (cube_shader) {
    cube_shader->set_uniform_mat4("modelview_mat", view_mat.data());
  }
}
struct : yavin::window_listener {
  void on_resize(int width, int height) override {
    on_window_resize(width, height);
  }
  void on_mouse_motion(int x, int y) override {
    cur_mouse_x = x;
    cur_mouse_y = y;

    if (mouse_down) {
      auto off_x = cur_mouse_x - old_mouse_x;
      auto off_y = cur_mouse_y - old_mouse_y;
      old_mouse_x = cur_mouse_x;
      old_mouse_y = cur_mouse_y;

      theta += off_x * 0.001;
      phi += off_y * 0.001;
      phi = std::min<GLfloat>(M_PI, phi);
      phi = std::max<GLfloat>(0, phi);
      update_modelview_matrices();
    }
  }
  void on_button_pressed(yavin::button /*b*/) override {
    old_mouse_x = cur_mouse_x;
    old_mouse_y = cur_mouse_y;
    mouse_down = true;
  }
  void on_button_released(yavin::button /*b*/) override { mouse_down = false; }
  void on_wheel_up() override {
    radius -= 0.05;

    radius = std::max<GLfloat>(radius, 0.001f);
    std::cerr << radius << '\n';
    update_modelview_matrices();
  }
  void on_wheel_down() {
    radius += 0.05;
    std::cerr << radius<< '\n';
    update_modelview_matrices();
  }
} listener;
auto main() -> int {
  init();
  render_loop();
}
//==============================================================================
void render_loop() {
  bool run = true;
  while (run) {
    win.refresh();
    yavin::gl::clear_color(255, 255, 255, 255);
    yavin::clear_color_depth_buffer();
    yavin::gl::viewport(0, 0, width, height);

    render_cube();

    win.render_imgui();
    win.swap_buffers();
  }
}
//------------------------------------------------------------------------------
void init() {
  on_window_resize(width, height);
  win.add_listener(listener);
  init_cube();
  init_textures();
  init_shaders();
}
//------------------------------------------------------------------------------
void init_cube() {
  cube_data = std::make_unique<yavin::indexeddata<vec3, vec3>>();
  cube_data->vertexbuffer().reserve(8);
  cube_data->vertexbuffer().push_back({-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 0.0f});
  cube_data->vertexbuffer().push_back({1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f});
  cube_data->vertexbuffer().push_back({-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f});
  cube_data->vertexbuffer().push_back({1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, 0.0f});

  cube_data->vertexbuffer().push_back({-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f});
  cube_data->vertexbuffer().push_back({1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 1.0f});
  cube_data->vertexbuffer().push_back({-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f});
  cube_data->vertexbuffer().push_back({1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f});

  cube_data->indexbuffer().reserve(12 * 3);

  cube_data->indexbuffer().push_back(0);
  cube_data->indexbuffer().push_back(1);
  cube_data->indexbuffer().push_back(2);

  cube_data->indexbuffer().push_back(1);
  cube_data->indexbuffer().push_back(3);
  cube_data->indexbuffer().push_back(2);

  cube_data->indexbuffer().push_back(4);
  cube_data->indexbuffer().push_back(6);
  cube_data->indexbuffer().push_back(5);

  cube_data->indexbuffer().push_back(5);
  cube_data->indexbuffer().push_back(6);
  cube_data->indexbuffer().push_back(7);

  cube_data->indexbuffer().push_back(1);
  cube_data->indexbuffer().push_back(5);
  cube_data->indexbuffer().push_back(3);

  cube_data->indexbuffer().push_back(5);
  cube_data->indexbuffer().push_back(7);
  cube_data->indexbuffer().push_back(3);
}
//------------------------------------------------------------------------------
void init_textures() {
  cube_back_tex = std::make_unique<yavin::tex2rgb<GLfloat>>(width, height);
  cube_front_tex = std::make_unique<yavin::tex2rgb<GLfloat>>(width, height);
}
//------------------------------------------------------------------------------
void init_shaders() {
  cube_shader = std::make_unique<yavin::shader>();
  cube_shader->add_stage<yavin::vertexshader>(
      "#version 330 core\n"
      "layout(location = 0) in vec3 pos_vert;\n"
      "layout(location = 1) in vec3 uvw_vert;\n"
      "out vec3 uvw_frag;\n"
      "uniform mat4 modelview_mat;\n"
      "uniform mat4 projection_mat;\n"
      "void main() {\n"
      "  gl_Position = projection_mat * modelview_mat * vec4(pos_vert, 1);"
      "  uvw_frag = uvw_vert;\n"
      "}\n",
      yavin::SOURCE);
  cube_shader->add_stage<yavin::fragmentshader>(
      "#version 330 core\n"
      "in vec3 uvw_frag;\n"
      "out vec3 outcol;\n"
      "void main() {\n"
      "  outcol = uvw_frag;\n"
      "}",
      yavin::SOURCE);
  cube_shader->create();
  update_modelview_matrices();
}
//------------------------------------------------------------------------------
void on_window_resize(size_t new_width, size_t new_height) {
  width = new_width;
  height = new_height;
  auto const projection_matrix =
      perspective_matrix(60, (GLfloat)width / (GLfloat)height, 0.01, 1000);
  if (cube_shader) {
    cube_shader->set_uniform_mat4("projection_mat", projection_matrix.data());
  }
}
//------------------------------------------------------------------------------
void render_cube() {
  cube_shader->bind();
  yavin::enable_face_culling();
  //
   //yavin::set_front_face_culling();
  // yavin::framebuffer fbo_back{*cube_back_tex};
  // fbo_back->bind();
  // yavin::clear_color_buffer();
  // cube_data->draw_triangles();
  //
  yavin::set_back_face_culling();
  // yavin::framebuffer fbo_front{*cube_front_tex};
  // fbo_front->bind();
  cube_data->draw_triangles();

  yavin::disable_face_culling();
}
