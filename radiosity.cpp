#include <stdio.h>
#include <SDL.h>
#include <OpenGL/gl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define fail { printf("FAIL %d\n", __LINE__); return 1; }

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof(x[0]))

void tick();
GLuint createShader(const char* name, GLenum shaderType);

bool quit = false;


SDL_Window* window;

GLuint vbo;
GLuint vao;
GLuint gouraudProgram;

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
      { glm::vec3(0.0f, 0.0f, -0.5f), glm::vec3(0.0f, 0.0f, 1.0f) },
      { glm::vec3(1.0f, 0.0f, -0.5f), glm::vec3(0.0f, 0.0f, 1.0f) },
      { glm::vec3(1.0f, 1.0f, -0.5f), glm::vec3(0.0f, 0.0f, 1.0f) },
      { glm::vec3(0.0f, 1.0f, -0.5f), glm::vec3(0.0f, 0.0f, 1.0f) }
    }},
  {{
      { glm::vec3(1.0f, 1.0f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f) },
      { glm::vec3(1.0f, 0.0f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f) },
      { glm::vec3(1.0f, 0.0f, 0.5f), glm::vec3(-1.0f, 0.0f, 0.0f) },
      { glm::vec3(1.0f, 1.0f, 0.5f), glm::vec3(-1.0f, 0.0f, 0.0f) }
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


  GLuint gouraudVert = createShader("shaders/gouraud.vert.glsl", GL_VERTEX_SHADER);
  GLuint gouraudFrag = createShader("shaders/gouraud.frag.glsl", GL_FRAGMENT_SHADER);

  gouraudProgram = glCreateProgram();
  {
    glAttachShader(gouraudProgram, gouraudVert);
    glAttachShader(gouraudProgram, gouraudFrag);

    glBindAttribLocation(gouraudProgram, POSITION_ATTRIB, "position");
    glBindAttribLocation(gouraudProgram, NORMAL_ATTRIB, "normal");

    glLinkProgram(gouraudProgram);

    glDeleteShader(gouraudVert);
    glDeleteShader(gouraudFrag);
  }


  {
    GLint projLoc = glGetUniformLocation(gouraudProgram, "proj");
    glm::mat4 ident = glm::mat4();
    glm::mat4 proj = glm::perspective(60.0f, 640.0f/480.0f, 0.1f, 100.0f);
    glUseProgram(gouraudProgram);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
    glUseProgram(0);
  }

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

  while (!quit) tick();

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

  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(gouraudProgram);

  {
    glm::mat4 ident = glm::mat4();
    {
      GLint cameraLoc = glGetUniformLocation(gouraudProgram, "camera");
      glUniformMatrix4fv(cameraLoc, 1, GL_FALSE, glm::value_ptr(ident));
    }
    {
      GLint modelLoc = glGetUniformLocation(gouraudProgram, "model");
      glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(ident));
    }

    {
      GLint colorLoc = glGetUniformLocation(gouraudProgram, "color");
      glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);
    }
    {
      GLint directionalIntensityLoc = glGetUniformLocation(gouraudProgram, "directional_intensity");
      glUniform1f(directionalIntensityLoc, 0.0f);
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
