#include <stdio.h>
#include <SDL.h>
#include <OpenGL/gl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#define fail { printf("FAIL %d\n", __LINE__); return 1; }

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof(x[0]))

void tick();
GLuint createShader(const char* name, GLenum shaderType);
void render(glm::mat4 camera);

bool quit = false;

glm::vec3 cameraPosition = glm::vec3(10.0f, 20.0f, 3.5f);
float cameraRotateZ = 2.75f;
float cameraRotateUp = 0.0f;

#define MOVE_SPEED 0.1f
#define ROTATE_SPEED 0.01f

SDL_Window* window;

GLuint vbo;
GLuint vao;
GLuint directProgram;

#define POSITION_ATTRIB 0
#define NORMAL_ATTRIB 1
#define COLOR_ATTRIB 2

struct Color {
  float r;
  float g;
  float b;
};

struct Material {
  Color color;
};

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  Color color;
};

struct Quad {
  Vertex vertices[6];
};

const Color WHITE = {0.85f, 0.85f, 0.85f};
const Color RED = {0.8f, 0.0f, 0.0f};

#include "geometry.cpp"

int main(int argc, char** argv) {

  if (SDL_Init(SDL_INIT_VIDEO) < 0) fail;

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

  window = SDL_CreateWindow("Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_OPENGL);
  if (!window) fail;

  SDL_GL_CreateContext(window);
  SDL_GL_SetSwapInterval(1);

  SDL_SetRelativeMouseMode(SDL_TRUE);


  GLuint directVert = createShader("shaders/direct.vert.glsl", GL_VERTEX_SHADER);
  GLuint directFrag = createShader("shaders/direct.frag.glsl", GL_FRAGMENT_SHADER);

  directProgram = glCreateProgram();
  {
    glAttachShader(directProgram, directVert);
    glAttachShader(directProgram, directFrag);

    glBindAttribLocation(directProgram, POSITION_ATTRIB, "position");
    glBindAttribLocation(directProgram, NORMAL_ATTRIB, "normal");
    glBindAttribLocation(directProgram, COLOR_ATTRIB, "color");

    glLinkProgram(directProgram);

    glDeleteShader(directVert);
    glDeleteShader(directFrag);
  }

  glUseProgram(directProgram);
  {
    GLint projLoc = glGetUniformLocation(directProgram, "proj");
    glm::mat4 proj = glm::perspective(45.0f, 640.0f/480.0f, 0.1f, 100.0f);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
  }
  {
    GLint directionalDirLoc = glGetUniformLocation(directProgram, "directional_dir");
    glm::vec3 lightDir = glm::normalize(glm::vec3(0.2f, 1.0f, -1.0f));
    glUniform3fv(directionalDirLoc, 1, glm::value_ptr(lightDir));
  }
  {
    GLint directionalIntensityLoc = glGetUniformLocation(directProgram, "directional_intensity");
    glUniform1f(directionalIntensityLoc, 0.3f);
  }
  glUseProgram(0);


  {
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quads), quads, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glEnableVertexAttribArray(POSITION_ATTRIB);
    glVertexAttribPointer(POSITION_ATTRIB, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

    glEnableVertexAttribArray(NORMAL_ATTRIB);
    glVertexAttribPointer(NORMAL_ATTRIB, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) (3 * sizeof(float)));

    glEnableVertexAttribArray(COLOR_ATTRIB);
    glVertexAttribPointer(COLOR_ATTRIB, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) (6 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glDepthFunc(GL_LEQUAL);
  glDepthRange(0.0f, 1.0f);

  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CW);
  glCullFace(GL_BACK);

  while (!quit) {
    tick();

  }

  return 0;
}

void tick() {
  {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT: {
        quit = true;
      } break;
      case SDL_KEYDOWN: {
        if (event.key.keysym.sym == SDLK_ESCAPE) {
          quit = true;
        }
      } break;
      case SDL_MOUSEMOTION: {
        cameraRotateZ += event.motion.xrel * ROTATE_SPEED;
        cameraRotateZ = fmodf(cameraRotateZ, M_PI * 2);

        cameraRotateUp += event.motion.yrel * ROTATE_SPEED;
        if (cameraRotateUp < -M_PI_2) cameraRotateUp = -M_PI_2;
        if (cameraRotateUp > M_PI_2) cameraRotateUp = M_PI_2;
      } break;
      }
    }
  }

  {
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W]) {
      cameraPosition += glm::rotateZ(glm::vec3(0.0f, MOVE_SPEED, 0.0f), -cameraRotateZ);
    }
    if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]) {
      cameraPosition -= glm::rotateZ(glm::vec3(0.0f, MOVE_SPEED, 0.0f), -cameraRotateZ);
    }
    if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) {
      cameraPosition += glm::rotateZ(glm::vec3(MOVE_SPEED, 0.0f, 0.0f), -cameraRotateZ);
    }
    if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) {
      cameraPosition -= glm::rotateZ(glm::vec3(MOVE_SPEED, 0.0f, 0.0f), -cameraRotateZ);
    }
  }

  {
    glm::mat4 cameraReorient = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
                                           glm::vec3(0.0f, 1.0f, 0.0f),
                                           glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 cameraRotated =
      glm::rotate(glm::rotate(cameraReorient, cameraRotateUp, glm::vec3(1.0f, 0.0f, 0.0f)),
                  cameraRotateZ, glm::vec3(0.0, 0.0, 1.0f));
    glm::mat4 camera = glm::translate(cameraRotated, -cameraPosition);

    render(camera);
  }

  SDL_GL_SwapWindow(window);
}

void render(glm::mat4 camera) {
  glClearColor(1.0, 1.0, 1.0, 1.0);
  glClearDepth(1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(directProgram);

  GLint cameraLoc = glGetUniformLocation(directProgram, "camera");
  glUniformMatrix4fv(cameraLoc, 1, GL_FALSE, glm::value_ptr(camera));

  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, ARRAY_LENGTH(quads)*6);
  glBindVertexArray(0);

  glUseProgram(0);
}

GLuint createShader(const char* filename, GLenum shaderType) {
  FILE* file = fopen(filename, "r");

  fseek(file, 0L, SEEK_END);
  long filesize = ftell(file);
  rewind(file);

  char* body = (char*) malloc(filesize+1);
  fread(body, 1, filesize, file);
  body[filesize] = 0;

  GLuint shader = glCreateShader(shaderType);
  glShaderSource(shader, 1, &body, NULL);
  free(body);

  glCompileShader(shader);

  {
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
      GLint length;
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

      char* error = (char*) malloc(length);
      glGetShaderInfoLog(shader, length, NULL, error);

      printf("body: %s\n", body);
      printf("error: %s\n", error);
      exit(1);
    }
  }


  return shader;
}
