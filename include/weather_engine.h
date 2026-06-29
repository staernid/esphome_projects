#pragma once
#include "esphome.h"
#include "lvgl.h"
#include "esp_system.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <cstdio>

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

class DrawingUtils {
 public:
  static void draw_rect(lv_layer_t *layer, int x, int y, int w, int h, int radius, bool is_white) {
    lv_draw_rect_dsc_t dsc;
    lv_draw_rect_dsc_init(&dsc);
    dsc.bg_color = is_white ? lv_color_make(255, 255, 255) : lv_color_make(0, 0, 0);
    dsc.radius = radius;
    lv_area_t area = {x, y, x + w - 1, y + h - 1};
    lv_draw_rect(layer, &dsc, &area);
  }

  static void draw_arc(lv_layer_t *layer, int cx, int cy, int radius, int start_angle, int end_angle, int width, bool is_white) {
    lv_draw_arc_dsc_t dsc;
    lv_draw_arc_dsc_init(&dsc);
    dsc.color = is_white ? lv_color_make(255, 255, 255) : lv_color_make(0, 0, 0);
    dsc.width = width;
    dsc.center.x = cx;
    dsc.center.y = cy;
    dsc.radius = radius;
    dsc.start_angle = start_angle;
    dsc.end_angle = end_angle;
    lv_draw_arc(layer, &dsc);
  }

  static void draw_circle(lv_layer_t *layer, int cx, int cy, int radius, bool is_white) {
    draw_rect(layer, cx - radius, cy - radius, radius * 2 + 1, radius * 2 + 1, LV_RADIUS_CIRCLE, is_white);
  }

  static void draw_line(lv_layer_t *layer, int x1, int y1, int x2, int y2, bool is_white) {
    lv_draw_line_dsc_t dsc;
    lv_draw_line_dsc_init(&dsc);
    dsc.color = is_white ? lv_color_make(255, 255, 255) : lv_color_make(0, 0, 0);
    dsc.width = 1;
    dsc.p1.x = x1;
    dsc.p1.y = y1;
    dsc.p2.x = x2;
    dsc.p2.y = y2;
    lv_draw_line(layer, &dsc);
  }

  static void draw_pixel(lv_layer_t *layer, int x, int y, bool is_white) {
    if (x < 0 || x >= 128 || y < 0 || y >= 64) return;
    draw_rect(layer, x, y, 1, 1, 0, is_white);
  }
};

class CelestialRenderer {
 public:
  bool meteor_active = false;
  float meteor_x = 0.0f;
  float meteor_y = 0.0f;
  float meteor_vx = -3.5f;
  float meteor_vy = 2.0f;

  void update(uint32_t step, int mode) {
    if (mode == 2) {
      if (!meteor_active && (fast_rand() % 130 == 0)) {
        meteor_active = true;
        bool fall_left = (fast_rand() % 100 > 40);
        meteor_vx = fall_left ? (-(2.0f + (fast_rand() % 25) * 0.1f)) : (2.0f + (fast_rand() % 25) * 0.1f);
        meteor_vy = 1.8f + (fast_rand() % 20) * 0.1f;
        
        meteor_x = fall_left ? (40.0f + (fast_rand() % 75)) : (5.0f + (fast_rand() % 75));
        meteor_y = 1.0f + (fast_rand() % 12);
      }
      if (meteor_active) {
        meteor_x += meteor_vx;
        meteor_y += meteor_vy;
        if (meteor_x < -20.0f || meteor_x > 145.0f || meteor_y > 64.0f) {
          meteor_active = false;
        }
      }
    }
  }

