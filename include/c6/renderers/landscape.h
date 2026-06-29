#pragma once
#include "../core/drawing.h"
#include "celestial.h"

namespace weather {

class GrassRenderer {
 public:
  void render(lv_layer_t *layer, uint32_t step, int mode, float gust_impulse, float continuous_wind) const {
    if (mode == 6 || mode == 9) {
      if (mode == 9) return; // Ocean scene has its own dynamic water wave strips
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

      bool grass_white = (mode != 8);

      int k = 0;
      while (k <= h) {
        int start_x = x_pos[k];
        int start_k = k;
        while (k <= h && x_pos[k] == start_x) {
          k++;
        }
        int run_len = k - start_k;
        int top_y = 63 - (k - 1);
        DrawingUtils::draw_rect(layer, start_x, top_y, 1, run_len, 0, grass_white);
      }

      if ((g % 4 == 1 || g % 7 == 0) && mode != 7) {
        int tip_x = x_pos[h];
        int tip_y = 63 - h;
        DrawingUtils::draw_rect(layer, tip_x - 1, tip_y - 1, 3, 1, 0, grass_white);
        DrawingUtils::draw_rect(layer, tip_x, tip_y - 2, 1, 1, 0, grass_white);
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

class LeafRenderer {
 public:
  struct Leaf {
    float x;
    float y;
    float vx;
    float vy;
    float rot_phase;
  };

  Leaf leaves[6];
  bool initialized = false;

  void init() {
    for (int i = 0; i < 6; i++) {
      leaves[i].x = 32.0f + (fast_rand() % 12);
      leaves[i].y = 40.0f + (fast_rand() % 15);
      leaves[i].vx = 0.8f + (fast_rand() % 10) * 0.1f;
      leaves[i].vy = 0.2f + (fast_rand() % 8) * 0.05f;
      leaves[i].rot_phase = (fast_rand() % 100) * 0.0628f;
    }
    initialized = true;
  }

  void update(uint32_t step, int mode, float continuous_wind) {
    if (!initialized) init();

    for (int i = 0; i < 6; i++) {
      Leaf &l = leaves[i];
      float wind_push = std::max(0.2f, continuous_wind * 0.15f);
      if (mode == 7 || mode == 8) wind_push += 0.8f;

      l.x += l.vx + wind_push;
      l.y += l.vy + MathUtils::fast_sin(step * 0.1f + l.rot_phase) * 0.4f;

      if (l.x > 140.0f || l.y > 62.0f) {
        l.x = 30.0f + (fast_rand() % 8);
        l.y = 38.0f + (fast_rand() % 14);
      }
    }
  }

  void render(lv_layer_t *layer, uint32_t step) const {
    for (int i = 0; i < 6; i++) {
      int lx = (int)roundf(leaves[i].x);
      int ly = (int)roundf(leaves[i].y);
      if (lx >= 0 && lx < 128 && ly >= 0 && ly < 64) {
        int tumble = (int)(MathUtils::fast_sin(step * 0.2f + leaves[i].rot_phase) * 1.5f);
        DrawingUtils::draw_pixel(layer, lx, ly, true);
        DrawingUtils::draw_pixel(layer, lx + (tumble > 0 ? 1 : -1), ly + 1, true);
      }
    }
  }
};

class OceanRenderer {
 public:
  float scroll_fg = 0.0f;
  float scroll_bg = 0.0f;
  CelestialRenderer sun;

  struct Dolphin {
    float x;
    float arc_t;
    bool active;
  };
  mutable Dolphin dolphin = {-20.0f, 0.0f, false};

  struct SprayParticle {
    float x, y;
    float vx, vy;
    bool active;
  };
  mutable SprayParticle sprays[8] = {};

  void update(uint32_t step, float continuous_wind) {
    float speed = 0.35f + continuous_wind * 0.1f;
    scroll_fg += speed;
    if (scroll_fg > 360.0f) scroll_fg -= 360.0f;
    scroll_bg += speed * 0.6f;
    if (scroll_bg > 360.0f) scroll_bg -= 360.0f;

    // Leaping dolphin animation logic
    if (!dolphin.active && (fast_rand() % 160 == 0)) {
      dolphin.active = true;
      dolphin.x = 10.0f + (fast_rand() % 90);
      dolphin.arc_t = 0.0f;
    }
    if (dolphin.active) {
      dolphin.arc_t += 0.08f;
      if (dolphin.arc_t > 3.14159f) {
        dolphin.active = false;
      }
    }

    // Airborne wave sea spray physics
    for (int i = 0; i < 8; i++) {
      if (!sprays[i].active) {
        if (fast_rand() % 24 == 0) {
          sprays[i].active = true;
          sprays[i].x = (float)(fast_rand() % 128);
          float raw_h = WaveGenerator::calculate_wave_height((int)sprays[i].x, scroll_fg, step, 1.0f, 1.2f);
          sprays[i].y = 56.0f - raw_h * 1.4f;
          sprays[i].vx = 0.4f + (fast_rand() % 100) * 0.006f;
          sprays[i].vy = -0.8f - (fast_rand() % 100) * 0.008f;
        }
      } else {
        sprays[i].x += sprays[i].vx;
        sprays[i].y += sprays[i].vy;
        sprays[i].vy += 0.12f; // Gravity pulling spray back to water
        if (sprays[i].y > 58.0f || sprays[i].x > 130.0f) {
          sprays[i].active = false;
        }
      }
    }
  }

  void render(lv_layer_t *layer, uint32_t step) const {
    // 1. Render reusable pretty animated sun face centered over the ocean horizon
    sun.render_sun(layer, step, 64, 22);

    // 2. Render background ocean wave strip (OUTLINE_ONLY at y=48)
    DrawingUtils::render_wave_strip(layer, step, scroll_bg, 48, 1.1f, false, WaveGenerator::OUTLINE_ONLY, 1.2f, 0.9f);

    // 3. Render foreground ocean wave strip (OUTLINE_FILL solid water body at y=56)
    DrawingUtils::render_wave_strip(layer, step, scroll_fg, 56, 1.4f, false, WaveGenerator::OUTLINE_FILL, 1.0f, 1.2f);

    // 4. Wave foam crest streaks along water surface
    for (int x = 0; x < 128; x += 4) {
      float raw_w = WaveGenerator::calculate_wave_height(x, scroll_fg, step, 1.0f, 1.2f);
      int wave_y = 56 - (int)roundf(raw_w * 1.4f);
      if ((x + (int)step) % 7 < 3 && wave_y < 63) {
        DrawingUtils::draw_pixel(layer, x, wave_y + 1, true); // Wave foam flecks
        if (x % 8 == 0 && wave_y < 62) DrawingUtils::draw_pixel(layer, x + 1, wave_y + 2, true);
      }
    }

    // 5. Render airborne wave sea spray particles
    for (int i = 0; i < 8; i++) {
      if (sprays[i].active) {
        int sx = (int)roundf(sprays[i].x);
        int sy = (int)roundf(sprays[i].y);
        if (sx >= 0 && sx < 128 && sy >= 0 && sy < 64) {
          DrawingUtils::draw_pixel(layer, sx, sy, true);
        }
      }
    }

    // 6. Water shimmer specular highlights reflection beneath sun
    int scx = 64;
    for (int y = 50; y < 64; y += 3) {
      int shimmer_w = 16 - (y - 50);
      if (shimmer_w > 2 && (step + y) % 3 != 0) {
        int sx = scx + (int)(MathUtils::fast_sin(step * 0.1f + y) * 4.0f);
        DrawingUtils::draw_line(layer, sx - shimmer_w / 2, y, sx + shimmer_w / 2, y, true);
      }
    }

    // 7. Render leaping dolphin vector arc
    if (dolphin.active) {
      float dx = dolphin.x + dolphin.arc_t * 12.0f;
      float dy = 54.0f - sinf(dolphin.arc_t) * 14.0f;
      int idx = (int)roundf(dx);
      int idy = (int)roundf(dy);
      if (idx >= 0 && idx < 128 && idy >= 0 && idy < 64) {
        DrawingUtils::draw_pixel(layer, idx, idy, true);
        DrawingUtils::draw_pixel(layer, idx - 1, idy + 1, true);
        DrawingUtils::draw_pixel(layer, idx + 1, idy + 1, true);
        DrawingUtils::draw_pixel(layer, idx - 2, idy + 2, true);
      }
    }
  }
};

}  // namespace weather
