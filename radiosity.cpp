#include <stdio.h>
#include <SDL.h>
#include <OpenGL/gl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/perpendicular.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/vector_angle.hpp>

#define fail { printf("FAIL %d\n", __LINE__); return 1; }

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof(x[0]))

#define PRINT(v) printf("%s\n", glm::to_string(v).c_str())

using glm::vec3;

#define HEMICUBE_RESOLUTION 50
const int HEMICUBE_TEXTURE_WIDTH = HEMICUBE_RESOLUTION*2;
const int HEMICUBE_TEXTURE_HEIGHT = HEMICUBE_RESOLUTION*2;

const int TOP_X = HEMICUBE_RESOLUTION/2;
const int TOP_Y = 0;
const int BOTTOM_X = HEMICUBE_RESOLUTION/2;
const int BOTTOM_Y = HEMICUBE_RESOLUTION*3/2;
const int LEFT_X = 0;
const int LEFT_Y = HEMICUBE_RESOLUTION/2;
const int RIGHT_X = HEMICUBE_RESOLUTION*3/2;
const int RIGHT_Y = HEMICUBE_RESOLUTION/2;
const int FRONT_X = HEMICUBE_RESOLUTION/2;
const int FRONT_Y = HEMICUBE_RESOLUTION/2;

#define TEXEL_DENSITY 4
#define PASSES 8

void radiosify();
void hemicubeSetup();
void loadTextures();
void tick();
GLuint createShader(const char* name, GLenum shaderType);
void render(glm::mat4 camera, GLuint program);
void renderHemicube(vec3 location, vec3 normal);
void setDisplaySize(int width, int height);
void generateTextures();
void prepareMultiplierMap();

bool quit = false;

vec3 cameraPosition = vec3(10.0f, 20.0f, 3.5f);
float cameraRotateZ = 2.75f;
float cameraRotateUp = 0.0f;

#define MOVE_SPEED 0.1f
#define ROTATE_SPEED 0.01f

SDL_Window* window;

GLuint vbo;
GLuint vao;

GLuint programs[2];
int currentProgram = 1;

#define POSITION_ATTRIB 0
#define NORMAL_ATTRIB 1
#define COLOR_ATTRIB 2
#define TEXCOORD_ATTRIB 3

struct Color {
  float r;
  float g;
  float b;

  Color& operator+=(const Color& other) {
    this->r += other.r;
    this->g += other.g;
    this->b += other.b;
    return *this;
  }

  Color operator+(const Color& other) {
    Color result = {r + other.r, g + other.g, b + other.b};
    return result;
  }

  Color operator*(float scale) {
    Color result = {r * scale, g * scale, b * scale};
    return result;
  }
};

struct Material {
  Color color;
};

struct Vertex {
  vec3 position;
  vec3 normal;
  Color color;
  float uv[2];
};

struct Rect {
  vec3 origin;
  vec3 da;
  vec3 db;
  Color color;
};

struct Quad {
  Vertex vertices[6];
};

const Color WHITE = {0.85f, 0.85f, 0.85f};
const Color RED = {0.8f, 0.0f, 0.0f};

#include "geometry.cpp"

GLuint textures[ARRAY_LENGTH(rects)];
Color *textureData[ARRAY_LENGTH(rects)];

Color hemicubeAverage();