  void render_sun(lv_layer_t *layer, uint32_t step) const {
    int scx = 112, scy = 16;
    for (int i = 0; i < 8; i++) {
      float angle = step * 0.04f + i * (M_PI / 4.0f);
      float r1 = 9.0f + MathUtils::fast_sin(step * 0.15f + i) * 0.8f;
      float r2 = 13.5f + MathUtils::fast_cos(step * 0.2f + i * 1.5f) * 1.5f;
      int x1 = (int)roundf(scx + MathUtils::fast_cos(angle) * r1);
      int y1 = (int)roundf(scy + MathUtils::fast_sin(angle) * r1);
      int x2 = (int)roundf(scx + MathUtils::fast_cos(angle) * r2);
      int y2 = (int)roundf(scy + MathUtils::fast_sin(angle) * r2);
      DrawingUtils::draw_line(layer, x1, y1, x2, y2, true);
    }

    DrawingUtils::draw_circle(layer, scx, scy, 7, true);

    float rot_angle = MathUtils::fast_sin(step * 0.08f) * 0.5236f;
    int rot_deg = (int)(MathUtils::fast_sin(step * 0.08f) * 30.0f);

    float lx_rel = -3.5f * MathUtils::fast_cos(rot_angle) - (-2.5f) * MathUtils::fast_sin(rot_angle);
    float ly_rel = -3.5f * MathUtils::fast_sin(rot_angle) + (-2.5f) * MathUtils::fast_cos(rot_angle);
    int lex = (int)roundf(scx + lx_rel);
    int ley = (int)roundf(scy + ly_rel);

    float rx_rel = 3.5f * MathUtils::fast_cos(rot_angle) - (-2.5f) * MathUtils::fast_sin(rot_angle);
    float ry_rel = 3.5f * MathUtils::fast_sin(rot_angle) + (-2.5f) * MathUtils::fast_cos(rot_angle);
    int rex = (int)roundf(scx + rx_rel);
    int rey = (int)roundf(scy + ry_rel);

    float mx_rel = 0.0f * MathUtils::fast_cos(rot_angle) - 1.0f * MathUtils::fast_sin(rot_angle);
    float my_rel = 0.0f * MathUtils::fast_sin(rot_angle) + 1.0f * MathUtils::fast_cos(rot_angle);
    int mcx = (int)roundf(scx + mx_rel);
    int mcy = (int)roundf(scy + my_rel);

    DrawingUtils::draw_rect(layer, lex - 1, ley - 2, 3, 4, 1, false);
    DrawingUtils::draw_rect(layer, rex - 1, rey - 2, 3, 4, 1, false);
    DrawingUtils::draw_rect(layer, lex, ley - 2, 1, 2, 0, true);
    DrawingUtils::draw_rect(layer, rex, rey - 2, 1, 2, 0, true);

    DrawingUtils::draw_arc(layer, mcx, mcy, 5, 20 + rot_deg, 160 + rot_deg, 2, false);
  }

  void render_moon(lv_layer_t *layer, uint32_t step) const {
    int mcx = 72, mcy = 16;
    DrawingUtils::draw_arc(layer, mcx, mcy, 9, 100, 260, 1, true);
    DrawingUtils::draw_circle(layer, mcx, mcy, 8, true);
    DrawingUtils::draw_circle(layer, mcx + 4, mcy - 4, 7, false);

    DrawingUtils::draw_rect(layer, mcx - 5, mcy - 2, 1, 1, 0, false);
    DrawingUtils::draw_rect(layer, mcx - 3, mcy + 2, 2, 1, 0, false);
    DrawingUtils::draw_rect(layer, mcx - 6, mcy + 2, 1, 1, 0, false);

    for (int i = 0; i < 24; i++) {
      float speed_mult = 0.08f + (NIGHT_STARS[i].freq % 7) * 0.025f;
      float twinkle = MathUtils::fast_sin(step * speed_mult + NIGHT_STARS[i].phase * 1.5f);
      
      if (twinkle > -0.3f) {
        if (NIGHT_STARS[i].big && twinkle > 0.6f) {
          DrawingUtils::draw_rect(layer, NIGHT_STARS[i].x - 1, NIGHT_STARS[i].y, 3, 1, 0, true);
          DrawingUtils::draw_rect(layer, NIGHT_STARS[i].x, NIGHT_STARS[i].y - 1, 1, 3, 0, true);
        } else {
          DrawingUtils::draw_pixel(layer, NIGHT_STARS[i].x, NIGHT_STARS[i].y, true);
        }
      }
    }

    if (meteor_active && meteor_x >= -15.0f && meteor_x <= 140.0f && meteor_y >= 0.0f) {
      int mx = (int)meteor_x, my = (int)meteor_y;
      int tail_len = 12;
      float norm = sqrtf(meteor_vx * meteor_vx + meteor_vy * meteor_vy);
      float dir_x = meteor_vx / (norm > 0.001f ? norm : 1.0f);
      float dir_y = meteor_vy / (norm > 0.001f ? norm : 1.0f);
      
      for (int t = 0; t < tail_len; t++) {
        int tx = mx - (int)roundf(dir_x * t);
        int ty = my - (int)roundf(dir_y * t);
        if (tx >= 0 && tx < 128 && ty >= 0 && ty < 64) {
          if (t < 4 || (t % 2 == 0)) {
            DrawingUtils::draw_pixel(layer, tx, ty, true);
          }
        }
      }
      if (mx >= 0 && mx < 128 && my >= 0 && my < 64) {
        DrawingUtils::draw_pixel(layer, mx, my, true);
      }
    }
  }
};

