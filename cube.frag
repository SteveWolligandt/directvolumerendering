#version 330 core
in vec3 uvw_frag;
out vec3 outcol;
void main() {
  outcol = uvw_frag;
}
