#pragma once
#include "../core/types.h"

namespace weather {

class WiFiCSIMotionDetector {
 private:
  float signal_perturbation = 0.0f;
  bool motion_detected = false;
  uint32_t last_detection_time = 0;

 public:
  void process_csi_frame(float amplitude_delta, float phase_variance) {
    signal_perturbation = amplitude_delta * 0.7f + phase_variance * 0.3f;
    if (signal_perturbation > 1.5f) {
      motion_detected = true;
      last_detection_time = get_sim_millis();
    } else if (get_sim_millis() - last_detection_time > 3000) {
      motion_detected = false;
    }
  }

  bool is_motion_detected() const {
    return motion_detected;
  }

  float get_perturbation_level() const {
    return signal_perturbation;
  }
};

}  // namespace weather