class CloudRenderer {
 public:
  struct CloudInstance {
    float x;
    float y;
    float speed;
    uint8_t type;  // 0: Large, 1: Medium, 2: Small, 3: Wide
    bool bg;       // Background vs Foreground parallax layer
  };

  CloudInstance clouds[7] = {
    {-25.0f,  2.0f, 0.18f, 2, true},   // BG Small Puff
    { 30.0f,  4.0f, 0.22f, 1, true},   // BG Medium Double-Dome
    { 85.0f,  3.0f, 0.15f, 2, true},   // BG Small Puff
    {-45.0f,  7.0f, 0.42f, 0, false},  // FG Large Cumulus
    { 15.0f, 13.0f, 0.35f, 3, false},  // FG Wide Stratocumulus
    { 65.0f,  9.0f, 0.48f, 1, false},  // FG Medium Double-Dome
    {115.0f,  6.0f, 0.38f, 0, false}   // FG Large Cumulus
  };

  void update(int mode, float continuous_wind) {
    if (mode == 3 || mode == 4 || mode == 7) {
      float wind_mod = continuous_wind * 0.08f;
      static const float WRAP_LEFT[4] = {-42.0f, -32.0f, -24.0f, -48.0f};
      for (int i = 0; i < 7; i++) {
        float speed_scale = clouds[i].bg ? 0.6f : 1.0f;
        float move_step = (clouds[i].speed + wind_mod * speed_scale);
        clouds[i].x += move_step;
        float w_left = WRAP_LEFT[clouds[i].type];
        if (clouds[i].x > 135.0f) clouds[i].x = w_left;
        if (clouds[i].x < w_left) clouds[i].x = 135.0f;
      }
    }
  }

