// ============================================================================
// irrigation_control_z2m_converter.mjs
// ESPHome ESP32-C6 Zigbee Irrigation Controller — Z2M External Converter
// ============================================================================
// Compatible with Z2M >= 2.9.2 (zigbee-herdsman >= 10.0.0)
// ============================================================================

import {Zcl} from 'zigbee-herdsman';
import {deviceAddCustomCluster} from 'zigbee-herdsman-converters/lib/modernExtend';
import * as exposes from 'zigbee-herdsman-converters/lib/exposes';
import * as reporting from 'zigbee-herdsman-converters/lib/reporting';

const e = exposes.presets;
const ea = exposes.access;

// ============================================================================
// Cluster definitions
// ============================================================================

const CLUSTER_ZONE_ID   = 0xFFF0;
const CLUSTER_CYCLE_ID  = 0xFFF1;
const CLUSTER_GLOBAL_ID = 0xFFF2;
const CLUSTER_ZONE   = 'irrigationZoneConfig';
const CLUSTER_CYCLE  = 'irrigationSchedule';
const CLUSTER_GLOBAL = 'irrigationGlobal';

const clusterZoneConfig = deviceAddCustomCluster(CLUSTER_ZONE, {
    name: CLUSTER_ZONE,
    ID: CLUSTER_ZONE_ID,
    manufacturerCode: null,
    attributes: {
        irr_duration:    {name: 'irr_duration',    ID: 0x0001, type: Zcl.DataType.UINT8,  write: true},
        short_duration:  {name: 'short_duration',  ID: 0x0002, type: Zcl.DataType.UINT8,  write: true},
        manual_duration: {name: 'manual_duration', ID: 0x0003, type: Zcl.DataType.UINT8,  write: true},
        flags:           {name: 'flags',           ID: 0x0004, type: Zcl.DataType.UINT8,  write: true},
    },
    commands: {},
    commandsResponse: {},
});

const clusterSchedule = deviceAddCustomCluster(CLUSTER_CYCLE, {
    name: CLUSTER_CYCLE,
    ID: CLUSTER_CYCLE_ID,
    manufacturerCode: null,
    attributes: {
        sched1_en:   {name: 'sched1_en',   ID: 0x0001, type: Zcl.DataType.UINT8,  write: true},
        sched1_time: {name: 'sched1_time', ID: 0x0002, type: Zcl.DataType.UINT16, write: true},
        sched2_en:   {name: 'sched2_en',   ID: 0x0003, type: Zcl.DataType.UINT8,  write: true},
        sched2_time: {name: 'sched2_time', ID: 0x0004, type: Zcl.DataType.UINT16, write: true},
    },
    commands: {},
    commandsResponse: {},
});

const clusterGlobal = deviceAddCustomCluster(CLUSTER_GLOBAL, {
    name: CLUSTER_GLOBAL,
    ID: CLUSTER_GLOBAL_ID,
    manufacturerCode: null,
    attributes: {
        zb_duration_scale:     {name: 'zb_duration_scale',     ID: 0x0001, type: Zcl.DataType.UINT8,    write: true},
        zb_scale_reset_days:   {name: 'zb_scale_reset_days',   ID: 0x0002, type: Zcl.DataType.UINT8,    write: true},
        zb_scale_enable:       {name: 'zb_scale_enable',       ID: 0x0003, type: Zcl.DataType.UINT8,    write: true},
        zb_pump_lockout:       {name: 'zb_pump_lockout',       ID: 0x0004, type: Zcl.DataType.UINT8,    write: true},
        zb_pump_start_offset:  {name: 'zb_pump_start_offset',  ID: 0x0005, type: Zcl.DataType.UINT8,    write: true},
        zb_pump_stop_offset:   {name: 'zb_pump_stop_offset',   ID: 0x0006, type: Zcl.DataType.UINT8,    write: true},
        zb_zone_switch_delay:  {name: 'zb_zone_switch_delay',  ID: 0x0007, type: Zcl.DataType.UINT8,    write: true},
        zb_cycle_repeat:       {name: 'zb_cycle_repeat',       ID: 0x0008, type: Zcl.DataType.UINT8,    write: true},
        zb_auto_resume:        {name: 'zb_auto_resume',        ID: 0x0009, type: Zcl.DataType.UINT8,    write: true},
        zb_pause:              {name: 'zb_pause',              ID: 0x000A, type: Zcl.DataType.BOOLEAN,  write: true},
        zb_skip:               {name: 'zb_skip',               ID: 0x000B, type: Zcl.DataType.BOOLEAN,  write: true},
        zb_maintenance_lock:   {name: 'zb_maintenance_lock',   ID: 0x000C, type: Zcl.DataType.BOOLEAN,  write: true},
        zb_network_switch:     {name: 'zb_network_switch',     ID: 0x000D, type: Zcl.DataType.BOOLEAN,  write: true},
        zb_reboot:             {name: 'zb_reboot',             ID: 0x000E, type: Zcl.DataType.BOOLEAN,  write: true},
        zb_cycle_state:        {name: 'zb_cycle_state',        ID: 0x000F, type: Zcl.DataType.UINT8},
        zb_queue_state:        {name: 'zb_queue_state',        ID: 0x0010, type: Zcl.DataType.UINT8},
        zb_active_zone:        {name: 'zb_active_zone',        ID: 0x0011, type: Zcl.DataType.UINT8},
        zb_cycle_remaining:    {name: 'zb_cycle_remaining',    ID: 0x0012, type: Zcl.DataType.UINT16},
        zb_zone_remaining:     {name: 'zb_zone_remaining',     ID: 0x0013, type: Zcl.DataType.UINT16},
        zb_daily_water:        {name: 'zb_daily_water',        ID: 0x0014, type: Zcl.DataType.UINT32},
        zb_pump_lockout_active:{name: 'zb_pump_lockout_active',ID: 0x0015, type: Zcl.DataType.BOOLEAN},
        zb_power_loss:         {name: 'zb_power_loss',         ID: 0x0016, type: Zcl.DataType.BOOLEAN},
        zb_fault_state:        {name: 'zb_fault_state',        ID: 0x0017, type: Zcl.DataType.UINT8},
        zb_uptime:             {name: 'zb_uptime',             ID: 0x0018, type: Zcl.DataType.UINT32},
    },
    commands: {},
    commandsResponse: {},
});

