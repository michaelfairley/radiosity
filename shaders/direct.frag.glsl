#version 150

in vec3 fcolor;
out vec4 out_color;

in vec3 fnormal;

uniform vec3 directional_dir;
uniform float directional_intensity;

void main() {
  float cos_directional_angle_incidence = clamp(dot(fnormal, -directional_dir), 0, 1);

  out_color = vec4(
                   fcolor * cos_directional_angle_incidence * directional_intensity
                   + fcolor * (1 - directional_intensity),
                   0.0);
}
