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

bool quit = false;

glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 0.8f);
float cameraRotateZ = 0.0f;

#define MOVE_SPEED 0.02f
#define ROTATE_SPEED 0.02f

SDL_Window* window;

GLuint vbo;
GLuint vao;
GLuint directProgram;

#define POSITION_ATTRIB 0
#define NORMAL_ATTRIB 1

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
};

struct Quad {
  Vertex vertices[4];
};

Quad quads[] = {
  {{
      { glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f) },
      { glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f) },
      { glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f) },
      { glm::vec3(0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f) }
    }},
  {{
      { glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f) },
      { glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f) },
      { glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f) },
      { glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f) }
      }}
};


int main(int argc, char** argv) {

  if (SDL_Init(SDL_INIT_VIDEO) < 0) fail;

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  window = SDL_CreateWindow("Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_OPENGL);
  if (!window) fail;

  SDL_GLContext context = SDL_GL_CreateContext(window);
  SDL_GL_SetSwapInterval(1);


  GLuint directVert = createShader("shaders/direct.vert.glsl", GL_VERTEX_SHADER);
  GLuint directFrag = createShader("shaders/direct.frag.glsl", GL_FRAGMENT_SHADER);

  directProgram = glCreateProgram();
  {
    glAttachShader(directProgram, directVert);
    glAttachShader(directProgram, directFrag);

    glBindAttribLocation(directProgram, POSITION_ATTRIB, "position");
    glBindAttribLocation(directProgram, NORMAL_ATTRIB, "normal");

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
    glm::vec3 lightDir = glm::normalize(glm::vec3(0.2f, -1.0f, -1.0f));
    glUniform3fv(directionalDirLoc, 1, glm::value_ptr(lightDir));
  }
  {
    GLint directionalIntensityLoc = glGetUniformLocation(directProgram, "directional_intensity");
    glUniform1f(directionalIntensityLoc, 0.1f);
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

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

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
      }
    }
  }

  {
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    if (keys[SDL_SCANCODE_UP]) {
      cameraPosition += glm::rotateZ(glm::vec3(0.0f, MOVE_SPEED, 0.0f), -cameraRotateZ);
    }
    if (keys[SDL_SCANCODE_DOWN]) {
      cameraPosition -= glm::rotateZ(glm::vec3(0.0f, MOVE_SPEED, 0.0f), -cameraRotateZ);
    }
    if (keys[SDL_SCANCODE_RIGHT]) {
      cameraRotateZ += ROTATE_SPEED;
    }
    if (keys[SDL_SCANCODE_LEFT]) {
      cameraRotateZ -= ROTATE_SPEED;
    }
  }

  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(directProgram);

  {
    glm::mat4 ident = glm::mat4();
    {
      glm::mat4 cameraReorient = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
                                             glm::vec3(0.0f, 1.0f, 0.0f),
                                             glm::vec3(0.0f, 0.0f, 1.0f));
      glm::mat4 cameraRotated = glm::rotate(cameraReorient, cameraRotateZ, glm::vec3(0.0f, 0.0f, 1.0f));
      glm::mat4 camera = glm::translate(cameraRotated, -cameraPosition);

      GLint cameraLoc = glGetUniformLocation(directProgram, "camera");
      glUniformMatrix4fv(cameraLoc, 1, GL_FALSE, glm::value_ptr(camera));
    }
    {
      GLint colorLoc = glGetUniformLocation(directProgram, "color");
      glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);
    }
  }

  glBindVertexArray(vao);
  for (int i = 0; i < ARRAY_LENGTH(quads); i++) {
    glDrawArrays(GL_TRIANGLE_FAN, i * 4, 4);
  }
  glBindVertexArray(0);

  glUseProgram(0);

  SDL_GL_SwapWindow(window);
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
