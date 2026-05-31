# ESP32-C6 Zigbee Irrigation Controller

A self-hosted irrigation controller built on ESPHome and Zigbee, designed for integration with Home Assistant via Zigbee2MQTT. Runs on the ESP32-C6 with a custom PCB, supporting up to 15 zones with local OLED control and full remote control over Zigbee. Designed, tested and debugged by human, coded by Claude.

![Prototype|400](/docs/images/hardware/irr_control_protoype.jpg)

## Highlights

- **Up to 15 irrigation zones** (configurable at compile time: 7, 11, or 15)
- **Two scheduled cycle types** (irrigation and short) with two daily schedules each
- **Manual zone control** with adjustable runtime
- **Local UI**: 128×128 OLED, three buttons, full settings menu - fully usable without a network
- **Zigbee integration**: native Zigbee 3.0 endpoint designed to be used with Zigbee2MQTT
- **Home Assistant**: Custom dashboard based on button-card
- **Power-loss recovery**: interrupted cycles resume after reboot; missed schedules detected
- **Cycle queue**: a second cycle triggered while one is running waits its turn
- **Weather scaling**: percentage multiplier for cycle durations with optional auto-reset
- **DS3231 RTC**: keeps time across power loss; Zigbee sync writes to RTC hardware
- **Pump sequencing**: configurable start/stop offsets relative to zone valves
- **Localization**: English and Hungarian included; easy to add more

## Hardware

| Component                                | Purpose                                    |
| ---------------------------------------- | ------------------------------------------ |
| ESP32-C6 (DevKitC-1 or custom PCB)       | Main controller, Zigbee 3.0 router         |
| MCP23017 I/O expander                    | Drives up to 15 zone relays + 1 pump relay |
| DS3231 RTC                               | Persistent timekeeping across power loss   |
| SH1107 128×128 OLED                      | Local display                              |
| WS2812 RGB LED                           | Status indication                          |
| 3 momentary buttons (with optional LEDs) | Local control                              |

See [Specification](docs/Specification.md) for the full hardware reference (GPIO map, I2C addresses, MCP23017 pin assignment, partition table).

## Software Architecture

