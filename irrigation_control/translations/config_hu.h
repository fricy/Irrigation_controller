// ============================================================================
// localisation/config_hu.h  —  Hungarian UI strings
// ============================================================================
// All TXT_* display strings and weekday abbreviations in Hungarian.
// Included before helpers.h. Icon constants and display constants are in
// the shared config.h.
// ============================================================================

#pragma once

// Weekday abbreviations (Sunday=0 .. Saturday=6)
static const char* const WEEKDAY_ABBR[] = {"Va", "Hé", "Ke", "Sz", "Cs", "Pé", "Sz"};

static const char* TXT_CYCLE_IRR            = "Öntözés";
static const char* TXT_CYCLE_SHORT          = "Rövid";
static const char* TXT_MANUAL               = "Zóna indítás";
static const char* TXT_SETTINGS             = "Beállítások";
static const char* TXT_SCHEDULES            = "Időzítő";
static const char* TXT_SCHED_IRR1           = "Öntözés";
static const char* TXT_SCHED_IRR2           = "Öntözés";
static const char* TXT_SCHED_SHORT1         = "Rövid";
static const char* TXT_SCHED_SHORT2         = "Rövid";
static const char* TXT_DISABLE_ALL          = "Mind KI";
static const char* TXT_ENABLE_ALL           = "Mind BE";
static const char* TXT_ZONE                 = "Zóna";
static const char* TXT_PAUSED               = "SZÜNET";
static const char* TXT_PAUSE_TIME           = "Megállítva:";
static const char* TXT_LOCKOUT              = "ZÁROLT";
static const char* TXT_POWERLOSS            = "Áramszünet";
static const char* TXT_RESUME               = "Folytat?";
static const char* TXT_PUMP_ERR             = "SZIVATTYÚ HIBA";
static const char* TXT_NO_SCHED             = "Nincs ütemezett";
static const char* TXT_MINUTES_ABBR         = " p";
static const char* TXT_LESS_1MIN            = "< 1 p";

static const char* TXT_FAULT_MAINTENANCE_L1 = "Karbantartás";
static const char* TXT_FAULT_MAINTENANCE_L2 = "Indítás letiltva";
static const char* TXT_FAULT_IO             = "RELÉ HIBA";
static const char* TXT_FAULT_OLED           = "KIJELŐ HIBA";
static const char* TXT_FAULT_RTC            = "ÓRA HIBA";
static const char* TXT_FAULT_BATT           = "ELEM HIBA";
static const char* TXT_FAULT_BLOCKED        = "HIBÁS CIKLUS";
static const char* TXT_FAULT_HELP_1         = "Kérj segítséget";
static const char* TXT_FAULT_HELP_2         = "ha a hiba visszatér";
static const char* TXT_FAULT_BAT_TYPE       = "Elem: CR2032";
static const char* TXT_FAULT_BLOCK_1        = "Skálázás: 0%";
static const char* TXT_FAULT_BLOCK_2        = "Hiányzó zónák";

static const char* TXT_SETTINGS_WEATHER     = "Időjárás";
static const char* TXT_SETTINGS_VALVE       = "Szolenoid";
static const char* TXT_SETTINGS_ADV         = "Rendszer";
static const char* TXT_SETTINGS_OLED        = "Kijelző";

static const char* TXT_ADV_SCALE_ENABLE     = "Szorzó ciklus";
static const char* TXT_ADV_REPEAT           = "Ismétlés";
static const char* TXT_ADV_SCALE            = "Időszorzó";
static const char* TXT_ADV_SCALE_RESET      = "Szorz. alap";
static const char* TXT_ADV_PUMP_START       = "Ind. késlelt.";
static const char* TXT_ADV_PUMP_STOP        = "Zár. késlelt.";
static const char* TXT_ADV_ZONE_DELAY       = "Sz. késlelt.";
static const char* TXT_ADV_LOCKOUT          = "Védelem";
static const char* TXT_ADV_AUTO_RESUME      = "Áramsz. folyt.";
static const char* TXT_ADV_NET              = "Hálózat";
static const char* TXT_ADV_NET_TOOLTIP      = "Hálózat váltó (Wifi/Zb)";
static const char* TXT_ADV_DATETIME         = "Dátum/Idő";
static const char* TXT_ADV_MAINTENANCE      = "Karbantartás";
static const char* TXT_ADV_DISP_TIMEOUT     = "Oled kímélő";
static const char* TXT_ADV_NIGHT_OFF        = "Éjszakai mód";

static const char* TXT_ADV_SCALE_ENABLE_TOOLTIP = "Szorzó érintett ciklusok";
static const char* TXT_ADV_REPEAT_TOOLTIP       = "Öntözés ismétlése";
static const char* TXT_ADV_SCALE_TOOLTIP        = "Öntözési hossz szorzó";
static const char* TXT_ADV_SCALE_RESET_TOOLTIP  = "Szorzó alapállásba(nap)";
static const char* TXT_ADV_PUMP_START_TOOLTIP   = "Szelepindító késleltetés";
static const char* TXT_ADV_PUMP_STOP_TOOLTIP    = "Szelepzáró késleltetés";
static const char* TXT_ADV_ZONE_DELAY_TOOLTIP   = "Szelepváltás késletetés";
static const char* TXT_ADV_LOCKOUT_TOOLTIP      = "Szivattyú hűtési idő";
static const char* TXT_ADV_AUTO_RESUME_TOOLTIP  = "Folytat: (auto/kérd.)";
static const char* TXT_ADV_DATETIME_TOOLTIP     = "Dátum beállítása";
static const char* TXT_ADV_MAINTENANCE_TOOLTIP  = "Szivattyú indítás tiltása";
static const char* TXT_ADV_DISP_TIMEOUT_TOOLTIP = "Képernyőkímélő (perc)";
static const char* TXT_ADV_NIGHT_OFF_TOOLTIP    = "Kijelző kikapcsolása";
