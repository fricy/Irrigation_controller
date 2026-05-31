// ============================================================================
// localisation/config_en.h  —  English UI strings
// ============================================================================
// All TXT_* display strings and weekday abbreviations in English.
// Included before helpers.h. Icon constants and display constants are in
// the shared config.h.
//
// NOTE: Short strings required — OLED display is 128px wide at ~8px/char
// gives ~16 chars max per line at font_main size. Tooltips ~20 chars max.
// Zone names live in zones/zone_config_N.h, not here.
// ============================================================================

#pragma once

// Weekday abbreviations (Sunday=0 .. Saturday=6)
static const char* const WEEKDAY_ABBR[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};

static const char* TXT_CYCLE_IRR            = "Irrigation";
static const char* TXT_CYCLE_SHORT          = "Short";
static const char* TXT_MANUAL               = "Manual zone";
static const char* TXT_SETTINGS             = "Settings";
static const char* TXT_SCHEDULES            = "Schedule";
static const char* TXT_SCHED_IRR1           = "Irrigation";
static const char* TXT_SCHED_IRR2           = "Irrigation";
static const char* TXT_SCHED_SHORT1         = "Short";
static const char* TXT_SCHED_SHORT2         = "Short";
static const char* TXT_DISABLE_ALL          = "All OFF";  //stub
static const char* TXT_ENABLE_ALL           = "All ON"; //stub
static const char* TXT_ZONE                 = "Zone";
static const char* TXT_PAUSED               = "PAUSED";
static const char* TXT_PAUSE_TIME           = "Paused:";
static const char* TXT_LOCKOUT              = "LOCKOUT";
static const char* TXT_POWERLOSS            = "Power loss";
static const char* TXT_RESUME               = "Resume?";
static const char* TXT_PUMP_ERR             = "PUMP ERROR";
static const char* TXT_NO_SCHED             = "No schedule";
static const char* TXT_MINUTES_ABBR         = " m";
static const char* TXT_LESS_1MIN            = "< 1 m";

static const char* TXT_FAULT_MAINTENANCE_L1 = "Maintenance";
static const char* TXT_FAULT_MAINTENANCE_L2 = "Start blocked";
static const char* TXT_FAULT_IO             = "RELAY ERROR";
static const char* TXT_FAULT_OLED           = "DISPLAY ERROR";
static const char* TXT_FAULT_RTC            = "CLOCK ERROR";
static const char* TXT_FAULT_BATT           = "BATTERY ERROR";
static const char* TXT_FAULT_HELP_1         = "Seek assistance";
static const char* TXT_FAULT_HELP_2         = "if error recurs";
static const char* TXT_FAULT_BAT_TYPE       = "Battery: CR2032";

static const char* TXT_SETTINGS_WEATHER     = "Weather";
static const char* TXT_SETTINGS_VALVE       = "Solenoid";
static const char* TXT_SETTINGS_ADV         = "System";
static const char* TXT_SETTINGS_OLED        = "Display";

static const char* TXT_ADV_SCALE_ENABLE     = "Apply scale";
static const char* TXT_ADV_REPEAT           = "Repeat";
static const char* TXT_ADV_SCALE            = "Dur. scale";
static const char* TXT_ADV_SCALE_RESET      = "Scale reset";
static const char* TXT_ADV_PUMP_START       = "Start offset";
static const char* TXT_ADV_PUMP_STOP        = "Stop offset";
static const char* TXT_ADV_ZONE_DELAY       = "Zone delay";
static const char* TXT_ADV_LOCKOUT          = "Protection";
static const char* TXT_ADV_AUTO_RESUME      = "Auto resume";
static const char* TXT_ADV_NET              = "Network";
static const char* TXT_ADV_DATETIME         = "Date/Time";
static const char* TXT_ADV_MAINTENANCE      = "Maintenance";
static const char* TXT_ADV_DISP_TIMEOUT     = "Screensaver";
static const char* TXT_ADV_NIGHT_OFF        = "Night mode";

static const char* TXT_ADV_SCALE_ENABLE_TOOLTIP = "1: irr; 3: both";
static const char* TXT_ADV_REPEAT_TOOLTIP       = "Repeat irrigation";
static const char* TXT_ADV_SCALE_TOOLTIP        = "Duration multiplier";
static const char* TXT_ADV_SCALE_RESET_TOOLTIP  = "Reset scale (days)";
static const char* TXT_ADV_PUMP_START_TOOLTIP   = "Valve open delay";
static const char* TXT_ADV_PUMP_STOP_TOOLTIP    = "Valve close delay";
static const char* TXT_ADV_ZONE_DELAY_TOOLTIP   = "Zone switch delay";
static const char* TXT_ADV_LOCKOUT_TOOLTIP      = "Pump cooldown time";
static const char* TXT_ADV_AUTO_RESUME_TOOLTIP  = "Resume: auto/confirm";
static const char* TXT_ADV_NET_TOOLTIP          = "Switch Wifi/Zb";
static const char* TXT_ADV_DATETIME_TOOLTIP     = "Set date and time";
static const char* TXT_ADV_MAINTENANCE_TOOLTIP  = "Block pump start";
static const char* TXT_ADV_DISP_TIMEOUT_TOOLTIP = "Screensaver (min)";
static const char* TXT_ADV_NIGHT_OFF_TOOLTIP    = "Display off at night";
