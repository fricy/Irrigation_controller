# ESP32-C6 Irrigation Controller — TODO List
**Firmware:** v1.1.4
**Updated:** 2026-06-09
**Status:** Release candidate

---

## Bugs — Active

| # | Description | Priority |
|---|-------------|----------|
| B3 | `cycle_remaining_sec` vs `zone_remaining_sec` 1s drift on multiple skips. Cosmetic only. NOFIX. | — |
| B5 | Manual zone timer-expiry with negative stop offset: valve closes with pump, no drain delay. Fixing requires restructuring `zone_countdown_timer`. Acceptable limitation. NOFIX. | — |
| B6 | DS3231 OSF bit occasional false positive on warm reset (I2C noise from relay switching). Handled gracefully but IC_BATT_ERROR shows in header. Pending further testing. | Low |
| B7 | Cold boot ZB sync timing: if coordinator response arrives at exactly the 10s NVS seed timeout, race condition possible. `nvs_seed_active` flag mitigates but edge case remains. | Low |
| B8 | Stale ZB state in Z2M if power is lost during active cycle and device comes online before coordinator. Mitigated by stale cleanup in `zb_push_boot_status` but window exists. | Low |
| B9 | Forward jump `cycle_remaining_sec` drift: a skip drops one switch delay pending at jump time. Extreme edge case. NOFIX. | — |
| B10 | pump_start_offset vs zone_switch_delay interaction: if `\|pump_start_offset\|` > zone_duration, valve opens during next zone's switch delay. Unrealistic config. NOFIX. | — |
| B11 | Maximum zone count is 16 (15 zones + pump). APS binding table defaults to 16 slots; 15 zone EPs + 2 cycle EP binds = 17, causing TABLE_FULL. Workaround: cycle EPs use explicit `reportAttr` instead of binding (implemented). To increase beyond 15 zones, edit cached external component `zigbee.cpp` before `esp_zb_init()` (line 538 in fadf848a): `esp_zb_aps_src_binding_table_size_set(32); esp_zb_aps_dst_binding_table_size_set(32);` Edit lost if ESPHome cache is cleared. Planned: own fork of v1.x with the table-size patch committed, pinned ref + `refresh: never` (HANDOVER_AUDIT item 13). | Low |

---

## Bugs — Known / No Fix

- **B1 — "Time has jumped back" display freeze.** Unable to reproduce after RTC implementation. NOFIX.
- **B2/B4/B3/B5/B9/B10** — See active bugs table above, all marked NOFIX.
- **Very short zone race condition** — tested: 8% scale on 1min zone = ~5s zone with ±3s offsets behaved correctly. No fix needed.
- **Normal cycle end negative stop offset** — `zone_countdown_timer` closes last relay before `abort_cycle_script`, so `stop_all_zones` is idempotent. Correct behavior confirmed.

---

## Needs Testing / Retesting

- **B6/B7** — RTC OSF and cold boot ZB sync. Deferred until stable environment.
- **Screensaver fault view** — all 5 fault conditions visible in screensaver rotation.
- **ZB boot push poller** — verify `zb_wait_and_push_boot_status` triggers on reboot reconnect (not just fresh pair).
- **ZC5** — Schedule time format: verify HH:MM string ↔ minutes-since-midnight conversion both directions.
- **ZC8** — Pump lockout active / power loss / fault state: verify NULL values resolve after device push.
- **Midnight boundary boot** — power off before midnight, restore after.
- **Manual zone ZB active_zone** — verify `zb_active_zone` shows correct zone number when manual zone running.
- **Queued cycle survives manual zone** — verify `zb_queue_state` not cleared when manual zone ends with a queued cycle.

---

## Tested / Confirmed This Session

