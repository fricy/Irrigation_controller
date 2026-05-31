// ============================================================================
// irrigation_control/helpers.h
// ============================================================================
// Technical helpers — config struct, persistence, zone accessors, relay list,
// and Zigbee attribute arrays. Not user-editable; see config.h for that.
//
// Contents:
//   1. Includes / forward declarations
//   2. Config struct             - IrrigationConfig, ZoneConfig, ScheduleConfig
//   3. Config checksum + magic
//   4. Config defaults
//   5. Config validation         - config_valid()
//   6. Config persistence        - config_load(), config_save()
//   7. Zone relay helpers        - relay list array, zone_relay_on/off/state(),
//                                  any_zone_on(), stop_all_zones_inline()
//   8. Fault status codes        - FAULT_NONE/RTC_BATT/RTC/OLED/IO
//   9. Zone config accessors     - zone_irr/short/manual_duration_sec(),
//                                  zone_irr/short_enabled()
//  10. Zigbee attribute arrays   - zb_irr/short/flags/manual_attrs[],
//                                  zb_init_attr_arrays()
//  10a. Zone ZB switch array     - zone_zb_list[], zone_zb_init()
// ============================================================================

#pragma once
#include "esphome.h"

// config.h must be included before helpers.h (provides IC_* and display constants)
// zone_config_N.h must be included before helpers.h (provides zone counts and X-macros)
// localisation/config_LL.h must be included before helpers.h (provides TXT_* strings)
// display.h must be included after helpers.h (uses types defined here)

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

struct IrrigationConfig {
    uint32_t       magic;
    uint8_t        pump_lockout_sec;
    int8_t         pump_start_offset;
    int8_t         pump_stop_offset;
    uint8_t        duration_scale_percent;
    uint8_t        scale_reset_days;         // 0 = no auto-reset; 1-14 days
    uint8_t        scale_enable;             // 1=irr only, 2=short only, 3=both (default 3)
    uint8_t        cycle_repeat_count;       // stored 0-2, displayed as 1-3
    uint8_t        zone_switch_delay_sec;
    uint8_t        auto_resume_on_powerloss; // 0=show PAGE_POWERLOSS, 1=auto-resume
    uint8_t        maintenance_lock;         // 0=normal, 1=maintenance active (blocks cycles)
    uint8_t        display_timeout_min;      // 0=disabled, 1-10: minutes to screensaver
    uint8_t        display_night_off;        // 0=disabled, 1=display off during night window
    uint8_t        frequency_enable;         // stub: 1=irr only, 2=short only, 3=both (default 3)
                                             // controls which cycle types are affected by cycle_frequency
                                             // full implementation: see TODO list
    uint8_t        cycle_frequency;          // stub: run cycle every Nth day (1=every day, default)
                                             // full implementation: see TODO list
    ZoneConfig     zones[TOTAL_ZONE_COUNT];
    ScheduleConfig irr_schedule1;
    ScheduleConfig irr_schedule2;
    ScheduleConfig short_schedule1;
    ScheduleConfig short_schedule2;
    uint8_t        checksum;
};

// ============================================================================
// 3. Config checksum
// ============================================================================

static uint8_t config_checksum(const IrrigationConfig& cfg) {
    const uint8_t* p = reinterpret_cast<const uint8_t*>(&cfg);
    uint8_t xor_val = 0;
    for (size_t i = 0; i < offsetof(IrrigationConfig, checksum); i++)
        xor_val ^= p[i];
    return xor_val;
}

// ============================================================================
// 4. Config defaults
// ============================================================================