  void draw_cloud_template(lv_layer_t *layer, int cx, int cy, uint8_t type) const {
    switch (type) {
      case 0: {  // Large Volumetric Cumulus (Width ~38, Height 14)
        // Fast solid rectangular fills (radius = 0 eliminates LVGL vector overhead)
        DrawingUtils::draw_rect(layer, cx + 2, cy + 8, 34, 5, 0, true);
        DrawingUtils::draw_rect(layer, cx + 4, cy + 13, 30, 1, 0, true);
        DrawingUtils::draw_rect(layer, cx + 13, cy + 1, 12, 8, 0, true);
        DrawingUtils::draw_rect(layer, cx + 15, cy + 0, 8, 1, 0, true);
        DrawingUtils::draw_rect(layer, cx + 6, cy + 3, 10, 6, 0, true);
        DrawingUtils::draw_rect(layer, cx + 8, cy + 2, 6, 1, 0, true);
        DrawingUtils::draw_rect(layer, cx + 22, cy + 4, 10, 5, 0, true);
        DrawingUtils::draw_rect(layer, cx + 24, cy + 3, 6, 1, 0, true);
        DrawingUtils::draw_rect(layer, cx + 0, cy + 7, 7, 5, 0, true);
        DrawingUtils::draw_rect(layer, cx + 1, cy + 6, 5, 1, 0, true);
        DrawingUtils::draw_rect(layer, cx + 31, cy + 7, 7, 5, 0, true);
        DrawingUtils::draw_rect(layer, cx + 32, cy + 6, 5, 1, 0, true);

        // Internal crevice shadow accents for 3D depth
        DrawingUtils::draw_rect(layer, cx + 13, cy + 4, 1, 4, 0, false);
        DrawingUtils::draw_rect(layer, cx + 22, cy + 5, 1, 4, 0, false);
        break;
      }
      case 1: {  // Medium Double-Dome Cloud (Width ~28, Height 11)
        DrawingUtils::draw_rect(layer, cx + 2, cy + 6, 24, 4, 0, true);
        DrawingUtils::draw_rect(layer, cx + 4, cy + 10, 20, 1, 0, true);
        DrawingUtils::draw_rect(layer, cx + 5, cy + 1, 10, 6, 0, true);
        DrawingUtils::draw_rect(layer, cx + 7, cy + 0, 6, 1, 0, true);
        DrawingUtils::draw_rect(layer, cx + 13, cy + 2, 11, 5, 0, true);
        DrawingUtils::draw_rect(layer, cx + 15, cy + 1, 7, 1, 0, true);
        DrawingUtils::draw_rect(layer, cx + 0, cy + 5, 6, 4, 0, true);
        DrawingUtils::draw_rect(layer, cx + 22, cy + 5, 6, 4, 0, true);

        // Crevice accent
        DrawingUtils::draw_rect(layer, cx + 13, cy + 3, 1, 3, 0, false);
        break;
      }
      case 2: {  // Small Fluffy Puff (Width ~20, Height 9)
        DrawingUtils::draw_rect(layer, cx + 1, cy + 5, 18, 3, 0, true);
        DrawingUtils::draw_rect(layer, cx + 3, cy + 8, 14, 1, 0, true);
        DrawingUtils::draw_rect(layer, cx + 5, cy + 1, 9, 5, 0, true);
        DrawingUtils::draw_rect(layer, cx + 7, cy + 0, 5, 1, 0, true);
        DrawingUtils::draw_rect(layer, cx + 0, cy + 4, 6, 3, 0, true);
        DrawingUtils::draw_rect(layer, cx + 12, cy + 3, 7, 4, 0, true);
        break;
      }
      case 3: {  // Wide Stratocumulus Bank (Width ~44, Height 10)
        DrawingUtils::draw_rect(layer, cx + 2, cy + 5, 40, 4, 0, true);
        DrawingUtils::draw_rect(layer, cx + 5, cy + 9, 34, 1, 0, true);
        DrawingUtils::draw_rect(layer, cx + 4, cy + 2, 9, 4, 0, true);
        DrawingUtils::draw_rect(layer, cx + 6, cy + 1, 5, 1, 0, true);
        DrawingUtils::draw_rect(layer, cx + 14, cy + 1, 11, 5, 0, true);
        DrawingUtils::draw_rect(layer, cx + 16, cy + 0, 7, 1, 0, true);
        DrawingUtils::draw_rect(layer, cx + 26, cy + 2, 10, 4, 0, true);
        DrawingUtils::draw_rect(layer, cx + 28, cy + 1, 6, 1, 0, true);
        DrawingUtils::draw_rect(layer, cx + 35, cy + 4, 8, 3, 0, true);
        DrawingUtils::draw_rect(layer, cx + 0, cy + 4, 6, 3, 0, true);

        // Crevice accents
        DrawingUtils::draw_rect(layer, cx + 13, cy + 3, 1, 3, 0, false);
        DrawingUtils::draw_rect(layer, cx + 25, cy + 3, 1, 3, 0, false);
        break;
      }
    }
  }

  void render(lv_layer_t *layer, int mode = 3) const {
    static const int TEMPLATE_WIDTHS[4] = {38, 28, 20, 44};
    for (int i = 0; i < 7; i++) {
      if (mode == 4 && (i == 1 || i == 4 || i == 6)) continue;

      int cx = (int)clouds[i].x;
      int cy = (int)clouds[i].y;
      int w = TEMPLATE_WIDTHS[clouds[i].type];

      if (cx > -w && cx < 128) {
        draw_cloud_template(layer, cx, cy, clouds[i].type);
      }
    }
  }
};


class FogRenderer {
 public:
  void render(lv_layer_t *layer, uint32_t step, float continuous_wind) const {
    static const int FOG_YS[4] = {38, 43, 48, 52};
    static const int FOG_LENS[4] = {36, 48, 30, 42};
    for (int i = 0; i < 4; i++) {
      float speed = (0.07f + i * 0.025f) + continuous_wind * 0.003f;
      float head_x = fmodf(step * speed + i * 35.0f, 180.0f) - 40.0f;
      if (head_x < -40.0f) head_x += 180.0f;
      int y_wave = (int)roundf(MathUtils::fast_sin(step * 0.04f + i * 1.2f) * 1.2f);
      int fy = FOG_YS[i] + y_wave;
      int fx1 = (int)head_x;
      int fx2 = fx1 + FOG_LENS[i];
      
      int draw_x1 = std::max(0, std::min(127, fx1));
      int draw_x2 = std::max(0, std::min(127, fx2));
      if (draw_x2 > draw_x1) {
        DrawingUtils::draw_line(layer, draw_x1, fy, draw_x2, fy, true);
      }
    }
  }
};

