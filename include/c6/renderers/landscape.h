#pragma once
#include "../core/drawing.h"

namespace weather {

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

}  // namespace weather
