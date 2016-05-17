#version 150

in vec3 position;
in vec3 normal;
in vec3 color;

out vec3 fnormal;
out vec3 fcolor;

uniform mat4 proj;
uniform mat4 camera;

void main() {
  gl_Position = proj * camera * vec4(position, 1.0);
  fnormal = normal;
  fcolor = color;
}
