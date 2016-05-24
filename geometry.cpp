
Quad makeQuad(vec3 a,
              vec3 b,
              vec3 c,
              vec3 d,
              vec3 normal,
              Color color) {
  Quad quad =  {{
      { a, normal, color, {0.0, 0.0} },
      { b, normal, color, {1.0, 0.0} },
      { c, normal, color, {1.0, 1.0} },
      { a, normal, color, {0.0, 0.0} },
      { c, normal, color, {1.0, 1.0} },
      { d, normal, color, {0.0, 1.0} }
    }};

  return quad;
}

Rect makeFloor(float x, float y,
               float dx, float dy,
               Color color) {
  assert(dx != 0 && dy != 0);

  Rect rect = {vec3(x, y, 0.0f),
               vec3(0.0f, dy, 0.0f),
               vec3(dx, 0.0f, 0.0f),
               color};

  return rect;
}

Rect makeCeiling(float x, float y,
                 float dx, float dy,
                 Color color) {
  assert(dx != 0 && dy != 0);

  Rect rect = {vec3(x, y, 6.0f),
               vec3(dx, 0.0f, 0.0f),
               vec3(0.0f, dy, 0.0f),
               BLUE};

  return rect;
}

vec3 normal(Rect rect) {
  return glm::normalize(glm::cross(rect.db, rect.da));
}

Quad makeQuad(Rect rect) {
  return makeQuad(rect.origin,
                  rect.origin + rect.da,
                  rect.origin + rect.da + rect.db,
                  rect.origin + rect.db,
                  normal(rect),
                  rect.color);
}