// ─── Simple Wind Ribbon Renderer ─────────────────────────────────────────────
// Ultra-clean, smooth wind ribbons drifting across the screen and waving gently.
class WindRenderer {
 public:
  struct Streamer {
    float x;
    float y_base;
    int length;
    float speed;
  };

  Streamer streamers[3];
  bool initialized = false;

  void init() {
    streamers[0] = {-20.0f, 16.0f, 30, 1.2f};
    streamers[1] = {-60.0f, 32.0f, 35, 1.6f};
    streamers[2] = {-100.0f, 46.0f, 25, 1.0f};
    initialized = true;
  }

  void update(uint32_t step, float continuous_wind) {
    if (!initialized) init();
    for (int i = 0; i < 3; i++) {
      Streamer &s = streamers[i];
      s.x += s.speed + continuous_wind * 0.15f;
      if (s.x > 160.0f) {
        s.x = -40.0f;
        s.y_base = 12.0f + (fast_rand() % 38);
      }
    }
  }

  void render(lv_layer_t *layer, uint32_t step) const {
    for (int i = 0; i < 3; i++) {
      const Streamer &s = streamers[i];
      for (int k = 0; k < s.length; k++) {
        int px = (int)s.x - k;
        if (px >= 0 && px < 128) {
          int py = (int)(s.y_base + MathUtils::fast_sin(px * 0.08f + step * 0.08f + i * 1.8f) * 2.2f);
          if (py >= 0 && py < 64) {
            DrawingUtils::draw_pixel(layer, px, py, true);
          }
        }
      }
    }
  }
};

class PrecipitationRenderer {
 public:
  Particle particles[16];

  PrecipitationRenderer() {
    float init_x[16] = {5.0f, 13.0f, 21.0f, 29.0f, 37.0f, 45.0f, 53.0f, 61.0f,
                       69.0f, 77.0f, 85.0f, 93.0f, 101.0f, 109.0f, 117.0f, 123.0f};
    float init_y[16] = {0.0f, 12.0f, 24.0f, 36.0f, 48.0f, 8.0f, 20.0f, 32.0f,
                       44.0f, 4.0f, 16.0f, 28.0f, 40.0f, 52.0f, 10.0f, 30.0f};
    for (int i = 0; i < 16; i++) {
      particles[i] = {init_x[i], init_y[i], 1, 5};
    }
  }

  void update(uint32_t step, int mode, float continuous_wind, float precip_rate) {
    auto update_p = [&](int idx, float vx, float vy, int w, int h, int reset_type) {
      Particle &p = particles[idx - 1];
      p.x += vx;
      p.y += vy;
      if (reset_type == 1 && p.y > 60) { p.y = 0; p.x = (step * 17 + idx * 11) % 120; }
      else if (reset_type == 2 && p.y > 60) { p.y = 0; p.x = (step * 19 + idx * 13) % 120; }
      else if (reset_type == 4 && p.y < 0) { p.y = 60; p.x = (step * 13 + idx * 17) % 120; }
      p.w = w;
      p.h = h;
    };

    if (mode == 5 || mode == 7) {
      float precip_mult = (precip_rate > 0.0f) ? std::max(0.6f, std::min(2.2f, 0.5f + precip_rate * 0.15f)) : 1.0f;
      float vy_base = ((mode == 7) ? 6.5f : 5.0f) * precip_mult;
      float vx_wind = 0.8f + continuous_wind * 0.35f;
      for (int i = 1; i <= 16; i++) {
        update_p(i, vx_wind, vy_base + (i % 3) * 0.5f, 1, 10, 1);
      }
    } else if (mode == 6) {
      for (int i = 1; i <= 16; i++) {
        float sway = MathUtils::fast_sin(step * 0.08f + i * 0.7f) * 0.8f + MathUtils::fast_cos(step * 0.14f + i * 0.3f) * 0.4f + continuous_wind * 0.3f;
        int sz = (i <= 4) ? 3 : ((i <= 10) ? 2 : 1);
        update_p(i, sway, 0.7f + (i % 4) * 0.18f, sz, sz, 2);
      }
    } else if (mode == 1 || mode == 4) {
      for (int i = 1; i <= 16; i++) {
        float shimmer_x = MathUtils::fast_sin(step * 0.1f + i) * 0.5f - 0.2f + continuous_wind * 0.1f;
        update_p(i, shimmer_x, -0.4f, 1, 1, 4);
      }
    }
  }