| Scenario | Result |
|---|---|
| Auto-resume=true: no missed cycle, interrupt c1z5 | OK: starts c1z5 |
| Auto-resume=true: interrupt c1z7, missed c1 | OK: starts c1z1 |
| Auto-resume=true: interrupt c1z3, missed c2 | OK: continues c1z3, queues c2 |
| Auto-resume=true: interrupt c2z3, missed c2 | OK: starts c2z1 |
| Auto-resume=true: interrupt c2z2, queued c1, missed c2 | OK: starts c1z1 |
| Auto-resume=true: cycle/zone ZB switch states | OK after fixes |
| Auto-resume=false: interrupted cycle, cycle/zone state | Zone OK, cycle switch fixed |
| Auto-resume=false: interrupt c2z3, missed c1 | Zone OK, cycle switch fixed |
| Powerloss page resume: interrupt c2z3 | Zone turns on after resume — OK |
| Scale=0% cycle abort | Aborts cleanly, NVS cleared |
| Scale=0% powerloss recovery | Discarded in decision tree before PAGE_POWERLOSS |
| Stuck zone detection (zone_start_counter) | Skip after 3 attempts works correctly |

---

## Display / UX

| # | Description | Priority |
|---|-------------|----------|
| D2 | `time_source=0` indicator on boot when no time available. | Low |
| D3 | Footer button highlight: 2px rect default, filled rect on medium threshold. Dual icon for Btn1. | Low |

---

## System / Architecture

| # | Description | Priority |
|---|-------------|----------|
| S2 | NVS lifetime analysis: REOPENED. Firmware reality: `flash_write_interval: 10s`, `daily_water_time_sec` flushed every 10s during watering (~6× the analyzed 1/min rate; ~34yr → ~6yr ballpark). Re-close after 1/min flush decoupling (HANDOVER_AUDIT item 8). | Medium |
| S3 | Zigbee router backup path — verify re-join on extended outage. | Low |
| S4 | WiFi / HA API parallel entity integration. Post-release. | Low |
| S5 | `zigbee_connected` global: simplify by calling `id(zb).is_connected()` directly. Low urgency. | Low |

---

## Hardware / Sensors

| # | Description | Priority |
|---|-------------|----------|
| H1 | Current sensor revival: SCT-013-000 with comparator. See `current_sensor_archived.md`. | Low |

---

## Notes / Design Decisions

- **pump_start_offset convention**: positive = valve opens first (low pressure start), negative = pump starts first.
- **pump_stop_offset convention**: positive = pump stops first (pressure bleeds through valve), negative = valve closes first.
- **`irrigation_run_today` removed**: both irr schedules can fire on the same day independently.
- **`boot_fresh_start` removed**: replaced by `saved_repeat_remaining` initialized in `do_start(resume=false)`.
- **`do_start` contract**: single authority for all pending cycle state. Both auto-resume and manual-confirm paths set identical ZB state before diverging. Auto-resume delays 2s for display init then starts immediately. Manual-confirm shows PAGE_POWERLOSS and waits for B3.
- **`zb_network_switch`**: renamed from `zb_wifi_ota`. Stub only. AP mode not supported in ESP-IDF on ESP32-C6 with Zigbee. Full implementation deferred.
- **`wifi_active`**: renamed from `wifi_ap_active`. Stub only.
- **`zone_start_counter`**: global (restore_value: yes). Counts powerloss-resume attempts per zone. Resets on zone advance, skip, abort, fresh start, midnight. Skip triggered at ≥3 attempts. Advance uses zone duration accessors (accounts for scale_enable and scale=0%).
- **Modifying repeat modifier while cycle running**: increasing is safe; reducing can produce unexpected zone counts. Not recommended mid-cycle.
- **ZB crash on network removal**: quiet reboot / occasional kernel panic on coordinator-side removal. ZHC/SDK issue. Not reproducible on current firmware.

---

## Post-Release: WiFi Native API Integration

### Background

Successfully confirmed simultaneous Zigbee + WiFi operation on ESP32-C6 (2026-05-21). HA native API and Zigbee2MQTT work in parallel. Zone switch state updates arrive via both networks. Two kernel panics observed during cycle operation with both networks active — likely WiFi/ZB radio coexistence instability. Dual-network firmware deferred until coexistence is proven stable.

### Architecture Decision: Parallel entities

