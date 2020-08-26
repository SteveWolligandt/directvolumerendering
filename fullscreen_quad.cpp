#include "fullscreen_quad.h"
yavin::indexeddata<vec2> fullscreen_quad() {
  yavin::indexeddata<vec2> f;
  f.vertexbuffer().reserve(4);
  f.vertexbuffer().push_back({0.0f, 0.0f});
  f.vertexbuffer().push_back({1.0f, 0.0f});
  f.vertexbuffer().push_back({0.0f, 1.0f});
  f.vertexbuffer().push_back({1.0f, 1.0f});
  f.vertexbuffer().reserve(6);
  f.indexbuffer().push_back(0);
  f.indexbuffer().push_back(1);
  f.indexbuffer().push_back(2);
  f.indexbuffer().push_back(1);
  f.indexbuffer().push_back(3);
  f.indexbuffer().push_back(2);
  return f;
}
