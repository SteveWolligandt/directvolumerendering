#version 330 core
layout(location = 0) in vec3 pos_vert;
layout(location = 1) in vec3 uvw_vert;
out vec3 uvw_frag;
uniform mat4 modelview_mat;
uniform mat4 projection_mat;
void main() {
  gl_Position = projection_mat * inverse(modelview_mat) * vec4(pos_vert, 1);
  uvw_frag = uvw_vert;
}
