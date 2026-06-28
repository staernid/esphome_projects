# ESPHome Modular Base Configurations

This repository contains reusable, modular ESPHome configurations for **ESP32-C3** and **ESP32-C6** microcontrollers. It uses ESPHome's package system to separate shared infrastructure (Wi-Fi, API, OTA, Logger, Time) from hardware-specific features.

---

## 📁 Repository Structure

```text
.
├── common.yaml    # Core shared services (Wi-Fi, Home Assistant API, OTA, Logger, HA Time)
├── c3.yaml        # ESP32-C3 SuperMini base configuration
├── c6.yaml        # ESP32-C6 DevKitC-1 display & weather engine configuration
└── secrets.yaml   # Local Wi-Fi and API credentials (not committed to version control)
```

---

## 🧱 Local Architecture

Both `c3.yaml` and `c6.yaml` import `common.yaml` locally using ESPHome packages:

```yaml
substitutions:
  name: "c3"
  friendly_name: "c3"

packages:
  common: !include common.yaml
```

- **`common.yaml`**: Standardizes network connection, security encryption keys, logging, and real-time clock synchronization.
- **Device YAMLs**: Define board hardware specs, pin definitions, sensors, and custom logic while inheriting the base services.

---

## 🌐 Remote Package Guide: Reusing Configurations Across Repositories

If you have multiple devices (e.g., `c3-0`, `c3-1`, `c6-bedroom`) and want to keep your base hardware configs central in this repository, you can create lightweight YAMLs in external projects or repositories that reference this Git repository directly.

### 1. Basic Setup (`c3-0.yaml`)

Create a new YAML file for your specific device and use ESPHome's remote package syntax:

```yaml
# c3-0.yaml (e.g., in a separate device repo or directory)
substitutions:
  name: "c3-0"
  friendly_name: "Living Room C3"

packages:
  base_c3:
    url: https://github.com/your-username/your-esphome-repo
    ref: main            # Git branch, tag, or commit hash
    files: [c3.yaml]     # Imports c3.yaml (which automatically fetches common.yaml)
    refresh: 1d          # Cache refresh interval

# (Optional) Device-specific overrides or extra sensors
sensor:
  - platform: dht
    pin: GPIO0
    temperature:
      name: "Living Room Temperature"
    humidity:
      name: "Living Room Humidity"
    update_interval: 15s
```

### 2. Private Repositories

If this base repository is private, authenticate using SSH keys:

```yaml
packages:
  base_c3:
    url: git@github.com:your-username/your-esphome-repo.git
    ref: main
    files: [c3.yaml]
```

### 3. How Overrides Work

When ESPHome compiles the target device (`c3-0.yaml`):
1. The local `substitutions:` block overrides `${name}` and `${friendly_name}` everywhere in `c3.yaml` and `common.yaml` (including fallback AP names and entity IDs).
2. Any extra components defined in the child file (like extra sensors or modified GPIO pins) are seamlessly merged into the base configuration.

---

## 🔑 Secrets Configuration

Ensure your local `secrets.yaml` includes the required keys referenced by `common.yaml`:

```yaml
wifi_ssid: "Your_WiFi_SSID"
wifi_password: "Your_WiFi_Password"
fallback_ap_password: "Your_Fallback_AP_Password"
ota_password: "Your_OTA_Update_Password"
encryption_key: "Your_Home_Assistant_API_Key"
```
