// ============================================================================
// irrigation_control/helpers.h
// ============================================================================
// C++ helpers for the ESPHome irrigation controller.
// Included via esphome: includes: in the main YAML.
//
// Contents:
//   1. Zone count and testing defines
//   2. Config struct             - IrrigationConfig, ZoneConfig, ScheduleConfig
//   3. Default values            - config_defaults()
//   4. Config validation         - config_valid()
//   5. Config persistence        - config_load(), config_save()
//   6. Zone relay helpers        - zone_relay_on/off/state(), any_zone_on(), zone_name()
//   7. Zone config accessors     - zone_irr/short/manual_duration_sec(), zone_irr/short_enabled()
//
// To add a zone (complete checklist):
//   1. Increment TOTAL_ZONE_COUNT below
//   2. Increment CYCLE_ZONE_COUNT below if the zone should run in cycles
//   3. Add case N: to zone_relay_on()    in section 6 below
//   4. Add case N: to zone_relay_off()   in section 6 below
//   5. Add case N: to zone_relay_state() in section 6 below
//   6. Add case N: to zone_name()        in section 6 below
//   7. Add relay GPIO switch + zoneN_zb template switch in hardware.yaml
//   8. Add Zigbee endpoint block in zigbee_endpoints.yaml
//   9. Add one ternary line to sync_zone_zb_state() in scripts_manual.yaml
//   No changes needed to scripts_cycles.yaml, scripts_buttons.yaml,
//   scripts_system.yaml, globals.yaml, or schedules.yaml.
// ============================================================================

#pragma once
#include "esphome.h"

// ============================================================================
// 1. Zone count and testing defines
// ============================================================================

#define TOTAL_ZONE_COUNT 11
#define CYCLE_ZONE_COUNT 10

// Duration scale factor - compile-time only until EP14 runtime attribute is
// implemented. Set to 1.0f for production, 0.0167f for fast testing.
#define DURATION_SCALE 1.0f

static_assert(CYCLE_ZONE_COUNT <= TOTAL_ZONE_COUNT,
    "CYCLE_ZONE_COUNT cannot exceed TOTAL_ZONE_COUNT");

// ============================================================================
// 2. Config struct
// ============================================================================

#define CONFIG_MAGIC 0xCAFE0001

struct ZoneConfig {
    uint8_t irr_duration_min;      // Irrigation cycle duration (minutes); 0 = disabled
    uint8_t short_duration_min;    // Short cycle duration (minutes); 0 = disabled
    uint8_t manual_duration_min;   // Manual zone duration (minutes); default 5
    uint8_t flags;                 // bit0 = irr_enabled, bit1 = short_enabled
};

struct ScheduleConfig {
    uint8_t enabled;
    uint8_t hour;                  // 0-23
    uint8_t minute;                // 0-59
};

// Total size: 4 + 1 + 1 + 1 + 1 + (11 × 4) + (3 × 3) + 1 = 58 bytes
struct IrrigationConfig {
    uint32_t       magic;
    uint8_t        pump_lockout_sec;           // 0 = disabled; default 30
    uint8_t        pump_start_delay_sec;       // default 1
    uint8_t        duration_scale_percent;     // 0-100; default 100 (runtime rain scaling)
    uint8_t        scale_reset_hours;          // 0-168; default 24 (0 = no auto-reset)
    ZoneConfig     zones[TOTAL_ZONE_COUNT];
    ScheduleConfig irr_schedule;
    ScheduleConfig short_schedule1;
    ScheduleConfig short_schedule2;
    uint8_t        checksum;
};

// ============================================================================
// 3. Default values
// ============================================================================

static uint8_t config_checksum(const IrrigationConfig& cfg) {
    const uint8_t* p = reinterpret_cast<const uint8_t*>(&cfg);
    uint8_t xor_val = 0;
    for (size_t i = 0; i < offsetof(IrrigationConfig, checksum); i++)
        xor_val ^= p[i];
    return xor_val;
}

