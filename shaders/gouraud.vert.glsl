#version 150

in vec3 position;
in vec3 normal;

out vec3 normal_world;

uniform mat4 proj;
uniform mat4 camera;
uniform mat4 model;

void main() {
  gl_Position = proj * camera * model * vec4(position, 1.0);
  normal_world = vec3(model * vec4(normal, 0.0));
}
