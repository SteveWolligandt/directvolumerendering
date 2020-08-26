#version 330 core
layout(location = 0) in vec2 pos_vert;
uniform mat4 projection_mat;
out vec2 uv_frag;
void main() {
  gl_Position = projection_mat * vec4(pos_vert, 0, 1);
  uv_frag = pos_vert;
}
