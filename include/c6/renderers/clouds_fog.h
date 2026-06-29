#pragma once
#include "../core/drawing.h"

namespace weather {

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

}  // namespace weather
