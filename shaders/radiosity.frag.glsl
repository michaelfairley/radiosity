#version 150

in vec3 fcolor;
out vec4 out_color;

in vec3 fnormal;
in vec2 ftexcoord;

uniform vec3 directional_dir;
uniform float directional_intensity;

uniform sampler2D tex;

void main() {
  float cos_directional_angle_incidence = clamp(dot(fnormal, -directional_dir), 0, 1);

  vec4 xcolor = vec4(
                   fcolor * cos_directional_angle_incidence * directional_intensity
                   + fcolor * (1 - directional_intensity),
                   0.0);

  out_color = mix(xcolor, texture(tex, ftexcoord), 0.5);
}