ESPHome multi-file YAML with C++ headers for shared logic. Built around the [luar123/zigbee_esphome](https://github.com/luar123/zigbee_esphome) fork for ESP32-C6 Zigbee support.

```
irrigation_control.yaml              - top-level config
irrigation_control_z2m_converter.mjs - Zigbee2MQTT external converter
irrigation_control/
├── config.h           - user-tunable constants
├── helpers.h          - config struct, helpers, X-macro zone arrays
├── display.h          - OLED page rendering
├── globals.yaml       - global state variables
├── hardware.yaml      - I2C, expander, display, buttons, RGB, relays
├── schedules.yaml     - time platforms, schedule trigger, scale reset
├── scripts_system.yaml   - boot sequence, decision tree, ZB push, LED patterns
├── scripts_buttons.yaml  - button handlers, settings navigation
├── scripts_cycles.yaml   - cycle execution, queue, skip, abort
├── scripts_manual.yaml   - manual zone scripts, pump sequencing
├── zigbee_global_endpoints.yaml - EP14 globals cluster
├── boards/            - hardware_{$dev_board}.yaml - board, I2C, IO expander,
|                         display, buttons, RGB
├── translations/      - config_en.h, config_hu.h
└── zones/             - zone_config_{7,11,15}.h, relays_{7,11,15}.yaml,
                         zigbee_zone_endpoints_{7,11,15}.yaml
```

## Requirements

- **ESPHome** ≥ 2026.4.0
- **Zigbee component**: `github://luar123/zigbee_esphome` (pinned to v1.x branch)
- **Zigbee2MQTT** ≥ 2.9.2
- **Home Assistant** (any recent version) with the Zigbee2MQTT integration
- **button-card** HACS module dependency for the included HA dashboard

## Getting Started

### 1. Flash the firmware

Clone this repository to your ESPHome folder and edit the top-level YAML to match your zone count and language:

```yaml
substitutions:
  timezone: "Europe/Budapest"
  dev_board: "c6-devkitc-1"  
  zones: "11"      # 7, 11, or 15
  language: "en"   # en or hu
```

Compile and flash through the ESPHome GUI or use the command line:

```bash
esphome run irrigation_control.yaml
```

If you need to flash a compiled firmware you can use the [Web flasher](https://web.esphome.io/) or [ESPConnect](https://thelastoutpostworkshop.github.io/ESPConnect/)


#### Advanced Configuration

- display defaults are found in `config.h` Section 1
- config defaults are found in `helpers.h` Section 4
- Cycle names shown in the UI can be customized in `\localisation\config_xx.h`
- Zone names shown in the UI can be modified in `\zones\zone_config_##.h`
- `TOTAL_ZONE_COUNT` and `CYCLE_ZONE_COUNT` in `zone_config_##.h` can be used to permanently exclude zones from automation. Zones excluded are always from the end of the list, eg: `TOTAL_ 15` with `CYCLE_ 14` excludes Zone 15. This zone can only be started in manual zone mode, and is always put first on the manual zone list in the UI. Use case: Garden hose on a zone valve. Hose gets a failsafe timer with adjustable duration, can be started remotely or on the UI, and is always excluded from starting by mistake with a wrong configuration.

#### Zone customization

The scripts work with any number of zones defined. 7/11/15 relay configurations are provided for convenience to be used with 8, 8+4 or 8+8 relay boards. The firmware can be customized with minimal work to operate with 1-15 zones.   
Files to update:
 - `\zones\relays_15.yaml` :  add/remove zone relays and zone template switches.  
 - `\zones\zone_config_15.h` : set zone defines, add/remove X() macros and zone names in the name array.   
 - `\zones\zigbee_zone_endpoints_15.yaml` : add/remove Zigbee endpoints. Above zone 15  `zigbee_global_endpoints.yaml` needs to renumbered.  

Zone 16 can be added on GPIO: 2;3;6;7;10;11. 
Going above 16 zones is possible, but untested. A second MCP23017 I/O board is recommended, and the default binding table size needs to be increased in the external component (luar123/zigbee_esphome) before compiling the firmware.

Add to `zigbee.cpp` `#L538` before `esp_zb_init(&zb_nwk_cfg);`
```
  esp_zb_aps_src_binding_table_size_set(32);
  esp_zb_aps_dst_binding_table_size_set(32);
```
  
### 2. Install the Z2M converter

Copy `irrigation_control_z2m_converter.mjs` into your Zigbee2MQTT `external_converters/` directory, restart Z2M and permit device pairing.

### 3. Pair the device

The controller enters pairing mode on every reboot while not paired. Holding B1 for 5 seconds resets the Zigbee stack to default deleting all stored network credentials. Use it when you force-removed the device from Z2M and need to reconnect, or for joining another network.  
The device appears in Z2M as an irrigation controller with auto-detected zone count. All controls and configuration exposes are populated automatically.
If pairing is unsuccessful (red exclamation mark in Z2M) remove the device, reboot Z2M and pair again. After firmware upgrades it's recommended to do the re-pairing process to clean any cached values on the coordinator.

### 4. Configure on device

Install the Dashboard into Home Assistant or use the local OLED menu to set:

- Zone durations (per zone, per cycle type)
- Zone enable flags
- Daily schedules
- Pump timings, lockout
- Cycle repeat count
- Weather scaling

See the [User](docs/User%20Guide.md) and the [Dashboard](Home%20Assistant.md) guide for full operating instructions.

## Documentation

- **[User Guide](docs/User%20Guide.md)** - operating the device locally
- **[Home assistant](Home%20Assistant.md)** - Operating the device remotely via Home Assistant
- **[Specification](docs/Specification.md)** - full hardware and firmware reference
- **[TODO list](docs/TODO%20list.md)** - open issues and planned features
- **[Hardware](docs/Hardware.md)** - Parts list and wiring schematic

## Project Status

Active development. Functional and in daily use, with periodic firmware refinements. See the TODO list for current work.
Planned:
- Irrigation frequency: set programming to run only every 2nd/3rd/etc. day
- Zigbee sdk 2.0 - subject to [luar123](https://github.com/luar123/zigbee_esphome/tree/v2) release
- WIFI OTA - AP mode depends on SDK 2.0
- WIFI concurrency/dual mode - if stability testing is successful

## License

Personal project using The Unlicense terms. Use at your own risk; pull requests and discussion welcome.

## Acknowledgements

- [ESPHome](https://esphome.io/) - the YAML-driven embedded framework that makes this possible
- [luar123/zigbee_esphome](https://github.com/luar123/zigbee_esphome) - ESP32-C6 Zigbee component
- [Home Assistant](https://www.home-assistant.io/) - The Swiss knife of Home Automation
- [Zigbee2MQTT](https://www.zigbee2mqtt.io/) - Zigbee gateway and converter ecosystem
- [Antrophic](https://claude.ai/) - Coded with Claude
- [Romrider](https://github.com/RomRider) for button-card
- [Google fonts](https://fonts.google.com/) for the Material Design Icons and the Roboto font used in the UI