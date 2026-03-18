// ============================================================================
// kutvezerles.mjs - Zigbee2MQTT external converter
// ESP32-C6 Zigbee Irrigation Controller (luar123/zigbee_esphome)
//
// Place in: /docker/zigbee2mqtt_data/external_converters/kutvezerles.mjs
//
// EP 1-11:  ON_OFF + 0xFFF0 (zone config: irr/short/manual duration, flags)
// EP 12:    ON_OFF + 0xFFF1 (irr schedule: enabled U8, time_min U16)
// EP 13:    ON_OFF + 0xFFF1 (short schedules: enabled U8, time_min U16 each)
// EP 14:    0xFFF2 (device globals: scale, pump config, runtime status)
//
// Schedule times are minutes-since-midnight (0-1439).
// Decode: hour = Math.floor(val/60), minute = val%60
// Encode: val = hour*60 + minute
// ============================================================================

import {presets as e, access as ea} from 'zigbee-herdsman-converters/lib/exposes';
import * as reporting from 'zigbee-herdsman-converters/lib/reporting';
import * as zh from 'zigbee-herdsman';

const CLUSTER_ZONE   = 0xFFF0;
const CLUSTER_SCHED  = 0xFFF1;
const CLUSTER_GLOBAL = 0xFFF2;

const ZONE_ATTRS = {
    irr_duration_min:    1,
    short_duration_min:  2,
    manual_duration_min: 3,
    flags:               4,
};

// EP14 attribute map
const GLOBAL_ATTRS = {
    1:  {key: 'duration_scale_percent', type: zh.Zcl.DataType.UINT8},
    2:  {key: 'scale_reset_hours',      type: zh.Zcl.DataType.UINT8},
    3:  {key: 'pump_lockout_sec',        type: zh.Zcl.DataType.UINT8},
    4:  {key: 'pump_start_delay_sec',    type: zh.Zcl.DataType.UINT8},
    5:  {key: 'irrigation_allowed',      type: zh.Zcl.DataType.BOOLEAN},
    6:  {key: 'cycle_running',           type: zh.Zcl.DataType.BOOLEAN},
    7:  {key: 'current_zone_num',        type: zh.Zcl.DataType.UINT8},
    8:  {key: 'zone_remaining_sec',      type: zh.Zcl.DataType.UINT16},
    9:  {key: 'cycle_remaining_sec',     type: zh.Zcl.DataType.UINT16},
    10: {key: 'daily_water_time_sec',    type: zh.Zcl.DataType.UINT32},
    11: {key: 'pump_lockout_active',     type: zh.Zcl.DataType.BOOLEAN},
    12: {key: 'power_loss_detected',     type: zh.Zcl.DataType.BOOLEAN},
    13: {key: 'pump_running',            type: zh.Zcl.DataType.BOOLEAN},
    14: {key: 'pump_fault',              type: zh.Zcl.DataType.BOOLEAN},
};
const GLOBAL_KEY_TO_ID = Object.fromEntries(
    Object.entries(GLOBAL_ATTRS).map(([id, {key}]) => [key, parseInt(id)])
);

// --- fromZigbee ---

const fzOnOff = {
    cluster: 'genOnOff',
    type: ['attributeReport', 'readResponse'],
    convert: (model, msg, publish, options, meta) => {
        const ep = msg.endpoint.ID;
        if (ep >= 1 && ep <= 11)
            return {[`zone_${ep}_state`]: msg.data.onOff ? 'ON' : 'OFF'};
        if (ep === 12) return {irrigation_cycle: msg.data.onOff ? 'ON' : 'OFF'};
        if (ep === 13) return {short_cycle:      msg.data.onOff ? 'ON' : 'OFF'};
    },
};

const fzZoneConfig = {
    cluster: CLUSTER_ZONE,
    type: ['attributeReport', 'readResponse'],
    convert: (model, msg, publish, options, meta) => {
        const ep = msg.endpoint.ID;
        const result = {};
        for (const [name, id] of Object.entries(ZONE_ATTRS))
            if (msg.data[String(id)] !== undefined)
                result[`zone_${ep}_${name}`] = msg.data[String(id)];
        return result;
    },
};

