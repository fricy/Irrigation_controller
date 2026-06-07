// ============================================================================
// irrigation_control/display.h
// ============================================================================
// OLED display draw functions. Not user-editable; see config.h for strings
// and icons, helpers.h for data accessors.
//
// Contents:
//   1. Page enum                 - IrrigationPage
//   2. Layout helpers            - draw_header(), draw_footer(), draw_navbar()
//   3. Page draw functions       - one per IrrigationPage value
//
// Layout constants (SH1107 128×128):
//   Header:    y=0..14   (font_header size 12)
//   Separator: y=15
//   Content:   y=16..93  (4 rows × ~21px each)
//   Navbar:    y=94..108 (font_header)
//   Separator: y=109
//   Footer:    y=110..127 (font_icons_footer size 18)
//
// Footer button x positions:
//   btn1 (left):   x=4
//   btn2 (center): x=68, TextAlign::TOP_CENTER
//   btn3 (right):  x=124, TextAlign::TOP_RIGHT
// ============================================================================

#pragma once

// ============================================================================
// 1. Page enum
// ============================================================================

enum IrrigationPage {
    PAGE_IDLE             = 0,
    PAGE_CYCLE_RUNNING    = 1,
    PAGE_MANUAL_IDLE      = 2,
    PAGE_MANUAL_RUNNING   = 3,
    PAGE_PAUSE            = 4,
    PAGE_LOCKOUT          = 5,
    PAGE_POWERLOSS        = 6,
    PAGE_PUMP_FAULT       = 7,
    PAGE_SETTINGS_MAIN    = 8,
    PAGE_SETTINGS_SCHED   = 9,
    PAGE_SETTINGS_IRR     = 10,
    PAGE_SETTINGS_SHORT   = 11,
    PAGE_SETTINGS_ADV     = 12,
    PAGE_SETTINGS_DATETIME = 13,
    PAGE_SCREENSAVER        = 14,
    PAGE_SETTINGS_WEATHER   = 15,
    PAGE_SETTINGS_VALVE     = 16,
    PAGE_SETTINGS_OLED      = 17,
};

// ============================================================================
// Edit step-size helper
// ============================================================================
// Returns hold-repeat step size for a given page and cursor position.
// Lives here (after IrrigationPage enum) so PAGE_* constants are visible.
// B2 passes result as negative delta, B3 as positive.

static int calc_edit_step(int page, int cursor) {
    if (page == PAGE_MANUAL_RUNNING)                                           return 5;
    if (page == PAGE_PAUSE)                                                    return 5;
    if (page == PAGE_SETTINGS_SCHED   && cursor % 3 == 2)                     return 5;
    if ((page == PAGE_SETTINGS_IRR || page == PAGE_SETTINGS_SHORT)
        && cursor % 2 == 1)                                                    return 5;
    if (page == PAGE_SETTINGS_WEATHER && cursor == 0)                          return 10;
    if (page == PAGE_SETTINGS_ADV     && cursor == 2)                          return 5;
    if (page == PAGE_SETTINGS_DATETIME
        && (cursor == 0 || cursor == 3 || cursor == 4))                        return 5;
    return 1;
}

// ============================================================================
// 2. Layout helpers
// ============================================================================

static void draw_selection_rect(esphome::display::DisplayBuffer& it, int row_y, int row_h) {
    it.rectangle(0, row_y - 1, 128, row_h + 2);
}

// set_display_brightness: sets SH1107 contrast register via set_contrast().
// DISPLAY_BRIGHTNESS_FULL (100) = full, DISPLAY_BRIGHTNESS_DIM (20) = 20%.
static void set_display_brightness(uint8_t level) {
    id(oled_display).set_contrast(level / 100.f);
}

static void draw_header(esphome::display::DisplayBuffer& it,
                        esphome::ESPTime& t,
                        bool zigbee_ok) {
    char time_str[6], date_str[16];
    if (t.is_valid()) {
        snprintf(time_str, sizeof(time_str), "%02d:%02d", t.hour, t.minute);
        const char* wd = (t.day_of_week >= 1 && t.day_of_week <= 7)
                         ? WEEKDAY_ABBR[t.day_of_week - 1] : "--";
        snprintf(date_str, sizeof(date_str), "%s %s %02d",
                 wd, t.strftime("%b").c_str(), t.day_of_month);
    } else {
        strcpy(time_str, "--:--");
        strcpy(date_str, "-- --- --");
    }
    it.printf(2, 1, &id(font_header), "%s", time_str);
    it.printf(38, 1, &id(font_header), "%s", date_str);
    // Right side: network icon always last (x=126).
    // Second icon (x=112): highest-priority status indicator, one at a time.
    //   Priority: io_fault > maintenance > wifi_ota_active > queue > rtc_batt/rtc_fault
    it.print(126, 1, &id(font_icons_header), esphome::display::TextAlign::TOP_RIGHT,
             zigbee_ok ? IC_NET_OK : IC_NET_OFF);
    const char* status_icon = nullptr;
    if (id(io_fault))
        status_icon = IC_WARNING;
    else if (g_config.maintenance_lock)
        status_icon = IC_MAINTENANCE;
    else if (id(queued_cycle_num) > 0)
        status_icon = IC_QUEUE;
    else if (g_config.duration_scale_percent > 100)
        status_icon = IC_SUNNY;
    else if (g_config.duration_scale_percent < 100)
        status_icon = IC_RAINY;
    else if (id(rtc_battery_low) || id(rtc_fault))
        status_icon = IC_BATT_ERROR;
    if (status_icon)
        it.print(112, 1, &id(font_icons_header), esphome::display::TextAlign::TOP_RIGHT, status_icon);
    it.line(0, 15, 128, 15);
}

static void draw_footer_separator(esphome::display::DisplayBuffer& it) {
    it.line(0, 109, 128, 109);
}

static void draw_footer(esphome::display::DisplayBuffer& it,
                        const char* btn1_icon,
                        const char* btn2_icon,
                        const char* btn3_icon) {
    if (btn1_icon && btn1_icon[0])
        it.print(4, 110, &id(font_icons_footer), btn1_icon);
    if (btn2_icon && btn2_icon[0])
        it.print(68, 110, &id(font_icons_footer),
                 esphome::display::TextAlign::TOP_CENTER, btn2_icon);
    if (btn3_icon && btn3_icon[0])
        it.print(124, 110, &id(font_icons_footer),
                 esphome::display::TextAlign::TOP_RIGHT, btn3_icon);
}

