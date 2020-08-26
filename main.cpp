#include <chrono>
#include <memory>
#include <yavin>
//==============================================================================
GLsizei width = 500, height = 500;
using vec2 = std::array<GLfloat, 2>;
using vec3 = std::array<GLfloat, 3>;
using mat4 = std::array<GLfloat, 16>;
yavin::window win{"dvr", width, height};
std::unique_ptr<yavin::tex2rgb<GLfloat>> cube_back_tex, cube_front_tex;
std::unique_ptr<yavin::texdepth32f> depth_tex;
std::unique_ptr<yavin::shader> cube_shader;
std::unique_ptr<yavin::shader> volume_shader;
std::unique_ptr<yavin::tex3r<GLfloat>> data_tex;
std::unique_ptr<yavin::indexeddata<vec3, vec3>> cube_data;
std::unique_ptr<yavin::indexeddata<vec2>> fullscreen_quad;

GLfloat theta = M_PI, phi = M_PI / 2, radius = 3;
GLfloat old_mouse_x, old_mouse_y;
GLfloat cur_mouse_x, cur_mouse_y;
bool mouse_down = false;
//==============================================================================
void on_window_resize(size_t width, size_t height);
void render_cube();
void render_volume();
void init();
void init_cube();
void init_textures();
void init_shaders();
void render_loop();
constexpr mat4 orthographic_matrix(GLfloat const l, GLfloat const r,
                                   GLfloat const b, GLfloat const t,
                                   GLfloat const n, GLfloat const f);
//------------------------------------------------------------------------------
mat4 perspective_matrix(GLfloat const l, GLfloat const r, GLfloat const b,
                        GLfloat const t, GLfloat const n, GLfloat const f);
//------------------------------------------------------------------------------
mat4 perspective_matrix(GLfloat const angle_of_view,
                        GLfloat const image_aspect_ratio, GLfloat const near,
                        GLfloat const far);
