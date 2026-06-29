#pragma once
#include "types.h"

namespace weather {

class DrawingUtils {
 public:
  static void draw_rect(lv_layer_t *layer, int x, int y, int w, int h, int radius, bool is_white) {
    if (w <= 0 || h <= 0) return;
    static bool inited = false;
    static lv_draw_rect_dsc_t white_dsc, black_dsc;
    if (!inited) {
      lv_draw_rect_dsc_init(&white_dsc);
      white_dsc.bg_color = lv_color_make(255, 255, 255);
      white_dsc.radius = 0;

      lv_draw_rect_dsc_init(&black_dsc);
      black_dsc.bg_color = lv_color_make(0, 0, 0);
      black_dsc.radius = 0;
      inited = true;
    }

    if (radius <= 0) {
      lv_area_t area = {(int16_t)x, (int16_t)y, (int16_t)(x + w - 1), (int16_t)(y + h - 1)};
      lv_draw_rect(layer, is_white ? &white_dsc : &black_dsc, &area);
    } else {
      for (int i = 0; i < h; i++) {
        int cur_y = y + i;
        int inset = 0;
        if (radius == 1) {
          if (i == 0 || i == h - 1) inset = 1;
        } else {
          if (i < radius) {
            int dy = radius - i;
            inset = radius - (int)roundf(sqrtf(radius * radius - dy * dy));
          } else if (i >= h - radius) {
            int dy = i - (h - 1 - radius);
            inset = radius - (int)roundf(sqrtf(radius * radius - dy * dy));
          }
        }
        int line_w = w - 2 * inset;
        if (line_w > 0) {
          lv_area_t area = {(int16_t)(x + inset), (int16_t)cur_y, (int16_t)(x + inset + line_w - 1), (int16_t)cur_y};
          lv_draw_rect(layer, is_white ? &white_dsc : &black_dsc, &area);
        }
      }
    }
  }

  static void draw_pixel(lv_layer_t *layer, int x, int y, bool is_white) {
    if (x < 0 || x >= 128 || y < 0 || y >= 64) return;
    static bool inited = false;
    static lv_draw_rect_dsc_t white_dsc, black_dsc;
    if (!inited) {
      lv_draw_rect_dsc_init(&white_dsc);
      white_dsc.bg_color = lv_color_make(255, 255, 255);
      white_dsc.radius = 0;

      lv_draw_rect_dsc_init(&black_dsc);
      black_dsc.bg_color = lv_color_make(0, 0, 0);
      black_dsc.radius = 0;
      inited = true;
    }
    lv_area_t area = {(int16_t)x, (int16_t)y, (int16_t)x, (int16_t)y};
    lv_draw_rect(layer, is_white ? &white_dsc : &black_dsc, &area);
  }

  static void draw_line(lv_layer_t *layer, int x1, int y1, int x2, int y2, bool is_white) {
    if (x1 == x2) {
      int ymin = std::min(y1, y2);
      int ymax = std::max(y1, y2);
      int h = ymax - ymin + 1;
      int x = x1;
      if (x < 0 || x >= 128) return;
      if (ymin < 0) { h += ymin; ymin = 0; }
      if (ymin + h > 64) { h = 64 - ymin; }
      if (h > 0) draw_rect(layer, x, ymin, 1, h, 0, is_white);
      return;
    }
    if (y1 == y2) {
      int xmin = std::min(x1, x2);
      int xmax = std::max(x1, x2);
      int w = xmax - xmin + 1;
      int y = y1;
      if (y < 0 || y >= 64) return;
      if (xmin < 0) { w += xmin; xmin = 0; }
      if (xmin + w > 128) { w = 128 - xmin; }
      if (w > 0) draw_rect(layer, xmin, y, w, 1, 0, is_white);
      return;
    }
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    int curr_x = x1;
    int curr_y = y1;
    while (true) {
      draw_pixel(layer, curr_x, curr_y, is_white);
      if (curr_x == x2 && curr_y == y2) break;
      int e2 = 2 * err;
      if (e2 > -dy) {
        err -= dy;
        curr_x += sx;
      }
      if (e2 < dx) {
        err += dx;
        curr_y += sy;
      }
    }
  }