  void render(lv_layer_t *layer, int mode, float continuous_wind, float precip_rate = 0.0f) const {
    float precip_mult = (precip_rate > 0.0f) ? std::max(0.6f, std::min(2.2f, 0.5f + precip_rate * 0.15f)) : 1.0f;
    for (int i = 0; i < 16; i++) {
      int px = (int)particles[i].x;
      int py = (int)particles[i].y;

      if (mode == 5 || mode == 7) {
        int dy = (int)roundf(((mode == 7) ? 9 : 7) * precip_mult);
        float vy_base = ((mode == 7) ? 6.5f : 5.0f) * precip_mult;
        float vx_wind = 0.8f + continuous_wind * 0.35f;
        
        int x1 = px;
        int y1 = py;
        float inv_vy = 1.0f / (vy_base > 0.1f ? vy_base : 1.0f);
        int dx_total = (int)roundf(vx_wind * (dy * inv_vy));
        float inv_dy = (dy == 0) ? 0.0f : (1.0f / (float)dy);

        for (int r = 0; r <= dy; r++) {
          int rx = x1 + (int)roundf(r * dx_total * inv_dy);
          int ry = y1 + r;
          if (rx >= 0 && rx < 128 && ry >= 0 && ry < 64) {
            DrawingUtils::draw_pixel(layer, rx, ry, true);
          }
        }

        int x2 = px + dx_total;
        int y2 = py + dy;
        if (y2 >= 56 && x2 >= 0 && x2 < 128) {
          int splash_w = std::max(3, (int)roundf(3.0f * precip_mult));
          DrawingUtils::draw_rect(layer, x2 - 1, 62, splash_w, 1, 0, true);
        }
      } else {
        int cpx = std::max(0, std::min(127, px));
        int cpy = std::max(0, std::min(63, py));
        DrawingUtils::draw_rect(layer, cpx, cpy, particles[i].w, particles[i].h, 0, true);
      }
    }
  }
};

class LightningRenderer {
 public:
  int lightning_flash = 0;
  float lightning_x = 64.0f;

  void update(uint32_t step, int mode) {
    if (mode == 7) {
      if (lightning_flash > 0) {
        lightning_flash--;
      } else if ((step % 280) == 0 || (fast_rand() % 350 == 0)) {
        lightning_flash = 2; // ~60ms flash
        lightning_x = 25.0f + (fast_rand() % 75);
      }
    } else {
      lightning_flash = 0;
    }
  }

  void render(lv_layer_t *layer) const {
    if (lightning_flash > 0) {
      int lx = std::max(10, std::min(115, (int)lightning_x));
      DrawingUtils::draw_line(layer, lx, 12, lx + 4, 24, true);
      DrawingUtils::draw_line(layer, lx + 4, 24, lx - 2, 38, true);
      DrawingUtils::draw_line(layer, lx - 2, 38, lx + 5, 54, true);
      DrawingUtils::draw_line(layer, lx + 4, 24, lx + 10, 34, true);
      DrawingUtils::draw_line(layer, lx - 2, 38, lx - 7, 46, true);
    }
  }
};