static IrrigationConfig config_defaults() {
    IrrigationConfig cfg = {};
    cfg.magic                    = CONFIG_MAGIC;
    cfg.pump_lockout_sec         = 30;
    cfg.pump_start_delay_sec     = 1;
    cfg.duration_scale_percent   = 100;
    cfg.scale_reset_hours        = 24;

    for (int i = 0; i < TOTAL_ZONE_COUNT; i++) {
        int zone_num = i + 1;
        bool wired    = !(zone_num == 6 || zone_num == 9 || zone_num == 10);
        bool in_cycle = wired && (zone_num <= CYCLE_ZONE_COUNT);
        cfg.zones[i].irr_duration_min    = in_cycle ? 15 : 0;
        cfg.zones[i].short_duration_min  = in_cycle ? 10 : 0;
        cfg.zones[i].manual_duration_min = wired    ?  5 : 0;
        cfg.zones[i].flags               = in_cycle ? 0x03 : 0x00;
    }

    cfg.irr_schedule    = {1,  6,  0};
    cfg.short_schedule1 = {1,  6, 30};
    cfg.short_schedule2 = {1, 18,  0};
    cfg.checksum = config_checksum(cfg);
    return cfg;
}

// ============================================================================
// 4. Config validation
// ============================================================================

static bool config_valid(const IrrigationConfig& cfg) {
    if (cfg.magic    != CONFIG_MAGIC)         return false;
    if (cfg.checksum != config_checksum(cfg)) return false;
    for (int i = 0; i < TOTAL_ZONE_COUNT; i++) {
        if (cfg.zones[i].irr_duration_min    > 60) return false;
        if (cfg.zones[i].short_duration_min  > 60) return false;
        if (cfg.zones[i].manual_duration_min > 60) return false;
    }
    if (cfg.duration_scale_percent > 100) return false;
    if (cfg.scale_reset_hours > 168)      return false;
    auto sched_ok = [](const ScheduleConfig& s) {
        return s.hour < 24 && s.minute < 60;
    };
    if (!sched_ok(cfg.irr_schedule))    return false;
    if (!sched_ok(cfg.short_schedule1)) return false;
    if (!sched_ok(cfg.short_schedule2)) return false;
    return true;
}

// ============================================================================
// 5. Config persistence
// ============================================================================

// Increment last byte when struct layout changes to force defaults reload.
static const uint32_t CONFIG_PREF_HASH = 0xAB010002;

static IrrigationConfig g_config = {};
static bool g_config_loaded = false;

static void config_save() {
    g_config.checksum = config_checksum(g_config);
    auto pref = global_preferences->make_preference<IrrigationConfig>(CONFIG_PREF_HASH, true);
    if (!pref.save(&g_config))
        ESP_LOGE("config", "Failed to save config");
    else
        ESP_LOGD("config", "Config saved (%d bytes)", (int)sizeof(IrrigationConfig));
}

static void config_load() {
    auto pref = global_preferences->make_preference<IrrigationConfig>(CONFIG_PREF_HASH, true);
    IrrigationConfig loaded = {};
    if (pref.load(&loaded) && config_valid(loaded)) {
        g_config = loaded;
        ESP_LOGI("config", "Config loaded from flash (%d bytes)", (int)sizeof(IrrigationConfig));
    } else {
        ESP_LOGW("config", "Invalid or missing config - loading defaults");
        g_config = config_defaults();
        config_save();
    }
    g_config_loaded = true;
}

// ============================================================================
// 6. Zone relay helpers
// ============================================================================

static void zone_relay_on(int zone) {
    switch (zone) {
        case 1:  id(zone1_relay).turn_on();  break;
        case 2:  id(zone2_relay).turn_on();  break;
        case 3:  id(zone3_relay).turn_on();  break;
        case 4:  id(zone4_relay).turn_on();  break;
        case 5:  id(zone5_relay).turn_on();  break;
        case 6:  id(zone6_relay).turn_on();  break;
        case 7:  id(zone7_relay).turn_on();  break;
        case 8:  id(zone8_relay).turn_on();  break;
        case 9:  id(zone9_relay).turn_on();  break;
        case 10: id(zone10_relay).turn_on(); break;
        case 11: id(zone11_relay).turn_on(); break;
        default: ESP_LOGW("zone", "zone_relay_on: invalid zone %d", zone); break;
    }
}

