/*

RELAY MODULE

Copyright (C) 2019 by Shaeed Khan

*/
#include "relay.h"

typedef struct {

    // Configuration variables

    unsigned char pin;          // GPIO pin for the relay
    unsigned char type;         // RELAY_TYPE_NORMAL, RELAY_TYPE_INVERSE, RELAY_TYPE_LATCHED or RELAY_TYPE_LATCHED_INVERSE
    //unsigned char reset_pin;    // GPIO to reset the relay if RELAY_TYPE_LATCHED
    //unsigned long delay_on;     // Delay to turn relay ON
    //unsigned long delay_off;    // Delay to turn relay OFF
    //unsigned char pulse;        // RELAY_PULSE_NONE, RELAY_PULSE_OFF or RELAY_PULSE_ON
    //unsigned long pulse_ms;     // Pulse length in millis

    // Status variables

    bool current_status;        // Holds the current (physical) status of the relay
    bool target_status;         // Holds the target status
    unsigned long fw_start;     // Flood window start time
    unsigned char fw_count;     // Number of changes within the current flood window
    unsigned long change_time;  // Scheduled time to change
    bool report;                // Whether to report to own topic
    bool group_report;          // Whether to report to group topic

    // Helping objects

    //Ticker pulseTicker;         // Holds the pulse back timer

} relay_t;
Vector<relay_t> _relays;
bool _relayRecursive = false;
//Ticker _relaySaveTicker;

// -----------------------------------------------------------------------------
// RELAY PROVIDERS
// -----------------------------------------------------------------------------

void _relayProviderStatus(unsigned char id, bool status) {
    // Check relay ID
    if (id >= _relays.size()) return;

    // Store new current status
    _relays[id].current_status = status;

    if (_relays[id].type == RELAY_TYPE_NORMAL) {
        digitalWrite(_relays[id].pin, status);
    } else if (_relays[id].type == RELAY_TYPE_INVERSE) {
        digitalWrite(_relays[id].pin, !status);
    } else { 
        DEBUG_MSG_P(PSTR("[RELAY] Invalid type for #%d %s\n"), id);
    }
}

/**
 * Walks the relay vector processing only those relays
 * that have to change to the requested mode
 * @bool mode Requested mode
 */
void _relayProcess(bool mode) {

    unsigned long current_time = millis();

    for (unsigned char id = 0; id < _relays.size(); id++) {

        bool target = _relays[id].target_status;

        // Only process the relays we have to change
        if (target == _relays[id].current_status) continue;

        // Only process the relays we have to change to the requested mode
        if (target != mode) continue;

        // Only process if the change_time has arrived
        if (current_time < _relays[id].change_time) continue;

        DEBUG_MSG_P(PSTR("[RELAY] #%d set to %s\n"), id, target ? "ON" : "OFF");

        // Call the provider to perform the action
        _relayProviderStatus(id, target);

        // Send MQTT
        #if MQTT_SUPPORT
            relayMQTT(id);
        #endif

        if (!_relayRecursive) {
            unsigned char boot_mode = getSetting(K_RELAY_BOOT_MODE, id, RELAY_BOOT_MODE).toInt();
            bool do_commit = ((RELAY_BOOT_SAME == boot_mode) || (RELAY_BOOT_TOGGLE == boot_mode));
            relaySave(do_commit);
        }

        _relays[id].report = false;
        _relays[id].group_report = false;
    }
}

bool relayStatus(unsigned char id, bool status, bool report, bool group_report) {

    if (id >= _relays.size()) return false;

    bool changed = false;

    if (_relays[id].current_status == status) {

    } else {
        unsigned long current_time = millis();
        unsigned long fw_end = _relays[id].fw_start + 1000 * RELAY_FLOOD_WINDOW;
        //unsigned long delay = status ? _relays[id].delay_on : _relays[id].delay_off;

        _relays[id].fw_count++;
        _relays[id].change_time = current_time;

        // If current_time is off-limits the floodWindow...
        if (current_time < _relays[id].fw_start || fw_end <= current_time) {
            // We reset the floodWindow
            _relays[id].fw_start = current_time;
            _relays[id].fw_count = 1;

        // If current_time is in the floodWindow and there have been too many requests...
        } else if (_relays[id].fw_count >= RELAY_FLOOD_CHANGES) {
            // We schedule the changes to the end of the floodWindow
            // unless it's already delayed beyond that point
            if (fw_end > current_time) {
                _relays[id].change_time = fw_end;
            }
        }

        _relays[id].target_status = status;
        if (report) _relays[id].report = true;
        if (group_report) _relays[id].group_report = true;

        DEBUG_MSG_P(PSTR("[RELAY] #%d scheduled %s in %u ms\n"),
                id, status ? "ON" : "OFF",
                (_relays[id].change_time - current_time));

        changed = true;
    }

    return changed;
}

