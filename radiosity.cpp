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

glm::vec3 cameraPosition = glm::vec3(10.0f, 20.0f, 3.5f);
float cameraRotateZ = 2.75f;

#define MOVE_SPEED 0.1f
#define ROTATE_SPEED 0.02f

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

Quad makeQuad(glm::vec3 a,
              glm::vec3 b,
              glm::vec3 c,
              glm::vec3 d,
              glm::vec3 normal,
              Color color) {
  Quad quad =  {{
      { a, normal, color },
      { b, normal, color },
      { c, normal, color },
      { a, normal, color },
      { c, normal, color },
      { d, normal, color }
    }};

  return quad;
}

Quad makeWall(float x, float y, float z,
              float dx, float dy, float dz,
              Color color) {
  assert(dx == 0 || dy == 0);
  assert(dx != 0 || dy != 0);
  assert(dz > 0);

  glm::vec3 normal;
  if(dx > 0) {
    normal = glm::vec3(0.0f, 1.0f, 0.0f);
  } else if (dx < 0) {
    normal = glm::vec3(0.0f, -1.0f, 0.0f);
  } else if(dy > 0) {
    normal = glm::vec3(-1.0f, 0.0f, 0.0f);
  } else if (dy < 0) {
    normal = glm::vec3(1.0f, 0.0f, 0.0f);
  }

  return makeQuad(glm::vec3(x, y, z),
                  glm::vec3(x + dx, y + dy, z),
                  glm::vec3(x + dx, y + dy, z + dz),
                  glm::vec3(x, y, z + dz),
                  normal,
                  color);
}

Quad makeFloor(float x, float y,
               float dx, float dy,
               Color color) {
  assert(dx != 0 && dy != 0);

  return makeQuad(glm::vec3(x, y, 0.0f),
                  glm::vec3(x, y + dy, 0.0f),
                  glm::vec3(x + dx, y + dy, 0.0f),
                  glm::vec3(x + dx, y, 0.0f),
                  glm::vec3(0.0f, 0.0f, 1.0f),
                  color);
}

Quad makeCeiling(float x, float y,
                 float dx, float dy,
                 Color color) {
  assert(dx != 0 && dy != 0);

  return makeQuad(glm::vec3(x, y, 5.0f),
                  glm::vec3(x + dx, y, 5.0f),
                  glm::vec3(x + dx, y + dy, 5.0f),
                  glm::vec3(x, y + dy, 5.0f),
                  glm::vec3(0.0f, 0.0f, -1.0f),
                  color);
}

const Color WHITE = {1.0f, 1.0f, 1.0f};
const Color RED = {1.0f, 0.0f, 0.0f};

