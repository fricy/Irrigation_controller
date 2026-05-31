// ============================================================================
// zones/zone_config_15.h  —  15-zone variant
// ============================================================================
// Zone count defines, X-macro lists, and zone name strings.
// Included before helpers.h and config.h.
//
// Hardware: PA6-PA0 (Zones 1-7) + PB0-PB7 (Zones 8-15) + PA7 (Pump)
// All 15 zones participate in cycles.
//
// NOTE: 15-zone build requires APS binding table verification before release.
// Default table = 16 slots; 15 zone EPs + 2 cycle EPs = 17 binds needed.
// Test esp_zb_aps_src_binding_table_size_set() or reduce cycle EP binds.
// ============================================================================

#pragma once

// ============================================================================
// 1. Zone count defines
// ============================================================================

#define TOTAL_ZONE_COUNT 15 // All zones including manual-only
#define CYCLE_ZONE_COUNT 15 // Zones that participate in irrigation/short cycles

static_assert(CYCLE_ZONE_COUNT <= TOTAL_ZONE_COUNT,
    "CYCLE_ZONE_COUNT cannot exceed TOTAL_ZONE_COUNT");

// ============================================================================
// 2. Zone X-macro lists
// ============================================================================

#define ZONE_CYCLE_LIST \
    X(1)  X(2)  X(3)  X(4)  X(5) \
    X(6)  X(7)  X(8)  X(9)  X(10) \
    X(11) X(12) X(13) X(14) X(15)

#define ZONE_MANUAL_LIST \
    X(1)  X(2)  X(3)  X(4)  X(5) \
    X(6)  X(7)  X(8)  X(9)  X(10) \
    X(11) X(12) X(13) X(14) X(15)

// ============================================================================
// 3. Zone name strings
// ============================================================================

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
        case 12: return "Zone_12";
        case 13: return "Zone_13";
        case 14: return "Zone_14";
        case 15: return "Zone_15";
        default: return "?";
    }
}