// ============================================================================
// EP classification — cluster-ID based autodetection
//
// The converter never hardcodes EP numbers. Instead it inspects each EP's
// input cluster list at runtime:
//
//   0xFFF0 present → zone EP  (key: ctrl_zone_NN, sorted by EP number)
//   0xFFF1 present → cycle EP (lowest EP# = irr, next = short)
//   0xFFF2 present → global EP
//
// This makes the converter EP-agnostic: a 15-zone firmware with EPs 1-15
// (zones), 16 (irr), 17 (short), 18 (global) works without any code change.
//
// The device reports its full endpoint list to Z2M on join, so the converter
// always has accurate cluster information when it runs.
// ============================================================================
// EP classification cache — keyed by device IEEE address.
// Built once on first use per device, reused for all subsequent converter calls.
// Cache is invalidated only by Z2M restart or device re-pair, which is exactly
// when EP assignments can legitimately change (new firmware = new pair).
const epMapCache = new Map();

function getCachedMap(device) {
    if (!device || !device.ieeeAddr) return classifyEPs(device);
    if (!epMapCache.has(device.ieeeAddr)) {
        const map = classifyEPs(device);
        // Only cache if we got a non-empty result — device may still be mid-interview
        if (map.zoneEPs.length > 0 || map.irrEP !== null || map.globalEP !== null) {
            epMapCache.set(device.ieeeAddr, map);
        }
        return map;
    }
    return epMapCache.get(device.ieeeAddr);
}

function hasCluster(ep, clusterId) {
    try { return !!ep.getInputClusters().find(c => c.ID === clusterId); }
    catch (_) { return false; }
}

// Build a full EP classification map from a real device object.
// Returns { zoneEPs: [sorted EP nums], irrEP: N, shortEP: N, globalEP: N }
// Falls back gracefully if device is null/dummy.
//
// Classification logic:
//   0xFFF0 present                    → zone EP
//   0xFFF2 present                    → global EP
//   genOnOff present, not zone/global → cycle EP (lowest = irr, next = short)
//
// Cycle EPs are identified by exclusion rather than requiring 0xFFF1 to appear
// in getInputClusters(). This is more reliable because getInputClusters() returns
// clusters from the ZDO Simple Descriptor, and custom cluster IDs registered via
// deviceAddCustomCluster (extend) may not always appear there depending on ZHC
// version and how the device advertised them.
function classifyEPs(device) {
    const fallback = {zoneEPs: [], irrEP: null, shortEP: null, globalEP: null};
    if (!device || !device.endpoints || typeof device.endpoints[Symbol.iterator] !== 'function')
        return fallback;

    const all = Array.from(device.endpoints);
    const zoneEPs = all
        .filter(ep => hasCluster(ep, CLUSTER_ZONE_ID))
        .map(ep => ep.ID)
        .sort((a, b) => a - b);

    const globalEPs = all
        .filter(ep => hasCluster(ep, CLUSTER_GLOBAL_ID))
        .map(ep => ep.ID);

    // Cycle EPs: have genOnOff (0x0006) but are not zone or global EPs
    const zoneSet   = new Set(zoneEPs);
    const globalSet = new Set(globalEPs);
    const cycleEPs = all
        .filter(ep => !zoneSet.has(ep.ID) && !globalSet.has(ep.ID) && hasCluster(ep, 0x0006))
        .map(ep => ep.ID)
        .sort((a, b) => a - b);

    return {
        zoneEPs,
        irrEP:    cycleEPs[0] ?? null,
        shortEP:  cycleEPs[1] ?? null,
        globalEP: globalEPs[0] ?? null,
    };
}

// Classify a single EP object (used in fzOnOff where we have the EP, not the device).
// Returns 'zone', 'irr', 'short', 'global', or null.
// For cycle EPs we need the sorted cycle EP list to distinguish irr from short,
// so we accept a pre-built classification map as second arg.
function classifyOneEP(ep, map) {
    if (hasCluster(ep, CLUSTER_ZONE_ID))   return 'zone';
    if (hasCluster(ep, CLUSTER_GLOBAL_ID)) return 'global';
    if (hasCluster(ep, CLUSTER_CYCLE_ID)) {
        if (ep.ID === map.irrEP)   return 'irr';
        if (ep.ID === map.shortEP) return 'short';
    }
    return null;
}