bool relayStatus(unsigned char id, bool status) {
    return relayStatus(id, status, true, true);
}

bool relayStatus(unsigned char id) {
    // Check relay ID
    if (id >= _relays.size()) return false;

    // Get status from storage
    return _relays[id].current_status;
}

void relaySave(bool do_commit) {
    // Relay status is stored in a single byte
    // This means that, it will be stored in group of 8
    unsigned char sizeOfCurrentBatch;
    unsigned char bit = 1;
    unsigned char mask = 0;
    unsigned char count = _relays.size();
    unsigned char currentRelay;
    bool save = false;

    for(unsigned char j = 0; j <= _relays.size() / 8; j++){
        sizeOfCurrentBatch = _relays.size() > 8*(j+1) ? 8 : _relays.size()-8*j;
        bit = 1;
        mask = 0;
        save = false;

        for (unsigned char i = 0; i < sizeOfCurrentBatch; i++) {
            currentRelay = i + 8* j;
            if (relayStatus(currentRelay)) {
                save = true;
                mask += bit;
            }
            bit += bit;
        }
        
        if(do_commit && save){
            setSetting(K_RELAY_STATUS_ALL, j, mask);
            DEBUG_MSG_P(PSTR("[RELAY] Setting relay mask: %d\n"), mask);
        }
    }//End outer for loop
}

void relaySave() {
    relaySave(true);
}

void relayToggle(unsigned char id, bool report, bool group_report) {
    if (id >= _relays.size()) return;
    relayStatus(id, !relayStatus(id), report, group_report);
}

void relayToggle(unsigned char id) {
    relayToggle(id, true, true);
}

unsigned char relayCount() {
    return _relays.size();
}

unsigned char relayParsePayload(const char * payload) {

    // Payload could be "OFF", "ON", "TOGGLE"
    // or its number equivalents: 0, 1 or 2

    if (payload[0] == '0') return 0;
    if (payload[0] == '1') return 1;
    if (payload[0] == '2') return 2;

    // trim payload
    char * p = ltrim((char *)payload);

    // to lower
    unsigned int l = strlen(p);
    if (l>6) l=6;
    for (unsigned char i=0; i<l; i++) {
        p[i] = tolower(p[i]);
    }

    unsigned int value = 0xFF;
    if (strcmp(p, "off") == 0) {
        value = 0;
    } else if (strcmp(p, "on") == 0) {
        value = 1;
    } else if (strcmp(p, "toggle") == 0) {
        value = 2;
    } else if (strcmp(p, "query") == 0) {
        value = 3;
    }

    return value;
}

void _relayBoot() {

    _relayRecursive = true;
    bool trigger_save = false;
    unsigned char bit = 1;
    unsigned char mask;
    
    // Walk the relays
    bool status;
    for(unsigned char j = 0; j <= _relays.size() / 8; j++){
        unsigned char sizeOfCurrentBatch = _relays.size() > 8*(j+1) ? 8 : _relays.size()-8*j;
        bit = 1;
        mask = getSetting(K_RELAY_STATUS_ALL, j, 0x00).toInt();
        DEBUG_MSG_P(PSTR("[RELAY] Retrieving mask: %d\n"), mask);
        trigger_save = false;

        for (unsigned char i = 0; i < sizeOfCurrentBatch; i++) {
            unsigned char currentRelay = i + 8* j;
            unsigned char boot_mode = getSetting(K_RELAY_BOOT_MODE, currentRelay, RELAY_BOOT_MODE).toInt();
            DEBUG_MSG_P(PSTR("[RELAY] Relay #%d boot mode %d\n"), currentRelay, boot_mode);

            status = false;
            switch (boot_mode) {
                case RELAY_BOOT_SAME:
                    status = ((mask & bit) == bit);
                    break;
                case RELAY_BOOT_TOGGLE:
                    status = ((mask & bit) != bit);
                    mask ^= bit;
                    trigger_save = true;
                    break;
                case RELAY_BOOT_ON:
                    status = true;
                    break;

                case RELAY_BOOT_OFF:
                default:
                    break;
            }

            _relays[currentRelay].current_status = !status;
            _relays[currentRelay].target_status = status;
            //_relays[currentRelay].change_time = millis();

            bit <<= 1;
        }

        // Save if there is any relay in the RELAY_BOOT_TOGGLE mode
        if (trigger_save) {
            //EEPROMr.write(EEPROM_RELAY_STATUS, mask);
            //eepromCommit();
            setSetting(K_RELAY_BOOT_MODE, j, mask);
        }
    }

    _relayRecursive = false;
}