ZB entities and WiFi entities are parallel — HA sees both sets. ZB side handles native device control and Z2M integration. WiFi side adds HA automations, OTA, and diagnostic access. No arbitration layer; existing `cycle_running` / maintenance guards prevent conflicts.

### Required changes for WiFi variant

**1. Mark internal** — btn LEDs, RGB, zone relays, ZB template switches, pump relay, `irrigation_allowed` (fix `device_class: none` regardless)

**2. Cycle-aware zone switches** — `platform: template`, one per zone: turn_on calls `start_single_zone_manual`, turn_off calls `stop_single_zone_manual`, aware of `cycle_running`

**3. Cycle control switches** — `irrigation_cycle`, `short_cycle`, `cycle_pause`, `cycle_skip` as template switches backed by existing scripts

**4. Duplicate EP14 RW attrs as ESPHome entities** — number/select/switch entities writing to same `g_config` fields via `config_save()`

**5. Duplicate EP14 RO status as ESPHome sensors** — template sensors with `update_interval: 5s` reading globals

### Known issues to resolve first

- Kernel panic during cycle with both networks active — must be resolved before dual-network firmware
- `irrigation_allowed` binary sensor `device_class: moisture` — fix regardless of networking

---

## Planned Features

### F1 — Cycle Frequency
Run irrigation every Nth day. Configurable per cycle type via `frequency_enable`.

**Config struct** (`IrrigationConfig`, both stubbed as of CONFIG_PREF_HASH 0xAB01000D):
- `cycle_frequency`: uint8, 1-5, default 1 (every day). The Nth day interval — user-facing setting.
- `frequency_enable`: uint8, 1=irr only, 2=short only, 3=both (default 3). Mirrors `scale_enable`.

**Global** (ESPHome, `restore_value: yes`):
- `cycle_frequency_counter`: int, default 1. Runtime counter tracking current day in cycle. NOT in config struct.

**Helper** (`helpers.h`):
```cpp
inline bool frequency_applies(int ct) {
    if (ct == 1) return g_config.frequency_enable == 1 || g_config.frequency_enable == 3;
    if (ct == 2) return g_config.frequency_enable == 2 || g_config.frequency_enable == 3;
    return false;
}
```
Eligibility check: `frequency_applies(ct) && g_config.cycle_frequency > 1 && id(cycle_frequency_counter) != g_config.cycle_frequency` → skip.

**Midnight logic** (`schedules.yaml`, midnight on_time):
```
if g_config.cycle_frequency == 1: do nothing
if cycle_frequency_counter < g_config.cycle_frequency: counter++
else: counter = 1
```
Also reset counter in: midnight boot crossing block (decision tree).

**UI** — Settings/Weather page, 2 new entries inserted at cursor 2-3, shifting Repeat to 4, Scale enable to 5. Cursor max 4→6.
- Cursor 2: `cycle_frequency` (n, range 1-5, step 1). IC: `IC_FREQ = "\ue650"` (mdi:pulse)
- Cursor 3: `frequency_enable` (s, cycle 1→2→3→1)
- `TXT_WEATHER_FREQ` / `TXT_WEATHER_FREQ_TOOLTIP`: "Cycle freq." / "Ciklus frekvencia"
- `TXT_WEATHER_FREQ_ENABLE` / `TXT_WEATHER_FREQ_ENABLE_TOOLTIP`: "Apply Freq." / "1: irr; 3: mindkettő"
- `IC_FREQ = "\ue650"` added to `config.h`
- `calc_edit_step`: cursor 2 → step 1, cursor 3 → cycle (no step needed)

**ZB attrs** (EP14 0xFFF2):
- `cycle_frequency`: attr 0x0019, U8, RW
- `frequency_enable`: attr 0x001A, U8, RW
- Add to `zb_push_adv_save`, `on_value` handlers
- Converter: `cycle_frequency` numeric 1-5; `frequency_enable` enum (irr/short/both)