// State key helpers — zero-padded zone numbers for correct alphabetical sort
function zoneKey(zoneNum) {
    return `ctrl_zone_${String(zoneNum).padStart(2, '0')}`;
}

// EP number → state key, given a classification map
function epToKey(epNum, map) {
    if (epNum === map.irrEP)   return 'ctrl_irr';
    if (epNum === map.shortEP) return 'ctrl_short';
    if (map.zoneEPs.includes(epNum)) return zoneKey(map.zoneEPs.indexOf(epNum) + 1);
    return null;
}

// State key → EP number, given a classification map
function keyToEP(key, map) {
    if (key === 'ctrl_irr')   return map.irrEP;
    if (key === 'ctrl_short') return map.shortEP;
    const m = key.match(/^ctrl_zone_(\d+)$/);
    if (m) {
        const idx = parseInt(m[1]) - 1;
        return map.zoneEPs[idx] ?? null;
    }
    return null;
}

// ============================================================================
// Formatters
// ============================================================================

function minsToTime(mins) {
    const h = Math.floor(mins / 60) % 24;
    const m = mins % 60;
    return `${String(h).padStart(2, '0')}:${String(m).padStart(2, '0')}`;
}

function timeToMins(str) {
    if (!str || !str.includes(':')) return 0;
    const [h, m] = str.split(':').map(Number);
    return (h * 60 + m);
}

function secToMmSs(sec) {
    if (sec === undefined || sec === null || typeof sec !== 'number' || isNaN(sec)) return null;
    const s = Math.max(0, Math.round(sec));
    return `${String(Math.floor(s / 60)).padStart(2, '0')}:${String(s % 60).padStart(2, '0')}`;
}

function secToMinutes(sec) {
    return Math.round(Math.max(0, sec) / 60);
}

function u8ToSigned(v) { return v > 127 ? v - 256 : v; }
function signedToU8(v) { return v < 0 ? v + 256 : v; }

// Seconds → "Xd HH:MM:SS" or "HH:MM:SS"
function formatUptime(sec) {
    if (sec === undefined || sec === null || isNaN(sec)) return null;
    const s = Math.max(0, Math.floor(sec));
    const d = Math.floor(s / 86400);
    const h = Math.floor((s % 86400) / 3600);
    const m = Math.floor((s % 3600) / 60);
    const ss = s % 60;
    const hms = `${String(h).padStart(2,'0')}:${String(m).padStart(2,'0')}:${String(ss).padStart(2,'0')}`;
    return d > 0 ? `${d}d ${hms}` : hms;
}

// ============================================================================
// fromZigbee
// ============================================================================

const fzOnOff = {
    cluster: 'genOnOff',
    type: ['attributeReport', 'readResponse'],
    convert(model, msg, publish, options, meta) {
        const map = getCachedMap(meta.device);
        const epNum = msg.endpoint.ID;
        const key = epToKey(epNum, map);
        if (!key) return {};
        return {[key]: msg.data.onOff ? 'ON' : 'OFF'};
    },
};

const fzZoneConfig = {
    cluster: CLUSTER_ZONE,
    type: ['attributeReport', 'readResponse'],
    convert(model, msg, publish, options, meta) {
        const map = getCachedMap(meta.device);
        const zoneNum = map.zoneEPs.indexOf(msg.endpoint.ID) + 1;
        if (zoneNum === 0) return {};
        const d = msg.data;
        const result = {};
        if (d.irr_duration    !== undefined) result[`cycle_irr_zone_${zoneNum}_duration`]   = d.irr_duration;
        if (d.short_duration  !== undefined) result[`cycle_short_zone_${zoneNum}_duration`]  = d.short_duration;
        if (d.manual_duration !== undefined) result[`manual_zone_${zoneNum}_duration`]       = d.manual_duration;
        if (d.flags !== undefined) {
            result[`cycle_irr_zone_${zoneNum}_enable`]   = !!(d.flags & 0x01);
            result[`cycle_short_zone_${zoneNum}_enable`] = !!(d.flags & 0x02);
        }
        return result;
    },
};

const fzSchedule = {
    cluster: CLUSTER_CYCLE,
    type: ['attributeReport', 'readResponse'],
    convert(model, msg, publish, options, meta) {
        const map = getCachedMap(meta.device);
        const epNum = msg.endpoint.ID;
        const d = msg.data;
        const p = epNum === map.irrEP ? 'cycle_irr' : 'cycle_short';
        const result = {};
        if (d.sched1_en   !== undefined) result[`${p}_schedule_1_enable`] = !!d.sched1_en;
        if (d.sched1_time !== undefined) result[`${p}_schedule_1_time`]   = minsToTime(d.sched1_time);
        if (d.sched2_en   !== undefined) result[`${p}_schedule_2_enable`] = !!d.sched2_en;
        if (d.sched2_time !== undefined) result[`${p}_schedule_2_time`]   = minsToTime(d.sched2_time);
        return result;
    },
};