const fzSchedConfig = {
    cluster: CLUSTER_SCHED,
    type: ['attributeReport', 'readResponse'],
    convert: (model, msg, publish, options, meta) => {
        const ep = msg.endpoint.ID;
        const result = {};
        if (ep === 12) {
            if (msg.data['1'] !== undefined) result['irr_enabled']   = msg.data['1'];
            if (msg.data['2'] !== undefined) result['irr_time_min']  = msg.data['2'];
        } else if (ep === 13) {
            if (msg.data['1'] !== undefined) result['short1_enabled']  = msg.data['1'];
            if (msg.data['2'] !== undefined) result['short1_time_min'] = msg.data['2'];
            if (msg.data['3'] !== undefined) result['short2_enabled']  = msg.data['3'];
            if (msg.data['4'] !== undefined) result['short2_time_min'] = msg.data['4'];
        }
        return result;
    },
};

const fzGlobalConfig = {
    cluster: CLUSTER_GLOBAL,
    type: ['attributeReport', 'readResponse'],
    convert: (model, msg, publish, options, meta) => {
        const result = {};
        for (const [id, {key}] of Object.entries(GLOBAL_ATTRS))
            if (msg.data[String(id)] !== undefined)
                result[key] = msg.data[String(id)];
        return result;
    },
};

// --- toZigbee ---

const tzOnOff = {
    key: [
        ...Array.from({length: 11}, (_, i) => `zone_${i+1}_state`),
        'irrigation_cycle', 'short_cycle',
    ],
    convertSet: async (entity, key, value, meta) => {
        const cmd = value === 'ON' ? 'on' : 'off';
        let epNum;
        if (key.startsWith('zone_'))         epNum = parseInt(key.split('_')[1]);
        else if (key === 'irrigation_cycle') epNum = 12;
        else if (key === 'short_cycle')      epNum = 13;
        await meta.device.getEndpoint(epNum).command('genOnOff', cmd, {});
        return {state: {[key]: value}};
    },
    convertGet: async (entity, key, meta) => {
        let epNum;
        if (key.startsWith('zone_'))         epNum = parseInt(key.split('_')[1]);
        else if (key === 'irrigation_cycle') epNum = 12;
        else if (key === 'short_cycle')      epNum = 13;
        await meta.device.getEndpoint(epNum).read('genOnOff', ['onOff']);
    },
};

const tzZoneConfig = {
    key: Array.from({length: 11}, (_, i) =>
        Object.keys(ZONE_ATTRS).map(n => `zone_${i+1}_${n}`)).flat(),
    convertSet: async (entity, key, value, meta) => {
        const match = key.match(/^zone_(\d+)_(.+)$/);
        const epNum = parseInt(match[1]);
        const attrId = ZONE_ATTRS[match[2]];
        if (attrId === undefined) return;
        await meta.device.getEndpoint(epNum).write(
            CLUSTER_ZONE,
            {[String(attrId)]: {value: parseInt(value), type: zh.Zcl.DataType.UINT8}},
            {disableDefaultResponse: true}
        );
        return {state: {[key]: value}};
    },
    convertGet: async (entity, key, meta) => {
        const match = key.match(/^zone_(\d+)_(.+)$/);
        const epNum = parseInt(match[1]);
        const attrId = ZONE_ATTRS[match[2]];
        if (attrId === undefined) return;
        await meta.device.getEndpoint(epNum).read(CLUSTER_ZONE, [attrId]);
    },
};

