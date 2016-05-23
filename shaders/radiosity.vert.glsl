#version 150

in vec3 position;
in vec2 texcoord;

out vec2 ftexcoord;

uniform mat4 proj;
uniform mat4 camera;

void main() {
  gl_Position = proj * camera * vec4(position, 1.0);
  ftexcoord = texcoord;
}
