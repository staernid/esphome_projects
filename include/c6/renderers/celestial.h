#pragma once
#include "../core/drawing.h"

namespace weather {

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

    DrawingUtils::draw_arc(layer, mcx, mcy, 4, 35 + rot_deg, 145 + rot_deg, 2, false);
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

}  // namespace weather