const fzGlobal = {
    cluster: CLUSTER_GLOBAL,
    type: ['attributeReport', 'readResponse'],
    convert(model, msg, publish, options, meta) {
        const d = msg.data;
        const result = {};
        if (d.zb_duration_scale    !== undefined) result['zb_duration_scale']     = d.zb_duration_scale;
        if (d.zb_scale_reset_days  !== undefined) result['zb_scale_reset_days']   = d.zb_scale_reset_days;
        if (d.zb_scale_enable      !== undefined) result['zb_scale_enable']       = (['irr','short','both'][d.zb_scale_enable - 1] || 'both');
        if (d.zb_pump_lockout      !== undefined) result['zb_pump_lockout']       = d.zb_pump_lockout;
        if (d.zb_pump_start_offset !== undefined) result['zb_pump_start_offset']  = u8ToSigned(d.zb_pump_start_offset);
        if (d.zb_pump_stop_offset  !== undefined) result['zb_pump_stop_offset']   = u8ToSigned(d.zb_pump_stop_offset);
        if (d.zb_zone_switch_delay !== undefined) result['zb_zone_switch_delay']  = d.zb_zone_switch_delay;
        if (d.zb_cycle_repeat      !== undefined) result['zb_cycle_repeat']       = d.zb_cycle_repeat;
        if (d.zb_auto_resume       !== undefined) result['zb_auto_resume']        = !!d.zb_auto_resume;
        if (d.zb_pause             !== undefined) result['ctrl_pause']            = !!d.zb_pause;
        if (d.zb_maintenance_lock  !== undefined) result['zb_maintenance_lock']   = !!d.zb_maintenance_lock;
        if (d.zb_network_switch    !== undefined) result['zb_network_switch']    = !!d.zb_network_switch;
        // zb_reboot and zb_skip are momentary — report false back so UI resets
        if (d.zb_reboot            !== undefined && !d.zb_reboot) result['zb_reboot'] = false;
        if (d.zb_skip              !== undefined && !d.zb_skip)   result['zb_skip']   = false;
        if (d.zb_cycle_state       !== undefined) result['status_cycle_state']    = (['none','irrigation','short'][d.zb_cycle_state] || 'none');
        if (d.zb_queue_state       !== undefined) result['status_queue_state']    = (['none','irrigation','short'][d.zb_queue_state] || 'none');
        if (d.zb_active_zone       !== undefined) result['status_active_zone']    = d.zb_active_zone;
        if (d.zb_cycle_remaining   !== undefined) result['status_cycle_remaining']= formatUptime(d.zb_cycle_remaining);
        if (d.zb_zone_remaining    !== undefined) result['status_zone_remaining'] = secToMmSs(d.zb_zone_remaining);
        if (d.zb_daily_water       !== undefined) result['status_daily_water']    = secToMinutes(d.zb_daily_water);
        if (d.zb_pump_lockout_active !== undefined) result['status_pump_lockout'] = !!d.zb_pump_lockout_active;
        if (d.zb_power_loss        !== undefined) result['status_power_loss']     = !!d.zb_power_loss;
        if (d.zb_fault_state       !== undefined) result['status_fault']          = (['none','rtc_battery','rtc_fault','oled_fault','io_fault'][d.zb_fault_state] || 'none');
        if (d.zb_uptime            !== undefined) result['status_uptime']         = formatUptime(d.zb_uptime);
        return result;
    },
};

// ============================================================================
// toZigbee
// ============================================================================

// All ON/OFF switches handled by explicit key matching.
// EP number is resolved at runtime via classifyEPs() — no hardcoded EP numbers.
// Static key list covers the maximum expected zone count (32) plus cycle keys.
const MAX_ZONES = 32;
const tzOnOff = {
    key: [
        'ctrl_irr',
        'ctrl_short',
        ...Array.from({length: MAX_ZONES}, (_, i) => zoneKey(i + 1)),
    ],
    convertSet: async (entity, key, value, meta) => {
        const map = getCachedMap(meta.device);
        const epNum = keyToEP(key, map);
        if (epNum === null) return {};
        const ep = meta.device.getEndpoint(epNum);
        if (!ep) throw new Error(`EP ${epNum} not found`);
        await ep.command('genOnOff', value === 'ON' ? 'on' : 'off', {});
        return {state: {[key]: value}};
    },
    convertGet: async (entity, key, meta) => {
        const map = getCachedMap(meta.device);
        const epNum = keyToEP(key, map);
        if (epNum === null) return;
        const ep = meta.device.getEndpoint(epNum);
        if (ep) await ep.read('genOnOff', ['onOff']);
    },
};

