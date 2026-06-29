#pragma once
#if __has_include("esphome.h")
#include "esphome.h"
#endif
#if __has_include("lvgl.h")
#include "lvgl.h"
#else
struct lv_layer_t {};
struct lv_obj_t {};
#endif
#if __has_include("esp_system.h")
#include "esp_system.h"
#endif
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <cstdio>
#include <string>
#include <chrono>

namespace weather {

inline uint32_t get_sim_millis() {
#if __has_include("esphome.h")
  return ::millis();
#else
  using namespace std::chrono;
  static auto start = steady_clock::now();
  return duration_cast<milliseconds>(steady_clock::now() - start).count();
#endif
}

inline uint32_t get_sim_micros() {
#if __has_include("esphome.h")
  return ::micros();
#else
  using namespace std::chrono;
  static auto start = steady_clock::now();
  return duration_cast<microseconds>(steady_clock::now() - start).count();
#endif
}


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

struct WaveGenerator {
  enum WaveRenderMode {
    SOLID_FILL,   // Solid filled band from baseline to wave height
    OUTLINE_ONLY, // Pure 1-pixel vector outline line with transparent background
    OUTLINE_FILL  // 1-pixel outline line with solid fill behind it
  };

  static float calculate_wave_height(float x, float scroll_x, uint32_t step, float freq_mult = 1.0f, float amp_mult = 1.0f) {
    float wave1 = MathUtils::fast_sin((x + scroll_x) * 0.06f * freq_mult) * (2.5f * amp_mult);
    float wave2 = MathUtils::fast_cos((x + scroll_x * 1.3f) * 0.12f * freq_mult + step * 0.03f) * (1.8f * amp_mult);
    float wave3 = MathUtils::fast_sin(step * 0.04f + x * 0.08f * freq_mult) * (1.0f * amp_mult);
    return wave1 + wave2 + wave3;
  }
};

}  // namespace weather