class GrassRenderer {
 public:
  void render(lv_layer_t *layer, uint32_t step, int mode, float gust_impulse, float continuous_wind) const {
    if (mode == 6) {
      DrawingUtils::draw_rect(layer, 0, 62, 128, 2, 0, true);
      for (int x = 0; x < 128; x += 3) {
        int drift_h = 1 + (int)(MathUtils::fast_sin(x * 0.12f + step * 0.03f) * 1.2f + 1.2f);
        DrawingUtils::draw_rect(layer, x, 62 - drift_h, 3, drift_h, 0, true);
      }
      return;
    }

    float sway_mag = (mode == 8) ? 6.0f : ((mode == 7) ? 7.5f : ((mode == 5) ? 3.5f : 2.0f));
    float wind_speed = (mode == 8) ? 0.16f : ((mode == 7) ? 0.22f : ((mode == 5) ? 0.10f : 0.06f));
    float spatial_freq = 0.08f;
    float wind_bias = (mode == 8) ? 2.5f : ((mode == 7) ? 3.0f : 0.5f);
    float gust_env = 1.0f + 0.3f * MathUtils::fast_sin(step * 0.03f);

    for (int g = 0; g < 33; g++) {
      int gx = 2 + g * 4 + ((g * 7) % 3);
      if (gx < 0 || gx >= 128) continue;

      int h = 3 + ((g * 11 + 3) % 7);
      
      float wave1 = MathUtils::fast_sin(step * wind_speed - gx * spatial_freq);
      float wave2 = MathUtils::fast_sin(step * wind_speed * 1.6f - gx * spatial_freq * 1.3f + 1.5f) * 0.35f;
      float total_wave = (wave1 + wave2) * gust_env;

      float sway_float = total_wave * sway_mag + wind_bias + gust_impulse + continuous_wind * 0.75f;
      int sway = (int)roundf(sway_float);

      int x_pos[16];
      for (int k = 0; k <= h; k++) {
        float t = (float)k / (float)h;
        x_pos[k] = gx + (int)roundf(sway * t * t);
      }

      int k = 0;
      while (k <= h) {
        int start_x = x_pos[k];
        int start_k = k;
        while (k <= h && x_pos[k] == start_x) {
          k++;
        }
        int run_len = k - start_k;
        int top_y = 63 - (k - 1);
        DrawingUtils::draw_rect(layer, start_x, top_y, 1, run_len, 0, true);
      }

      if ((g % 4 == 1 || g % 7 == 0) && mode != 7) {
        int tip_x = x_pos[h];
        int tip_y = 63 - h;
        DrawingUtils::draw_rect(layer, tip_x - 1, tip_y - 1, 3, 1, 0, true);
        DrawingUtils::draw_rect(layer, tip_x, tip_y - 2, 1, 1, 0, true);
      }
    }
  }
};

class TreeRenderer {
 public:
  void render(lv_layer_t *layer, uint32_t step, float continuous_wind) const {
    float root_x = 32.0f;
    float root_y = 63.0f;

    auto draw_branch = [&](auto &self, float x, float y, float angle, float len, int depth, float parent_flex) -> void {
      if (depth > 3) return;

      float ambient_sway = MathUtils::fast_sin(step * 0.05f + depth * 0.7f + x * 0.05f) * (0.04f + depth * 0.03f);
      float wind_sway = continuous_wind * 0.015f * parent_flex;

      float total_angle = angle + ambient_sway + wind_sway;
      float end_x = x + MathUtils::fast_cos(total_angle) * len;
      float end_y = y + MathUtils::fast_sin(total_angle) * len;

      DrawingUtils::draw_line(layer, (int)roundf(x), (int)roundf(y), (int)roundf(end_x), (int)roundf(end_y), true);
      if (depth == 0) {
        DrawingUtils::draw_line(layer, (int)roundf(x) + 1, (int)roundf(y), (int)roundf(end_x) + 1, (int)roundf(end_y), true);
      }

      if (depth == 3) {
        int lx = (int)roundf(end_x);
        int ly = (int)roundf(end_y);
        DrawingUtils::draw_pixel(layer, lx - 1, ly, true);
        DrawingUtils::draw_pixel(layer, lx + 1, ly, true);
        DrawingUtils::draw_pixel(layer, lx, ly - 1, true);
        DrawingUtils::draw_pixel(layer, lx, ly + 1, true);
        return;
      }

      float spread = 0.48f - depth * 0.06f;
      float child_len = len * 0.74f;
      float next_flex = parent_flex * 1.45f;

      self(self, end_x, end_y, total_angle - spread, child_len, depth + 1, next_flex);
      self(self, end_x, end_y, total_angle + spread, child_len * 0.9f, depth + 1, next_flex);
    };

    draw_branch(draw_branch, root_x, root_y, -1.5707963f, 13.0f, 0, 1.0f);
  }
};

class Engine {
 private:
  float gust_impulse = 0.0f;
  float continuous_wind = 0.0f;
  float target_wind = 0.0f;
  float ha_wind_speed = 0.0f;
  float ha_precipitation_rate = 0.0f;

  CelestialRenderer celestial;
  CloudRenderer clouds;
  FogRenderer fog;
  WindRenderer wind;
  PrecipitationRenderer precipitation;
  LightningRenderer lightning;
  GrassRenderer grass;
  TreeRenderer tree;

 public:
  void set_ha_wind_speed(float speed) { ha_wind_speed = std::max(0.0f, speed); }
  void set_ha_precipitation_rate(float rate) { ha_precipitation_rate = std::max(0.0f, rate); }