  static void draw_circle(lv_layer_t *layer, int cx, int cy, int radius, bool is_white) {
    if (radius <= 0) {
      draw_pixel(layer, cx, cy, is_white);
      return;
    }
    for (int dy = -radius; dy <= radius; dy++) {
      int py = cy + dy;
      if (py < 0 || py >= 64) continue;
      int dx = (int)roundf(sqrtf(radius * radius - dy * dy));
      int px1 = cx - dx;
      int px2 = cx + dx;
      int w = px2 - px1 + 1;
      int start_x = px1;
      if (start_x < 0) { w += start_x; start_x = 0; }
      if (start_x + w > 128) { w = 128 - start_x; }
      if (w > 0) {
        draw_rect(layer, start_x, py, w, 1, 0, is_white);
      }
    }
  }

  static void draw_arc(lv_layer_t *layer, int cx, int cy, int radius, int start_angle, int end_angle, int width, bool is_white) {
    if (radius <= 0 || width <= 0) return;
    int min_r = std::max(1, radius - width + 1);
    for (int r = min_r; r <= radius; r++) {
      for (int a = start_angle; a <= end_angle; a += 2) {
        float rad = a * 0.0174532925f;
        int px = (int)roundf(cx + MathUtils::fast_cos(rad) * r);
        int py = (int)roundf(cy + MathUtils::fast_sin(rad) * r);
        draw_pixel(layer, px, py, is_white);
      }
      float rad_end = end_angle * 0.0174532925f;
      int px_end = (int)roundf(cx + MathUtils::fast_cos(rad_end) * r);
      int py_end = (int)roundf(cy + MathUtils::fast_sin(rad_end) * r);
      draw_pixel(layer, px_end, py_end, is_white);
    }
  }

  static void render_wave_strip(lv_layer_t *layer, uint32_t step, float scroll_x, 
                                int base_y, float mode_mult, bool is_ceiling, 
                                WaveGenerator::WaveRenderMode render_mode = WaveGenerator::SOLID_FILL,
                                float freq_mult = 1.0f, float amp_mult = 1.0f) {
    for (int x = 0; x < 128; x++) {
      float raw_wave = WaveGenerator::calculate_wave_height(x, scroll_x, step, freq_mult, amp_mult);
      int wave_h = (int)roundf(raw_wave * mode_mult);
      
      if (is_ceiling) {
        int target_y = base_y + wave_h;
        if (target_y < 0) target_y = 0;
        if (target_y >= 64) target_y = 63;

        if (render_mode == WaveGenerator::SOLID_FILL) {
          draw_rect(layer, x, 0, 1, target_y + 1, 0, true);
        } else if (render_mode == WaveGenerator::OUTLINE_ONLY) {
          draw_pixel(layer, x, target_y, true);
        } else if (render_mode == WaveGenerator::OUTLINE_FILL) {
          if (target_y > 0) draw_rect(layer, x, 0, 1, target_y, 0, false); // Clear interior to black
          draw_pixel(layer, x, target_y, true); // Strict 1-pixel white vector outline edge
        }
      } else {
        int target_y = base_y - wave_h;
        if (target_y < 0) target_y = 0;
        if (target_y >= 64) target_y = 63;

        if (render_mode == WaveGenerator::SOLID_FILL) {
          draw_rect(layer, x, target_y, 1, 64 - target_y, 0, true);
        } else if (render_mode == WaveGenerator::OUTLINE_ONLY) {
          draw_pixel(layer, x, target_y, true);
        } else if (render_mode == WaveGenerator::OUTLINE_FILL) {
          draw_pixel(layer, x, target_y, true); // Strict 1-pixel white vector outline crest
          if (target_y < 63) draw_rect(layer, x, target_y + 1, 1, 63 - target_y, 0, false); // Clear body below
        }
      }
    }
  }
};

}  // namespace weather