**Touch points** (same as scale=0% — all must be updated):
1. `schedules.yaml` on_time trigger: add frequency check before `start_irrigation`/`start_short`
2. `do_start` in `run_boot_decision_tree`: add frequency check for fresh starts only (`resume=false`). Exempt `resume=true` (cycle already started — don't stop mid-cycle)
3. Boot decision tree — missed cycle pre-validation: discard `missed_t1`/`missed_t2` if frequency check would block them (helper != frequency at that point)
4. `update_next_schedule_display` — `has_valid_zones` candidate filter: add frequency check to exclude ineligible schedules from display
5. `start_cycle_actual`: add check before zone queue is built (same path as scale=0%)

**Exempt from frequency check:**
- `resume=true` path in `do_start` (already running cycle)
- Manual button trigger on idle page (B1 on PAGE_IDLE)
- ZB manual trigger via EP16/EP17 ON (user explicitly requested)

**EC6 — Settings change behavior (decided):**
When user changes `cycle_frequency` in settings: reset `cycle_frequency_counter = g_config.cycle_frequency`. This means the cycle runs on the next eligible day counting from today. Change takes effect immediately.

**EC7 — CONFIG_PREF_HASH:** Already bumped to `0xAB01000D` with `frequency_enable` stub field.

**ZB attr IDs:** 0x0019 and 0x001A are currently unassigned — verify no conflicts before adding.

---

## Next Session Plan

1. **Finish leftover testing** — screensaver fault view, ZB boot push poller, ZC5, ZC8, midnight boundary boot
2. **Documentation** — README, user guide, hardware build instructions
3. **HA dashboard** — design and implementation

## Implemented / Closed (this session additions)

| # | Description | Version |
|---|-------------|--------|
| — | B1 long push LED pulse fixed: `button1_long_led` script starts at 5s on press, pulses while held, stopped on release | v1.0.3 |
| — | `flash_write_interval` set to 30s (was 10s). NVS lifetime ~34 years at scenario usage. (Superseded — firmware currently 10s.) | v1.0.3 |
| — | `zb_push_runtime_status`: split uptime to separate 1min interval; countdown guard — only runs during `cycle_running \|\| manual_zone_mode`; interval changed 60s→15s (superseded — currently 10s) | v1.0.3 |
| — | `zb_push_boot_status`: cycle genOnOff clear moved to +3s push (atomic with authoritative state) — eliminates flicker | v1.0.3 |
| — | `zb_push_powerloss_pending`: pushes cycle EP genOnOff ON for pending cycle type, enabling cancel via HA/Z2M | v1.0.3 |
| — | `start_single_zone_manual`: calls `zb_push_zone_start` — `zb_active_zone` now shows correct zone in HA during manual run | v1.0.3 |
| — | `stop_single_zone_manual`: calls `zb_push_cycle_state_zero` — clears active_zone/remaining without touching `zb_queue_state` | v1.0.3 |
| — | `powerloss_resume`: calls `zb_push_cycle_start` on successful resume (was missing — `ctrl_irr`/`ctrl_short` stayed OFF) | v1.0.3 |
| — | `do_start` auto-resume: sets identical ZB state as manual-confirm path before starting; 2s delay for display init | v1.0.3 |
| ZC2 | `zb_cycle_repeat` display value 1-3 confirmed correct | v1.0.3 |
| ZC6 | `zb_pause` write: pause/resume via ZB confirmed working during cycle | v1.0.3 |
| ZC8 | Pump lockout / power loss / fault state ZB attrs confirmed resolving correctly after boot push | v1.0.3 |
| — | `irr_schedule2` trigger, missed-cycle detection, screensaver display — confirmed | v1.0.3 |
| — | `scale_enable` irr-only/short-only/both filtering — confirmed | v1.0.3 |
| — | `maintenance_lock` full flow ZB→firmware→fault page→B3→ZB sync — confirmed | v1.0.3 |
| — | LED patterns cycle_start/end/zone_transition — confirmed | v1.0.3 |
| — | `zone_start_counter` stuck zone skip 3-attempt limit and repeat boundary advance — confirmed | v1.0.3 |
| S2 | NVS lifetime analysis — CLOSED. `flash_write_interval: 30s`. ~34 years at scenario usage. | v1.0.3 |

---

## Release Notes (current)

- **CONFIG_PREF_HASH `0xAB01000D`**: current. Bump on IrrigationConfig struct changes.
- **`flash_write_interval: 10s`**: dominant write is `daily_water_time_sec` (currently flushed every 10s during active pump — ~6× the S2 analysis rate; 1/min decoupling planned, see HANDOVER_AUDIT item 8). Interval value does not change write count — only batching within the window.
- **`zb_push_runtime_status`**: guarded — only runs during `cycle_running || manual_zone_mode`. Interval 10s. Pushes zone_remaining, cycle_remaining, daily_water. Uptime pushed separately on 1min interval unconditionally.
- **`zb_push_powerloss_pending`**: pushes cycle EP genOnOff ON (via `saved_cycle_type`) alongside cycle_state/active_zone/queue/power_loss — so HA/Z2M shows a cancel button in powerloss-waiting state.
- **`zb_push_boot_status` stale cleanup**: cycle genOnOff clear and authoritative state push now happen atomically at +3s — eliminates flicker where stale cycle switch was cleared at t=0 then re-set at t=3s.
- **Manual zone `zb_active_zone`**: `start_single_zone_manual` calls `zb_push_zone_start`; `stop_single_zone_manual` calls `zb_push_cycle_state_zero` (clears zone/cycle/remaining without touching `zb_queue_state`).
- **`maintenance_lock`**: ON=maintenance active=irrigation BLOCKED. `irrigation_allowed` binary sensor is the inverse.
- **`boot_decision_ran`**: guards `run_boot_decision_tree` against double execution.
- **`nvs_seed_active`**: prevents NVS echo being treated as ZB coordinator sync in `on_time_sync`.
- **RTC poll guard**: 5-min RTC poll skipped entirely when `zb.is_connected()`.
- **`cycle_repeat_count`**: stored 0-2, displayed 1-3. ZB sends/receives display value.
- **`scale_enable`**: 1=irr only, 2=short only, 3=both. Filtering in `zone_irr_duration_sec()` / `zone_short_duration_sec()`.
- **`irr_schedule2`**: second irrigation schedule on EP12 attrs 0x0003/0x0004.
- **EP18 attr map**: RW 0x0001–0x000E, RO 0x000F–0x0018. See `zigbee_global_endpoints.yaml` header comment.
- **`zb_network_switch`** (0x000D): stub, renamed from `zb_wifi_ota`. AP mode not supported.
- **`zb_skip`** (0x000B): WO momentary. Self-resets after activation. Calls `skip_zone_script`.
- **`zb_reboot`** (0x000E): WO momentary. Self-resets before reboot (500ms delay). Reports false on boot.
- **`zb_uptime`** (0x0018): U32 seconds since boot. Pushed every 60s on its own 1-min interval.
- **`irr_onoff`/`short_onoff`**: named genOnOff attrs on EP16/EP17. No APS binding. State pushed explicitly via `zb_push_cycle_genOnOff` (called from `zb_push_cycle_start`, `zb_push_cycle_stop`, `powerloss_resume`).
- **`zone_start_counter`**: restore_value yes. Overwatering protection for powerloss-resume. Skip after 3 failed attempts on same zone. Resets on zone advance/skip/abort/fresh start/midnight.
- **`do_start` unified**: both auto-resume and manual-confirm paths set identical ZB state (EP16/17 genOnOff, cycle_state, active_zone, queue_state, power_loss, pause). Auto-resume delays 2s then starts. Manual shows PAGE_POWERLOSS.
- **`zb_push_boot_status` stale cleanup**: uses `current_zone_num` if `cycle_running` to avoid zeroing active zone. Cycle genOnOff only cleared if not running. 3s push dispatches to `zb_push_cycle_start` / `zb_push_powerloss_pending` / `zb_push_cycle_stop` based on state.
- **Z2M converter**: ESM `.mjs`, Z2M ≥ 2.9.2. Auto-detects zones via cluster 0xFFF0. EP map cached per IEEE address. Cycle EPs (16/17) not bound in `configure()` — explicit push only.
- **APS binding table**: default 16 slots. Current usage: 11 zone EPs (11-zone build). Max zones without table edit: 15.
- **Firmware variants**: `zones: 7|11|15`, `language: hu|en` substitutions. Files in `zones/` and `localisation/`. 15-zone EP file is a stub — run `zones/generate_endpoints.py 15`.
- **External component**: fadf848a. `zigbee.cpp` has `esp_zb_aps_src_binding_table_size_set(32)` before `esp_zb_init()` — edit lost on cache clear.
- **Button LED feedback**: all three buttons — LED off on press, on at 1000ms threshold (medium push), B1 blinks at 5000ms (long push).
- **Status bar priority**: io_fault > maintenance > queue > scale≠100% (IC_WEATHER) > rtc_battery/rtc_fault.
- **`wifi_active`** (renamed from `wifi_ap_active`): stub global, restore_value yes. Used by ADV settings entry 3 (network selector placeholder).
- **Midnight cleanup**: paused cycles cleared at midnight on both boot path and runtime path. Running cycles not interrupted.

---

## Implemented / Closed

| #   | Description                                                                                                  | Version   |
| --- | ------------------------------------------------------------------------------------------------------------ | --------- |
| —   | RTC (DS3231) primary time source; OSF validation; NVS fallback seed                                          | v0.9.2    |
| —   | WS2812 RGB LED state machine                                                                                 | v0.9.2    |
| —   | Zigbee factory reset via B1 long press only                                                                  | v0.9.4    |
| —   | Auto-resume on power loss (ADV toggle)                                                                       | v0.9.2    |
| —   | Manual date/time settings page + DST-aware UTC conversion                                                    | v0.9.2/50 |
| —   | Cycle repeat: persist, display, counter                                                                      | v0.9.2    |
| —   | Current sensor dropped; archived                                                                             | v0.9.2    |
| —   | Powerloss resume: zone, repeat, zone_run_count correct                                                       | v0.9.3    |
| —   | Cycle queue: `queued_cycle_num` (restore_value: yes); t1 > t2 priority                                       | v0.9.3    |
| —   | Boot decision tree: powerloss + queue + missed cycle unified                                                 | v0.9.3    |
| —   | PAGE_PUMP_FAULT: io/maintenance/rtc/battery/oled fault pages                                                 | v0.9.4    |
| —   | I2C health checks at boot: MCP23017, SH1107                                                                  | v0.9.4    |
| —   | `zb_fault_state` EP14; `zb_push_fault_state` script                                                          | v0.9.4    |
| —   | Schedule trigger: moved from 15-min interval to on_time per-minute                                           | v0.9.50   |
| —   | `nvs_seed_active` guard                                                                                      | v0.9.50   |
| —   | RTC poll guard: skipped when ZB connected                                                                    | v0.9.50   |
| —   | LED pattern overhaul                                                                                         | v0.9.50   |
| —   | Screensaver: 3 views, night-off, dim brightness                                                              | v0.9.50   |
| —   | Settings split into 8 subpages                                                                               | v0.9.50   |
| —   | `maintenance_lock` replaces `zb_irr_allowed`                                                                 | v0.9.50   |
| —   | `irr_schedule2`: second irrigation schedule                                                                  | v0.9.50   |
| —   | `scale_enable`: per-cycle-type scale filtering                                                               | v0.9.50   |
| —   | EP14 renumbered; EP12/13 unified 2-schedule structure                                                        | v0.9.50   |
| —   | Z2M converter: full rewrite for Z2M 2.9.2, ESM, auto-detect                                                  | v0.9.50   |
| ZC7 | Z2M TABLE_FULL fix: custom cluster binds removed; APS table 16 slots                                         | v0.9.80   |
| —   | `do_start` refactored: uniform pending state, single authority                                               | v0.9.80   |
| —   | `zb_push_boot_status`: stale ZCL cleanup; conditional cycle/powerloss/stop dispatch                          | v0.9.80   |
| —   | `sync_zone_zb_state`: pointer array; respects `cycle_running` in boot cleanup                                | v0.9.80   |
| —   | `start_cycle_actual`: shared script; wrappers for irr/short                                                  | v0.9.80   |
| —   | Zone count variants: `zones/` with `zone_config_7/11/15.h`, `relays_7/11/15.yaml`, ZB endpoints, generator   | v0.9.80   |
| —   | Localisation: `localisation/config_hu.h` and `config_en.h`; `config.h` IC_* only                             | v0.9.80   |
| —   | `hardware.yaml` split: relay/switch section → `zones/relays_${zones}.yaml`                                   | v0.9.80   |
| —   | Substitutions: `zones`, `language` in `irrigation_control.yaml`                                              | v0.9.80   |
| —   | EP14 new attrs: `zb_skip` (0x000B), `zb_network_switch` (0x000D), `zb_reboot` (0x000E), `zb_uptime` (0x0018) | v0.9.80   |
| —   | `irr_onoff`/`short_onoff` named genOnOff attrs on EP16/EP17; `zb_push_cycle_genOnOff`                        | v0.9.80   |
| —   | Cycle EP (16/17) binding removed from converter `configure()`                                                | v0.9.80   |
| —   | `zb_reboot` and `zb_skip` self-reset; `fzGlobal` processes false reports                                     | v0.9.80   |
| —   | `zb_wifi_ota` → `zb_network_switch`; `wifi_ap_active` → `wifi_active`                                        | v0.9.90   |
| —   | Button medium push LED feedback: all 3 buttons, 1000ms threshold                                             | v0.9.90   |
| —   | Status bar: `IC_WEATHER` at scale≠100%; priority order updated                                               | v0.9.90   |
| —   | B2/B3 weather page: use `calc_edit_step` instead of hardcoded ±1                                             | v0.9.90   |
| —   | `start_cycle_actual`: NVS written after `zone_queue_size` check; abort clears `saved_cycle_type=0`           | v0.9.90   |
| —   | Boot decision tree: discards interrupted cycle with zone=0 as invalid                                        | v0.9.90   |
| —   | Boot decision tree: discards missed cycles with no valid zones (scale=0% or scale_enable mismatch)           | v0.9.90   |
| —   | `do_start` validation: checks valid zone count before any state write                                        | v0.9.90   |
| —   | `zone_start_counter`: overwatering protection for powerloss-resume; skip after 3 attempts                    | v0.9.95   |
| —   | `powerloss_resume`: zone resolution loop (no recursion); handles zero-duration and stuck zones               | v0.9.95   |
| —   | `powerloss_resume`: calls `zb_push_cycle_start` on successful resume                                         | v0.9.80   |
| —   | `do_start` unified: both paths set identical ZB state; auto-resume delays 2s via `auto_resume_delay`         | v0.9.95   |
| —   | B1 skip lockout: calls `zb_push_lockout_stop` + `rgb_update_state` before restarting cycle                   | v0.9.80   |
| —   | `pump_lockout_countdown`: cleanup runs unconditionally after loop exit                                       | v0.9.80   |
| —   | `selected_cycle_num` set correctly when cycle is blocked by pump protection                                  | v0.9.80   |
| —   | Edit inverted rect width: 30px for numeric/signed fields (was 20/26, cut off 3-digit values)                 | v0.9.50   |
| —   | Manual running page: queue indicator uses IC_QUEUE right of text (replaced IC_SPRINKLER overlap)             | v0.9.100  |
| —   | `draw_page_wifi_ota` removed; `PAGE_WIFI_OTA` removed from enum                                              | v0.9.80   |
| ZC1 | Full converter functional test vs Z2M 2.9.2: all 11 zones, cycle start/stop confirmed                        | v0.9.80   |
| ZC4 | Zone ON/OFF via ZB: all 11 zones confirmed                                                                   | v0.9.80   |
| ZC9 | `ctrl_irr`/`ctrl_short` not reporting — fixed via named attrs + explicit push                                | v0.9.80   |
| S7  | Button medium push timing (1000ms) + LED feedback — implemented all 3 buttons                                | v0.9.80   |
| H2  | Zone count variants 7/11/15 — implemented via substitutions                                                  | v0.9.80   |