// Zone config — keys for MAX_ZONES zones; only reachable zones will match
const tzZoneConfig = {
    key: [
        ...Array.from({length: MAX_ZONES}, (_, i) => `cycle_irr_zone_${i+1}_duration`),
        ...Array.from({length: MAX_ZONES}, (_, i) => `cycle_irr_zone_${i+1}_enable`),
        ...Array.from({length: MAX_ZONES}, (_, i) => `cycle_short_zone_${i+1}_duration`),
        ...Array.from({length: MAX_ZONES}, (_, i) => `cycle_short_zone_${i+1}_enable`),
        ...Array.from({length: MAX_ZONES}, (_, i) => `manual_zone_${i+1}_duration`),
    ],
    convertSet: async (entity, key, value, meta) => {
        const match = key.match(/^(cycle_irr|cycle_short|manual)_zone_(\d+)_(duration|enable)$/);
        if (!match) return;
        const [, cycleType, zoneStr, field] = match;
        const zoneNum = parseInt(zoneStr);
        const map = getCachedMap(meta.device);
        const epNum = map.zoneEPs[zoneNum - 1];
        if (epNum === undefined) return;
        const ep = meta.device.getEndpoint(epNum);
        if (!ep) throw new Error(`Zone ${zoneNum} EP ${epNum} not found`);

        if (field === 'duration') {
            const attr = cycleType === 'cycle_irr' ? 'irr_duration'
                       : cycleType === 'cycle_short' ? 'short_duration' : 'manual_duration';
            await ep.write(CLUSTER_ZONE, {[attr]: parseInt(value)}, {disableDefaultResponse: false});
        } else {
            // Read-modify-write flags
            // NOTE: not atomic — concurrent irr_enable + short_enable writes may race
            const resp = await ep.read(CLUSTER_ZONE, ['flags']);
            const bit = cycleType === 'cycle_irr' ? 0x01 : 0x02;
            const flags = value ? (resp.flags | bit) : (resp.flags & ~bit);
            await ep.write(CLUSTER_ZONE, {flags}, {disableDefaultResponse: false});
        }
        return {state: {[key]: value}};
    },
    convertGet: async (entity, key, meta) => {
        const match = key.match(/^(cycle_irr|cycle_short|manual)_zone_(\d+)_(duration|enable)$/);
        if (!match) return;
        const zoneNum = parseInt(match[2]);
        const map = getCachedMap(meta.device);
        const epNum = map.zoneEPs[zoneNum - 1];
        if (epNum === undefined) return;
        const ep = meta.device.getEndpoint(epNum);
        if (!ep) return;
        const attrs = match[3] === 'enable' ? ['flags']
                    : match[1] === 'cycle_irr' ? ['irr_duration']
                    : match[1] === 'cycle_short' ? ['short_duration'] : ['manual_duration'];
        await ep.read(CLUSTER_ZONE, attrs);
    },
};

const tzSchedule = {
    key: ['cycle_irr_schedule_1_enable', 'cycle_irr_schedule_1_time',
          'cycle_irr_schedule_2_enable', 'cycle_irr_schedule_2_time',
          'cycle_short_schedule_1_enable', 'cycle_short_schedule_1_time',
          'cycle_short_schedule_2_enable', 'cycle_short_schedule_2_time'],
    convertSet: async (entity, key, value, meta) => {
        const match = key.match(/^(cycle_irr|cycle_short)_schedule_(\d)_(enable|time)$/);
        if (!match) return;
        const map = getCachedMap(meta.device);
        const epNum = match[1] === 'cycle_irr' ? map.irrEP : map.shortEP;
        if (epNum === null) return;
        const ep = meta.device.getEndpoint(epNum);
        if (!ep) throw new Error(`Cycle EP ${epNum} not found`);
        const base = `sched${match[2]}`;
        await ep.write(CLUSTER_CYCLE, {
            [match[3] === 'enable' ? `${base}_en` : `${base}_time`]:
            match[3] === 'enable' ? (value ? 1 : 0) : timeToMins(value)
        }, {disableDefaultResponse: false});
        return {state: {[key]: value}};
    },
    convertGet: async (entity, key, meta) => {
        const match = key.match(/^(cycle_irr|cycle_short)_schedule_(\d)_(enable|time)$/);
        if (!match) return;
        const map = getCachedMap(meta.device);
        const epNum = match[1] === 'cycle_irr' ? map.irrEP : map.shortEP;
        if (epNum === null) return;
        const ep = meta.device.getEndpoint(epNum);
        if (!ep) return;
        const base = `sched${match[2]}`;
        await ep.read(CLUSTER_CYCLE, [match[3] === 'enable' ? `${base}_en` : `${base}_time`]);
    },
};

