#pragma once
#include "core/types.h"
#include "core/drawing.h"
#include "renderers/celestial.h"
#include "renderers/clouds_fog.h"
#include "renderers/precipitation.h"
#include "renderers/landscape.h"
#include "renderers/entities.h"
#include "subsystems/audio_chiptune.h"
#include "subsystems/haptics.h"
#include "subsystems/diagnostics.h"
#include "subsystems/wifi_csi.h"

namespace weather {

class Engine {
 private:
  float gust_impulse = 0.0f;
  float continuous_wind = 0.0f;
  float target_wind = 0.0f;
  float ha_wind_speed = 0.0f;
  float ha_precipitation_rate = 0.0f;

  CelestialRenderer celestial;
  CloudRenderer clouds;
  FogRenderer fog;
  WindRenderer wind;
  PrecipitationRenderer precipitation;
  LightningRenderer lightning;
  GrassRenderer grass;
  TreeRenderer tree;
  BoidRenderer boids;

  ChiptuneAudioEngine audio;
  HapticEngine haptics;
  SystemDiagnostics diagnostics;
  WiFiCSIMotionDetector csi_motion;

 public:
  void set_ha_wind_speed(float speed) { ha_wind_speed = std::max(0.0f, speed); }
  void set_ha_precipitation_rate(float rate) { ha_precipitation_rate = std::max(0.0f, rate); }

  void trigger_gust(float impulse) {
    gust_impulse += impulse;
    if (gust_impulse > 14.0f) gust_impulse = 14.0f;
    if (gust_impulse < -14.0f) gust_impulse = -14.0f;
  }

  void adjust_manual_wind(float delta) {
    target_wind += delta * 2.5f;
    if (target_wind > 20.0f) target_wind = 20.0f;
    if (target_wind < -20.0f) target_wind = -20.0f;
  }

  void reset_manual_wind() {
    target_wind = 0.0f;
    continuous_wind = 0.0f;
  }

  float get_target_wind() const { return target_wind; }
  bool is_lightning_flashing() const { return lightning.is_lightning_flashing(); }
  void trigger_lightning() { lightning.trigger_lightning(); }

  ChiptuneAudioEngine& get_audio() { return audio; }
  HapticEngine& get_haptics() { return haptics; }
  SystemDiagnostics& get_diagnostics() { return diagnostics; }
  WiFiCSIMotionDetector& get_csi() { return csi_motion; }

  void update(uint32_t step, int mode) {
    diagnostics.mark_frame_start();
    gust_impulse *= 0.91f;

    continuous_wind += (target_wind - continuous_wind) * 0.14f;
    target_wind *= 0.965f;
    if (std::abs(target_wind) < 0.01f) target_wind = 0.0f;

    float ha_wind_offset = ha_wind_speed * 0.12f;
    float effective_wind = continuous_wind + ha_wind_offset;

    celestial.update(step, mode);
    clouds.update(mode, effective_wind);
    boids.update(step, mode, effective_wind);
    if (mode != 8) precipitation.update(step, mode, effective_wind, ha_precipitation_rate);
    if (mode == 8) wind.update(step, effective_wind);
    lightning.update(step, mode);
  }

  void render(lv_obj_t *canvas_obj, uint32_t step, int mode) {
    if (!canvas_obj) {
      diagnostics.mark_frame_end();
      return;
    }

    lv_layer_t layer;
    lv_canvas_init_layer(canvas_obj, &layer);

    if (mode == 7 && lightning.lightning_flash == 2) {
      for (int y = 0; y < 64; y += 2) {
        DrawingUtils::draw_line(&layer, 0, y, 127, y, true);
      }
    } else {
      lv_canvas_fill_bg(canvas_obj, lv_color_make(0, 0, 0), LV_OPA_TRANSP);
    }

    float ha_wind_offset = ha_wind_speed * 0.12f;
    float effective_wind = continuous_wind + ha_wind_offset;

    if (mode == 1 || mode == 4) celestial.render_sun(&layer, step);
    if (mode == 2) celestial.render_moon(&layer, step);
    if (mode == 1 || mode == 4) tree.render(&layer, step, effective_wind);
    if (mode == 3 || mode == 4 || mode == 7) clouds.render(&layer, mode);
    if (mode == 1 || mode == 3 || mode == 4 || mode == 8) boids.render(&layer, step);
    if (mode == 8) fog.render(&layer, step, effective_wind);
    if (mode == 8) wind.render(&layer, step);
    if (mode == 7 && lightning.lightning_flash > 0) lightning.render(&layer);
    if ((mode >= 5 && mode != 8) || mode == 1 || mode == 4) precipitation.render(&layer, mode, effective_wind, ha_precipitation_rate);
    grass.render(&layer, step, mode, gust_impulse, effective_wind);

    lv_canvas_finish_layer(canvas_obj, &layer);
    diagnostics.mark_frame_end();
  }
};

inline Engine global_weather_engine;

inline const char* get_mode_name(int mode) {
  switch (mode) {
    case 1: return "Sunny";
    case 2: return "Clear Night";
    case 3: return "Cloudy";
    case 4: return "Partly Cloudy";
    case 5: return "Torrential Rain";
    case 6: return "Snowy Blizzard";
    case 7: return "Thunderstorm";
    case 8: return "Windy / Fog";
    default: return "Unknown";
  }
}

inline int get_mode_from_condition(const std::string &cond, const std::string &sun_state) {
  if (cond == "sunny") {
    return (sun_state == "below_horizon") ? 2 : 1;
  }
  if (cond == "clear-night") return 2;
  if (cond == "cloudy") return 3;
  if (cond == "partlycloudy") return 4;
  if (cond == "rainy" || cond == "pouring") return 5;
  if (cond == "snowy" || cond == "snowy-rainy") return 6;
  if (cond == "lightning" || cond == "lightning-rainy") return 7;
  if (cond == "windy" || cond == "fog") return 8;
  return 1;
}

inline std::string format_temp_with_trend(float current, float &last_val) {
  std::string trend = "=";
  if (last_val != 0.0f) {
    float diff = current - last_val;
    if (diff > 0.05f) trend = "^";
    else if (diff < -0.05f) trend = "v";
  }
  last_val = current;
  return str_sprintf("%.1f°C %s", current, trend.c_str());
}

}  // namespace weather
