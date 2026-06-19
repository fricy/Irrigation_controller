// ============================================================================
// irrigation_control/config.h
// ============================================================================
// Language-agnostic, zone-agnostic constants.
// Zone counts, X-macros, and zone names: zones/zone_config_N.h
// UI text strings and weekday abbreviations: localisation/config_LL.h
//
// Contents:
//   1. Display constants         - timeout, brightness, screensaver timing
//   2. Icon codepoints           - Material Symbols Outlined
// ============================================================================

#pragma once

// ============================================================================
// 1. Display constants
// ============================================================================
// DISPLAY_TIMEOUT_DEFAULT: default minutes before screensaver activates (0=disabled).
// DISPLAY_NIGHT_START/END: hours (0-23) for display-off window (no cycle running).
// DISPLAY_BRIGHTNESS_FULL/DIM:
// DISPLAY_SCREENSAVER_SWAP_MIN: minutes between schedule/clock view swap.
// DISPLAY_SCREENSAVER_POS_SWAP_MIN: minutes between top/bottom position swap.

#define DISPLAY_TIMEOUT_DEFAULT          5     // minutes to screensaver
#define DISPLAY_NIGHT_START              22    // 22:00 display off
#define DISPLAY_NIGHT_END                5     // 06:00 display back on
#define DISPLAY_BRIGHTNESS_FULL          80    // 0-100, tune for your environment
#define DISPLAY_BRIGHTNESS_DIM           20    // ~10%, tune on device
#define DISPLAY_SCREENSAVER_SWAP_MIN     1     // swap view every 1 min
#define DISPLAY_SCREENSAVER_POS_SWAP_MIN 5     // swap position every 5 min

// ============================================================================
// Developer / test flags
// ============================================================================
// BYPASS_IO_FAULT_CHECK: set to 1 to skip MCP23017 fault blocking at boot and
// in pump_watchdog. Allows cycle/zone start testing on a bare board without the
// relay expander. MUST be 0 in production firmware.
#define BYPASS_IO_FAULT_CHECK            0

// ============================================================================
// 2. Icon codepoints (Material Symbols Outlined)
// ============================================================================
// Edit codepoints here if using a different icon font or glyph set.

static const char* IC_START       = "\ue037";
static const char* IC_STOP        = "\ue047";
static const char* IC_PAUSE       = "\ue034";
static const char* IC_RESUME      = "\uf7d0";
static const char* IC_SKIP        = "\ue044";
static const char* IC_QUEUE       = "\ue16d";
static const char* IC_OK          = "\ue31b";
static const char* IC_SAVE        = "\ue161";
static const char* IC_CANCEL      = "\ue5cd";
static const char* IC_LEFT        = "\ue314";
static const char* IC_RIGHT       = "\ue315";
static const char* IC_UP          = "\ue316";
static const char* IC_DOWN        = "\ue313";
static const char* IC_CHECK       = "\uf1fe";
static const char* IC_VALVE       = "\ue224";
static const char* IC_SPRINKLER   = "\ue29a";
static const char* IC_PUMP        = "\uf5d8";
static const char* IC_SETTINGS    = "\ue869";
static const char* IC_SCHEDULE    = "\ue855";
static const char* IC_ALARM_ADD   = "\ue856";
static const char* IC_ALARM_OFF   = "\ue857";
static const char* IC_CLOCK       = "\ue8b5";
static const char* IC_EDIT        = "\ue3c9";
static const char* IC_MAINTENANCE = "\uea3c";
static const char* IC_PERCENT     = "\ueb58";
static const char* IC_REPEAT      = "\ue040";
static const char* IC_POWERLOSS   = "\ue646";
static const char* IC_WARNING     = "\ue002";
static const char* IC_NET_OK      = "\uef16";
static const char* IC_NET_OFF     = "\ue1da";
static const char* IC_PLUS        = "\uf3dd";
static const char* IC_MINUS       = "\ue15B";
static const char* IC_QUESTION    = "\uf1c0";
static const char* IC_BATT_ERROR  = "\uf7ea";
static const char* IC_REBOOT      = "\uf16f";
static const char* IC_SCREENSAVER = "\ue1c3";
static const char* IC_NIGHT_OFF   = "\ueb76";
static const char* IC_NETWORK     = "\ue1e2";
static const char* IC_WEATHER     = "\ue2bd";
static const char* IC_RAINY       = "\uf176";
static const char* IC_SUNNY       = "\uf172";