static void draw_navbar(esphome::display::DisplayBuffer& it, int selected) {
    struct NavEntry { const char* icon; const char* name; };
    static const NavEntry pages[] = {
        { IC_SPRINKLER,   TXT_CYCLE_IRR   },
        { IC_SPRINKLER,   TXT_CYCLE_SHORT },
        { IC_VALVE,       TXT_MANUAL      },
        { IC_MAINTENANCE, TXT_SETTINGS    },
    };
    const int N = 4;
    int prev = (selected + N - 1) % N;
    int next = (selected + 1) % N;
    int y = 94;
    it.print(0,   y, &id(font_icons_header), pages[prev].icon);
    it.print(12,  y, &id(font_icons_header), IC_LEFT);
    it.printf(64, y, &id(font_header), esphome::display::TextAlign::TOP_CENTER,
              "%s", pages[selected].name);
    it.print(104, y, &id(font_icons_header), IC_RIGHT);
    it.print(116, y, &id(font_icons_header), pages[next].icon);
}

// ============================================================================
// 3. Page draw functions
// ============================================================================

static void draw_page_idle(esphome::display::DisplayBuffer& it) {
    it.print(0, 18, &id(font_icons_main), IC_SCHEDULE);
    it.print(126, 20, &id(font_main), esphome::display::TextAlign::TOP_RIGHT,
             id(next_schedule_text));

    int nc = id(next_schedule_cycle);
    const char* sched_name = (nc == 1) ? TXT_CYCLE_IRR
                           : (nc == 2) ? TXT_CYCLE_SHORT
                           : TXT_NO_SCHED;
    it.printf(0, 45, &id(font_main), "%s", sched_name);

    if (nc > 0) {
        int enabled = 0;
        for (int i = 1; i <= CYCLE_ZONE_COUNT; i++) {
            bool en = (nc == 1) ? zone_irr_enabled(i) : zone_short_enabled(i);
            if (en) enabled++;
        }
        it.printf(0, 66, &id(font_main), "%d/%d %s", enabled, CYCLE_ZONE_COUNT, TXT_ZONE);
        // Show repeat count right-justified on same line if configured.
        // IC_REPEAT icon (font_icons_main) + count (font_main), right-justified as a block.
        if (g_config.cycle_repeat_count > 0) {
            char rep_str[4];
            snprintf(rep_str, sizeof(rep_str), "%d", (int)g_config.cycle_repeat_count + 1);
            int x1, y1, tw, th;
            it.get_text_bounds(0, 0, rep_str, &id(font_main),
                               esphome::display::TextAlign::TOP_LEFT, &x1, &y1, &tw, &th);
            // icon ~15px wide at size 15, 1px gap
            it.printf(126, 66, &id(font_main), esphome::display::TextAlign::TOP_RIGHT,
                      "%s", rep_str);
            it.print(126 - tw - 1, 66, &id(font_icons_main),
                     esphome::display::TextAlign::TOP_RIGHT, IC_REPEAT);
        }
    }

    int nav_idx = id(selected_cycle_num) - 1;
    if (nav_idx < 0) nav_idx = 0;
    if (nav_idx > 3) nav_idx = 3;
    draw_navbar(it, nav_idx);
    draw_footer_separator(it);

    // Footer B1 icon: IC_START for cycle start (irr/short), IC_VALVE for manual, IC_MAINTENANCE for lockout
    static const char* page_icons[] = { IC_START, IC_START, IC_VALVE, IC_MAINTENANCE };
    draw_footer(it, page_icons[nav_idx], IC_LEFT, IC_RIGHT);
}

static void draw_page_cycle_running(esphome::display::DisplayBuffer& it) {
    const char* cycle_name = (id(current_cycle_num) == 1) ? TXT_CYCLE_IRR : TXT_CYCLE_SHORT;
    int cr = id(cycle_remaining_sec);
    int zr = id(zone_remaining_sec);

    it.print(0, 20, &id(font_icons_main), IC_SPRINKLER);
    it.printf(18, 22, &id(font_main), "%s", cycle_name);
    if (cr < 60)
        it.print(126, 22, &id(font_main), esphome::display::TextAlign::TOP_RIGHT, TXT_LESS_1MIN);
    else {
        int h = cr / 3600;
        int m = (cr % 3600) / 60;
        if (h > 0)
            it.printf(126, 22, &id(font_main), esphome::display::TextAlign::TOP_RIGHT,
                      "%d:%02d p", h, m);
        else
            it.printf(126, 22, &id(font_main), esphome::display::TextAlign::TOP_RIGHT,
                      "%d p", m);
    }

    it.print(0, 45, &id(font_icons_main), IC_VALVE);
    it.printf(18, 47, &id(font_main), "%s", id(current_zone_name).c_str());
    it.printf(126, 47, &id(font_main), esphome::display::TextAlign::TOP_RIGHT,
              "%02d:%02d", zr / 60, zr % 60);

    // Line 3: "N/M" left | right-justified: icon(s) + space + next zone name
    // N = zone_run_count (monotonic across repeats)
    // M = zone_queue_size × total_passes (zone_queue_size holds per-pass count)
    {
      int total_passes = 1 + (int)g_config.cycle_repeat_count;
      it.printf(0, 71, &id(font_main), "%d/%d",
                id(zone_run_count), id(zone_queue_size) * total_passes);
    }
    {
      int cn = id(current_cycle_num);
      int next_zone = 0;
      for (int i = id(current_zone_index); i < CYCLE_ZONE_COUNT; i++) {
        int zn = i + 1;
        bool en  = (cn == 1) ? zone_irr_enabled(zn)  : zone_short_enabled(zn);
        int  dur = (cn == 1) ? zone_irr_duration_sec(zn) : zone_short_duration_sec(zn);
        if (en && dur > 0) { next_zone = zn; break; }
      }
      // If no zone found in this pass but repeats remain, next zone is zone 1 of next pass
      if (next_zone == 0 && id(cycle_repeat_remaining) > 0) {
        for (int i = 0; i < CYCLE_ZONE_COUNT; i++) {
          int zn = i + 1;
          bool en  = (cn == 1) ? zone_irr_enabled(zn)  : zone_short_enabled(zn);
          int  dur = (cn == 1) ? zone_irr_duration_sec(zn) : zone_short_duration_sec(zn);
          if (en && dur > 0) { next_zone = zn; break; }
        }
      }
      // Right side: IC_SKIP + next zone name (or IC_SKIP + IC_STOP on last zone),
      // right-justified as a block. Icon is placed left of the text.
      if (next_zone == 0) {
        // Last zone: IC_SKIP + IC_STOP, or IC_SKIP + IC_QUEUE if a cycle is queued
        const char* last_icon = (id(queued_cycle_num) > 0) ? IC_QUEUE : IC_STOP;
        it.print(126, 70, &id(font_icons_main),
                 esphome::display::TextAlign::TOP_RIGHT, last_icon);
        it.print(108, 70, &id(font_icons_main),
                 esphome::display::TextAlign::TOP_RIGHT, IC_SKIP);
      } else {
        // Normal: IC_SKIP + " " + next zone name, right-justified as a block.
        // Render name first to measure its pixel width, then place icon left of it.
        int x1, y1, w, h;
        it.get_text_bounds(0, 0, zone_name(next_zone), &id(font_main),
                           esphome::display::TextAlign::TOP_LEFT, &x1, &y1, &w, &h);
        // icon width at size 15 ~16px; add 2px gap between icon and text
        int icon_right = 126 - w - 2;
        it.print(icon_right, 70, &id(font_icons_main),
                 esphome::display::TextAlign::TOP_RIGHT, IC_SKIP);
        it.printf(126, 71, &id(font_main),
                  esphome::display::TextAlign::TOP_RIGHT,
                  "%s", zone_name(next_zone));
      }
    }

    draw_footer_separator(it);
    draw_footer(it, IC_STOP, IC_PAUSE, IC_SKIP);
}

