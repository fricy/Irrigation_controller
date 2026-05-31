// ============================================================================
// zones/zone_config_7.h  —  7-zone variant
// ============================================================================
// Zone count defines, X-macro lists, and zone name strings.
// Included before helpers.h and config.h.
//
// Hardware: PA6-PA0 (Zones 1-7) + PA7 (Pump)
// All 7 zones participate in cycles.
// ============================================================================

#pragma once

// ============================================================================
// 1. Zone count defines
// ============================================================================

#define TOTAL_ZONE_COUNT 7 // All zones including manual-only
#define CYCLE_ZONE_COUNT 7 // Zones that participate in irrigation/short cycles

static_assert(CYCLE_ZONE_COUNT <= TOTAL_ZONE_COUNT,
    "CYCLE_ZONE_COUNT cannot exceed TOTAL_ZONE_COUNT");

// ============================================================================
// 2. Zone X-macro lists
// ============================================================================

#define ZONE_CYCLE_LIST \
    X(1)  X(2)  X(3)  X(4)  X(5)  X(6)  X(7)

#define ZONE_MANUAL_LIST \
    X(1)  X(2)  X(3)  X(4)  X(5)  X(6)  X(7)

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
        default: return "?";
    }
}
