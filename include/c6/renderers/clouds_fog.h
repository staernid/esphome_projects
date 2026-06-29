#pragma once
#include "../core/drawing.h"

namespace weather {

class CloudRenderer {
 public:
  struct LowerCloud {
    float x;
    float y;
    float speed;
    int width;
  };

  float scroll_fg = 0.0f;
  float scroll_bg = 0.0f;
  mutable LowerCloud lower_clouds[3] = {
    {-30.0f, 15.0f, 0.30f, 16},
    { 45.0f, 19.0f, 0.35f, 12},
    {110.0f, 16.0f, 0.25f, 18}
  };

  void update(int mode, float continuous_wind) {
    if (mode >= 3 && mode <= 7) {
      float wind_mod = continuous_wind * 0.085f;
      float speed_fg = 0.21f + wind_mod;
      float speed_bg = 0.12f + wind_mod * 0.5f; // Slower background parallax layer speed

      scroll_fg += speed_fg;
      if (scroll_fg > 360.0f) scroll_fg -= 360.0f;

      scroll_bg += speed_bg;
      if (scroll_bg > 360.0f) scroll_bg -= 360.0f;

      for (int c = 0; c < 3; c++) {
        lower_clouds[c].x += lower_clouds[c].speed + wind_mod * 0.7f;
        if (lower_clouds[c].x > 145.0f) {
          lower_clouds[c].x = -35.0f;
          lower_clouds[c].y = 14.0f + (fast_rand() % 8);
        }
      }
    }
  }

  void render(lv_layer_t *layer, uint32_t step, int mode = 3) const {
    if (mode < 3 || mode > 7) return;

    int base_depth = (mode == 4) ? 4 : 7;
    float mode_mult = (mode == 7 || mode == 5) ? 1.3f : 1.0f;

    if (mode == 7) {
      // Background wave layer rendered first (lower down, slower scroll_bg parallax)
      DrawingUtils::render_wave_strip(layer, step, scroll_bg + 40.0f, 9, mode_mult * 1.1f, true, WaveGenerator::OUTLINE_ONLY, 1.1f, 1.0f);
      // Foreground wave layer rendered second (higher up, faster scroll_fg parallax, solid occluding fill)
      DrawingUtils::render_wave_strip(layer, step, scroll_fg, 2, mode_mult * 0.8f, true, WaveGenerator::OUTLINE_FILL);
    } else {
      DrawingUtils::render_wave_strip(layer, step, scroll_fg, base_depth, mode_mult, true, WaveGenerator::SOLID_FILL);
    }

    // 2. Render occasional small lower cloud puffs drifting underneath the carpet
    for (int c = 0; c < 3; c++) {
      if (mode == 4 && c == 1) continue; // Slightly fewer lower clouds in partly cloudy mode
      int lx = (int)roundf(lower_clouds[c].x);
      int ly = (int)roundf(lower_clouds[c].y);
      int w = lower_clouds[c].width;
      if (lx > -w && lx < 128) {
        DrawingUtils::draw_rect(layer, lx + 2, ly, w - 4, 4, 1, true);
        DrawingUtils::draw_rect(layer, lx, ly + 2, w, 2, 0, true);
        DrawingUtils::draw_rect(layer, lx + 4, ly - 1, w - 8, 2, 0, true);
      }
    }
  }
};

class FogRenderer {
 public:
  mutable float scroll_fog = 0.0f;

  void update(uint32_t step, float continuous_wind) {
    scroll_fog += 0.25f + continuous_wind * 0.08f;
    if (scroll_fog > 360.0f) scroll_fog -= 360.0f;
  }

  void render(lv_layer_t *layer, uint32_t step, float continuous_wind) const {
    // Solid opaque white vector wave mist rolling along ground horizon (is_ceiling = false, SOLID_FILL)
    DrawingUtils::render_wave_strip(layer, step, scroll_fog, 44, 1.2f, false, WaveGenerator::SOLID_FILL, 1.1f, 1.0f);
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
    streamers[0] = {-20.0f, 10.0f, 30, 1.2f};
    streamers[1] = {-60.0f, 22.0f, 35, 1.6f};
    streamers[2] = {-100.0f, 34.0f, 25, 1.0f};
    initialized = true;
  }

  void update(uint32_t step, float continuous_wind) {
    if (!initialized) init();
    for (int i = 0; i < 3; i++) {
      Streamer &s = streamers[i];
      s.x += s.speed + continuous_wind * 0.15f;
      if (s.x > 160.0f) {
        s.x = -40.0f;
        s.y_base = 8.0f + (fast_rand() % 28);
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