static void draw_page_manual_idle(esphome::display::DisplayBuffer& it) {
    int zone_list[TOTAL_ZONE_COUNT];
    int list_size = 0;
    for (int z = CYCLE_ZONE_COUNT + 1; z <= TOTAL_ZONE_COUNT; z++) zone_list[list_size++] = z;
    for (int z = 1; z <= CYCLE_ZONE_COUNT; z++) zone_list[list_size++] = z;

    int offset = id(display_page_offset);
    for (int row = 0; row < 4 && (offset + row) < list_size; row++) {
        int z = zone_list[offset + row];
        int dur_min = g_config.zones[z - 1].manual_duration_min;
        int y = 20 + row * 21;
        it.print(0, y - 2, &id(font_icons_main), IC_VALVE);
        it.printf(18, y + 2, &id(font_main), "%s", zone_name(z));
        it.printf(126, y + 2, &id(font_main), esphome::display::TextAlign::TOP_RIGHT,
                  "%d%s", dur_min, TXT_MINUTES_ABBR);
        if (id(display_cursor_pos) == z) draw_selection_rect(it, y, 20);
    }
    draw_footer_separator(it);
    draw_footer(it, IC_START, IC_UP, IC_DOWN);
}

static void draw_page_manual_running(esphome::display::DisplayBuffer& it) {
    int zr = id(zone_remaining_sec);

    it.print(0, 18, &id(font_icons_main), IC_SCHEDULE);
    it.print(126, 20, &id(font_main), esphome::display::TextAlign::TOP_RIGHT,
             id(next_schedule_text));
    // Queue indicator: show queued cycle below schedule line if queue is active
    if (id(queued_cycle_num) > 0) {
        int q = id(queued_cycle_num);
        it.print(126, 39, &id(font_icons_main), esphome::display::TextAlign::TOP_RIGHT, IC_QUEUE);
        it.printf(110, 41, &id(font_main), esphome::display::TextAlign::TOP_RIGHT, "%s",
                  q == 1 ? TXT_CYCLE_IRR : TXT_CYCLE_SHORT);
    }

    it.print(0, 61, &id(font_icons_main), IC_VALVE);
    it.print(126, 63, &id(font_main), esphome::display::TextAlign::TOP_RIGHT,
             id(current_zone_name).c_str());
    it.printf(126, 84, &id(font_main), esphome::display::TextAlign::TOP_RIGHT,
              "%02d:%02d", zr / 60, zr % 60);

    draw_footer_separator(it);
    it.print(4,   110, &id(font_icons_footer), IC_STOP);
    it.print(68,  110, &id(font_icons_footer),
             esphome::display::TextAlign::TOP_CENTER, IC_MINUS);
    it.print(124, 110, &id(font_icons_footer),
             esphome::display::TextAlign::TOP_RIGHT, IC_PLUS);
}

static void draw_page_pause(esphome::display::DisplayBuffer& it) {
    if (id(pause_countdown_sec) > 0) {
        char cd[8];
        int m = id(pause_countdown_sec) / 60;
        int s = id(pause_countdown_sec) % 60;
        snprintf(cd, sizeof(cd), "%02d:%02d", m, s);
        it.print(0,   19, &id(font_icons_main), IC_PAUSE);
        it.printf(64, 19, &id(font_main), esphome::display::TextAlign::TOP_CENTER, "%s", cd);
        it.print(128, 19, &id(font_icons_main), esphome::display::TextAlign::TOP_RIGHT, IC_PAUSE);
    } else if (id(pause_input_active)) {
        it.print(0,   19, &id(font_icons_main), IC_PAUSE);
        it.printf(64, 19, &id(font_main), esphome::display::TextAlign::TOP_CENTER,
                  ":%02d", id(pause_timer_input_min));
        it.filled_rectangle(44, 18, 42, 19);
        it.printf(64, 19, &id(font_main), COLOR_OFF,
                  esphome::display::TextAlign::TOP_CENTER,
                  ":%02d", id(pause_timer_input_min));
        it.print(128, 19, &id(font_icons_main), esphome::display::TextAlign::TOP_RIGHT, IC_PAUSE);
    } else {
        it.print(0,   19, &id(font_icons_main), IC_PAUSE);
        it.printf(64, 19, &id(font_main), esphome::display::TextAlign::TOP_CENTER,
                  "%s", TXT_PAUSED);
        it.print(128, 19, &id(font_icons_main), esphome::display::TextAlign::TOP_RIGHT, IC_PAUSE);
    }
    it.printf(0, 41, &id(font_main), "%s", TXT_PAUSE_TIME);
    it.print(126, 41, &id(font_main), esphome::display::TextAlign::TOP_RIGHT,
             id(pause_timestamp_str).c_str());
    it.printf(0, 61, &id(font_main), "%s", id(current_zone_name).c_str());
    it.printf(0, 81, &id(font_main), "%d/%d %s",
              id(current_zone_index),
              id(zone_queue_size) * (1 + (int)g_config.cycle_repeat_count),
              TXT_ZONE);
    draw_footer_separator(it);
    if (id(pause_input_active))
        draw_footer(it, IC_OK, IC_DOWN, IC_UP);
    else if (id(pause_countdown_sec) > 0)
        draw_footer(it, IC_STOP, IC_ALARM_OFF, IC_START);
    else
        draw_footer(it, IC_STOP, IC_ALARM_ADD, IC_START);
}