const tzSchedConfig = {
    key: ['irr_enabled', 'irr_time_min',
          'short1_enabled', 'short1_time_min',
          'short2_enabled', 'short2_time_min'],
    convertSet: async (entity, key, value, meta) => {
        let epNum, attrId, dataType, val;
        if      (key === 'irr_enabled')      { epNum=12; attrId=1; dataType=zh.Zcl.DataType.UINT8;  val=parseInt(value); }
        else if (key === 'irr_time_min')     { epNum=12; attrId=2; dataType=zh.Zcl.DataType.UINT16; val=parseInt(value); }
        else if (key === 'short1_enabled')   { epNum=13; attrId=1; dataType=zh.Zcl.DataType.UINT8;  val=parseInt(value); }
        else if (key === 'short1_time_min')  { epNum=13; attrId=2; dataType=zh.Zcl.DataType.UINT16; val=parseInt(value); }
        else if (key === 'short2_enabled')   { epNum=13; attrId=3; dataType=zh.Zcl.DataType.UINT8;  val=parseInt(value); }
        else if (key === 'short2_time_min')  { epNum=13; attrId=4; dataType=zh.Zcl.DataType.UINT16; val=parseInt(value); }
        else return;
        await meta.device.getEndpoint(epNum).write(
            CLUSTER_SCHED,
            {[String(attrId)]: {value: val, type: dataType}},
            {disableDefaultResponse: true}
        );
        return {state: {[key]: value}};
    },
    convertGet: async (entity, key, meta) => {
        let epNum, attrId;
        if      (key === 'irr_enabled')      { epNum=12; attrId=1; }
        else if (key === 'irr_time_min')     { epNum=12; attrId=2; }
        else if (key === 'short1_enabled')   { epNum=13; attrId=1; }
        else if (key === 'short1_time_min')  { epNum=13; attrId=2; }
        else if (key === 'short2_enabled')   { epNum=13; attrId=3; }
        else if (key === 'short2_time_min')  { epNum=13; attrId=4; }
        else return;
        await meta.device.getEndpoint(epNum).read(CLUSTER_SCHED, [attrId]);
    },
};

const GLOBAL_RW_KEYS = ['duration_scale_percent', 'scale_reset_hours',
                        'pump_lockout_sec', 'pump_start_delay_sec', 'irrigation_allowed'];

const tzGlobalConfig = {
    key: GLOBAL_RW_KEYS,
    convertSet: async (entity, key, value, meta) => {
        const attrId = GLOBAL_KEY_TO_ID[key];
        if (attrId === undefined) return;
        const dataType = GLOBAL_ATTRS[attrId].type;
        let val;
        if (dataType === zh.Zcl.DataType.BOOLEAN)
            val = (value === true || value === 'true' || value === 1) ? 1 : 0;
        else
            val = parseInt(value);
        await meta.device.getEndpoint(14).write(
            CLUSTER_GLOBAL,
            {[String(attrId)]: {value: val, type: dataType}},
            {disableDefaultResponse: true}
        );
        return {state: {[key]: value}};
    },
    convertGet: async (entity, key, meta) => {
        const attrId = GLOBAL_KEY_TO_ID[key];
        if (attrId === undefined) return;
        await meta.device.getEndpoint(14).read(CLUSTER_GLOBAL, [attrId]);
    },
};

// --- exposes ---

const zoneExposes = [];
for (let z = 1; z <= 11; z++) {
    zoneExposes.push(
        e.binary(`zone_${z}_state`, ea.STATE_SET, 'ON', 'OFF')
            .withDescription(`Zone ${z}`),
        e.numeric(`zone_${z}_irr_duration_min`, ea.ALL)
            .withValueMin(0).withValueMax(60).withUnit('min')
            .withDescription(`Zone ${z} irrigation duration (0=disabled)`),
        e.numeric(`zone_${z}_short_duration_min`, ea.ALL)
            .withValueMin(0).withValueMax(60).withUnit('min')
            .withDescription(`Zone ${z} short cycle duration (0=disabled)`),
        e.numeric(`zone_${z}_manual_duration_min`, ea.ALL)
            .withValueMin(0).withValueMax(60).withUnit('min')
            .withDescription(`Zone ${z} manual duration`),
        e.numeric(`zone_${z}_flags`, ea.ALL)
            .withValueMin(0).withValueMax(3)
            .withDescription(`Zone ${z} flags: bit0=irr_enabled bit1=short_enabled`),
    );
}

