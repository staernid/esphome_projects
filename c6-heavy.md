# ESP32-C6 Heavy Animated Weather Physics Engine (`c6-heavy`)

An advanced, high-performance ESPHome firmware target (`c6-heavy.yaml`) for the **ESP32-C6 DevKitC-1** featuring a 128x64 OLED display (`SH1106_128X64`) powered by LVGL version 9 and a **hardware-accelerated algorithmic particle/vector rendering engine** (`include/c6/weather_engine.h`) showcasing the 160MHz RISC-V compute performance of the ESP32-C6.

---

## Showcase Features & 3-Mode UX Architecture

The heavy firmware features an intuitive, zero-clutter 3-mode user experience navigated via tactile push buttons and rotary encoder interactions:

1. 🌌 **Auto Showcase (Screensaver Mode)**: Pure visual aesthetic mode with zero text clutter. Automatically cycles through all 8 procedural weather animations every 5 seconds.
2. 🎛️ **Manual Showcase (Interactive Physics Playroom Mode)**: Activated by turning the rotary encoder or pressing buttons. Displays pure full-screen weather animations where knob turns directly adjust real-time continuous wind velocity ($\pm 15\text{px/s}$), dynamically deflecting rain streaks, accelerating cloud drift, blowing snowflakes, and whipping the grass horizon! Manual cycling includes the secret Mode 9 Coastal Ocean Easter Egg.
3. 🌤️ **Passive Live Weather (Unified Screen)**: Background plays live weather animations synchronized with Home Assistant forecasts (`weather.forecast_home`) and celestial elevation (`sun.sun`). Foreground features a space-efficient single-screen layout with a small `IN/OUT` header above clean temperature values (`21.5°C`, `18.2°C`) alongside a sleek medium clock (`14:32`).

- **Hardware Interaction & Controls**:
  - 🔘 **Mode Button (GPIO 21)**: Dedicated UI Mode switcher. Cleanly cycles through Operating Modes ($1 \rightarrow 2 \rightarrow 3 \rightarrow 1$) with zero timing ambiguity.
  - 🌤️ **Weather Button (GPIO 22)**: Dedicated Weather Preset switcher. Steps through weather effects ($1 \rightarrow 2 \dots \rightarrow 9 \rightarrow 1$). In Mode 3, automatically switches to Manual Showcase mode to preview the effect.
  - 🔘 **Encoder Push Button (GPIO 20)**: Dedicated Display Contrast controller. Pressing cleanly cycles contrast between `1.0`, `0.5`, and `0.0`.
  - 🔄 **Rotary Encoder (GPIO 18/19)**: Continuous physics controller for atmospheric wind force and particle deflection. Automatically activates Playroom mode when rotated.
- **Hardware-Accelerated 60 FPS Engine**: Overclocked I2C bus (`800kHz`) and synchronized hardware display refresh (`update_interval: 16ms`) for ultra-fluid vector animations.
- **Algorithmic Weather FX Showcase (Overlay Canvas)**: Zero UI widget bloat. All weather visuals are procedurally rendered in modular C++ vector math and a particle physics engine onto a single transparent LVGL overlay canvas (`top_layer`):
  - ☀️ **Sunny / Clear Day**: Solid sun disk located at `(112, 16)` featuring an expressive cute face with specular-highlighted pupil eyes that smoothly rotate $\pm 30^\circ$, double-blinking eyes, solar prominence motor eruptions, 8 dynamic rotating line vectors with pulsating solar flares, and atmospheric shimmer motes.
  - 🌙 **Clear Night**: Positioned slightly off-center to the right (`x = 72`) to perfectly balance between the left temperature labels and right clock. Features a crescent moon cut, geometric crater marks, a whole-screen 24-star multi-phased twinkling starfield with cross-star markers, and procedural shooting star meteors.
  - ☁️ **Cloudy / Overhead Carpet Waves**: Continuous undulating overhead cloud carpet wave strips (`WaveGenerator::render_wave_strip`) drifting across the ceiling with small lower cloud puffs.
  - 🌧️ **Torrential Rain / Pouring**: 16 razor-sharp, strictly 1-pixel wide slanted rain streaks rendered via integer-stepped vertical runs to eliminate anti-aliased blurring/worminess, complete with ground impact splash ripples and wind deflection.
  - ❄️ **Snowy Blizzard**: 16 multi-sized snowflakes ($1\times1, 2\times2, 3\times3$) fluttering with dual-axis trigonometric sine turbulence over a solid 2-pixel thick snow bank baseline along the bottom screen edge.
  - ⚡ **Thunderstorm**: Multi-branching volcanic lightning bolts with rare, non-blinding 50% scanline dithered atmospheric strobe flashes across dual-layer 1-pixel vector wave ceiling outlines with independent parallax scrolling speeds (`scroll_fg` vs `scroll_bg`) and solid occlusion fill!
  - 💨 **Windy / Vector Fog**: Opaque solid white rolling mist wave along the ground horizon ($y=44$) paired with high wind streamer lines and orbital sine-wave dust swirls.
  - 🌾 **Swaying Grass Horizon**: A dense 28-blade 1-pixel thick grass horizon spanning the bottom edge with strict 3 to 9 px heights. Uses spatial traveling wave physics ($\sin(\text{step} \cdot v - gx \cdot k)$) to simulate rhythmic rolling wind-waves. Automatically transitions into black silhouette grass blades during fog conditions and solid snow banks during blizzard conditions.
  - 🌊 **Coastal Ocean (Secret Mode 9 Easter Egg)**: Multi-layer ocean waves (`OUTLINE_ONLY` background waves and `OUTLINE_FILL` foreground water), reusable animated sun face with solar flares and double blinks, airborne sea spray particles, wave foam crest streaks, and leaping dolphins!

---

## Hardware Specifications

| Hardware Component | Details |
| :--- | :--- |
| **Microcontroller** | Espressif ESP32-C6 DevKitC-1 (RISC-V, 4MB Flash, Wi-Fi 6, BLE 5, Thread/Zigbee) |
| **Framework** | `esp-idf` |
| **Display** | SH1106 / SSD1306 128x64 Monochrome I2C OLED |
| **I2C Pins** | SCL: GPIO 7, SDA: GPIO 6 (`800kHz` frequency) |
| **Rotary Encoder** | EC11 Rotary Encoder (Pin A: GPIO 18, Pin B: GPIO 19) |
| **Encoder Push Button** | SW: GPIO 20 (Display Contrast Controller: Press to cycle 1.0 -> 0.5 -> 0.0) |
| **Display Mode Button** | Tactile Switch: GPIO 21 (Dedicated UI Operating Mode Switcher) |
| **Weather Preset Button**| Tactile Switch: GPIO 22 (Dedicated Weather Effect Selector) |
| **Onboard RGB LED**    | Addressable WS2812B / NeoPixel: GPIO 8 (Lightning Effect Strobe Light) |

---

## Target Build Files

- **`c6-heavy.yaml`**: Firmware configuration for ESP32-C6 hardware device.
- **`c6_sim-heavy.yaml`**: Desktop SDL host simulator target for testing animated physics on Linux.
- **`packages/c6/c6_display_heavy.yaml`**: LVGL display engine package for 60 FPS canvas rendering loops.

> [!NOTE]  
> BLE device tracking (`esp32_ble_tracker`) and iBeacon broadcasting are intentionally disabled in `c6-heavy.yaml` to conserve flash storage and RAM for the 60 FPS vector math rendering engine.
