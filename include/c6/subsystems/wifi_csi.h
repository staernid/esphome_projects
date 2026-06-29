#pragma once
#include "../core/types.h"

namespace weather {

class WiFiCSIMotionDetector {
 public:
  bool is_motion_detected() const {
    return false;
  }
};

}  // namespace weather
