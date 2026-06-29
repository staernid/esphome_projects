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
    draw_arc(layer, cx, cy, radius, 0, 360, radius, is_white);
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
};

class CelestialRenderer {
 public:
  bool meteor_active = false;
  float meteor_x = 0.0f;
  float meteor_y = 0.0f;

  void update(uint32_t step, int mode) {
    if (mode == 2) {
      if (!meteor_active && (step % 90 == 0 || rand() % 110 == 0)) {
        meteor_active = true;
        meteor_x = 124.0f;
        meteor_y = 2.0f;
      }
      if (meteor_active) {
        meteor_x -= 5.5f;
        meteor_y += 3.0f;
        if (meteor_x < -10.0f || meteor_y > 64.0f) {
          meteor_active = false;
        }
      }
    }
  }

  void render_sun(lv_layer_t *layer, uint32_t step) const {
    int scx = 112, scy = 16;
    for (int i = 0; i < 8; i++) {
      float angle = step * 0.04f + i * (M_PI / 4.0f);
      float r1 = 9.0f + sinf(step * 0.15f + i) * 0.8f;
      float r2 = 13.5f + cosf(step * 0.2f + i * 1.5f) * 1.5f;
      int x1 = (int)roundf(scx + cosf(angle) * r1);
      int y1 = (int)roundf(scy + sinf(angle) * r1);
      int x2 = (int)roundf(scx + cosf(angle) * r2);
      int y2 = (int)roundf(scy + sinf(angle) * r2);
      DrawingUtils::draw_line(layer, x1, y1, x2, y2, true);
    }

    DrawingUtils::draw_circle(layer, scx, scy, 7, true);

    float rot_angle = sinf(step * 0.08f) * 0.5236f;
    int rot_deg = (int)(sinf(step * 0.08f) * 30.0f);

    float lx_rel = -3.5f * cosf(rot_angle) - (-2.5f) * sinf(rot_angle);
    float ly_rel = -3.5f * sinf(rot_angle) + (-2.5f) * cosf(rot_angle);
    int lex = (int)roundf(scx + lx_rel);
    int ley = (int)roundf(scy + ly_rel);

    float rx_rel = 3.5f * cosf(rot_angle) - (-2.5f) * sinf(rot_angle);
    float ry_rel = 3.5f * sinf(rot_angle) + (-2.5f) * cosf(rot_angle);
    int rex = (int)roundf(scx + rx_rel);
    int rey = (int)roundf(scy + ry_rel);

    float mx_rel = 0.0f * cosf(rot_angle) - 1.0f * sinf(rot_angle);
    float my_rel = 0.0f * sinf(rot_angle) + 1.0f * cosf(rot_angle);
    int mcx = (int)roundf(scx + mx_rel);
    int mcy = (int)roundf(scy + my_rel);

    DrawingUtils::draw_rect(layer, lex - 1, ley - 2, 3, 4, 1, false);
    DrawingUtils::draw_rect(layer, rex - 1, rey - 2, 3, 4, 1, false);
    DrawingUtils::draw_rect(layer, lex, ley - 2, 1, 2, 0, true);
    DrawingUtils::draw_rect(layer, rex, rey - 2, 1, 2, 0, true);

    DrawingUtils::draw_arc(layer, mcx, mcy, 5, 20 + rot_deg, 160 + rot_deg, 2, false);
  }

  void render_moon(lv_layer_t *layer, uint32_t step) const {
    // Positioned slightly off-center to the right (x = 72) between temp labels and clock
    int mcx = 72, mcy = 16;
    DrawingUtils::draw_arc(layer, mcx, mcy, 9, 100, 260, 1, true);
    DrawingUtils::draw_circle(layer, mcx, mcy, 8, true);
    DrawingUtils::draw_circle(layer, mcx + 4, mcy - 4, 7, false); // Crescent cut

    // Subtle crater marks
    DrawingUtils::draw_rect(layer, mcx - 5, mcy - 2, 1, 1, 0, false);
    DrawingUtils::draw_rect(layer, mcx - 3, mcy + 2, 2, 1, 0, false);
    DrawingUtils::draw_rect(layer, mcx - 6, mcy + 2, 1, 1, 0, false);

    for (int i = 0; i < 24; i++) {
      float twinkle = sinf(step * (6.28318f / NIGHT_STARS[i].freq) + NIGHT_STARS[i].phase);
      if (twinkle > -0.3f) {
        if (NIGHT_STARS[i].big && twinkle > 0.4f) {
          DrawingUtils::draw_rect(layer, NIGHT_STARS[i].x - 1, NIGHT_STARS[i].y, 3, 1, 0, true);
          DrawingUtils::draw_rect(layer, NIGHT_STARS[i].x, NIGHT_STARS[i].y - 1, 1, 3, 0, true);
        } else {
          DrawingUtils::draw_rect(layer, NIGHT_STARS[i].x, NIGHT_STARS[i].y, 1, 1, 0, true);
        }
      }
    }

    if (meteor_active && meteor_x >= -10.0f && meteor_y >= 0.0f) {
      int mx = (int)meteor_x, my = (int)meteor_y;
      DrawingUtils::draw_line(layer, mx, my, mx + 8, my - 5, true);
      DrawingUtils::draw_rect(layer, mx, my, 2, 2, 0, true);
    }
  }
};

