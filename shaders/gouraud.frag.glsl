#version 150

uniform vec3 color;
out vec4 out_color;

in vec3 normal_world;

uniform vec3 directional_dir;
uniform float directional_intensity;

void main() {
  float cos_directional_angle_incidence = clamp(dot(normal_world, -directional_dir), 0, 1);

  out_color = vec4(
                   color * cos_directional_angle_incidence * directional_intensity
                   + color * (1 - directional_intensity),
                   0.0);
}