static IrrigationConfig config_defaults() {
    IrrigationConfig cfg = {};
    cfg.magic                    = CONFIG_MAGIC;
    cfg.pump_lockout_sec         = 30;
    cfg.pump_start_offset        = 2;
    cfg.pump_stop_offset         = 1;
    cfg.duration_scale_percent   = 100;
    cfg.scale_reset_days         = 0;
    cfg.scale_enable             = 3;   // both cycles scaled by default
    cfg.cycle_repeat_count       = 0;   // stored 0-2: display as 1-3 (1=no repeat)
    cfg.zone_switch_delay_sec    = 0;
    cfg.auto_resume_on_powerloss = 0;
    cfg.maintenance_lock         = 0;   // false: irrigation allowed
    cfg.display_timeout_min      = DISPLAY_TIMEOUT_DEFAULT;
    cfg.display_night_off        = 1;
    cfg.frequency_enable         = 3;   // stub: both cycles (full impl deferred)
    cfg.cycle_frequency          = 1;   // stub: every day (full impl deferred)

    for (int i = 0; i < TOTAL_ZONE_COUNT; i++) {
        int zone_num  = i + 1;
        bool wired    = !(zone_num == 0);
        bool in_cycle = wired && (zone_num <= CYCLE_ZONE_COUNT);
        cfg.zones[i].irr_duration_min    = in_cycle ? 15 : 0;
        cfg.zones[i].short_duration_min  = in_cycle ? 10 : 0;
        cfg.zones[i].manual_duration_min = wired    ? 05 : 0;
        cfg.zones[i].flags               = in_cycle ? 0x03 : 0x00;
    }

    cfg.irr_schedule1   = {1,  5,  0};
    cfg.irr_schedule2   = {0,  23,  0};  // disabled by default
    cfg.short_schedule1 = {1, 12, 00};
    cfg.short_schedule2 = {1, 18,  0};
    cfg.checksum = config_checksum(cfg);
    return cfg;
}

// ============================================================================
// 5. Config validation
// ============================================================================

static bool config_valid(const IrrigationConfig& cfg) {
    if (cfg.magic    != CONFIG_MAGIC)         return false;
    if (cfg.checksum != config_checksum(cfg)) return false;
    for (int i = 0; i < TOTAL_ZONE_COUNT; i++) {
        if (cfg.zones[i].irr_duration_min    > 60) return false;
        if (cfg.zones[i].short_duration_min  > 60) return false;
        if (cfg.zones[i].manual_duration_min > 60) return false;
    }
    if (cfg.duration_scale_percent > 200) return false;
    if (cfg.scale_reset_days  > 14)       return false;
    if (cfg.scale_enable < 1 || cfg.scale_enable > 3) return false;
    if (cfg.pump_start_offset < -5 || cfg.pump_start_offset > 5) return false;
    if (cfg.pump_stop_offset  < -5 || cfg.pump_stop_offset  > 5) return false;
    if (cfg.cycle_repeat_count > 2)       return false;
    if (cfg.zone_switch_delay_sec > 5)    return false;
    if (cfg.display_timeout_min > 10)     return false;
    if (cfg.duration_scale_percent > 0 && cfg.duration_scale_percent < 10) return false;
    auto sched_ok = [](const ScheduleConfig& s) {
        return s.hour < 24 && s.minute < 60;
    };
    if (!sched_ok(cfg.irr_schedule1))   return false;
    if (!sched_ok(cfg.irr_schedule2))   return false;
    if (!sched_ok(cfg.short_schedule1)) return false;
    if (!sched_ok(cfg.short_schedule2)) return false;
    return true;
}

// ============================================================================
// 6. Config persistence
// ============================================================================
// Increment last byte of CONFIG_PREF_HASH when struct layout changes
// to force defaults reload on next boot.

static const uint32_t CONFIG_PREF_HASH = 0xAB01000D;

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
// 7. Zone relay helpers
// ============================================================================

static esphome::switch_::Switch* zone_relay_list[TOTAL_ZONE_COUNT];
static bool zone_relay_list_initialized = false;

static void zone_relay_init() {
    if (zone_relay_list_initialized) return;
    int i = 0;
    #define X(n) zone_relay_list[i++] = &id(zone##n##_relay);
    ZONE_MANUAL_LIST
    #undef X
    zone_relay_list_initialized = true;
}

static void zone_relay_on(int zone) {
    zone_relay_init();
    if (zone < 1 || zone > TOTAL_ZONE_COUNT) {
        ESP_LOGW("zone", "zone_relay_on: invalid zone %d", zone);
        return;
    }
    zone_relay_list[zone - 1]->turn_on();
}

static void zone_relay_off(int zone) {
    zone_relay_init();
    if (zone < 1 || zone > TOTAL_ZONE_COUNT) {
        ESP_LOGW("zone", "zone_relay_off: invalid zone %d", zone);
        return;
    }
    zone_relay_list[zone - 1]->turn_off();
}

