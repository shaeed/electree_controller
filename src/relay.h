#ifndef RELAY_H
#define RELAY_H

#include <EEPROM.h>
//#include <Ticker.h>
#include <ArduinoJson.h>
#include "vector.h"
//#include <functional>
#include "settings.h"
#include "debug.h"
#include "utils.h"

#define GPIO_NONE           0x99
#define RELAY_DELAY_ON       0
#define RELAY_DELAY_OFF      0

#define RELAY_BOOT_OFF              0
#define RELAY_BOOT_ON               1
#define RELAY_BOOT_SAME             2
#define RELAY_BOOT_TOGGLE           3

#define RELAY_TYPE_NORMAL           0
#define RELAY_TYPE_INVERSE          1
#define RELAY_TYPE_LATCHED          2
#define RELAY_TYPE_LATCHED_INVERSE  3

#define RELAY_SYNC_ANY              0
#define RELAY_SYNC_NONE_OR_ONE      1
#define RELAY_SYNC_ONE              2
#define RELAY_SYNC_SAME             3

#define RELAY_PULSE_NONE            0
#define RELAY_PULSE_OFF             1
#define RELAY_PULSE_ON              2

#define RELAY_PROVIDER_RELAY        0
#define RELAY_PROVIDER_DUAL         1
#define RELAY_PROVIDER_LIGHT        2
#define RELAY_PROVIDER_RFBRIDGE     3
#define RELAY_PROVIDER_STM          4

#define RELAY_GROUP_SYNC_NORMAL      0
#define RELAY_GROUP_SYNC_INVERSE     1
#define RELAY_GROUP_SYNC_RECEIVEONLY 2

// Default boot mode: 0 means OFF, 1 ON and 2 whatever was before
#ifndef RELAY_BOOT_MODE
#define RELAY_BOOT_MODE             RELAY_BOOT_OFF
#endif

// 0 means ANY, 1 zero or one and 2 one and only one
#ifndef RELAY_SYNC
#define RELAY_SYNC                  RELAY_SYNC_ANY
#endif

// Default pulse mode: 0 means no pulses, 1 means normally off, 2 normally on
#ifndef RELAY_PULSE_MODE
#define RELAY_PULSE_MODE            RELAY_PULSE_NONE
#endif

// Default pulse time in seconds
#ifndef RELAY_PULSE_TIME
#define RELAY_PULSE_TIME            1.0
#endif

// Relay requests flood protection window - in seconds
#ifndef RELAY_FLOOD_WINDOW
#define RELAY_FLOOD_WINDOW          3
#endif

// Allowed actual relay changes inside requests flood protection window
#ifndef RELAY_FLOOD_CHANGES
#define RELAY_FLOOD_CHANGES         5
#endif

// Pulse with in milliseconds for a latched relay
#ifndef RELAY_LATCHING_PULSE
#define RELAY_LATCHING_PULSE        10
#endif

// Do not save relay state after these many milliseconds
#ifndef RELAY_SAVE_DELAY
#define RELAY_SAVE_DELAY            1000
#endif

// Configure the MQTT payload for ON/OFF
#ifndef RELAY_MQTT_ON
#define RELAY_MQTT_ON               "1"
#endif
#ifndef RELAY_MQTT_OFF
#define RELAY_MQTT_OFF              "0"
#endif

#define EEPROM_RELAY_STATUS     0               // Address for the relay status (1 byte)
#define EEPROM_ENERGY_COUNT     1               // Address for the energy counter (4 bytes)
#define EEPROM_CUSTOM_RESET     5               // Address for the reset reason (1 byte)
#define EEPROM_CRASH_COUNTER    6               // Address for the crash counter (1 byte)
#define EEPROM_MESSAGE_ID       7               // Address for the MQTT message id (4 bytes)
#define EEPROM_ROTATE_DATA      11              // Reserved for the EEPROM_ROTATE library (3 bytes)
#define EEPROM_DATA_END         14              // End of custom EEPROM data block

#define MQTT_TOPIC_RELAY            "relay"

void _relayProviderStatus(unsigned char id, bool status);
void _relayProcess(bool mode);
void relayPulse(unsigned char id);
bool relayStatus(unsigned char id, bool status, bool report, bool group_report);
bool relayStatus(unsigned char id, bool status);
bool relayStatus(unsigned char id);
void relaySync(unsigned char id);
void relaySave(bool do_commit);
void relaySave();
void relayToggle(unsigned char id, bool report, bool group_report);
void relayToggle(unsigned char id);
unsigned char relayCount();
unsigned char relayParsePayload(const char * payload);
void _relayBoot();
void _relayConfigure();
void _relayMQTTGroup(unsigned char id);
void relayMQTT(unsigned char id);
void relayMQTT();
void relayStatusWrap(unsigned char id, unsigned char value, bool is_group_topic);
void relayMQTTCallback(unsigned int type, const char * topic, const char * payload);
void relaySetupMQTT();
void _relayLoop();
void relaySetup();

#endif