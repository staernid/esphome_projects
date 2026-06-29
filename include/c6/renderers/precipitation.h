#pragma once
#include "../core/drawing.h"

namespace weather {

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
        if (y2 >= 58 && x2 >= 0 && x2 < 128) {
          int splash_w = std::max(3, (int)roundf(3.0f * precip_mult));
          DrawingUtils::draw_rect(layer, x2 - 1, 62, splash_w, 1, 0, true);
          // Dynamic upward splash droplet arcs
          DrawingUtils::draw_pixel(layer, x2 - 2, 60, true);
          DrawingUtils::draw_pixel(layer, x2 + splash_w + 1, 60, true);
          DrawingUtils::draw_pixel(layer, x2 + (splash_w / 2), 59, true);
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
  int strike_seed = 0;

  void trigger_lightning() {
    lightning_flash = 12; // ~380ms stylized multi-phase strike
    lightning_x = 25.0f + (fast_rand() % 75);
    strike_seed = fast_rand();
  }

  void update(uint32_t step, int mode) {
    if (mode == 7) {
      if (lightning_flash > 0) {
        lightning_flash--;
      } else if ((step % 110) == 0 || (fast_rand() % 130 == 0)) {
        trigger_lightning();
      }
    } else {
      lightning_flash = 0;
    }
  }

  void render(lv_layer_t *layer) const {
    if (lightning_flash > 0) {
      int lx = std::max(15, std::min(110, (int)lightning_x));
      
      // Dynamic jitter during re-strike flickering phases (phase 8..5)
      int jitter = (lightning_flash >= 5 && lightning_flash <= 8 && (lightning_flash % 2 == 0)) ? 2 : 0;

      int x1 = lx + jitter, y1 = 6;
      int x2 = lx + ((strike_seed % 11) - 5) - jitter, y2 = 22;
      int x3 = x2 + (((strike_seed / 7) % 15) - 7) + jitter, y3 = 38;
      int x4 = x3 + (((strike_seed / 13) % 11) - 5), y4 = 58;

      // Render main bolt trunk
      DrawingUtils::draw_line(layer, x1, y1, x2, y2, true);
      DrawingUtils::draw_line(layer, x2, y2, x3, y3, true);
      DrawingUtils::draw_line(layer, x3, y3, x4, y4, true);

      // Bold core during initial heavy impact (phase 12..7)
      if (lightning_flash >= 7) {
        DrawingUtils::draw_line(layer, x1 + 1, y1, x2 + 1, y2, true);
        DrawingUtils::draw_line(layer, x2 + 1, y2, x3 + 1, y3, true);
      }

      // Side-fork branches (flickers in secondary phase)
      if (lightning_flash >= 4 && (lightning_flash % 2 != 0 || lightning_flash >= 8)) {
        DrawingUtils::draw_line(layer, x2, y2, x2 + 8, y2 + 10, true);
        DrawingUtils::draw_line(layer, x2 + 8, y2 + 10, x2 + 12, y2 + 14, true);
        DrawingUtils::draw_line(layer, x3, y3, x3 - 9, y3 + 12, true);
      }

      // Ground impact sparks exploding outwards
      DrawingUtils::draw_rect(layer, x4 - 3, 61, 7, 1, 0, true);
      DrawingUtils::draw_pixel(layer, x4 - 4, 60, true);
      DrawingUtils::draw_pixel(layer, x4 + 4, 60, true);
      if (lightning_flash >= 6) {
        DrawingUtils::draw_pixel(layer, x4 - 2, 59, true);
        DrawingUtils::draw_pixel(layer, x4 + 2, 59, true);
      }
    }
  }

  bool is_lightning_flashing() const {
    return lightning_flash >= 8;
  }
};

struct HailParticle {
  float x, y;
  float vx, vy;
  int size;
  bool bouncing;
};

class HailSleetRenderer {
 public:
  HailParticle particles[12];

  HailSleetRenderer() {
    for (int i = 0; i < 12; i++) {
      particles[i] = { (float)(i * 10 + 5), (float)(i * 4), 0.5f, 6.0f + (i % 3) * 1.5f, 2, false };
    }
  }

  void update(uint32_t step, float continuous_wind) {
    for (int i = 0; i < 12; i++) {
      HailParticle &p = particles[i];
      p.x += p.vx + continuous_wind * 0.2f;
      p.y += p.vy;

      if (p.y >= 60.0f) {
        if (!p.bouncing) {
          p.y = 60.0f;
          p.vy = -p.vy * 0.45f;
          p.vx += (fast_rand() % 10 - 5) * 0.2f;
          p.bouncing = true;
        } else {
          p.y = (float)(fast_rand() % 10);
          p.x = (float)(fast_rand() % 120);
          p.vy = 6.0f + (i % 3) * 1.5f;
          p.vx = 0.5f;
          p.bouncing = false;
        }
      } else if (p.bouncing) {
        p.vy += 0.8f;
      }
    }
  }

  void render(lv_layer_t *layer) const {
    for (int i = 0; i < 12; i++) {
      int px = (int)particles[i].x;
      int py = (int)particles[i].y;
      if (px >= 0 && px < 127 && py >= 0 && py < 63) {
        DrawingUtils::draw_rect(layer, px, py, particles[i].size, particles[i].size, 0, true);
      }
    }
  }
};

}  // namespace weather