Quad quads[] = {
  // Left column
  makeWall(9.0f, 17.0f, 0.0f,
           1.0f, 0.0f, 5.0f,
           WHITE),
  makeWall(10.0f, 16.0f, 0.0f,
           -1.0f, 0.0f, 5.0f,
           WHITE),
  makeWall(9.0f, 16.0f, 0.0f,
           0.0f, 1.0f, 5.0f,
           WHITE),
  makeWall(10.0f, 17.0f, 0.0f,
           0.0f, -1.0f, 5.0f,
           WHITE),

  // Right column
  makeWall(15.0f, 17.0f, 0.0f,
           1.0f, 0.0f, 5.0f,
           WHITE),
  makeWall(16.0f, 16.0f, 0.0f,
           -1.0f, 0.0f, 5.0f,
           WHITE),
  makeWall(15.0f, 16.0f, 0.0f,
           0.0f, 1.0f, 5.0f,
           WHITE),
  makeWall(16.0f, 17.0f, 0.0f,
           0.0f, -1.0f, 5.0f,
           WHITE),

  // Back wall
  makeWall(20.0f, 22.0f, 0.0f,
           -15.0f, 0.0f, 5.0f,
           WHITE),

  // Left Wall
  makeWall(5.0f, 22.0f, 0.0f,
           0.0f, -3.0f, 5.0f,
           WHITE),
  makeWall(5.0f, 19.0f, 0.0f,
           1.0f, 0.0f, 5.0f,
           WHITE),
  makeWall(6.0f, 19.0f, 0.0f,
           0.0f, -5.0f, 5.0f,
           WHITE),
  makeWall(6.0f, 14.0f, 0.0f,
           -1.0f, 0.0f, 5.0f,
           WHITE),
  makeWall(5.0f, 14.0f, 0.0f,
           0.0f, -3.0f, 5.0f,
           WHITE),

  // Front wall
  makeWall(5.0f, 11.0f, 0.0f,
           3.0f, 0.0f, 1.0f,
           WHITE),
  makeWall(5.0f, 11.0f, 4.0f,
           3.0f, 0.0f, 1.0f,
           WHITE),
  makeWall(8.0f, 11.0f, 0.0f,
           0.0f, 0.5f, 5.0f,
           WHITE),
  makeWall(8.0f, 11.5f, 0.0f,
           3.0f, 0.0f, 5.0f,
           WHITE),
  makeWall(11.0f, 11.5f, 0.0f,
           0.0f, -0.5f, 5.0f,
           WHITE),
  makeWall(11.0f, 11.0f, 0.0f,
           3.0f, 0.0f, 1.0f,
           WHITE),
  makeWall(11.0f, 11.0f, 4.0f,
           3.0f, 0.0f, 1.0f,
           WHITE),
  makeWall(14.0f, 11.0f, 0.0f,
           0.0f, 0.5f, 5.0f,
           WHITE),
  makeWall(14.0f, 11.5f, 0.0f,
           3.0f, 0.0f, 5.0f,
           WHITE),
  makeWall(17.0f, 11.5f, 0.0f,
           0.0f, -0.5f, 5.0f,
           WHITE),
  makeWall(17.0f, 11.0f, 0.0f,
           3.0f, 0.0f, 1.0f,
           WHITE),
  makeWall(17.0f, 11.0f, 4.0f,
           3.0f, 0.0f, 1.0f,
           WHITE),

  // Right wall
  makeWall(20.0f, 11.0f, 0.0f,
           0.0f, 3.0f, 5.0f,
           WHITE),
  makeWall(20.0f, 14.0f, 0.0f,
           -1.0f, 0.0f, 5.0f,
           WHITE),
  makeWall(19.0f, 14.0f, 0.0f,
           0.0f, 5.0f, 5.0f,
           WHITE),
  makeWall(19.0f, 19.0f, 0.0f,
           1.0f, 0.0f, 5.0f,
           WHITE),
  makeWall(20.0f, 19.0f, 0.0f,
           0.0f, 3.0f, 5.0f,
           WHITE),

  // Floor
  makeFloor(5.0f, 11.0f,
            3.0f, 3.0f,
            RED),
  makeFloor(8.0f, 11.5f,
            3.0f, 2.5f,
            RED),
  makeFloor(11.0f, 11.0f,
            3.0f, 3.0f,
            RED),
  makeFloor(14.0f, 11.5f,
            3.0f, 2.5f,
            RED),
  makeFloor(17.0f, 11.0f,
            3.0f, 3.0f,
            RED),
  makeFloor(6.0f, 14.0f,
            13.0f, 2.0f,
            RED),
  makeFloor(6.0f, 16.0f,
            3.0f, 1.0f,
            RED),
  makeFloor(10.0f, 16.0f,
            5.0f, 1.0f,
            RED),
  makeFloor(16.0f, 16.0f,
            3.0f, 1.0f,
            RED),
  makeFloor(6.0f, 17.0f,
            13.0f, 2.0f,
            RED),
  makeFloor(5.0f, 19.0f,
            15.0f, 3.0f,
            RED),

  // Ceiling
  makeCeiling(5.0f, 11.0f,
            3.0f, 3.0f,
            WHITE),
  makeCeiling(8.0f, 11.5f,
            3.0f, 2.5f,
            WHITE),
  makeCeiling(11.0f, 11.0f,
            3.0f, 3.0f,
            WHITE),
  makeCeiling(14.0f, 11.5f,
            3.0f, 2.5f,
            WHITE),
  makeCeiling(17.0f, 11.0f,
            3.0f, 3.0f,
            WHITE),
  makeCeiling(6.0f, 14.0f,
            13.0f, 2.0f,
            WHITE),
  makeCeiling(6.0f, 16.0f,
            3.0f, 1.0f,
            WHITE),
  makeCeiling(10.0f, 16.0f,
            5.0f, 1.0f,
            WHITE),
  makeCeiling(16.0f, 16.0f,
            3.0f, 1.0f,
            WHITE),
  makeCeiling(6.0f, 17.0f,
            13.0f, 2.0f,
            WHITE),
  makeCeiling(5.0f, 19.0f,
            15.0f, 3.0f,
            WHITE)

};

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
  glClearDepth(1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(directProgram);

  {
    {
      glm::mat4 cameraReorient = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
                                             glm::vec3(0.0f, 1.0f, 0.0f),
                                             glm::vec3(0.0f, 0.0f, 1.0f));
      glm::mat4 cameraRotated = glm::rotate(cameraReorient, cameraRotateZ, glm::vec3(0.0f, 0.0f, 1.0f));
      glm::mat4 camera = glm::translate(cameraRotated, -cameraPosition);

      GLint cameraLoc = glGetUniformLocation(directProgram, "camera");
      glUniformMatrix4fv(cameraLoc, 1, GL_FALSE, glm::value_ptr(camera));
    }
  }

  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, ARRAY_LENGTH(quads)*6);
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