class CloudRenderer {
 public:
  float cloud_x = -35.0f;
  float cloud2_x = -75.0f;

  void update(int mode) {
    if (mode == 3 || mode == 4 || mode == 7) {
      cloud_x += 0.5f;
      if (cloud_x > 130.0f) cloud_x = -35.0f;
      cloud2_x += 0.25f;
      if (cloud2_x > 130.0f) cloud2_x = -45.0f;
    }
  }

  void render(lv_layer_t *layer) const {
    if (cloud2_x > -30.0f && cloud2_x < 130.0f) {
      int bx = (int)cloud2_x, by = 2;
      DrawingUtils::draw_rect(layer, bx, by + 4, 22, 5, 2, true);
      DrawingUtils::draw_circle(layer, bx + 5, by + 4, 4, true);
      DrawingUtils::draw_circle(layer, bx + 11, by + 3, 5, true);
      DrawingUtils::draw_circle(layer, bx + 17, by + 4, 4, true);
    }

    if (cloud_x > -35.0f && cloud_x < 130.0f) {
      int cx = (int)cloud_x, cy = 6;
      DrawingUtils::draw_rect(layer, cx, cy + 6, 32, 7, 3, true);
      DrawingUtils::draw_circle(layer, cx + 6, cy + 6, 6, true);
      DrawingUtils::draw_circle(layer, cx + 14, cy + 4, 8, true);
      DrawingUtils::draw_circle(layer, cx + 22, cy + 5, 7, true);
      DrawingUtils::draw_circle(layer, cx + 27, cy + 7, 5, true);
    }
  }
};