int main(int argc, char** argv) {
  setbuf(stdout, NULL);

  buildMesh();

  if (SDL_Init(SDL_INIT_VIDEO) < 0) fail;

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
  SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);

  window = SDL_CreateWindow("Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_OPENGL);
  if (!window) fail;

  SDL_GL_CreateContext(window);
  SDL_GL_SetSwapInterval(1);

  glEnable(GL_FRAMEBUFFER_SRGB);

  SDL_SetRelativeMouseMode(SDL_TRUE);

  prepareMultiplierMap();


  programs[0] = glCreateProgram();
  {
    GLuint directProgram = programs[0];
    GLuint directVert = createShader("shaders/direct.vert.glsl", GL_VERTEX_SHADER);
    GLuint directFrag = createShader("shaders/direct.frag.glsl", GL_FRAGMENT_SHADER);

    glAttachShader(directProgram, directVert);
    glAttachShader(directProgram, directFrag);

    glBindAttribLocation(directProgram, POSITION_ATTRIB, "position");
    glBindAttribLocation(directProgram, NORMAL_ATTRIB, "normal");
    glBindAttribLocation(directProgram, COLOR_ATTRIB, "color");
    glBindAttribLocation(directProgram, TEXCOORD_ATTRIB, "texcoord");

    glLinkProgram(directProgram);

    glDeleteShader(directVert);
    glDeleteShader(directFrag);
  }

  programs[1] = glCreateProgram();
  {
    GLuint radiosityProgram = programs[1];
    GLuint radiosityVert = createShader("shaders/radiosity.vert.glsl", GL_VERTEX_SHADER);
    GLuint radiosityFrag = createShader("shaders/radiosity.frag.glsl", GL_FRAGMENT_SHADER);

    glAttachShader(radiosityProgram, radiosityVert);
    glAttachShader(radiosityProgram, radiosityFrag);

    glBindAttribLocation(radiosityProgram, POSITION_ATTRIB, "position");
    glBindAttribLocation(radiosityProgram, NORMAL_ATTRIB, "normal");
    glBindAttribLocation(radiosityProgram, COLOR_ATTRIB, "color");
    glBindAttribLocation(radiosityProgram, TEXCOORD_ATTRIB, "texcoord");

    glLinkProgram(radiosityProgram);

    glDeleteShader(radiosityVert);
    glDeleteShader(radiosityFrag);
  }

  for (int i = 0; i < ARRAY_LENGTH(programs); i++) {
    glUseProgram(programs[i]);
    {
      GLint directionalDirLoc = glGetUniformLocation(programs[i], "directional_dir");
      vec3 lightDir = glm::normalize(vec3(0.2f, 1.0f, -1.0f));
      glUniform3fv(directionalDirLoc, 1, glm::value_ptr(lightDir));
    }
    {
      GLint directionalIntensityLoc = glGetUniformLocation(programs[i], "directional_intensity");
      glUniform1f(directionalIntensityLoc, 0.3f);
    }
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

    glEnableVertexAttribArray(TEXCOORD_ATTRIB);
    glVertexAttribPointer(TEXCOORD_ATTRIB, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) (9 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  generateTextures();
  loadTextures();

  hemicubeSetup();

  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glDepthFunc(GL_LEQUAL);
  glDepthRange(0.0f, 1.0f);

  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CW);
  glCullFace(GL_BACK);

  for (int i = 0; i < PASSES; i++) {
    printf("Pass %d\n", i+1);
    radiosify();
    loadTextures();
  }

  {
    int width;
    int height;
    SDL_GetWindowSize(window, &width, &height);

    for (int i = 0; i < ARRAY_LENGTH(programs); i++) {
      glUseProgram(programs[i]);
      {
        GLint projLoc = glGetUniformLocation(programs[i], "proj");
        glm::mat4 proj = glm::perspective((float) M_PI/3, (float) width / (float) height, 0.1f, 100.0f);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
      }
    }
    glUseProgram(0);

    glViewport(0, 0, width, height);
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
        } else if (event.key.keysym.sym == 'p') {
          currentProgram = (currentProgram + 1) % ARRAY_LENGTH(programs);
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
      cameraPosition += glm::rotateZ(vec3(0.0f, MOVE_SPEED, 0.0f), -cameraRotateZ);
    }
    if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]) {
      cameraPosition -= glm::rotateZ(vec3(0.0f, MOVE_SPEED, 0.0f), -cameraRotateZ);
    }
    if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) {
      cameraPosition += glm::rotateZ(vec3(MOVE_SPEED, 0.0f, 0.0f), -cameraRotateZ);
    }
    if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) {
      cameraPosition -= glm::rotateZ(vec3(MOVE_SPEED, 0.0f, 0.0f), -cameraRotateZ);
    }
  }

  {
    glm::mat4 cameraReorient = glm::lookAt(vec3(0.0f, 0.0f, 0.0f),
                                           vec3(0.0f, 1.0f, 0.0f),
                                           vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 cameraRotated =
      glm::rotate(glm::rotate(cameraReorient, cameraRotateUp, vec3(1.0f, 0.0f, 0.0f)),
                  cameraRotateZ, vec3(0.0, 0.0, 1.0f));
    glm::mat4 camera = glm::translate(cameraRotated, -cameraPosition);

    render(camera, programs[currentProgram]);
  }

  SDL_GL_SwapWindow(window);
}