export default {
    zigbeeModel: [],
    fingerprint: [{modelID: 'kutvezerles', manufacturerName: 'Custom'}],
    model: 'kutvezerles',
    vendor: 'Custom',
    description: 'ESP32-C6 Zigbee Irrigation Controller',
    fromZigbee: [fzOnOff, fzZoneConfig, fzSchedConfig, fzGlobalConfig],
    toZigbee:   [tzOnOff, tzZoneConfig, tzSchedConfig, tzGlobalConfig],
    exposes: [
        ...zoneExposes,
        // Cycle controls
        e.binary('irrigation_cycle', ea.STATE_SET, 'ON', 'OFF')
            .withDescription('Irrigation cycle (ON=start, OFF=abort)'),
        e.binary('short_cycle', ea.STATE_SET, 'ON', 'OFF')
            .withDescription('Short cycle (ON=start, OFF=abort)'),
        // Irrigation schedule (EP 12)
        e.numeric('irr_enabled',  ea.ALL).withValueMin(0).withValueMax(1)
            .withDescription('Irrigation schedule enabled (0/1)'),
        e.numeric('irr_time_min', ea.ALL).withValueMin(0).withValueMax(1439).withUnit('min')
            .withDescription('Irrigation schedule time (minutes since midnight; 360=06:00)'),
        // Short schedules (EP 13)
        e.numeric('short1_enabled',  ea.ALL).withValueMin(0).withValueMax(1)
            .withDescription('Short schedule 1 enabled'),
        e.numeric('short1_time_min', ea.ALL).withValueMin(0).withValueMax(1439).withUnit('min')
            .withDescription('Short schedule 1 time (minutes since midnight; 390=06:30)'),
        e.numeric('short2_enabled',  ea.ALL).withValueMin(0).withValueMax(1)
            .withDescription('Short schedule 2 enabled'),
        e.numeric('short2_time_min', ea.ALL).withValueMin(0).withValueMax(1439).withUnit('min')
            .withDescription('Short schedule 2 time (minutes since midnight; 1080=18:00)'),
        // EP14: RW config
        e.numeric('duration_scale_percent', ea.ALL).withValueMin(0).withValueMax(100).withUnit('%')
            .withDescription('Duration scaling (100=full, 0=skip)'),
        e.numeric('scale_reset_hours', ea.ALL).withValueMin(0).withValueMax(168).withUnit('h')
            .withDescription('Scale auto-reset window in hours (0=no reset)'),
        e.numeric('pump_lockout_sec', ea.ALL).withValueMin(0).withValueMax(255).withUnit('s')
            .withDescription('Post-cycle pump protection delay'),
        e.numeric('pump_start_delay_sec', ea.ALL).withValueMin(0).withValueMax(255).withUnit('s')
            .withDescription('Solenoid-to-pump delay'),
        e.binary('irrigation_allowed', ea.ALL, true, false)
            .withDescription('Irrigation enabled (false=maintenance lock)'),
        // EP14: RO status
        e.binary('cycle_running',        ea.STATE, true, false).withDescription('Cycle running'),
        e.numeric('current_zone_num',    ea.STATE).withValueMin(0).withValueMax(11).withDescription('Current zone'),
        e.numeric('zone_remaining_sec',  ea.STATE).withValueMin(0).withValueMax(65535).withUnit('s').withDescription('Zone time remaining'),
        e.numeric('cycle_remaining_sec', ea.STATE).withValueMin(0).withValueMax(65535).withUnit('s').withDescription('Cycle time remaining'),
        e.numeric('daily_water_time_sec',ea.STATE).withValueMin(0).withValueMax(4294967295).withUnit('s').withDescription('Total watering time today'),
        e.binary('pump_lockout_active',  ea.STATE, true, false).withDescription('Pump protection active'),
        e.binary('power_loss_detected',  ea.STATE, true, false).withDescription('Power loss on boot'),
        e.binary('pump_running',         ea.STATE, true, false).withDescription('Pump running (hall sensor - reserved)'),
        e.binary('pump_fault',           ea.STATE, true, false).withDescription('Pump fault (hall sensor - reserved)'),
    ],
    endpoint: (device) => ({
        zone1: 1, zone2: 2, zone3: 3, zone4: 4, zone5: 5,
        zone6: 6, zone7: 7, zone8: 8, zone9: 9, zone10: 10,
        zone11: 11, irrigation: 12, short: 13, globals: 14,
    }),
    configure: async (device, coordinatorEndpoint) => {
        for (let epNum = 1; epNum <= 13; epNum++) {
            const ep = device.getEndpoint(epNum);
            await reporting.bind(ep, coordinatorEndpoint, ['genOnOff']);
            await reporting.onOff(ep, {min: 0, max: 600, change: 1});
        }
        for (let epNum = 1; epNum <= 11; epNum++)
            await device.getEndpoint(epNum).read(CLUSTER_ZONE, [1, 2, 3, 4]);
        await device.getEndpoint(12).read(CLUSTER_SCHED, [1, 2]);
        await device.getEndpoint(13).read(CLUSTER_SCHED, [1, 2, 3, 4]);
        await device.getEndpoint(14).read(CLUSTER_GLOBAL, [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12]);
    },
};