static bool zone_relay_state(int zone) {
    zone_relay_init();
    if (zone < 1 || zone > TOTAL_ZONE_COUNT) return false;
    return zone_relay_list[zone - 1]->state;
}

static bool any_zone_on() {
    for (int i = 1; i <= TOTAL_ZONE_COUNT; i++)
        if (zone_relay_state(i)) return true;
    return false;
}

static void stop_all_zones_inline() {
    zone_relay_init();
    for (int i = 0; i < TOTAL_ZONE_COUNT; i++)
        zone_relay_list[i]->turn_off();
}

// ============================================================================
// 8. Fault status codes (for EP14 zb_fault_state attr 0x0014)
// ============================================================================
// Reported as U8 to Z2M. If multiple faults active: highest code wins.
#define FAULT_NONE          0
#define FAULT_RTC_BATT      1
#define FAULT_RTC           2
#define FAULT_OLED          3
#define FAULT_IO            4

// ============================================================================
// 9. Zone config accessors
// ============================================================================
// zone_irr_duration_sec applies scale only when scale_enable includes irr (bit0).
// zone_short_duration_sec applies scale only when scale_enable includes short (bit1).

static int zone_irr_duration_sec(int zone) {
    if (zone < 1 || zone > TOTAL_ZONE_COUNT) return 0;
    int base = g_config.zones[zone - 1].irr_duration_min * 60;
    if (g_config.scale_enable == 1 || g_config.scale_enable == 3)
        return base * g_config.duration_scale_percent / 100;
    return base;
}

static int zone_short_duration_sec(int zone) {
    if (zone < 1 || zone > TOTAL_ZONE_COUNT) return 0;
    int base = g_config.zones[zone - 1].short_duration_min * 60;
    if (g_config.scale_enable == 2 || g_config.scale_enable == 3)
        return base * g_config.duration_scale_percent / 100;
    return base;
}

static int zone_manual_duration_sec(int zone) {
    if (zone < 1 || zone > TOTAL_ZONE_COUNT) return 300;
    int sec = g_config.zones[zone - 1].manual_duration_min * 60;
    return sec > 0 ? sec : 300;
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

// ============================================================================
// 10. Zigbee attribute arrays
// ============================================================================

static esphome::zigbee::ZigBeeAttribute* zb_irr_attrs[CYCLE_ZONE_COUNT];
static esphome::zigbee::ZigBeeAttribute* zb_short_attrs[CYCLE_ZONE_COUNT];
static esphome::zigbee::ZigBeeAttribute* zb_flags_attrs[CYCLE_ZONE_COUNT];
static esphome::zigbee::ZigBeeAttribute* zb_manual_attrs[TOTAL_ZONE_COUNT];
static bool zb_attr_arrays_initialized = false;

static void zb_init_attr_arrays() {
    if (zb_attr_arrays_initialized) return;
    int i;
    i = 0;
    #define X(n) zb_irr_attrs[i++]    = &id(z##n##_irr);
    ZONE_CYCLE_LIST
    #undef X
    i = 0;
    #define X(n) zb_short_attrs[i++]  = &id(z##n##_short);
    ZONE_CYCLE_LIST
    #undef X
    i = 0;
    #define X(n) zb_flags_attrs[i++]  = &id(z##n##_flags);
    ZONE_CYCLE_LIST
    #undef X
    i = 0;
    #define X(n) zb_manual_attrs[i++] = &id(z##n##_manual);
    ZONE_MANUAL_LIST
    #undef X
    zb_attr_arrays_initialized = true;
}

// ============================================================================
// 10a. Zone ZB switch pointer array
// ============================================================================
// Mirrors zone_relay_list[] pattern for zone Zigbee template switches.
// Used by sync_zone_zb_state to iterate all zones without hardcoded IDs.
// To add a zone: add entry to ZONE_MANUAL_LIST macro in config.h (existing).

static esphome::template_::TemplateSwitch* zone_zb_list[TOTAL_ZONE_COUNT];
static bool zone_zb_list_initialized = false;

static void zone_zb_init() {
    if (zone_zb_list_initialized) return;
    int i = 0;
    #define X(n) zone_zb_list[i++] = &id(zone##n##_zb);
    ZONE_MANUAL_LIST
    #undef X
    zone_zb_list_initialized = true;
}