void render(glm::mat4 camera, GLuint program) {
  glClearColor(1.0, 1.0, 1.0, 1.0);
  glClearDepth(1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(program);
  GLint texLoc = glGetUniformLocation(program, "tex");
  glUniform1i(texLoc, 0);

  GLint cameraLoc = glGetUniformLocation(program, "camera");
  glUniformMatrix4fv(cameraLoc, 1, GL_FALSE, glm::value_ptr(camera));

  glBindVertexArray(vao);
  for (int i = 0; i < ARRAY_LENGTH(quads); i++) {
    glBindTexture(GL_TEXTURE_2D, textures[i]);
    glDrawArrays(GL_TRIANGLES, 6 * i, 6);
  }
  glBindVertexArray(0);

  glUseProgram(0);
}

GLuint hemicubeFrameBuffer;
GLuint hemicubeColorBuffer;
GLuint hemicubeDepthBuffer;

void hemicubeSetup() {
  glGenFramebuffers(1, &hemicubeFrameBuffer);

  glBindFramebuffer(GL_FRAMEBUFFER, hemicubeFrameBuffer);

  glGenTextures(1, &hemicubeColorBuffer);
  glBindTexture(GL_TEXTURE_2D, hemicubeColorBuffer);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, HEMICUBE_TEXTURE_WIDTH, HEMICUBE_TEXTURE_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hemicubeColorBuffer, 0);
  // glBindTexture(GL_TEXTURE_2D, 0);

  glGenRenderbuffers(1, &hemicubeDepthBuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, hemicubeDepthBuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, HEMICUBE_TEXTURE_WIDTH, HEMICUBE_TEXTURE_HEIGHT);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, hemicubeDepthBuffer);
  // glBindRenderbuffer(GL_RENDERBUFFER, 0);

  // glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderHemicube(vec3 location, vec3 normal) {
  GLuint program = programs[1];

  glBindFramebuffer(GL_FRAMEBUFFER, hemicubeFrameBuffer);

  float nearPlane = 0.05f;

  assert(nearPlane < 0.5f / TEXEL_DENSITY);

  {
    glUseProgram(program);
    GLint projLoc = glGetUniformLocation(program, "proj");
    glm::mat4 proj = glm::perspective((float) M_PI_2, 1.0f, nearPlane, 100.0f);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
    glUseProgram(0);
  }

  vec3 up;
  if (glm::angle(normal, vec3(0.0f, 0.0f, 1.0f)) > 0.1 && glm::angle(normal, vec3(0.0f, 0.0f, -1.0f)) > 0.1) {
    up = vec3(0.0f, 0.0f, 1.0f);
  } else {
    up = vec3(1.0f, 0.0f, 0.0f);
  }

  vec3 sideways = glm::cross(normal, up);

  // Front
  {
    glViewport(FRONT_X, FRONT_Y, HEMICUBE_RESOLUTION, HEMICUBE_RESOLUTION);
    glm::mat4 camera = glm::lookAt(location, location + normal, up);
    render(camera, program);
  }

  {
    glEnable(GL_SCISSOR_TEST);

    // Right
    {
      glViewport(RIGHT_X, RIGHT_Y, HEMICUBE_RESOLUTION, HEMICUBE_RESOLUTION);
      glScissor(RIGHT_X, RIGHT_Y, HEMICUBE_RESOLUTION/2, HEMICUBE_RESOLUTION);
      glm::mat4 camera = glm::lookAt(location, location + sideways, up);
      render(camera, program);
    }

    // Left
    {
      glScissor(LEFT_X, LEFT_Y, HEMICUBE_RESOLUTION/2, HEMICUBE_RESOLUTION);
      glViewport(LEFT_X - HEMICUBE_RESOLUTION/2, LEFT_Y, HEMICUBE_RESOLUTION, HEMICUBE_RESOLUTION);
      glm::mat4 camera = glm::lookAt(location, location - sideways, up);
      render(camera, program);
    }

    // Down
    {
      glViewport(TOP_X, TOP_Y - HEMICUBE_RESOLUTION/2, HEMICUBE_RESOLUTION, HEMICUBE_RESOLUTION);
      glScissor(TOP_X, TOP_Y, HEMICUBE_RESOLUTION, HEMICUBE_RESOLUTION/2);
      glm::mat4 camera = glm::lookAt(location, location - up, normal);
      render(camera, program);
    }

    // Up
    {
      glViewport(BOTTOM_X, BOTTOM_Y, HEMICUBE_RESOLUTION, HEMICUBE_RESOLUTION);
      glScissor(BOTTOM_X, BOTTOM_Y, HEMICUBE_RESOLUTION, HEMICUBE_RESOLUTION/2);
      glm::mat4 camera = glm::lookAt(location, location + up, -normal);
      render(camera, program);
    }

    glDisable(GL_SCISSOR_TEST);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Color hemicubeTextureData[HEMICUBE_TEXTURE_WIDTH][HEMICUBE_TEXTURE_HEIGHT];
float multiplierMap[HEMICUBE_TEXTURE_WIDTH][HEMICUBE_TEXTURE_HEIGHT];

float cosine(vec3 a, vec3 b) {
  return glm::dot(a, b) / glm::length(a) / glm::length(b);
}

void prepareMultiplierMap() {
  vec3 surfaceNormal = vec3(0.0, HEMICUBE_RESOLUTION/2, 0.0f);

  vec3 faceNormal = vec3(0.0f, 0.0f, HEMICUBE_RESOLUTION/2);
  for (int y = TOP_Y; y < TOP_Y + HEMICUBE_RESOLUTION/2; y++) {
    for (int x = TOP_X; x < TOP_X + HEMICUBE_RESOLUTION; x++) {
      vec3 loc = vec3(TOP_X + HEMICUBE_RESOLUTION/2 - x, y - TOP_Y, HEMICUBE_RESOLUTION/2);
      multiplierMap[y][x] = cosine(faceNormal, loc) * cosine(surfaceNormal, loc);
    }
  }

  faceNormal = vec3(0.0f, 0.0f, -HEMICUBE_RESOLUTION/2);
  for (int y = BOTTOM_Y; y < BOTTOM_Y + HEMICUBE_RESOLUTION/2; y++) {
    for (int x = BOTTOM_X; x < BOTTOM_X + HEMICUBE_RESOLUTION; x++) {
      vec3 loc = vec3(BOTTOM_X + HEMICUBE_RESOLUTION/2 - x, BOTTOM_Y + HEMICUBE_RESOLUTION/2 - y, -HEMICUBE_RESOLUTION/2);
      multiplierMap[y][x] = cosine(faceNormal, loc) * cosine(surfaceNormal, loc);
    }
  }

  faceNormal = vec3(-HEMICUBE_RESOLUTION/2, 0.0f, 0.0f);
  for (int y = LEFT_Y; y < LEFT_Y + HEMICUBE_RESOLUTION; y++) {
    for (int x = LEFT_X; x < LEFT_X + HEMICUBE_RESOLUTION/2; x++) {
      vec3 loc = vec3(-HEMICUBE_RESOLUTION/2, x - LEFT_X, LEFT_Y + HEMICUBE_RESOLUTION/2 - y);
      multiplierMap[y][x] = cosine(faceNormal, loc) * cosine(surfaceNormal, loc);
    }
  }

  faceNormal = vec3(HEMICUBE_RESOLUTION/2, 0.0f, 0.0f);
  for (int y = RIGHT_Y; y < RIGHT_Y + HEMICUBE_RESOLUTION; y++) {
    for (int x = RIGHT_X; x < RIGHT_X + HEMICUBE_RESOLUTION/2; x++) {
      vec3 loc = vec3(HEMICUBE_RESOLUTION/2, RIGHT_X + HEMICUBE_RESOLUTION/2 - x, RIGHT_Y + HEMICUBE_RESOLUTION/2 - y);
      multiplierMap[y][x] = cosine(faceNormal, loc) * cosine(surfaceNormal, loc);
    }
  }

  faceNormal = vec3(0.0f, HEMICUBE_RESOLUTION/2, 0.0f);
  for (int y = FRONT_Y; y < FRONT_Y + HEMICUBE_RESOLUTION; y++) {
    for (int x = FRONT_X; x < FRONT_X + HEMICUBE_RESOLUTION; x++) {
      vec3 loc = vec3(FRONT_X + HEMICUBE_RESOLUTION/2 - x, HEMICUBE_RESOLUTION/2, FRONT_Y + HEMICUBE_RESOLUTION/2 - y);
      multiplierMap[y][x] = cosine(faceNormal, loc) * cosine(surfaceNormal, loc);
    }
  }

  float total = 0.0f;
  for (int y = 0; y < HEMICUBE_TEXTURE_HEIGHT; y++) {
    for (int x = 0; x < HEMICUBE_TEXTURE_WIDTH; x++) {
      total += multiplierMap[y][x];
    }
  }
  for (int y = 0; y < HEMICUBE_TEXTURE_HEIGHT; y++) {
    for (int x = 0; x < HEMICUBE_TEXTURE_WIDTH; x++) {
      multiplierMap[y][x] /= total;
    }
  }

#if 0
  GLuint multiplierMapTexture;
  glGenTextures(1, &multiplierMapTexture);

  glBindTexture(GL_TEXTURE_2D, multiplierMapTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, HEMICUBE_TEXTURE_WIDTH, HEMICUBE_TEXTURE_HEIGHT, 0, GL_RED, GL_FLOAT, multiplierMap);
  glBindTexture(GL_TEXTURE_2D, 0);
#endif
}

Color hemicubeAverage() {
  glBindFramebuffer(GL_FRAMEBUFFER, hemicubeFrameBuffer);

  glReadPixels(0, 0,
               HEMICUBE_TEXTURE_WIDTH, HEMICUBE_TEXTURE_HEIGHT,
               GL_RGB, GL_FLOAT,
               hemicubeTextureData);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  Color result = {0.0f, 0.0f, 0.0f};

  for (int y = TOP_Y; y < TOP_Y + HEMICUBE_RESOLUTION/2; y++) {
    for (int x = TOP_X; x < TOP_X + HEMICUBE_RESOLUTION; x++) {
      result += hemicubeTextureData[y][x] * multiplierMap[y][x];
    }
  }

  for (int y = BOTTOM_Y; y < BOTTOM_Y + HEMICUBE_RESOLUTION/2; y++) {
    for (int x = BOTTOM_X; x < BOTTOM_X + HEMICUBE_RESOLUTION; x++) {
      result += hemicubeTextureData[y][x] * multiplierMap[y][x];
    }
  }

  for (int y = LEFT_Y; y < LEFT_Y + HEMICUBE_RESOLUTION; y++) {
    for (int x = LEFT_X; x < LEFT_X + HEMICUBE_RESOLUTION/2; x++) {
      result += hemicubeTextureData[y][x] * multiplierMap[y][x];
    }
  }

  for (int y = RIGHT_Y; y < RIGHT_Y + HEMICUBE_RESOLUTION; y++) {
    for (int x = RIGHT_X; x < RIGHT_X + HEMICUBE_RESOLUTION/2; x++) {
      result += hemicubeTextureData[y][x] * multiplierMap[y][x];
    }
  }

  for (int y = FRONT_Y; y < FRONT_Y + HEMICUBE_RESOLUTION; y++) {
    for (int x = FRONT_X; x < FRONT_X + HEMICUBE_RESOLUTION; x++) {
      result += hemicubeTextureData[y][x] * multiplierMap[y][x];
    }
  }

  return result;
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

void generateTextures() {
  glGenTextures(ARRAY_LENGTH(textures), textures);
  for (int i = 0; i < ARRAY_LENGTH(textures); i++) {
    int width = glm::length(rects[i].da) * TEXEL_DENSITY;
    int height = glm::length(rects[i].db) * TEXEL_DENSITY;

    textureData[i] = (Color*) malloc(sizeof(Color) * width * height * TEXEL_DENSITY * TEXEL_DENSITY);

    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        Color black = {0.0f, 0.0f, 0.0f};
        textureData[i][y*width + x] = black;
      }
    }
  }
}

