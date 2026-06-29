#pragma once
#include "../core/drawing.h"

namespace weather {

struct Boid {
  float x;
  float y;
  float vx;
  float vy;
  float flap_phase;
  bool is_apex;
};

class BoidRenderer {
 public:
  static const int NUM_BOIDS = 7;
  Boid boids[NUM_BOIDS];
  bool initialized = false;

  void init() {
    float start_x[NUM_BOIDS] = {-10.0f, -20.0f, -24.0f, -32.0f, -38.0f, -48.0f, -54.0f};
    float start_y[NUM_BOIDS] = {18.0f,  14.0f,  22.0f,  11.0f,  26.0f,  16.0f,  28.0f};
    for (int i = 0; i < NUM_BOIDS; i++) {
      boids[i].x = start_x[i];
      boids[i].y = start_y[i];
      boids[i].vx = 1.2f + (fast_rand() % 10) * 0.05f;
      boids[i].vy = ((fast_rand() % 10) - 5) * 0.03f;
      boids[i].flap_phase = (fast_rand() % 100) * 0.0628f;
      boids[i].is_apex = (i == 0);
    }
    initialized = true;
  }

  void update(uint32_t step, int mode, float continuous_wind) {
    if (!initialized) init();

    if (mode != 1 && mode != 3 && mode != 4) return;

    for (int i = 0; i < NUM_BOIDS; i++) {
      Boid &b = boids[i];

      if (b.is_apex) {
        float target_vy = MathUtils::fast_sin(step * 0.04f) * 0.4f;
        b.vy += (target_vy - b.vy) * 0.08f;
        float target_vx = 1.3f + continuous_wind * 0.04f;
        b.vx += (target_vx - b.vx) * 0.05f;
      } else {
        float sep_x = 0.0f, sep_y = 0.0f;
        float align_x = 0.0f, align_y = 0.0f;
        float coh_x = 0.0f, coh_y = 0.0f;
        int neighbors = 0;

        for (int j = 0; j < NUM_BOIDS; j++) {
          if (i == j) continue;
          const Boid &other = boids[j];
          float dx = other.x - b.x;
          float dy = other.y - b.y;
          float dist_sq = dx * dx + dy * dy;

          if (dist_sq < 1.0f) dist_sq = 1.0f;
          float dist = sqrtf(dist_sq);

          if (dist < 12.0f) {
            float force = (12.0f - dist) / dist;
            sep_x -= dx * force * 0.15f;
            sep_y -= dy * force * 0.15f;
          }

          if (dist < 35.0f) {
            align_x += other.vx;
            align_y += other.vy;
            coh_x += other.x;
            coh_y += other.y;
            neighbors++;
          }
        }

        if (neighbors > 0) {
          align_x /= neighbors;
          align_y /= neighbors;
          b.vx += (align_x - b.vx) * 0.06f;
          b.vy += (align_y - b.vy) * 0.06f;

          coh_x /= neighbors;
          coh_y /= neighbors;
          b.vx += (coh_x - b.x) * 0.015f;
          b.vy += (coh_y - b.y) * 0.015f;
        }

        const Boid &apex = boids[0];
        float side = (i % 2 == 1) ? 1.0f : -1.0f;
        int rank = (i + 1) / 2;
        float desired_x = apex.x - rank * 9.0f;
        float desired_y = apex.y + side * rank * 5.0f;

        b.vx += (desired_x - b.x) * 0.012f;
        b.vy += (desired_y - b.y) * 0.018f;

        b.vx += sep_x;
        b.vy += sep_y;
      }

      b.vx += continuous_wind * 0.01f;

      float speed = sqrtf(b.vx * b.vx + b.vy * b.vy);
      float max_speed = b.is_apex ? 1.8f : 1.6f;
      float min_speed = 0.7f;
      if (speed > max_speed) {
        b.vx = (b.vx / speed) * max_speed;
        b.vy = (b.vy / speed) * max_speed;
      } else if (speed < min_speed) {
        b.vx = (b.vx / speed) * min_speed;
        b.vy = (b.vy / speed) * min_speed;
      }

      b.x += b.vx;
      b.y += b.vy;

      if (b.y < 6.0f) b.vy += 0.15f;
      if (b.y > 38.0f) b.vy -= 0.15f;

      if (b.x > 145.0f) {
        b.x = -20.0f - (i * 8.0f);
        b.y = 10.0f + (fast_rand() % 22);
      }
    }
  }

  void render(lv_layer_t *layer, uint32_t step) const {
    for (int i = 0; i < NUM_BOIDS; i++) {
      const Boid &b = boids[i];
      int bx = (int)roundf(b.x);
      int by = (int)roundf(b.y);

      if (bx < -10 || bx > 135 || by < 0 || by > 64) continue;

      float flap_speed = b.is_apex ? 0.22f : 0.28f;
      float flap = MathUtils::fast_sin(step * flap_speed + b.flap_phase);
      
      int wing_span = b.is_apex ? 4 : 3;
      int flap_y = (int)roundf(flap * 2.2f);

      DrawingUtils::draw_line(layer, bx - wing_span, by - flap_y, bx, by + (flap_y > 0 ? 0 : 1), true);
      DrawingUtils::draw_line(layer, bx, by + (flap_y > 0 ? 0 : 1), bx + wing_span, by - flap_y, true);

      if (b.is_apex) {
        DrawingUtils::draw_pixel(layer, bx, by + 1, true);
      }
    }
  }
};

}  // namespace weather
