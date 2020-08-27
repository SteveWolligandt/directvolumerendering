#version 330 core
uniform sampler1D alpha_tex;
uniform sampler2D cube_front_tex;
uniform sampler2D cube_back_tex;
uniform sampler3D volume_data_tex;
uniform int mode;
in vec2 uv_frag;
out vec3 outcol;
const vec3 bgcol = vec3(1,1,1);
uniform float sample_distance;
vec3 render_volume() {
  vec3 cur_uvw = texture(cube_front_tex, uv_frag).rgb;
  vec3 uvw_back = texture(cube_back_tex, uv_frag).rgb;
  if (cur_uvw.r == -1) {
    return bgcol;
  } else {
    float accumulated_alpha = 0;
    vec3 accumulated_color = vec3(0);
    vec3 dir = uvw_back - cur_uvw;
    float len = length(dir);
    int num_steps = int(ceil(len / sample_distance));
    vec3 step = dir / num_steps;
    for (int i = 0 ; i < num_steps + 1; ++i) {
      float sample       = texture(volume_data_tex, cur_uvw).r;
      vec3  sample_color = vec3(sample);
      float sample_alpha = clamp(texture(alpha_tex, sample).r, 0.0, 1.0);
      accumulated_color +=
        (1 - accumulated_alpha) * sample_alpha * sample_color;
      accumulated_alpha +=
        (1 - accumulated_alpha) * sample_alpha;
      if (accumulated_alpha > 0.95) { break; }
      cur_uvw += step;
    }
    return accumulated_color * accumulated_alpha +
           bgcol * (1-accumulated_alpha);
  }
}
void main() {
  if (mode == 1) {
    outcol = render_volume();
  } else if (mode == 2) {
    outcol = texture(cube_front_tex, uv_frag).rgb;
  } else if(mode == 3) {
    outcol = texture(cube_back_tex, uv_frag).rgb;
  } else if(mode == 4) {
    vec3 uvw = texture(cube_back_tex, uv_frag).rgb;
    if (uvw.r == -1) {
      outcol = vec3(1);
    } else {
      float sample = texture(volume_data_tex, uvw).r;
      outcol = vec3(sample);
    }
  }
}