const tzGlobal = {
    key: ['zb_duration_scale', 'zb_scale_reset_days', 'zb_scale_enable',
          'zb_pump_lockout', 'zb_pump_start_offset', 'zb_pump_stop_offset',
          'zb_zone_switch_delay', 'zb_cycle_repeat', 'zb_auto_resume',
          'ctrl_pause', 'zb_skip', 'zb_maintenance_lock', 'zb_network_switch', 'zb_reboot'],
    convertSet: async (entity, key, value, meta) => {
        const map = getCachedMap(meta.device);
        const ep14 = meta.device.getEndpoint(map.globalEP);
        if (!ep14) throw new Error('Global EP not found');
        const attrWrites = {
            zb_duration_scale:    {zb_duration_scale:    parseInt(value)},
            zb_scale_reset_days:  {zb_scale_reset_days:  parseInt(value)},
            zb_scale_enable:      {zb_scale_enable:      ({'irr':1,'short':2,'both':3})[value] || 3},
            zb_pump_lockout:      {zb_pump_lockout:      parseInt(value)},
            zb_pump_start_offset: {zb_pump_start_offset: signedToU8(parseInt(value))},
            zb_pump_stop_offset:  {zb_pump_stop_offset:  signedToU8(parseInt(value))},
            zb_zone_switch_delay: {zb_zone_switch_delay: parseInt(value)},
            zb_cycle_repeat:      {zb_cycle_repeat:      parseInt(value)},
            zb_auto_resume:       {zb_auto_resume:       value ? 1 : 0},
            ctrl_pause:           {zb_pause:             (value === 'ON' || value === true) ? 1 : 0},
            zb_maintenance_lock:  {zb_maintenance_lock:  (value === 'ON' || value === true) ? 1 : 0},
            zb_skip:              {zb_skip:              (value === 'ON' || value === true) ? 1 : 0},
            zb_network_switch:    {zb_network_switch:    (value === 'ON' || value === true) ? 1 : 0},
            zb_reboot:            {zb_reboot:            (value === 'ON' || value === true) ? 1 : 0},
        };
        if (attrWrites[key]) await ep14.write(CLUSTER_GLOBAL, attrWrites[key], {disableDefaultResponse: false});
        return {state: {[key]: value}};
    },
    convertGet: async (entity, key, meta) => {
        const map = getCachedMap(meta.device);
        const ep14 = meta.device.getEndpoint(map.globalEP);
        if (!ep14) return;
        const attrMap = {
            zb_duration_scale:    'zb_duration_scale',
            zb_scale_reset_days:  'zb_scale_reset_days',
            zb_scale_enable:      'zb_scale_enable',
            zb_pump_lockout:      'zb_pump_lockout',
            zb_pump_start_offset: 'zb_pump_start_offset',
            zb_pump_stop_offset:  'zb_pump_stop_offset',
            zb_zone_switch_delay: 'zb_zone_switch_delay',
            zb_cycle_repeat:      'zb_cycle_repeat',
            zb_auto_resume:       'zb_auto_resume',
            ctrl_pause:           'zb_pause',
            zb_maintenance_lock:  'zb_maintenance_lock',
            zb_network_switch:    'zb_network_switch',
            zb_reboot:            'zb_reboot',
            zb_skip:              'zb_skip',
        };
        if (attrMap[key]) await ep14.read(CLUSTER_GLOBAL, [attrMap[key]]);
    },
};

// ============================================================================
// Exposes builder
// ============================================================================

