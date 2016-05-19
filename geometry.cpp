
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
