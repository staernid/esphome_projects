#pragma once
#include "esphome.h"
#include "lvgl.h"
#if __has_include("esp_system.h")
#include "esp_system.h"
#endif
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <cstdio>
#include <string>

namespace weather {

struct Particle {
  float x;
  float y;
  int w;
  int h;
};

struct Star {
  int x;
  int y;
  int freq;
  int phase;
  bool big;
};

static const Star NIGHT_STARS[24] = {
  {6, 8, 13, 0, true},    {18, 24, 17, 3, false}, {28, 10, 11, 5, false}, {38, 45, 15, 2, false},
  {48, 16, 19, 7, true},  {58, 36, 14, 1, false}, {68, 8, 12, 4, false},  {78, 28, 16, 6, true},
  {88, 14, 10, 2, false}, {96, 42, 18, 8, false}, {106, 8, 13, 3, false}, {118, 26, 15, 5, true},
  {12, 48, 11, 4, false}, {24, 34, 16, 1, true},  {34, 20, 12, 6, false}, {44, 54, 14, 0, false},
  {54, 8, 18, 3, false},  {64, 48, 10, 7, true},  {74, 18, 13, 2, false}, {84, 52, 17, 5, false},
  {94, 22, 11, 4, true},  {102, 50, 15, 1, false},{112, 40, 13, 6, false},{122, 12, 19, 0, false}
};

inline uint32_t xorshift32() {
  static uint32_t state = 0xACE1u;
  state ^= state << 13;
  state ^= state >> 17;
  state ^= state << 5;
  return state;
}

inline int fast_rand() {
  return static_cast<int>(xorshift32() & 0x7FFFFFFF);
}

struct MathUtils {
  static float fast_sin(float rad) {
    static float sin_lut[256];
    static bool initialized = false;
    if (!initialized) {
      for (int i = 0; i < 256; i++) {
        sin_lut[i] = sinf(i * 2.0f * 3.14159265f / 256.0f);
      }
      initialized = true;
    }
    int index = static_cast<int>(rad * (256.0f / (2.0f * 3.14159265f))) & 255;
    return sin_lut[index];
  }

  static float fast_cos(float rad) {
    return fast_sin(rad + 1.5707963f);
  }
};

}  // namespace weather