static void draw_page_lockout(esphome::display::DisplayBuffer& it) {
    it.print(0,   20, &id(font_icons_footer), IC_PUMP);
    it.printf(64, 20, &id(font_footer), esphome::display::TextAlign::TOP_CENTER,
              "%s", TXT_LOCKOUT);
    it.print(128, 20, &id(font_icons_footer), esphome::display::TextAlign::TOP_RIGHT, IC_PUMP);
    it.printf(64, 50, &id(font_footer), esphome::display::TextAlign::TOP_CENTER,
              ":%02d s", id(pump_lockout_remaining));
    draw_footer_separator(it);
    draw_footer(it, IC_START, "", IC_CANCEL);
}

static void draw_page_powerloss(esphome::display::DisplayBuffer& it) {
    // Line 1: blinking IC_POWERLOSS icons + TXT_POWERLOSS centre
    if (id(blink_state)) {
        it.print(0,   20, &id(font_icons_footer), IC_POWERLOSS);
        it.print(128, 20, &id(font_icons_footer), esphome::display::TextAlign::TOP_RIGHT, IC_POWERLOSS);
    }
    if (id(pause_countdown_sec) > 0) {
        char cd[8];
        int m = id(pause_countdown_sec) / 60;
        int s = id(pause_countdown_sec) % 60;
        snprintf(cd, sizeof(cd), "%02d:%02d", m, s);
        it.printf(64, 22, &id(font_footer), esphome::display::TextAlign::TOP_CENTER, "%s", cd);
    } else if (id(pause_input_active)) {
        it.filled_rectangle(44, 21, 42, 19);
        it.printf(64, 22, &id(font_main), COLOR_OFF,
                  esphome::display::TextAlign::TOP_CENTER,
                  ":%02d", id(pause_timer_input_min));
    } else {
        it.printf(64, 22, &id(font_footer), esphome::display::TextAlign::TOP_CENTER,
                  "%s", TXT_POWERLOSS);
    }
    // Line 2: IC_SPRINKLER + cycle name + zone_num/total_zones
    int ct = id(saved_cycle_type);
    if (ct > 0) {
        it.print(0, 42, &id(font_icons_main), IC_SPRINKLER);
        it.printf(18, 44, &id(font_main), "%s",
                  ct == 1 ? TXT_CYCLE_IRR : TXT_CYCLE_SHORT);
        int total_zones = 0;
        for (int i = 1; i <= CYCLE_ZONE_COUNT; i++) {
            bool en  = (ct == 1) ? zone_irr_enabled(i)  : zone_short_enabled(i);
            int  dur = (ct == 1) ? zone_irr_duration_sec(i) : zone_short_duration_sec(i);
            if (en && dur > 0) total_zones++;
        }
        int total_passes = 1 + (int)g_config.cycle_repeat_count;
        it.printf(126, 44, &id(font_main), esphome::display::TextAlign::TOP_RIGHT,
                  "%d/%d", id(saved_zone_num), total_zones * total_passes);
    }
    // Line 3: IC_QUEUE + queued cycle name left
    int q = id(queued_cycle_num);
    if (q > 0) {
        it.print(0, 62, &id(font_icons_main), IC_QUEUE);
        it.printf(18, 64, &id(font_main), "%s",
                  q == 1 ? TXT_CYCLE_IRR : TXT_CYCLE_SHORT);
    }
    // Line 4: TXT_RESUME right
    it.printf(126, 83, &id(font_main), esphome::display::TextAlign::TOP_RIGHT, TXT_RESUME);
    draw_footer_separator(it);
    if (id(pause_input_active))
        draw_footer(it, IC_OK, IC_DOWN, IC_UP);
    else if (id(pause_countdown_sec) > 0)
        draw_footer(it, IC_STOP, IC_ALARM_OFF, IC_RESUME);
    else
        draw_footer(it, IC_STOP, IC_ALARM_ADD, IC_RESUME);
}

static void draw_page_pump_fault(esphome::display::DisplayBuffer& it) {
    // Fault page priority: io_fault > maintenance > rtc_fault > rtc_battery_low > oled_fault

    if (id(io_fault)) {
        if (id(blink_state)) {
            it.print(0,   20, &id(font_icons_footer), IC_WARNING);
            it.print(128, 20, &id(font_icons_footer), esphome::display::TextAlign::TOP_RIGHT, IC_WARNING);
        }
        it.printf(64, 22, &id(font_footer), esphome::display::TextAlign::TOP_CENTER, "%s", TXT_FAULT_IO);
        it.printf(0,  44, &id(font_main), "%s", TXT_FAULT_HELP_1);
        draw_footer_separator(it);
        draw_footer(it, IC_REBOOT, "", IC_OK);
        return;
    }

    if (g_config.maintenance_lock) {
        if (id(blink_state)) {
            it.print(0,   20, &id(font_icons_footer), IC_MAINTENANCE);
            it.print(128, 20, &id(font_icons_footer), esphome::display::TextAlign::TOP_RIGHT, IC_MAINTENANCE);
        }
        it.printf(64, 22, &id(font_footer), esphome::display::TextAlign::TOP_CENTER, "%s", TXT_FAULT_MAINTENANCE_L1);
        it.printf(0, 44, &id(font_main), TXT_FAULT_MAINTENANCE_L2);
        draw_footer_separator(it);
        draw_footer(it, IC_REBOOT, IC_MAINTENANCE, IC_OK);
        return;
    }

    if (id(rtc_fault)) {
        // Clear OSF register when page is shown (best-effort; RTC may not respond)
        static bool rtc_fault_osf_cleared = false;
        if (!rtc_fault_osf_cleared) {
            uint8_t reg = 0;
            id(rtc_time).read_register(0x0F, &reg, 1);
            reg &= 0x7F;
            id(rtc_time).write_register(0x0F, &reg, 1);
            rtc_fault_osf_cleared = true;
        }
        if (id(blink_state)) {
            it.print(0,   20, &id(font_icons_footer), IC_WARNING);
            it.print(128, 20, &id(font_icons_footer), esphome::display::TextAlign::TOP_RIGHT, IC_WARNING);
        }
        it.printf(64, 22, &id(font_footer), esphome::display::TextAlign::TOP_CENTER, "%s", TXT_FAULT_RTC);
        it.printf(0,  44, &id(font_main), "%s", TXT_FAULT_HELP_1);
        it.printf(0,  64, &id(font_main), "%s", TXT_FAULT_HELP_2);
        draw_footer_separator(it);
        draw_footer(it, IC_REBOOT, IC_ALARM_ADD, IC_OK);
        return;
    }

    if (id(rtc_battery_low)) {
        // Clear OSF register immediately when page is shown
        static bool batt_osf_cleared = false;
        if (!batt_osf_cleared) {
            uint8_t reg = 0;
            id(rtc_time).read_register(0x0F, &reg, 1);
            reg &= 0x7F;
            id(rtc_time).write_register(0x0F, &reg, 1);
            batt_osf_cleared = true;
        }
        if (id(blink_state)) {
            it.print(0,   20, &id(font_icons_footer), IC_BATT_ERROR);
            it.print(128, 20, &id(font_icons_footer), esphome::display::TextAlign::TOP_RIGHT, IC_BATT_ERROR);
        }
        it.printf(64, 22, &id(font_footer), esphome::display::TextAlign::TOP_CENTER, "%s", TXT_FAULT_BATT);
        it.printf(0,  44, &id(font_main), "%s", TXT_FAULT_HELP_1);
        it.printf(0,  64, &id(font_main), "%s", TXT_FAULT_BAT_TYPE);
        draw_footer_separator(it);
        draw_footer(it, IC_REBOOT, IC_ALARM_ADD, IC_OK);
        return;
    }

    if (id(oled_fault)) {
        // Page likely invisible but ZB fault attr and B1 reboot still work
        if (id(blink_state)) {
            it.print(0,   20, &id(font_icons_footer), IC_WARNING);
            it.print(128, 20, &id(font_icons_footer), esphome::display::TextAlign::TOP_RIGHT, IC_WARNING);
        }
        it.printf(64, 22, &id(font_footer), esphome::display::TextAlign::TOP_CENTER, "%s", TXT_FAULT_OLED);
        it.printf(0,  44, &id(font_main), "%s", TXT_FAULT_HELP_1);
        draw_footer_separator(it);
        draw_footer(it, IC_REBOOT, "", "");
        return;
    }

    // Fallback: unknown fault
    it.printf(0, 44, &id(font_main), "Ismeretlen hiba");
    draw_footer_separator(it);
    draw_footer(it, IC_REBOOT, "", IC_OK);
}

