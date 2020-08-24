#include <yavin>
#include <chrono>
auto main() -> int {
  yavin::window win{"dvr", 500, 500};
  yavin::tex3r<float> data_tex{100, 100, 100};
  yavin::indexeddata<std::array<GLfloat, 3>, std::array<GLfloat, 3>> cube_data;
  auto time = std::chrono::system_clock::now();
  bool run = true;
  while (run) {
    win.refresh();
    yavin::gl::clear_color(255, 255, 255, 255);
    yavin::clear_color_depth_buffer();
    win.render_imgui();
    win.swap_buffers();
  }
}