  void trigger_gust(float impulse) {
    gust_impulse += impulse;
    if (gust_impulse > 14.0f) gust_impulse = 14.0f;
    if (gust_impulse < -14.0f) gust_impulse = -14.0f;
  }

  void adjust_manual_wind(float delta) {
    target_wind += delta * 2.5f;
    if (target_wind > 20.0f) target_wind = 20.0f;
    if (target_wind < -20.0f) target_wind = -20.0f;
  }

  void reset_manual_wind() {
    target_wind = 0.0f;
    continuous_wind = 0.0f;
  }

  float get_target_wind() const { return target_wind; }

  void update(uint32_t step, int mode) {
    gust_impulse *= 0.91f;

    // Smooth inertia delay: continuous_wind follows target_wind
    continuous_wind += (target_wind - continuous_wind) * 0.14f;
    // Decay physics: target_wind slowly decays back toward zero
    target_wind *= 0.965f;
    if (std::abs(target_wind) < 0.01f) target_wind = 0.0f;

    float ha_wind_offset = ha_wind_speed * 0.12f;
    float effective_wind = continuous_wind + ha_wind_offset;

    celestial.update(step, mode);
    clouds.update(mode, effective_wind);
    if (mode != 8) precipitation.update(step, mode, effective_wind, ha_precipitation_rate);
    if (mode == 8) wind.update(step, effective_wind);
    lightning.update(step, mode);
  }

  void render(lv_obj_t *canvas_obj, uint32_t step, int mode) {
    if (!canvas_obj) return;

    lv_layer_t layer;
    lv_canvas_init_layer(canvas_obj, &layer);

    if (mode == 7 && lightning.lightning_flash == 2) {
      for (int y = 0; y < 64; y += 2) {
        DrawingUtils::draw_line(&layer, 0, y, 127, y, true);
      }
    } else {
      lv_canvas_fill_bg(canvas_obj, lv_color_make(0, 0, 0), LV_OPA_TRANSP);
    }

    float ha_wind_offset = ha_wind_speed * 0.12f;
    float effective_wind = continuous_wind + ha_wind_offset;

    if (mode == 1 || mode == 4) celestial.render_sun(&layer, step);
    if (mode == 2) celestial.render_moon(&layer, step);
    if (mode == 1 || mode == 4) tree.render(&layer, step, effective_wind);
    if (mode == 3 || mode == 4 || mode == 7) clouds.render(&layer, mode);
    if (mode == 8) fog.render(&layer, step, effective_wind);
    if (mode == 8) wind.render(&layer, step);
    if (mode == 7 && lightning.lightning_flash > 0) lightning.render(&layer);
    if ((mode >= 5 && mode != 8) || mode == 1 || mode == 4) precipitation.render(&layer, mode, effective_wind, ha_precipitation_rate);
    grass.render(&layer, step, mode, gust_impulse, effective_wind);

    lv_canvas_finish_layer(canvas_obj, &layer);
  }
};

inline Engine global_weather_engine;

inline const char* get_mode_name(int mode) {
  switch (mode) {
    case 1: return "Sunny";
    case 2: return "Clear Night";
    case 3: return "Cloudy";
    case 4: return "Partly Cloudy";
    case 5: return "Torrential Rain";
    case 6: return "Snowy Blizzard";
    case 7: return "Thunderstorm";
    case 8: return "Windy / Fog";
    default: return "Unknown";
  }
}

inline int get_mode_from_condition(const std::string &cond, const std::string &sun_state) {
  if (cond == "sunny") {
    return (sun_state == "below_horizon") ? 2 : 1;
  }
  if (cond == "clear-night") return 2;
  if (cond == "cloudy") return 3;
  if (cond == "partlycloudy") return 4;
  if (cond == "rainy" || cond == "pouring") return 5;
  if (cond == "snowy" || cond == "snowy-rainy") return 6;
  if (cond == "lightning" || cond == "lightning-rainy") return 7;
  if (cond == "windy" || cond == "fog") return 8;
  return 1;
}

inline std::string format_temp_with_trend(float current, float &last_val) {
  std::string trend = "=";
  if (last_val != 0.0f) {
    float diff = current - last_val;
    if (diff > 0.05f) trend = "^";
    else if (diff < -0.05f) trend = "v";
  }
  last_val = current;
  return str_sprintf("%.1f°C %s", current, trend.c_str());
}

}  // namespace weather