static void draw_page_settings_main(esphome::display::DisplayBuffer& it) {
    struct SettingsEntry { const char* icon; const char* name; };
    static const SettingsEntry entries[] = {
        { IC_SCHEDULE,    TXT_SCHEDULES         }, // 0
        { IC_SPRINKLER,   TXT_CYCLE_IRR         }, // 1
        { IC_SPRINKLER,   TXT_CYCLE_SHORT       }, // 2
        { IC_WEATHER,     TXT_SETTINGS_WEATHER  }, // 3
        { IC_VALVE,       TXT_SETTINGS_VALVE    }, // 4
        { IC_MAINTENANCE, TXT_SETTINGS_ADV      }, // 5
        { IC_SCREENSAVER, TXT_SETTINGS_OLED     }, // 6
        { IC_CLOCK,       TXT_ADV_DATETIME      }, // 7
    };
    static const int ENTRY_COUNT = 8;
    int offset = id(display_page_offset);
    int cursor = id(display_cursor_pos);
    for (int row = 0; row < 4; row++) {
        int idx = offset + row;
        if (idx >= ENTRY_COUNT) break;
        int y = 20 + row * 21;
        it.print(0,  y, &id(font_icons_main), entries[idx].icon);
        it.printf(18, y + 2, &id(font_main), "%s", entries[idx].name);
        if (cursor == idx) draw_selection_rect(it, y, 20);
    }
    draw_footer_separator(it);
    draw_footer(it, entries[cursor].icon, IC_UP, IC_DOWN);
}

static void draw_page_settings_sched(esphome::display::DisplayBuffer& it) {
    struct SchedRow { bool enabled; uint8_t hour, minute; const char* name; };
    SchedRow rows[4] = {
        { (bool)g_config.irr_schedule1.enabled,    g_config.irr_schedule1.hour,    g_config.irr_schedule1.minute,    TXT_SCHED_IRR1   },
        { (bool)g_config.irr_schedule2.enabled,   g_config.irr_schedule2.hour,   g_config.irr_schedule2.minute,   TXT_SCHED_IRR2   },
        { (bool)g_config.short_schedule1.enabled,  g_config.short_schedule1.hour, g_config.short_schedule1.minute, TXT_SCHED_SHORT1 },
        { (bool)g_config.short_schedule2.enabled,  g_config.short_schedule2.hour, g_config.short_schedule2.minute, TXT_SCHED_SHORT2 },
    };
    static const int X_COLON = 104;
    static const int X_HH    = 87;
    static const int X_MM    = 107;
    static const int TRECT_W = 18;
    static const int TRECT_H = 19;
    static const int CRECT_H = 19;

    for (int r = 0; r < 4; r++) {
        int y = 20 + r * 21;
        int elem_base = r * 3;
        bool ck_selected = (id(display_cursor_pos) == elem_base);
        bool hh_active   = (id(display_cursor_pos) == elem_base + 1);
        bool mm_active   = (id(display_cursor_pos) == elem_base + 2);

        it.print(2, y + 2, &id(font_icons_main), rows[r].enabled ? IC_CHECK : IC_CANCEL);
        if (ck_selected && id(pause_input_active)) {
            it.filled_rectangle(0, y + 2, 19, CRECT_H);
            it.print(2, y + 2, &id(font_icons_main), COLOR_OFF, rows[r].enabled ? IC_CHECK : IC_CANCEL);
        } else if (ck_selected) {
            it.rectangle(0, y + 2, 19, CRECT_H);
        }

        it.printf(19, y + 2, &id(font_main), "%s", rows[r].name);
        it.printf(X_HH, y + 2, &id(font_main), "%02d", rows[r].hour);
        it.printf(X_COLON, y + 2, &id(font_main), ":");
        it.printf(X_MM, y + 2, &id(font_main), "%02d", rows[r].minute);

        if (hh_active && id(pause_input_active)) {
            it.filled_rectangle(X_HH - 3, y + 1, TRECT_W + 4, TRECT_H);
            it.printf(X_HH, y + 2, &id(font_main), COLOR_OFF, "%02d", rows[r].hour);
        } else if (hh_active) {
            it.rectangle(X_HH - 3, y + 1, TRECT_W + 4, TRECT_H);
        }
        if (mm_active && id(pause_input_active)) {
            it.filled_rectangle(X_MM - 3, y + 1, TRECT_W + 4, TRECT_H);
            it.printf(X_MM, y + 2, &id(font_main), COLOR_OFF, "%02d", rows[r].minute);
        } else if (mm_active) {
            it.rectangle(X_MM - 3, y + 1, TRECT_W + 4, TRECT_H);
        }
    }

    draw_footer_separator(it);
    draw_footer(it, id(pause_input_active) ? IC_OK : IC_EDIT,
                id(pause_input_active) ? IC_MINUS : IC_LEFT,
                id(pause_input_active) ? IC_PLUS  : IC_RIGHT);
}