class FogRenderer {
 public:
  void render(lv_layer_t *layer, uint32_t step) const {
    // Clean horizontal vector fog strands flowing across lower screen
    static const int FOG_YS[4] = {38, 43, 48, 52};
    static const int FOG_LENS[4] = {36, 48, 30, 42};
    for (int i = 0; i < 4; i++) {
      float speed = 1.2f + i * 0.4f;
      float head_x = fmodf(step * speed + i * 35.0f, 180.0f) - 40.0f;
      int y_wave = (int)roundf(sinf(step * 0.06f + i * 1.2f) * 1.5f);
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

  void update(uint32_t step, int mode) {
    auto update_p = [&](int idx, float vx, float vy, int w, int h, int reset_type) {
      Particle &p = particles[idx - 1];
      p.x += vx;
      p.y += vy;
      if (reset_type == 1 && p.y > 60) { p.y = 0; p.x = (step * 17 + idx * 11) % 120; }
      else if (reset_type == 2 && p.y > 60) { p.y = 0; p.x = (step * 19 + idx * 13) % 120; }
      else if (reset_type == 3 && p.x > 124) { p.x = -15; p.y = 6 + (idx * 4) % 48; }
      else if (reset_type == 4 && p.y < 0) { p.y = 60; p.x = (step * 13 + idx * 17) % 120; }
      p.w = w;
      p.h = h;
    };

    if (mode == 5 || mode == 7) {
      float vy_base = (mode == 7) ? 6.5f : 5.0f;
      for (int i = 1; i <= 16; i++) {
        update_p(i, 0.8f, vy_base + (i % 3) * 0.5f, 1, 10, 1);
      }
    } else if (mode == 6) {
      for (int i = 1; i <= 16; i++) {
        float sway = sinf(step * 0.08f + i * 0.7f) * 0.8f + cosf(step * 0.14f + i * 0.3f) * 0.4f;
        int sz = (i <= 4) ? 3 : ((i <= 10) ? 2 : 1);
        update_p(i, sway, 0.7f + (i % 4) * 0.18f, sz, sz, 2);
      }
    } else if (mode == 8) {
      for (int i = 1; i <= 16; i++) {
        if (i <= 8) {
          float gust_y = sinf(step * 0.15f + i) * 0.5f;
          update_p(i, 5.5f + (i % 4) * 0.6f, gust_y, 18 + (i % 3) * 8, 1, 3);
        } else {
          float swirl = sinf(step * 0.22f + i) * 2.2f;
          update_p(i, 3.8f, swirl, 2, 2, 3);
        }
      }
    } else if (mode == 1 || mode == 4) {
      for (int i = 1; i <= 16; i++) {
        float shimmer_x = sinf(step * 0.1f + i) * 0.5f - 0.2f;
        update_p(i, shimmer_x, -0.4f, 1, 1, 4);
      }
    }
  }

  void render(lv_layer_t *layer, uint32_t step, int mode) const {
    for (int i = 0; i < 16; i++) {
      int px = std::max(0, std::min(127, (int)particles[i].x));
      int py = std::max(0, std::min(63, (int)particles[i].y));

      if (mode == 5 || mode == 7) {
        int dy = (mode == 7) ? 9 : 8;
        int num_steps = (mode == 7) ? 3 : 2;
        int row = 0;
        while (row < dy) {
          int step_offset = (row * num_steps) / dy;
          int start_x = px + step_offset;
          int start_row = row;
          while (row < dy && ((row * num_steps) / dy) == step_offset) {
            row++;
          }
          int run_len = row - start_row;
          int start_y = py + start_row;
          DrawingUtils::draw_rect(layer, start_x, start_y, 1, run_len, 0, true);
        }
        if (py + dy >= 56) {
          int splash_x = px + num_steps;
          DrawingUtils::draw_rect(layer, splash_x - 1, 62, 3, 1, 0, true);
        }
      } else {
        DrawingUtils::draw_rect(layer, px, py, particles[i].w, particles[i].h, 0, true);
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
      } else if ((step % 280) == 0 || (rand() % 350 == 0)) {
        lightning_flash = 2; // ~60ms flash
        lightning_x = 25.0f + (rand() % 75);
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
  void render(lv_layer_t *layer, uint32_t step, int mode, float gust_impulse) const {
    // In Snowy mode (Mode 6), draw solid thicker snow bank baseline across bottom
    if (mode == 6) {
      // Full horizontal 2-px thick baseline for snow bank
      DrawingUtils::draw_rect(layer, 0, 62, 128, 2, 0, true);
      for (int x = 0; x < 128; x += 3) {
        int drift_h = 1 + (int)(sinf(x * 0.12f + step * 0.03f) * 1.2f + 1.2f);
        DrawingUtils::draw_rect(layer, x, 62 - drift_h, 3, drift_h, 0, true);
      }
      return;
    }

    // Grass height strictly 3 to 9 px per user request
    float sway_mag = (mode == 8) ? 6.0f : ((mode == 7) ? 7.5f : ((mode == 5) ? 3.5f : 2.0f));
    float wind_speed = (mode == 8) ? 0.16f : ((mode == 7) ? 0.22f : ((mode == 5) ? 0.10f : 0.06f));
    float spatial_freq = 0.08f;
    float wind_bias = (mode == 8) ? 2.5f : ((mode == 7) ? 3.0f : 0.5f);
    float gust_env = 1.0f + 0.3f * sinf(step * 0.03f);

    for (int g = 0; g < 28; g++) {
      int gx = 2 + g * 4 + ((g * 7) % 3);
      if (gx < 0 || gx >= 128) continue;

      // Height formula bound strictly between 3 and 9 px
      int h = 3 + ((g * 11 + 3) % 7);
      
      float wave1 = sinf(step * wind_speed - gx * spatial_freq);
      float wave2 = sinf(step * wind_speed * 1.6f - gx * spatial_freq * 1.3f + 1.5f) * 0.35f;
      float total_wave = (wave1 + wave2) * gust_env;
      
      float sway_float = total_wave * sway_mag + wind_bias + gust_impulse;
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

class Engine {
 private:
  float gust_impulse = 0.0f;

  CelestialRenderer celestial;
  CloudRenderer clouds;
  FogRenderer fog;
  PrecipitationRenderer precipitation;
  LightningRenderer lightning;
  GrassRenderer grass;

 public:
  void trigger_gust(float impulse) {
    gust_impulse += impulse;
    if (gust_impulse > 14.0f) gust_impulse = 14.0f;
    if (gust_impulse < -14.0f) gust_impulse = -14.0f;
  }

  void update(uint32_t step, int mode) {
    gust_impulse *= 0.91f;

    celestial.update(step, mode);
    clouds.update(mode);
    precipitation.update(step, mode);
    lightning.update(step, mode);
  }

  void render(lv_obj_t *canvas_obj, uint32_t step, int mode) {
    if (!canvas_obj) return;

    lv_layer_t layer;
    lv_canvas_init_layer(canvas_obj, &layer);

    // Dithered Soft Lightning Flash (Mode 7)
    if (mode == 7 && lightning.lightning_flash == 2) {
      for (int y = 0; y < 64; y += 2) {
        DrawingUtils::draw_line(&layer, 0, y, 127, y, true);
      }
    } else {
      lv_canvas_fill_bg(canvas_obj, lv_color_make(0, 0, 0), LV_OPA_TRANSP);
    }

    if (mode == 1 || mode == 4) celestial.render_sun(&layer, step);
    if (mode == 2) celestial.render_moon(&layer, step);
    if (mode == 3 || mode == 4 || mode == 7) clouds.render(&layer);
    if (mode == 8) fog.render(&layer, step);
    if (mode == 7 && lightning.lightning_flash > 0) lightning.render(&layer);
    if (mode >= 5 || mode == 1 || mode == 4 || mode == 8) precipitation.render(&layer, step, mode);
    grass.render(&layer, step, mode, gust_impulse);

    lv_canvas_finish_layer(canvas_obj, &layer);
  }
};

inline Engine global_weather_engine;

}  // namespace weather
