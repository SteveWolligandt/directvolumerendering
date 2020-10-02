#include "unit_cube.h"
yavin::indexeddata<vec3, vec3> unit_cube() {
  yavin::indexeddata<vec3, vec3> c;

  c.vertexbuffer().reserve(8);
  c.vertexbuffer().push_back({-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 0.0f});
  c.vertexbuffer().push_back({1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f});
  c.vertexbuffer().push_back({-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f});
  c.vertexbuffer().push_back({1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, 0.0f});

  c.vertexbuffer().push_back({-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f});
  c.vertexbuffer().push_back({1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 1.0f});
  c.vertexbuffer().push_back({-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f});
  c.vertexbuffer().push_back({1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f});

  c.indexbuffer().reserve(12 * 3);

  // front
  c.indexbuffer().push_back(0);
  c.indexbuffer().push_back(1);
  c.indexbuffer().push_back(2);
  c.indexbuffer().push_back(1);
  c.indexbuffer().push_back(3);
  c.indexbuffer().push_back(2);

  // back
  c.indexbuffer().push_back(4);
  c.indexbuffer().push_back(6);
  c.indexbuffer().push_back(5);
  c.indexbuffer().push_back(5);
  c.indexbuffer().push_back(6);
  c.indexbuffer().push_back(7);

  // left
  c.indexbuffer().push_back(1);
  c.indexbuffer().push_back(5);
  c.indexbuffer().push_back(3);
  c.indexbuffer().push_back(5);
  c.indexbuffer().push_back(7);
  c.indexbuffer().push_back(3);

  // right
  c.indexbuffer().push_back(4);
  c.indexbuffer().push_back(0);
  c.indexbuffer().push_back(6);
  c.indexbuffer().push_back(0);
  c.indexbuffer().push_back(2);
  c.indexbuffer().push_back(6);

  // top
  c.indexbuffer().push_back(2);
  c.indexbuffer().push_back(3);
  c.indexbuffer().push_back(6);
  c.indexbuffer().push_back(3);
  c.indexbuffer().push_back(7);
  c.indexbuffer().push_back(6);

  // bottom
  c.indexbuffer().push_back(0);
  c.indexbuffer().push_back(4);
  c.indexbuffer().push_back(1);
  c.indexbuffer().push_back(1);
  c.indexbuffer().push_back(4);
  c.indexbuffer().push_back(5);
  return c;
}
