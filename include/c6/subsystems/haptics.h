#pragma once
#include "../core/types.h"

namespace weather {

class HapticEngine {
 public:
  void pulse_lightning_rumble() {}
  void tick_encoder_detent() {}
  void pulse_wind_turbulence() {}
};

}  // namespace weather