//------------------------------------------------------------------------------
// Calculate the cross product and return it
constexpr vec3 cross(vec3 const& a, vec3 const& b);
// Calculate the cross product and return it
constexpr GLfloat dot(vec3 const& a, vec3 const& b);
// Normalize the input vector
void normalize(vec3& vec);
void scale(vec3& v, GLfloat s);
//------------------------------------------------------------------------------
mat4 look_at_matrix(vec3 const& eye, vec3 const& to, vec3 const& up);
//==============================================================================
void update_modelview_matrices() {
  vec3 eye{radius * std::sin(phi) * std::sin(theta),
           radius * std::cos(phi),
           radius * std::sin(phi) * std::cos(theta)};
  vec3 to{0, 0, 0};
  vec3 up{0, 1, 0};

  auto const V = look_at_matrix(eye, to, up);
  if (cube_shader) {
    cube_shader->set_uniform_mat4("modelview_mat", V.data());
  }
}
struct : yavin::window_listener {
  void on_resize(int width, int height) override {
    on_window_resize(width, height);
  }
  void on_key_pressed(yavin::key k) override {
    if (k == yavin::KEY_1) {
      volume_shader->set_uniform("mode", 1);
    } else if (k == yavin::KEY_2) {
      volume_shader->set_uniform("mode", 2);
    } else if (k == yavin::KEY_3) {
      volume_shader->set_uniform("mode", 3);
    } else if (k == yavin::KEY_4) {
      volume_shader->set_uniform("mode", 4);
    }
  }
  void on_mouse_motion(int x, int y) override {
    cur_mouse_x = x;
    cur_mouse_y = y;

    if (mouse_down) {
      auto off_x = cur_mouse_x - old_mouse_x;
      auto off_y = cur_mouse_y - old_mouse_y;
      old_mouse_x = cur_mouse_x;
      old_mouse_y = cur_mouse_y;

      theta -= off_x * 0.005;
      phi -= off_y * 0.005;
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
    update_modelview_matrices();
  }
  void on_wheel_down() override {
    radius += 0.05;
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
    render_volume();

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
  cube_data->vertexbuffer().push_back({-1.0f, -1.0f, -1.0f},
                                      {0.0f, 0.0f, 0.0f});
  cube_data->vertexbuffer().push_back({1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f});
  cube_data->vertexbuffer().push_back({-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f});
  cube_data->vertexbuffer().push_back({1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, 0.0f});

  cube_data->vertexbuffer().push_back({-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f});
  cube_data->vertexbuffer().push_back({1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 1.0f});
  cube_data->vertexbuffer().push_back({-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f});
  cube_data->vertexbuffer().push_back({1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f});

  cube_data->indexbuffer().reserve(12 * 3);

  // front
  cube_data->indexbuffer().push_back(0);
  cube_data->indexbuffer().push_back(1);
  cube_data->indexbuffer().push_back(2);
  cube_data->indexbuffer().push_back(1);
  cube_data->indexbuffer().push_back(3);
  cube_data->indexbuffer().push_back(2);

  // back
  cube_data->indexbuffer().push_back(4);
  cube_data->indexbuffer().push_back(6);
  cube_data->indexbuffer().push_back(5);
  cube_data->indexbuffer().push_back(5);
  cube_data->indexbuffer().push_back(6);
  cube_data->indexbuffer().push_back(7);

  // left
  cube_data->indexbuffer().push_back(1);
  cube_data->indexbuffer().push_back(5);
  cube_data->indexbuffer().push_back(3);
  cube_data->indexbuffer().push_back(5);
  cube_data->indexbuffer().push_back(7);
  cube_data->indexbuffer().push_back(3);

  // right
  cube_data->indexbuffer().push_back(4);
  cube_data->indexbuffer().push_back(0);
  cube_data->indexbuffer().push_back(6);
  cube_data->indexbuffer().push_back(0);
  cube_data->indexbuffer().push_back(2);
  cube_data->indexbuffer().push_back(6);

  // top
  cube_data->indexbuffer().push_back(2);
  cube_data->indexbuffer().push_back(3);
  cube_data->indexbuffer().push_back(6);
  cube_data->indexbuffer().push_back(3);
  cube_data->indexbuffer().push_back(7);
  cube_data->indexbuffer().push_back(6);

  // bottom
  cube_data->indexbuffer().push_back(0);
  cube_data->indexbuffer().push_back(4);
  cube_data->indexbuffer().push_back(1);
  cube_data->indexbuffer().push_back(1);
  cube_data->indexbuffer().push_back(4);
  cube_data->indexbuffer().push_back(5);

  fullscreen_quad = std::make_unique<yavin::indexeddata<vec2>>();
  fullscreen_quad->vertexbuffer().reserve(4);
  fullscreen_quad->vertexbuffer().push_back({0.0f, 0.0f});
  fullscreen_quad->vertexbuffer().push_back({1.0f, 0.0f});
  fullscreen_quad->vertexbuffer().push_back({0.0f, 1.0f});
  fullscreen_quad->vertexbuffer().push_back({1.0f, 1.0f});
  fullscreen_quad->vertexbuffer().reserve(6);
  fullscreen_quad->indexbuffer().push_back(0);
  fullscreen_quad->indexbuffer().push_back(1);
  fullscreen_quad->indexbuffer().push_back(2);
  fullscreen_quad->indexbuffer().push_back(1);
  fullscreen_quad->indexbuffer().push_back(3);
  fullscreen_quad->indexbuffer().push_back(2);
}
//------------------------------------------------------------------------------
void init_textures() {
  cube_back_tex = std::make_unique<yavin::tex2rgb<GLfloat>>(width, height);
  cube_front_tex = std::make_unique<yavin::tex2rgb<GLfloat>>(width, height);
  depth_tex = std::make_unique<yavin::texdepth32f>(width, height);

  size_t width = 100, height = 100, depth = 100;
  std::vector<GLfloat> data(width * height * depth);
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
  data_tex = std::make_unique<yavin::tex3r<GLfloat>>();
  data_tex->upload_data(data, width, height, depth);
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
      "  gl_Position = projection_mat * inverse(modelview_mat) * vec4(pos_vert, 1);"
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

  volume_shader = std::make_unique<yavin::shader>();
  volume_shader->add_stage<yavin::vertexshader>(
      "#version 330 core\n"
      "layout(location = 0) in vec2 pos_vert;\n"
      "uniform mat4 projection_mat;\n"
      "out vec2 uv_frag;\n"
      "void main() {\n"
      "  gl_Position = projection_mat * vec4(pos_vert, 0, 1);"
      "  uv_frag = pos_vert;\n"
      "}\n",
      yavin::SOURCE);
  volume_shader->add_stage<yavin::fragmentshader>(
      "#version 330 core\n"
      "uniform sampler2D cube_front_tex;\n"
      "uniform sampler2D cube_back_tex;\n"
      "uniform sampler3D data_tex;\n"
      "uniform int mode;\n"
      "in vec2 uv_frag;\n"
      "out vec3 outcol;\n"
      "const vec3 bgcol = vec3(1,1,1);\n"
      "void main() {\n"
      "  if (mode == 1) {\n"
      "    vec3 cur_uvw = texture(cube_front_tex, uv_frag).rgb;\n"
      "    vec3 uvw_back = texture(cube_back_tex, uv_frag).rgb;\n"
      "    if (cur_uvw.r == -1) {\n"
      "      outcol = bgcol;\n"
      "    } else {\n"
      "      float accumulated_alpha = 0;\n"
      "      vec3 accumulated_color = vec3(0);\n"
      "      vec3 dir = uvw_back - cur_uvw;\n"
      "      float len = length(dir);\n"
      "      int num_steps = int(ceil(len / 0.01));\n"
      "      vec3 step = dir / num_steps;\n"
      "      for (int i = 0 ; i < num_steps + 1; ++i) {\n"
      "        float sample       = texture(data_tex, cur_uvw).r;\n"
      "        vec3  sample_color = vec3(sample);\n"
      "        float sample_alpha = sample;\n"
      "        accumulated_color += (1 - accumulated_alpha) * sample_alpha * sample_color;\n"
      "        accumulated_alpha += (1 - accumulated_alpha) * sample_alpha;\n"
      "        if (accumulated_alpha > 0.95) { break; }\n"
      "        cur_uvw += step;\n"
      "      }\n"
      "      outcol = accumulated_color * accumulated_alpha + bgcol * (1-accumulated_alpha);\n"
      "    }\n"
      "  } else if (mode == 2) {\n"
      "    outcol = texture(cube_front_tex, uv_frag).rgb;\n"
      "  } else if(mode == 3) {\n"
      "    outcol = texture(cube_back_tex, uv_frag).rgb;\n"
      "  } else if(mode == 4) {\n"
      "    vec3 uvw = texture(cube_back_tex, uv_frag).rgb;\n"
      "    if (uvw.r == -1) {\n"
      "      outcol = vec3(1);\n"
      "    } else {\n"
      "      float sample = texture(data_tex, uvw).r;\n"
      "      outcol = vec3(sample);\n"
      "    }\n"
      "  }\n"
      "}\n",
      yavin::SOURCE);
  volume_shader->create();
  volume_shader->set_uniform("cube_front_tex", 0);
  volume_shader->set_uniform("cube_back_tex", 1);
  volume_shader->set_uniform("data_tex", 2);
  volume_shader->set_uniform("mode", 1);
  volume_shader->set_uniform_mat4(
      "projection_mat", orthographic_matrix(0, 1, 0, 1, -1, 1).data());

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
  if (cube_back_tex) {
    cube_back_tex->resize(new_width, new_height);
  }
  if (cube_front_tex) {
    cube_front_tex->resize(new_width, new_height);
  }
  if (depth_tex) {
    depth_tex->resize(new_width, new_height);
  }
}
//------------------------------------------------------------------------------
void render_cube() {
  cube_shader->bind();
  yavin::enable_face_culling();
  yavin::enable_depth_test();
  yavin::gl::front_face(GL_CW);

  yavin::set_back_face_culling();
  yavin::framebuffer fbo_front{*cube_front_tex, *depth_tex};
  fbo_front.bind();
  yavin::gl::clear_color(-1, -1, -1, -1);
  yavin::clear_color_depth_buffer();
  cube_data->draw_triangles();

  yavin::set_front_face_culling();
  yavin::framebuffer fbo_back{*cube_back_tex, *depth_tex};
  fbo_back.bind();
  yavin::clear_color_depth_buffer();
  cube_data->draw_triangles();

  yavin::disable_face_culling();
  yavin::disable_depth_test();
}
void render_volume() {
  cube_front_tex->bind(0);
  cube_back_tex->bind(1);
  data_tex->bind(2);
  volume_shader->bind();
  fullscreen_quad->draw_triangles();
}
mat4 perspective_matrix(GLfloat const l, GLfloat const r, GLfloat const b,
                        GLfloat const t, GLfloat const n, GLfloat const f) {
  GLfloat const z = 0, no = -1;
  return {
    2 * n / (r - l), z, z, z,
    z, 2 * n / (t - b), z,z,
    (r + l) / (r - l), (t + b) / (t - b), -(f + n) / (f - n), no,
    z, z, -2 * f * n / (f - n), z
  };
}
mat4 perspective_matrix(GLfloat const angle_of_view,
                        GLfloat const image_aspect_ratio, GLfloat const near,
                        GLfloat const far) {
  GLfloat const scale = std::tan(angle_of_view * 0.5 * M_PI / 180) * near;
  GLfloat const r = image_aspect_ratio * scale;
  GLfloat const l = -r;
  GLfloat const t = scale;
  GLfloat const b = -t;
  return perspective_matrix(l, r, b, t, near, far);
}
constexpr vec3 cross(vec3 const& a, vec3 const& b) {
  return {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2],
          a[0] * b[1] - a[1] * b[0]};
}
// Calculate the cross product and return it
constexpr GLfloat dot(vec3 const& a, vec3 const& b) {
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

// Normalize the input vector
void normalize(vec3& v) {
  GLfloat const inv_len = 1 / std::sqrt(dot(v, v));
  v[0] *= inv_len;
  v[1] *= inv_len;
  v[2] *= inv_len;
}
void scale(vec3& v, GLfloat s) {
  v[0] *= s;
  v[1] *= s;
  v[2] *= s;
}
mat4 look_at_matrix(vec3 const& eye, vec3 const& to, vec3 const& up) {
  vec3 Z{eye[0] - to[0], eye[1] - to[1], eye[2] - to[2]};
  normalize(Z);
  auto X = cross(up, Z);
  normalize(X);
  auto const Y = cross(Z, X);
  return mat4{  X[0],   X[1],   X[2], 0.0f,
                Y[0],   Y[1],   Y[2], 0.0f,
                Z[0],   Z[1],   Z[2], 0.0f,
              eye[0], eye[1], eye[2], 1.0f};
}
constexpr mat4 orthographic_matrix(GLfloat const l, GLfloat const r,
                                         GLfloat const b, GLfloat const t,
                                         GLfloat const n, GLfloat const f) {
  return {
      2 / (r - l),        GLfloat(0),         GLfloat(0),         GLfloat(0),
      GLfloat(0),         2 / (t - b),        GLfloat(0),         GLfloat(0),
      GLfloat(0),         GLfloat(0),         -2 / (f - n),       GLfloat(0),
      -(r + l) / (r - l), -(t + b) / (t - b), -(f + n) / (f - n), GLfloat(1)};
}
