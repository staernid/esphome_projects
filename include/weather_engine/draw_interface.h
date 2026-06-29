#pragma once
#include <cstdint>

namespace weather {

/**
 * @brief Abstract Drawing Target Interface for Weather Simulation Renderers.
 * Decouples rendering math and simulation state updates from specific display graphics frameworks
 * (such as LVGL, Adafruit_GFX, U8g2, SDL, or custom framebuffers).
 */
class IDrawTarget {
 public:
  virtual ~IDrawTarget() = default;

  virtual void draw_pixel(int x, int y, bool is_white) = 0;
  virtual void draw_rect(int x, int y, int w, int h, int radius, bool is_white) = 0;
  virtual void draw_line(int x1, int y1, int x2, int y2, bool is_white) = 0;
  virtual void draw_circle(int cx, int cy, int radius, bool is_white) = 0;
  virtual void draw_arc(int cx, int cy, int radius, int start_angle, int end_angle, int width, bool is_white) = 0;
  virtual void clear() = 0;
};

}  // namespace weather