static void draw_page_settings_irr(esphome::display::DisplayBuffer& it) {
    int offset = id(display_page_offset);
    for (int row = 0; row < 4; row++) {
        int z = offset + row + 1;
        if (z > CYCLE_ZONE_COUNT) break;
        int y = 20 + row * 21;
        bool en  = zone_irr_enabled(z);
        int  dur = g_config.zones[z - 1].irr_duration_min;
        int en_elem  = (z - 1) * 2;
        int dur_elem = (z - 1) * 2 + 1;
        bool ck_sel  = (id(display_cursor_pos) == en_elem);
        bool dur_sel = (id(display_cursor_pos) == dur_elem);

        it.print(2, y + 2, &id(font_icons_main), en ? IC_CHECK : IC_CANCEL);
        if (ck_sel && id(pause_input_active)) {
            it.filled_rectangle(0, y + 2, 19, 19);
            it.print(2, y + 2, &id(font_icons_main), COLOR_OFF, en ? IC_CHECK : IC_CANCEL);
        } else if (ck_sel) {
            it.rectangle(0, y + 2, 19, 19);
        }
        it.printf(19, y + 2, &id(font_main), "%s", zone_name(z));
        it.printf(126, y + 2, &id(font_main), esphome::display::TextAlign::TOP_RIGHT, "%d", dur);
        if (dur_sel && id(pause_input_active)) {
            it.filled_rectangle(102, y + 1, 26, 19);
            it.printf(126, y + 2, &id(font_main), COLOR_OFF,
                      esphome::display::TextAlign::TOP_RIGHT, "%d", dur);
        } else if (dur_sel) {
            it.rectangle(102, y + 1, 26, 19);
        }
    }
    draw_footer_separator(it);
    draw_footer(it, id(pause_input_active) ? IC_OK : IC_EDIT,
                id(pause_input_active) ? IC_MINUS : IC_LEFT,
                id(pause_input_active) ? IC_PLUS  : IC_RIGHT);
}

static void draw_page_settings_short(esphome::display::DisplayBuffer& it) {
    int offset = id(display_page_offset);
    for (int row = 0; row < 4; row++) {
        int z = offset + row + 1;
        if (z > CYCLE_ZONE_COUNT) break;
        int y = 20 + row * 21;
        bool en  = zone_short_enabled(z);
        int  dur = g_config.zones[z - 1].short_duration_min;
        int en_elem  = (z - 1) * 2;
        int dur_elem = (z - 1) * 2 + 1;
        bool ck_sel  = (id(display_cursor_pos) == en_elem);
        bool dur_sel = (id(display_cursor_pos) == dur_elem);

        it.print(2, y + 2, &id(font_icons_main), en ? IC_CHECK : IC_CANCEL);
        if (ck_sel && id(pause_input_active)) {
            it.filled_rectangle(0, y + 2, 19, 19);
            it.print(2, y + 2, &id(font_icons_main), COLOR_OFF, en ? IC_CHECK : IC_CANCEL);
        } else if (ck_sel) {
            it.rectangle(0, y + 2, 19, 19);
        }
        it.printf(19, y + 2, &id(font_main), "%s", zone_name(z));
        it.printf(126, y + 2, &id(font_main), esphome::display::TextAlign::TOP_RIGHT, "%d", dur);
        if (dur_sel && id(pause_input_active)) {
            it.filled_rectangle(102, y + 1, 26, 19);
            it.printf(126, y + 2, &id(font_main), COLOR_OFF,
                      esphome::display::TextAlign::TOP_RIGHT, "%d", dur);
        } else if (dur_sel) {
            it.rectangle(102, y + 1, 26, 19);
        }
    }
    draw_footer_separator(it);
    draw_footer(it, id(pause_input_active) ? IC_OK : IC_EDIT,
                id(pause_input_active) ? IC_MINUS : IC_LEFT,
                id(pause_input_active) ? IC_PLUS  : IC_RIGHT);
}

// draw_settings_page_3: shared renderer for 3-entry settings pages.
// Each page follows the PAGE_SETTINGS_ADV layout: 3 rows, tooltip above footer.
// Entry types: 'n'=numeric, 't'=toggle(bool), 's'=signed numeric.
struct SettingsPage3Entry {
    const char* icon;
    const char* name;
    const char* tooltip;
    char        type;    // 'n'=uint, 's'=int, 't'=bool toggle
};

static void draw_settings_page_3(
    esphome::display::DisplayBuffer& it,
    const SettingsPage3Entry* entries, int entry_count,
    // value_fn: called with entry index, returns int value to display
    std::function<int(int)> value_fn,
    // bool_fn: for toggles, returns true/false
    std::function<bool(int)> bool_fn
) {
    int cursor = id(display_cursor_pos);
    bool in_edit = id(pause_input_active);

    // Carousel offset: keep cursor visible, sliding window of 3 entries.
    // offset = max(0, min(cursor, entry_count - 3))
    int offset = cursor - 2;
    if (offset < 0) offset = 0;
    if (offset > entry_count - 3) offset = entry_count - 3;
    if (offset < 0) offset = 0;  // guard for entry_count < 3

    if (cursor < entry_count)
        it.printf(64, 95, &id(font_header), esphome::display::TextAlign::TOP_CENTER,
                  "%s", entries[cursor].tooltip);

    for (int row = 0; row < 3 && (offset + row) < entry_count; row++) {
        int idx = offset + row;
        int y = 20 + row * 21;
        it.print(0, y, &id(font_icons_main), entries[idx].icon);
        it.printf(18, y + 2, &id(font_main), "%s", entries[idx].name);
        bool is_sel  = (cursor == idx);
        bool is_edit = is_sel && in_edit;

        auto draw_val = [&](bool inv) {
            char t = entries[idx].type;
            if (t == 't') {
                it.print(126, y+2, &id(font_icons_main),
                         inv ? COLOR_OFF : (esphome::Color)esphome::Color::WHITE,
                         esphome::display::TextAlign::TOP_RIGHT,
                         bool_fn(idx) ? IC_CHECK : IC_CANCEL);
            } else if (t == 's') {
                it.printf(126, y+2, &id(font_main),
                          inv ? COLOR_OFF : (esphome::Color)esphome::Color::WHITE,
                          esphome::display::TextAlign::TOP_RIGHT,
                          "%+d", value_fn(idx));
            } else {
                it.printf(126, y+2, &id(font_main),
                          inv ? COLOR_OFF : (esphome::Color)esphome::Color::WHITE,
                          esphome::display::TextAlign::TOP_RIGHT,
                          "%d", value_fn(idx));
            }
        };

        draw_val(false);
        if (!is_sel) continue;
        if (!is_edit) { draw_selection_rect(it, y, 20); continue; }
        // Edit highlight: width must cover the widest possible value string.
        // 't' toggle: 20px (single icon)
        // 's' signed: 30px (covers "+200" style values)
        // 'n' numeric: 30px (covers "200" or "100" — 3 digits at font_main ~8px each + margin)
        int rw = (entries[idx].type == 't') ? 20 : 30;
        int rx = 128 - rw;
        it.filled_rectangle(rx, y + 1, rw, 19);
        draw_val(true);
    }
    draw_footer_separator(it);
    draw_footer(it, in_edit ? IC_OK : IC_EDIT,
                in_edit ? IC_MINUS : IC_UP,
                in_edit ? IC_PLUS  : IC_DOWN);
}