void _relayConfigure() {
    for (char i = 0; i < _relays.size(); i++) {
        if (GPIO_NONE == _relays[i].pin) continue;

        pinMode(_relays[i].pin, OUTPUT);
        /*if (GPIO_NONE != _relays[i].reset_pin) {
            pinMode(_relays[i].reset_pin, OUTPUT);
        }*/
        if (_relays[i].type == RELAY_TYPE_INVERSE) {
            //set to high to block short opening of relay
            digitalWrite(_relays[i].pin, HIGH);
        }
    }
}

//------------------------------------------------------------------------------
// MQTT
//------------------------------------------------------------------------------

void relayMQTT(unsigned char id) {

    if (id >= _relays.size()) return;

    // Send state topic
    if (_relays[id].report) {
        _relays[id].report = false;
        mqttSend(MQTT_TOPIC_RELAY, id, _relays[id].current_status ? RELAY_MQTT_ON : RELAY_MQTT_OFF);
    }

    // Send speed for IFAN02
    /*#if defined (ITEAD_SONOFF_IFAN02)
        char buffer[5];
        snprintf(buffer, sizeof(buffer), "%u", getSpeed());
        mqttSend(MQTT_TOPIC_SPEED, buffer);
    #endif*/
}

void relayMQTT() {
    for (unsigned int id=0; id < _relays.size(); id++) {
        mqttSend(MQTT_TOPIC_RELAY, id, _relays[id].current_status ? RELAY_MQTT_ON : RELAY_MQTT_OFF);
    }
}

void relayStatusWrap(unsigned char id, unsigned char value) {
    switch (value) {
        case 0:
            relayStatus(id, false, true, false);
            break;
        case 1:
            relayStatus(id, true, true, false);
            break;
        case 2:
            relayToggle(id, true, true);
            break;
        default:
            _relays[id].report = true;
            relayMQTT(id);
            break;
    }
}

void relayMQTTCallback(unsigned int type, const char * topic, const char * payload) {

    if (type == MQTT_CONNECT_EVENT) {

        // Send status on connect
        relayMQTT();

        // Subscribe to own /set topic
        char relay_topic[strlen(MQTT_TOPIC_RELAY) + 3];
        snprintf_P(relay_topic, sizeof(relay_topic), PSTR("%s/+"), MQTT_TOPIC_RELAY);
        mqttSubscribe(relay_topic);
    }

    if (type == MQTT_MESSAGE_EVENT) {

        String t = mqttMagnitude((char *) topic);

        // magnitude is relay/#
        if (t.startsWith(MQTT_TOPIC_RELAY)) {

            // Get relay ID
            unsigned int id = t.substring(strlen(MQTT_TOPIC_RELAY)+1).toInt();
            if (id >= relayCount()) {
                DEBUG_MSG_P(PSTR("[RELAY] Wrong relayID (%d)\n"), id);
                return;
            }

            // Get value
            unsigned char value = relayParsePayload(payload);
            if (value == 0xFF) return;

            relayStatusWrap(id, value, false);

            return;
        }
    }

    if (type == MQTT_DISCONNECT_EVENT) {
        /*for (unsigned int i=0; i < _relays.size(); i++){
            int reaction = getSetting("relayOnDisc", i, 0).toInt();
            if (1 == reaction) {     // switch relay OFF
                DEBUG_MSG_P(PSTR("[RELAY] Reset relay (%d) due to MQTT disconnection\n"), i);
                relayStatusWrap(i, false, false);
            } else if(2 == reaction) { // switch relay ON
                DEBUG_MSG_P(PSTR("[RELAY] Set relay (%d) due to MQTT disconnection\n"), i);
                relayStatusWrap(i, true, false);
            }
        }*/
    }
}

void relaySetupMQTT() {
    mqttRegister(relayMQTTCallback);
}


//------------------------------------------------------------------------------
// Setup
//------------------------------------------------------------------------------

void _relayLoop() {
    _relayProcess(false);
    _relayProcess(true);
}

void relaySetup() {
    //Number of relays
    char noOfRelays = getSetting(K_NO_OF_RELAYS, 1).toInt();
    for(char i = 0; i < noOfRelays; i++) {
        _relays.push_back((relay_t) { getSetting(K_RELAY_PIN, i, GPIO_NONE).toInt(),
                                    getSetting(K_RELAY_TYPE, i, RELAY_TYPE_INVERSE).toInt(),
                                    //GPIO_NONE,
                                    //RELAY_DELAY_ON,
                                    //RELAY_DELAY_OFF 
                                    });
    }

    _relayConfigure();
    _relayBoot();
    _relayLoop();

    relaySetupMQTT();

    // Main callbacks
    espurnaRegisterLoop(_relayLoop);
    espurnaRegisterReload(_relayConfigure);

    DEBUG_MSG_P(PSTR("[RELAY] Number of relays: %d\n"), _relays.size());
}