Rect rects[] = {
  // Left column
  {vec3(9.0f, 17.0f, 0.0f),
   vec3(1.0f, 0.0f, 0.0f),
   vec3(0.0f, 0.0f, 6.0f),
   MAGENTA},
  {vec3(10.0f, 16.0f, 0.0f),
   vec3(-1.0f, 0.0f, 0.0f),
   vec3(0.0f, 0.0f, 6.0f),
   MAGENTA},
  {vec3(9.0f, 16.0f, 0.0f),
   vec3(0.0f, 1.0f, 0.0f),
   vec3(0.0f, 0.0f, 6.0f),
   MAGENTA},
  {vec3(10.0f, 17.0f, 0.0f),
   vec3(0.0f, -1.0f, 0.0f),
   vec3(0.0f, 0.0f, 6.0f),
   MAGENTA},

  // Right column
  {vec3(15.0f, 17.0f, 0.0f),
   vec3(1.0f, 0.0f, 0.0f),
   vec3(0.0f, 0.0f, 6.0f),
   MAGENTA},
  {vec3(16.0f, 16.0f, 0.0f),
   vec3(-1.0f, 0.0f, 0.0f),
   vec3(0.0f, 0.0f, 6.0f),
   MAGENTA},
  {vec3(15.0f, 16.0f, 0.0f),
   vec3(0.0f, 1.0f, 0.0f),
   vec3(0.0f, 0.0f, 6.0f),
   MAGENTA},
  {vec3(16.0f, 17.0f, 0.0f),
   vec3(0.0f, -1.0f, 0.0f),
   vec3(0.0f, 0.0f, 6.0f),
   MAGENTA},

  // Back wall
  {vec3(20.0f, 22.0f, 0.0f),
   vec3(-15.0f, 0.0f, 0.0f),
   vec3(0.0f, 0.0f, 6.0f),
   WHITE},

  // Left Wall
  {vec3(5.0f, 22.0f, 0.0f),
   vec3(0.0f, -3.0f, 0.0f),
   vec3(0.0f, 0.0f, 6.0f),
   WHITE},
  {vec3(5.0f, 19.0f, 0.0f),
   vec3(1.0f, 0.0f, 0.0f),
   vec3(0.0f, 0.0f, 6.0f),
   WHITE},
  {vec3(6.0f, 19.0f, 0.0f),
   vec3(0.0f, -5.0f, 0.0f),
   vec3(0.0f, 0.0f, 6.0f),
   WHITE},
  {vec3(6.0f, 14.0f, 0.0f),
   vec3(-1.0f, 0.0f, 0.0f),
   vec3(0.0f, 0.0f, 6.0f),
   WHITE},
  {vec3(5.0f, 14.0f, 0.0f),
   vec3(0.0f, -3.0f, 0.0f),
   vec3(0.0f, 0.0f, 6.0f),
   WHITE},

  // Front wall
  {vec3(5.0f, 11.0f, 0.0f),
   vec3(3.0f, 0.0f, 0.0f),
   vec3(0.0f, 0.0f, 1.5f),
   WHITE},
  {vec3(5.0f, 11.0f, 5.0f),
   vec3(3.0f, 0.0f, 0.0f),
   vec3(0.0f, 0.0f, 1.0f),
   WHITE},
  {vec3(8.0f, 11.0f, 0.0f),
   vec3(0.0f, 0.5f, 0.0f),
   vec3(0.0f, 0.0f, 6.0f),
   WHITE},
  {vec3(8.0f, 11.5f, 0.0f),
   vec3(3.0f, 0.0f, 0.0f),
   vec3(0.0f, 0.0f, 6.0f),
   WHITE},
  {vec3(11.0f, 11.5f, 0.0f),
   vec3(0.0f, -0.5f, 0.0f),
   vec3(0.0f, 0.0f, 6.0f),
   WHITE},
  {vec3(11.0f, 11.0f, 0.0f),
   vec3(3.0f, 0.0f, 0.0f),
   vec3(0.0f, 0.0f, 1.5f),
   WHITE},
  {vec3(11.0f, 11.0f, 5.0f),
   vec3(3.0f, 0.0f, 0.0f),
   vec3(0.0f, 0.0f, 1.0f),
   WHITE},
  {vec3(14.0f, 11.0f, 0.0f),
   vec3(0.0f, 0.5f, 0.0f),
   vec3(0.0f, 0.0f, 6.0f),
   WHITE},
  {vec3(14.0f, 11.5f, 0.0f),
   vec3(3.0f, 0.0f, 0.0f),
   vec3(0.0f, 0.0f, 6.0f),
   WHITE},
  {vec3(17.0f, 11.5f, 0.0f),
   vec3(0.0f, -0.5f, 0.0f),
   vec3(0.0f, 0.0f, 6.0f),
   WHITE},
  {vec3(17.0f, 11.0f, 0.0f),
   vec3(3.0f, 0.0f, 0.0f),
   vec3(0.0f, 0.0f, 1.5f),
   WHITE},
  {vec3(17.0f, 11.0f, 5.0f),
   vec3(3.0f, 0.0f, 0.0f),
   vec3(0.0f, 0.0f, 1.0f),
   WHITE},

  // Right wall
  {vec3(20.0f, 11.0f, 0.0f),
   vec3(0.0f, 3.0f, 0.0f),
   vec3(0.0f, 0.0f, 6.0f),
   WHITE},
  {vec3(20.0f, 14.0f, 0.0f),
   vec3(-1.0f, 0.0f, 0.0f),
   vec3(0.0f, 0.0f, 6.0f),
   WHITE},
  {vec3(19.0f, 14.0f, 0.0f),
   vec3(0.0f, 5.0f, 0.0f),
   vec3(0.0f, 0.0f, 6.0f),
   WHITE},
  {vec3(19.0f, 19.0f, 0.0f),
   vec3(1.0f, 0.0f, 0.0f),
   vec3(0.0f, 0.0f, 6.0f),
   WHITE},
  {vec3(20.0f, 19.0f, 0.0f),
   vec3(0.0f, 3.0f, 0.0f),
   vec3(0.0f, 0.0f, 6.0f),
   WHITE},

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

Quad quads[ARRAY_LENGTH(rects)];

void buildMesh() {
  for (int i = 0; i < ARRAY_LENGTH(quads); i++) {
    quads[i] = makeQuad(rects[i]);
  }
}