static void draw_page_settings_weather(esphome::display::DisplayBuffer& it) {
    static const SettingsPage3Entry entries[] = {
        { IC_PERCENT,    TXT_ADV_SCALE,       TXT_ADV_SCALE_TOOLTIP,         'n' },  // 0: scale
        { IC_PERCENT,    TXT_ADV_SCALE_RESET, TXT_ADV_SCALE_RESET_TOOLTIP,   'n' },  // 1: scale_reset
        { IC_REPEAT,     TXT_ADV_REPEAT,       TXT_ADV_REPEAT_TOOLTIP,       'n' },  // 2: repeat
        { IC_SPRINKLER,  TXT_ADV_SCALE_ENABLE, TXT_ADV_SCALE_ENABLE_TOOLTIP, 'n' },  // 3: scale_enable (1-3)
    };
    draw_settings_page_3(it, entries, 4,
        [](int i) -> int {
            if (i == 0) return (int)g_config.duration_scale_percent;
            if (i == 1) return (int)g_config.scale_reset_days;
            if (i == 2) return (int)g_config.cycle_repeat_count + 1;  // display 1-3, stored 0-2
            return (int)g_config.scale_enable;
        },
        [](int) -> bool { return false; });
}

static void draw_page_settings_valve(esphome::display::DisplayBuffer& it) {
    static const SettingsPage3Entry entries[] = {
        { IC_VALVE, TXT_ADV_PUMP_START, TXT_ADV_PUMP_START_TOOLTIP, 's' },  // 0: pump_start_offset
        { IC_VALVE, TXT_ADV_PUMP_STOP,  TXT_ADV_PUMP_STOP_TOOLTIP,  's' },  // 1: pump_stop_offset
        { IC_VALVE, TXT_ADV_ZONE_DELAY, TXT_ADV_ZONE_DELAY_TOOLTIP, 'n' },  // 2: zone_switch_delay
    };
    draw_settings_page_3(it, entries, 3,
        [](int i) -> int {
            if (i == 0) return (int)g_config.pump_start_offset;
            if (i == 1) return (int)g_config.pump_stop_offset;
            return (int)g_config.zone_switch_delay_sec;
        },
        [](int) -> bool { return false; });
}

static void draw_page_settings_adv(esphome::display::DisplayBuffer& it) {
    // Rendszer: maintenance, auto_resume, lockout, network
    static const SettingsPage3Entry entries[] = {
        { IC_MAINTENANCE, TXT_ADV_MAINTENANCE, TXT_ADV_MAINTENANCE_TOOLTIP, 't' },  // 0: maintenance
        { IC_POWERLOSS,   TXT_ADV_AUTO_RESUME, TXT_ADV_AUTO_RESUME_TOOLTIP, 't' },  // 1: auto_resume
        { IC_PUMP,        TXT_ADV_LOCKOUT,     TXT_ADV_LOCKOUT_TOOLTIP,     'n' },  // 2: lockout
        { IC_NETWORK,     TXT_ADV_NET,         TXT_ADV_NET_TOOLTIP,         't' },  // 3: network selector
    };
    draw_settings_page_3(it, entries, 4,
        [](int i) -> int {
            if (i == 2) return (int)g_config.pump_lockout_sec;
            return 0;
        },
        [](int i) -> bool {
            if (i == 0) return (bool)g_config.maintenance_lock;
            if (i == 1) return (bool)g_config.auto_resume_on_powerloss;
            if (i == 3) return false;  // network selector: placeholder
            return false;
        });
}

static void draw_page_settings_oled(esphome::display::DisplayBuffer& it) {
    static const SettingsPage3Entry entries[] = {
        { IC_SCREENSAVER, TXT_ADV_DISP_TIMEOUT, TXT_ADV_DISP_TIMEOUT_TOOLTIP, 'n' },  // 0: timeout
        { IC_NIGHT_OFF,   TXT_ADV_NIGHT_OFF,    TXT_ADV_NIGHT_OFF_TOOLTIP,    't' },  // 1: night_off
    };
    draw_settings_page_3(it, entries, 2,
        [](int i) -> int {
            if (i == 0) return (int)g_config.display_timeout_min;
            return 0;
        },
        [](int i) -> bool {
            if (i == 1) return (bool)g_config.display_night_off;
            return false;
        });
}

// get_screensaver_fault: returns true if there is an active fault to show.
// Fills icon and text for the fault screensaver view (first line of fault page).
// Priority matches draw_page_pump_fault and header icon order.
static bool get_screensaver_fault(const char** icon, const char** text) {
    if (id(io_fault))                  { *icon = IC_WARNING;     *text = TXT_FAULT_IO;          return true; }
    if (g_config.maintenance_lock) { *icon = IC_MAINTENANCE; *text = TXT_FAULT_MAINTENANCE_L1; return true; }
    if (id(rtc_fault))                 { *icon = IC_WARNING;     *text = TXT_FAULT_RTC;         return true; }
    if (id(rtc_battery_low))           { *icon = IC_BATT_ERROR;  *text = TXT_FAULT_BATT;        return true; }
    if (id(oled_fault))                { *icon = IC_WARNING;     *text = TXT_FAULT_OLED;        return true; }
    return false;
}

