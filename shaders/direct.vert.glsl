#version 150

in vec3 position;
in vec3 normal;

out vec3 fnormal;

uniform mat4 proj;
uniform mat4 camera;

void main() {
  gl_Position = proj * camera * vec4(position, 1.0);
  fnormal = normal;
}
