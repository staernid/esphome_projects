#pragma once
#include "../core/drawing.h"

namespace weather {

class ForecastSparklineRenderer {
 public:
  float hourly_temps[8] = {18.5f, 19.2f, 21.0f, 22.5f, 21.8f, 20.1f, 18.9f, 17.6f};
  float hourly_pressures[8] = {1013.2f, 1013.8f, 1014.5f, 1014.1f, 1013.5f, 1012.8f, 1012.2f, 1011.9f};

  void render(lv_layer_t *layer, uint32_t step) const {
    // Render 24-Hour Sparkline Graph & Forecast Dashboard (Screen 4)
    // Header text / bounds box
    DrawingUtils::draw_rect(layer, 0, 0, 128, 12, 0, true);
    
    // Draw temperature sparkline polyline across (10, 20) -> (118, 38)
    for (int i = 0; i < 7; i++) {
      int x1 = 10 + i * 15;
      int y1 = 38 - (int)roundf((hourly_temps[i] - 17.0f) * 3.0f);
      int x2 = 10 + (i + 1) * 15;
      int y2 = 38 - (int)roundf((hourly_temps[i + 1] - 17.0f) * 3.0f);
      DrawingUtils::draw_line(layer, x1, y1, x2, y2, true);
      DrawingUtils::draw_pixel(layer, x1, y1, true);
    }
    DrawingUtils::draw_pixel(layer, 115, 38 - (int)roundf((hourly_temps[7] - 17.0f) * 3.0f), true);

    // Draw pressure trend baseline at bottom (y = 52..62)
    for (int i = 0; i < 7; i++) {
      int x1 = 10 + i * 15;
      int y1 = 62 - (int)roundf((hourly_pressures[i] - 1010.0f) * 2.0f);
      int x2 = 10 + (i + 1) * 15;
      int y2 = 62 - (int)roundf((hourly_pressures[i + 1] - 1010.0f) * 2.0f);
      DrawingUtils::draw_line(layer, x1, y1, x2, y2, true);
    }
  }
};

class EnvironmentalTelemetryRenderer {
 public:
  float aqi_val = 42.0f;        // Air Quality Index
  float humidity_val = 55.0f;   // Humidity %
  float uv_index = 3.4f;        // UV Index gauge
  float pressure_hpa = 1014.2f; // Barometric Pressure hPa

  void render(lv_layer_t *layer, uint32_t step) const {
    // Render Environmental Telemetry Dashboard (Screen 5)
    // Round arc gauge for UV Index / AQI in center-left
    DrawingUtils::draw_arc(layer, 32, 32, 22, 135, 405, 3, true);
    
    // Needle indicator for UV index
    float needle_angle = (135.0f + (uv_index / 12.0f) * 270.0f) * 0.0174533f;
    int nx = 32 + (int)roundf(MathUtils::fast_cos(needle_angle) * 16.0f);
    int ny = 32 + (int)roundf(MathUtils::fast_sin(needle_angle) * 16.0f);
    DrawingUtils::draw_line(layer, 32, 32, nx, ny, true);

    // Right side telemetry boxes (Humidity & Pressure indicators)
    DrawingUtils::draw_rect(layer, 68, 8, 56, 22, 2, true);
    DrawingUtils::draw_rect(layer, 68, 36, 56, 22, 2, true);
  }
};

}  // namespace weather
