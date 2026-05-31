// ============================================================================
// zones/zone_config_11.h  —  11-zone variant
// ============================================================================
// Zone count defines, X-macro lists, and zone name strings.
// Included before helpers.h and config.h.
//
// Hardware: PA6-PA0 (Zones 1-7) + PB4-PB7 (Zones 8-11) + PA7 (Pump)
// Zone 11: manual-only (no cycle participation)
// ============================================================================

#pragma once

// ============================================================================
// 1. Zone count defines
// ============================================================================

#define TOTAL_ZONE_COUNT 11   // All zones including manual-only
#define CYCLE_ZONE_COUNT 11   // Zones that participate in irrigation/short cycles

static_assert(CYCLE_ZONE_COUNT <= TOTAL_ZONE_COUNT,
    "CYCLE_ZONE_COUNT cannot exceed TOTAL_ZONE_COUNT");

// ============================================================================
// 2. Zone X-macro lists
// ============================================================================
// ZONE_CYCLE_LIST: zones in irrigation/short cycles (CYCLE_ZONE_COUNT entries)
// ZONE_MANUAL_LIST: all zones including manual-only (TOTAL_ZONE_COUNT entries)

#define ZONE_CYCLE_LIST \
    X(1)  X(2)  X(3)  X(4)  X(5) \
    X(6)  X(7)  X(8)  X(9)  X(10) \
    X(11)

#define ZONE_MANUAL_LIST \
    X(1)  X(2)  X(3)  X(4)  X(5) \
    X(6)  X(7)  X(8)  X(9)  X(10) \
    X(11)

// ============================================================================
// 3. Zone name strings
// ============================================================================
// Edit these for your deployment. Shown on the OLED display only.
// Default durations and wiring flags are set in config_defaults() in helpers.h.

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
        case 11: return "Zone_11";
        default: return "?";
    }
}