function buildExposes(device) {
    const map = classifyEPs(device);
    // Fallback zone list when called with a dummy device during Z2M startup
    const zones = map.zoneEPs.length > 0
        ? map.zoneEPs.map((_, i) => i + 1)
        : Array.from({length: 11}, (_, i) => i + 1);

    const ex = [];

    // Cycle and zone controls
    ex.push(e.binary('ctrl_irr',   ea.STATE_SET, 'ON', 'OFF').withLabel('Ctrl Irr').withDescription('Irrigation cycle (ON=start, OFF=abort)'));
    ex.push(e.binary('ctrl_short', ea.STATE_SET, 'ON', 'OFF').withLabel('Ctrl Short').withDescription('Short cycle (ON=start, OFF=abort)'));
    ex.push(e.binary('ctrl_pause', ea.STATE_SET, true, false).withLabel('Ctrl Pause').withDescription('Pause active cycle (or resume powerloss-pending cycle)'));
    ex.push(e.binary('zb_skip', ea.STATE_SET, true, false).withLabel('Ctrl Skip').withDescription('Skip current zone (push to activate)'));

    for (const z of zones) {
        ex.push(e.binary(zoneKey(z), ea.STATE_SET, 'ON', 'OFF').withLabel(`Ctrl Zone ${String(z).padStart(2, '0')}`).withDescription(`Zone ${z}`));
    }

    // cycle_irr schedules and per-zone config
    ex.push(e.binary('cycle_irr_schedule_1_enable', ea.ALL, true, false).withLabel('Irr Sched 1 Enable').withDescription('Irrigation schedule 1 enabled'));
    ex.push(e.text('cycle_irr_schedule_1_time', ea.ALL).withLabel('Irr Sched 1 Time').withDescription('Irrigation schedule 1 time (HH:MM, e.g. 06:00)'));
    ex.push(e.binary('cycle_irr_schedule_2_enable', ea.ALL, true, false).withLabel('Irr Sched 2 Enable').withDescription('Irrigation schedule 2 enabled'));
    ex.push(e.text('cycle_irr_schedule_2_time', ea.ALL).withLabel('Irr Sched 2 Time').withDescription('Irrigation schedule 2 time (HH:MM, e.g. 07:00)'));
    for (const z of zones) {
        ex.push(e.binary(`cycle_irr_zone_${z}_enable`, ea.ALL, true, false).withLabel(`Irr Zone ${z} Enable`).withDescription(`Zone ${z} enabled in irrigation cycle`));
        ex.push(e.numeric(`cycle_irr_zone_${z}_duration`, ea.ALL).withLabel(`Irr Zone ${z} Duration`).withUnit('min').withValueMin(0).withValueMax(60).withDescription(`Zone ${z} irrigation duration (0=disabled)`));
    }

    // cycle_short schedules and per-zone config
    ex.push(e.binary('cycle_short_schedule_1_enable', ea.ALL, true, false).withLabel('Short Sched 1 Enable').withDescription('Short schedule 1 enabled'));
    ex.push(e.text('cycle_short_schedule_1_time', ea.ALL).withLabel('Short Sched 1 Time').withDescription('Short schedule 1 time (HH:MM, e.g. 12:30)'));
    ex.push(e.binary('cycle_short_schedule_2_enable', ea.ALL, true, false).withLabel('Short Sched 2 Enable').withDescription('Short schedule 2 enabled'));
    ex.push(e.text('cycle_short_schedule_2_time', ea.ALL).withLabel('Short Sched 2 Time').withDescription('Short schedule 2 time (HH:MM, e.g. 18:00)'));
    for (const z of zones) {
        ex.push(e.binary(`cycle_short_zone_${z}_enable`, ea.ALL, true, false).withLabel(`Short Zone ${z} Enable`).withDescription(`Zone ${z} enabled in short cycle`));
        ex.push(e.numeric(`cycle_short_zone_${z}_duration`, ea.ALL).withLabel(`Short Zone ${z} Duration`).withUnit('min').withValueMin(0).withValueMax(60).withDescription(`Zone ${z} short cycle duration (0=disabled)`));
    }

    // manual durations
    for (const z of zones) {
        ex.push(e.numeric(`manual_zone_${z}_duration`, ea.ALL).withLabel(`Manual Zone ${z} Duration`).withUnit('min').withValueMin(1).withValueMax(60).withDescription(`Zone ${z} manual duration`));
    }

    // status_* — sensor, pushed by firmware on cycle events
    ex.push(e.enum('status_cycle_state', ea.STATE, ['none','irrigation','short']).withLabel('Status Cycle State').withDescription('Active cycle: none / irrigation / short'));
    ex.push(e.enum('status_queue_state', ea.STATE, ['none','irrigation','short']).withLabel('Status Queue State').withDescription('Queued cycle: none / irrigation / short'));
    ex.push(e.numeric('status_active_zone', ea.STATE).withLabel('Status Active Zone').withDescription('Current zone (0=none)'));
    ex.push(e.text('status_cycle_remaining', ea.STATE).withLabel('Status Cycle Remaining').withDescription('Cycle time remaining (HH:MM:SS)'));
    ex.push(e.text('status_zone_remaining',  ea.STATE).withLabel('Status Zone Remaining').withDescription('Zone time remaining (MM:SS)'));
    ex.push(e.numeric('status_daily_water', ea.STATE).withLabel('Status Daily Water').withUnit('min').withDescription('Total watering time today (minutes)'));

    // status_* — diagnostic
    ex.push(e.binary('status_pump_lockout', ea.STATE, true, false).withLabel('Status Pump Lockout').withDescription('Pump protection active').withCategory('diagnostic'));
    ex.push(e.binary('status_power_loss',   ea.STATE, true, false).withLabel('Status Power Loss').withDescription('Power loss detected on last boot').withCategory('diagnostic'));
    ex.push(e.enum('status_fault', ea.STATE, ['none','rtc_battery','rtc_fault','oled_fault','io_fault']).withLabel('Status Fault').withDescription('Hardware fault state').withCategory('diagnostic'));
    ex.push(e.text('status_uptime', ea.STATE).withLabel('Status Uptime').withDescription('Time since last boot (Xd HH:MM:SS)').withCategory('diagnostic'));

    // zb_* device config (EP14 RW)
    ex.push(e.numeric('zb_duration_scale', ea.ALL).withLabel('Zb Duration Scale').withUnit('%').withValueMin(0).withValueMax(200).withValueStep(10).withDescription('Duration scaling (100=full, 200=double, 0=skip; 10% steps)').withCategory('config'));
    ex.push(e.numeric('zb_scale_reset_days', ea.ALL).withLabel('Zb Scale Reset Days').withValueMin(0).withValueMax(14).withDescription('Scale auto-reset after N days (0=disabled)').withCategory('config'));
    ex.push(e.enum('zb_scale_enable', ea.ALL, ['irr','short','both']).withLabel('Zb Scale Enable').withDescription('Which cycle types are affected by duration scale (irr / short / both)').withCategory('config'));
    ex.push(e.numeric('zb_pump_lockout', ea.ALL).withLabel('Zb Pump Lockout').withUnit('s').withValueMin(0).withValueMax(30).withDescription('Post-cycle pump protection delay').withCategory('config'));
    ex.push(e.numeric('zb_pump_start_offset', ea.ALL).withLabel('Zb Pump Start Offset').withUnit('s').withValueMin(-5).withValueMax(5).withDescription('Start offset: +valve opens first (low pressure), -pump starts first').withCategory('config'));
    ex.push(e.numeric('zb_pump_stop_offset', ea.ALL).withLabel('Zb Pump Stop Offset').withUnit('s').withValueMin(-5).withValueMax(5).withDescription('Stop offset: +pump stops first (pressure bleeds), -valve closes first').withCategory('config'));
    ex.push(e.numeric('zb_zone_switch_delay', ea.ALL).withLabel('Zb Zone Switch Delay').withUnit('s').withValueMin(0).withValueMax(5).withDescription('Inter-zone relay switching delay (0=none)').withCategory('config'));
    ex.push(e.numeric('zb_cycle_repeat', ea.ALL).withLabel('Zb Cycle Repeat').withValueMin(1).withValueMax(3).withDescription('Repeat count (1=run once, 2=run twice, 3=run 3x)').withCategory('config'));
    ex.push(e.binary('zb_auto_resume', ea.ALL, true, false).withLabel('Zb Auto Resume').withDescription('Auto-resume interrupted cycle on power restore (false=show confirmation page)').withCategory('config'));

    // zb_* diagnostic controls
    ex.push(e.binary('zb_maintenance_lock', ea.ALL, true, false).withLabel('Zb Maintenance Lock').withDescription('Maintenance lock (true=irrigation blocked)').withCategory('config'));
    ex.push(e.binary('zb_network_switch', ea.ALL, true, false).withLabel('Network Switch').withDescription('Select ZB or Wifi. Not implemented yet').withCategory('config'));
    ex.push(e.binary('zb_reboot', ea.STATE_SET, true, false).withLabel('Zb Reboot').withDescription('Push to reboot device').withCategory('config'));

    return ex;
}