static void zone_relay_off(int zone) {
    switch (zone) {
        case 1:  id(zone1_relay).turn_off();  break;
        case 2:  id(zone2_relay).turn_off();  break;
        case 3:  id(zone3_relay).turn_off();  break;
        case 4:  id(zone4_relay).turn_off();  break;
        case 5:  id(zone5_relay).turn_off();  break;
        case 6:  id(zone6_relay).turn_off();  break;
        case 7:  id(zone7_relay).turn_off();  break;
        case 8:  id(zone8_relay).turn_off();  break;
        case 9:  id(zone9_relay).turn_off();  break;
        case 10: id(zone10_relay).turn_off(); break;
        case 11: id(zone11_relay).turn_off(); break;
        default: ESP_LOGW("zone", "zone_relay_off: invalid zone %d", zone); break;
    }
}

static bool zone_relay_state(int zone) {
    switch (zone) {
        case 1:  return id(zone1_relay).state;
        case 2:  return id(zone2_relay).state;
        case 3:  return id(zone3_relay).state;
        case 4:  return id(zone4_relay).state;
        case 5:  return id(zone5_relay).state;
        case 6:  return id(zone6_relay).state;
        case 7:  return id(zone7_relay).state;
        case 8:  return id(zone8_relay).state;
        case 9:  return id(zone9_relay).state;
        case 10: return id(zone10_relay).state;
        case 11: return id(zone11_relay).state;
        default: return false;
    }
}

static bool any_zone_on() {
    for (int i = 1; i <= TOTAL_ZONE_COUNT; i++)
        if (zone_relay_state(i)) return true;
    return false;
}

static const char* zone_name(int zone) {
    switch (zone) {
        case 1:  return "Zone_1";
        case 2:  return "Zone_2";
        case 3:  return "Zone_3";
        case 4:  return "Zone_4";
        case 5:  return "Zone_5";
        case 6:  return "Zone_6";
        case 7:  return "Zone_7";
        case 8:  return "Zone_8";
        case 9:  return "Zone_9";
        case 10: return "Zone_10";
        case 11: return "Tömlő";
        default: return "?";
    }
}

// ============================================================================
// 7. Zone config accessors
// ============================================================================

static int zone_irr_duration_sec(int zone) {
    if (zone < 1 || zone > TOTAL_ZONE_COUNT) return 0;
    return (int)(g_config.zones[zone - 1].irr_duration_min * 60 * DURATION_SCALE);
}

static int zone_short_duration_sec(int zone) {
    if (zone < 1 || zone > TOTAL_ZONE_COUNT) return 0;
    return (int)(g_config.zones[zone - 1].short_duration_min * 60 * DURATION_SCALE);
}

static int zone_manual_duration_sec(int zone) {
    if (zone < 1 || zone > TOTAL_ZONE_COUNT) return (int)(300 * DURATION_SCALE);
    int sec = (int)(g_config.zones[zone - 1].manual_duration_min * 60 * DURATION_SCALE);
    return sec > 0 ? sec : (int)(300 * DURATION_SCALE);
}

static bool zone_irr_enabled(int zone) {
    if (zone < 1 || zone > TOTAL_ZONE_COUNT) return false;
    const ZoneConfig& z = g_config.zones[zone - 1];
    return (z.flags & 0x01) && (z.irr_duration_min > 0);
}

static bool zone_short_enabled(int zone) {
    if (zone < 1 || zone > TOTAL_ZONE_COUNT) return false;
    const ZoneConfig& z = g_config.zones[zone - 1];
    return (z.flags & 0x02) && (z.short_duration_min > 0);
}
