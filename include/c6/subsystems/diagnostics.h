#pragma once
#include "../core/types.h"
#if __has_include("esphome/components/wifi/wifi_component.h")
#include "esphome/components/wifi/wifi_component.h"
#endif

namespace weather {

class SystemDiagnostics {
 private:
  uint32_t frame_count = 0;
  uint32_t last_fps_calc_time = 0;
  float current_fps = 0.0f;
  
  uint32_t last_frame_start_us = 0;
  float last_render_duration_us = 0.0f;
  float cpu_load_pct = 0.0f;

 public:
  void mark_frame_start() {
    last_frame_start_us = get_sim_micros();
  }

  void mark_frame_end() {
    uint32_t now_us = get_sim_micros();
    uint32_t duration = (now_us >= last_frame_start_us) ? (now_us - last_frame_start_us) : 0;
    last_render_duration_us = last_render_duration_us * 0.85f + duration * 0.15f;
    
    // CPU load relative to ~32ms (32000us) frame interval budget
    cpu_load_pct = std::min(100.0f, (last_render_duration_us / 32000.0f) * 100.0f);

    frame_count++;
    uint32_t now_ms = get_sim_millis();
    if (now_ms - last_fps_calc_time >= 1000) {
      current_fps = (frame_count * 1000.0f) / (now_ms - last_fps_calc_time);
      frame_count = 0;
      last_fps_calc_time = now_ms;
    }
  }

  float get_fps() const { return (current_fps > 0.0f) ? current_fps : 31.2f; }
  float get_cpu_load() const { return cpu_load_pct; }

  float get_free_heap_kb() const {
#if __has_include("esp_system.h")
    return esp_get_free_heap_size() / 1024.0f;
#else
    return 248.5f; // Simulated heap for host environment
#endif
  }

  int get_wifi_rssi() const {
#if defined(USE_WIFI)
    if (esphome::wifi::global_wifi_component != nullptr && esphome::wifi::global_wifi_component->is_connected()) {
      return esphome::wifi::global_wifi_component->wifi_rssi();
    }
#endif
    return -58; // Simulated RSSI when disconnected or on host
  }

  std::string get_formatted_uptime() const {
    uint32_t total_sec = get_sim_millis() / 1000;
    uint32_t hours = total_sec / 3600;
    uint32_t mins = (total_sec % 3600) / 60;
    uint32_t secs = total_sec % 60;
    return str_sprintf("%02u:%02u:%02u", hours, mins, secs);
  }
};

}  // namespace weather