// ============================================================================
// Export
// ============================================================================

export default {
    zigbeeModel: ['irrigation-control'],
    model: 'ESP32-C6 Irrigation Controller',
    vendor: 'ESPHome',
    description: 'ESP32-C6 Zigbee irrigation controller (up to 15 zones, 4 schedules)',
    extend: [clusterZoneConfig, clusterSchedule, clusterGlobal],
    fromZigbee: [fzOnOff, fzZoneConfig, fzSchedule, fzGlobal],
    toZigbee: [tzOnOff, tzZoneConfig, tzSchedule, tzGlobal],
    exposes: (device, options) => buildExposes(device),
    // Dynamic endpoint map — tells Z2M's message router which named endpoints
    // exist so incoming attribute reports from all EPs are dispatched correctly.
    // Built from classifyEPs() so it auto-adapts to any zone count.
    endpoint: (device) => {
        const map = classifyEPs(device);
        const eps = {};
        for (let i = 0; i < map.zoneEPs.length; i++) {
            eps[`ep_zone${i + 1}`] = map.zoneEPs[i];
        }
        if (map.irrEP   !== null) eps['ep_irr']    = map.irrEP;
        if (map.shortEP  !== null) eps['ep_short']   = map.shortEP;
        if (map.globalEP !== null) eps['ep_global']  = map.globalEP;
        return eps;
    },
    configure: async (device, coordinatorEndpoint) => {
        const map = classifyEPs(device);

        // Bind genOnOff on all zone + cycle EPs using ZHC's reporting library.
        // Custom clusters (0xFFF0/0xFFF1/0xFFF2) are NOT bound — the device
        // pushes attribute reports spontaneously, and binding them wastes
        // binding table slots (ESP32-C6 has 16 default), causing TABLE_FULL errors
        // that abort configure before the initial reads can execute.
        // Bind genOnOff on zone EPs only — not cycle EPs (16/17).
        // Cycle EP state is pushed by explicit reportAttr calls from firmware
        // (zb_push_cycle_genOnOff script) so no APS binding is needed.
        // This keeps binding table usage at TOTAL_ZONE_COUNT slots (max 15),
        // within the default 16-slot table for all variants.
        for (const epNum of map.zoneEPs) {
            const ep = device.getEndpoint(epNum);
            if (ep) {
                await reporting.bind(ep, coordinatorEndpoint, ['genOnOff']);
                await reporting.onOff(ep, {min: 0, max: 600, change: 1});
            }
        }

        // Explicit initial reads — populate state on join.
        // The device only pushes EP14 attrs on change, not as a dump on join.
        const safeRead = async (ep, cluster, attrs) => {
            try { await ep.read(cluster, attrs); } catch (e) { /* ignore */ }
        };

        // EP14 global attrs read first — RW config attrs were timing out when read
        // last after 11 zones x 5 reads + 2 cycle EPs x 5 reads. Reading EP14
        // first ensures config attrs populate reliably on every pairing.
        if (map.globalEP !== null) {
            const ep = device.getEndpoint(map.globalEP);
            if (ep) await safeRead(ep, CLUSTER_GLOBAL, [
                'zb_duration_scale', 'zb_scale_reset_days', 'zb_scale_enable',
                'zb_pump_lockout', 'zb_pump_start_offset', 'zb_pump_stop_offset',
                'zb_zone_switch_delay', 'zb_cycle_repeat', 'zb_auto_resume',
                'zb_pause', 'zb_skip', 'zb_maintenance_lock', 'zb_network_switch',
                'zb_cycle_state', 'zb_queue_state', 'zb_active_zone',
                'zb_cycle_remaining', 'zb_zone_remaining', 'zb_daily_water',
                'zb_pump_lockout_active', 'zb_power_loss', 'zb_fault_state', 'zb_uptime',
            ]);
        }
        for (const epNum of map.zoneEPs) {
            const ep = device.getEndpoint(epNum);
            if (ep) {
                await safeRead(ep, 'genOnOff', ['onOff']);
                await safeRead(ep, CLUSTER_ZONE, ['irr_duration', 'short_duration', 'manual_duration', 'flags']);
            }
        }
        for (const epNum of [map.irrEP, map.shortEP].filter(Boolean)) {
            const ep = device.getEndpoint(epNum);
            if (ep) {
                await safeRead(ep, 'genOnOff', ['onOff']);
                await safeRead(ep, CLUSTER_CYCLE, ['sched1_en', 'sched1_time', 'sched2_en', 'sched2_time']);
            }
        }
    },
};