// draw_page_screensaver: minimal burn-in screensaver.
// Views cycle: 0=schedule, 1=clock, 2=fault (only when fault active).
// View rotation handled by screensaver_view global (0/1/2).
// Position alternates between top half (y_base=24) and bottom half (y_base=72)
// controlled by screensaver_pos global (0=top, 1=bottom).
// No header, footer, or separators — minimal static elements.
static void draw_page_screensaver(esphome::display::DisplayBuffer& it) {
    auto t = id(the_time).now();
    int view = id(screensaver_view);   // 0=schedule, 1=clock, 2=fault
    int pos  = id(screensaver_pos);    // 0=top half, 1=bottom half
    int y    = (pos == 0) ? 24 : 72;  // y_base for two-line block

    static const int IX = 14;
    static const int TX = 18;

    if (view == 2) {
        // Fault view: icon + fault title line 1, blink on icon
        const char* ficon = nullptr;
        const char* ftext = nullptr;
        if (get_screensaver_fault(&ficon, &ftext)) {
            if (id(blink_state))
                it.print(IX, y, &id(font_icons_main),
                         esphome::display::TextAlign::TOP_RIGHT, ficon);
            it.print(TX, y + 2, &id(font_main), ftext);
            // Line 2: repeat icon on right side (mirrored blink)
            if (id(blink_state))
                it.print(128, y + 2, &id(font_icons_main),
                         esphome::display::TextAlign::TOP_RIGHT, ficon);
        }
    } else if (view == 0) {
        it.print(IX, y, &id(font_icons_main), esphome::display::TextAlign::TOP_RIGHT,
                 IC_SCHEDULE);
        it.print(TX, y + 2, &id(font_main), id(next_schedule_text));
        int nc = id(next_schedule_cycle);
        if (nc > 0) {
            const char* sched_name = (nc == 1) ? TXT_CYCLE_IRR : TXT_CYCLE_SHORT;
            int enabled = 0;
            for (int i = 1; i <= CYCLE_ZONE_COUNT; i++) {
                bool en = (nc == 1) ? zone_irr_enabled(i) : zone_short_enabled(i);
                if (en) enabled++;
            }
            it.print(IX, y + 24, &id(font_icons_main), esphome::display::TextAlign::TOP_RIGHT,
                     IC_SPRINKLER);
            it.printf(TX, y + 26, &id(font_main), "%s  %d/%d",
                      sched_name, enabled, CYCLE_ZONE_COUNT);
        } else {
            it.print(IX, y + 24, &id(font_icons_main), esphome::display::TextAlign::TOP_RIGHT,
                     IC_SCHEDULE);
            it.print(TX, y + 26, &id(font_main), TXT_NO_SCHED);
        }
    } else {
        // Clock view
        if (t.is_valid()) {
            it.printf(64, y, &id(font_footer), esphome::display::TextAlign::TOP_CENTER,
                      "%02d:%02d", t.hour, t.minute);
            it.printf(64, y + 28, &id(font_main), esphome::display::TextAlign::TOP_CENTER,
                      "%04d-%02d-%02d", t.year, t.month, t.day_of_month);
        } else {
            it.print(64, y,      &id(font_footer), esphome::display::TextAlign::TOP_CENTER, "--:--");
            it.print(64, y + 28, &id(font_main),   esphome::display::TextAlign::TOP_CENTER, "----  --  --");
        }
    }
}

static void draw_page_settings_datetime(esphome::display::DisplayBuffer& it) {
    // Cursor positions: 0=YYYY 1=MM 2=DD 3=HH 4=MI
    // Layout: YYYY-MM-DD line 1, HH:MM line 2, both centred on 128px display.
    // font_main ~9px/digit, ~5px separator. YYYY-MM-DD = 36+5+18+5+18 = 82px total.
    // Centre start = (128-82)/2 = 23. HH:MM = 18+5+18 = 41px, start = (128-41)/2 = 43.
    int cursor = id(display_cursor_pos);
    bool in_edit = id(pause_input_active);

    int d  = id(edit_day);
    int mo = id(edit_month);
    int yr = id(edit_year);
    int h  = id(edit_hour);
    int mi = id(edit_minute);

    static const int DY = 37;
    static const int TY = 58;
    static const int X_YYYY = 23;   // 4-digit year
    static const int X_S1   = 59;   // first '-'
    static const int X_MON  = 64;   // 2-digit month
    static const int X_S2   = 82;   // second '-'
    static const int X_DAY  = 87;   // 2-digit day
    static const int X_HH   = 43;   // 2-digit hour
    static const int X_COL  = 61;   // ':'
    static const int X_MI   = 66;   // 2-digit minute

    it.printf(X_YYYY, DY, &id(font_main), "%04d", yr);
    it.printf(X_S1,   DY, &id(font_main), "-");
    it.printf(X_MON,  DY, &id(font_main), "%02d", mo);
    it.printf(X_S2,   DY, &id(font_main), "-");
    it.printf(X_DAY,  DY, &id(font_main), "%02d", d);
    it.printf(X_HH,   TY, &id(font_main), "%02d", h);
    it.printf(X_COL,  TY, &id(font_main), ":");
    it.printf(X_MI,   TY, &id(font_main), "%02d", mi);

    // Field rects: x = text_x - 2, w = text_width + 4 (2px padding each side)
    // 2-digit: 18px text + 4 = 22px. YYYY: 36px text + 4 = 40px.
    struct FieldRect { int x, yl, w; };
    static const FieldRect fields[] = {
        { X_YYYY - 2, DY, 40 }, // 0 YYYY
        { X_MON  - 2, DY, 22 }, // 1 MM
        { X_DAY  - 2, DY, 22 }, // 2 DD
        { X_HH   - 2, TY, 22 }, // 3 HH
        { X_MI   - 2, TY, 22 }, // 4 MI
    };
    if (cursor >= 0 && cursor <= 4) {
        int fx = fields[cursor].x;
        int fy = fields[cursor].yl;
        int fw = fields[cursor].w;
        if (in_edit) {
            it.filled_rectangle(fx, fy - 1, fw, 19);
            switch (cursor) {
                case 0: it.printf(X_YYYY, DY, &id(font_main), COLOR_OFF, "%04d", yr); break;
                case 1: it.printf(X_MON,  DY, &id(font_main), COLOR_OFF, "%02d", mo); break;
                case 2: it.printf(X_DAY,  DY, &id(font_main), COLOR_OFF, "%02d", d);  break;
                case 3: it.printf(X_HH,   TY, &id(font_main), COLOR_OFF, "%02d", h);  break;
                case 4: it.printf(X_MI,   TY, &id(font_main), COLOR_OFF, "%02d", mi); break;
            }
        } else {
            it.rectangle(fx, fy - 1, fw, 19);
        }
    }

    draw_footer_separator(it);
    draw_footer(it, in_edit ? IC_SAVE : IC_EDIT,
                in_edit ? IC_MINUS : IC_LEFT,
                in_edit ? IC_PLUS  : IC_RIGHT);
}
