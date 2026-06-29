#pragma once
/**
 * @file weather_sim.h
 * @brief Standalone Weather Simulation Engine Library Header.
 * Reusable across external C++ projects, host testbeds, and alternative display platforms.
 */

#include "draw_interface.h"
#include "../c6/core/types.h"
#include "../c6/weather_engine.h"

namespace weather {

/**
 * @brief Helper to initialize standalone simulation parameters without ESPHome infrastructure.
 */
inline void init_standalone_simulation() {
  global_weather_engine.reset_manual_wind();
}

}  // namespace weather