void loadTextures() {
  for (int i = 0; i < ARRAY_LENGTH(textures); i++) {
    int width = glm::length(rects[i].da) * TEXEL_DENSITY;
    int height = glm::length(rects[i].db) * TEXEL_DENSITY;

    glBindTexture(GL_TEXTURE_2D, textures[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, textureData[i]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    float border[3] = {0.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }
}

void radiosify() {
  float error = 0.0f;
  for (int i = 0; i < ARRAY_LENGTH(rects); i++) {
    printf("Rect %d\r", i);
    Rect rect = rects[i];
    Color* texture = textureData[i];

    vec3 norm = normal(rect);

    int width = glm::length(rect.da) * TEXEL_DENSITY;
    int height = glm::length(rect.db) * TEXEL_DENSITY;

    vec3 da = rect.da / (float)width;
    vec3 db = rect.db / (float)height;

    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        vec3 location = rect.origin + da*(x+0.5f) + db*(y+0.5f);
        renderHemicube(location, norm);
        Color avg = hemicubeAverage();
        Color result = {avg.r * rect.color.r,
                        avg.g * rect.color.g,
                        avg.b * rect.color.b};
        error += fabs(texture[y*width + x].r - result.r)
          + fabs(texture[y*width + x].g - result.g)
          + fabs(texture[y*width + x].b - result.b);
        texture[y*width + x] = result;
      }
    }
  }

  printf("\n");
  printf("Error: %f\n", error);
}
