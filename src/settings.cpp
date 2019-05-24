/*

SETTINGS MODULE

Copyright (C) 2019 by Shaeed Khan

*/

#include "settings.h"

const size_t EEPROM_SIZE = E2END + 1;
// -----------------------------------------------------------------------------
// Key-value API
// -----------------------------------------------------------------------------

void moveSetting(const char * from, const char * to) {
    String value = getSetting(from);
    if (value.length() > 0) setSetting(to, value);
    delSetting(from);
}

String getSetting(const String& key) {
    return getSetting(key, "");
}

bool delSetting(const String& key) {
    return Embedis::del(key);
}

bool delSetting(const String& key, unsigned int index) {
    return delSetting(key + String(index));
}

bool hasSetting(const String& key) {
    return getSetting(key).length() != 0;
}

bool hasSetting(const String& key, unsigned int index) {
    return getSetting(key, index, "").length() != 0;
}

// -----------------------------------------------------------------------------
// Initialization
// -----------------------------------------------------------------------------

void settingsSetup() {

    Embedis::dictionary( F("EEPROM"),
        EEPROM_SIZE,
        [](size_t pos) -> char { return EEPROM.read(pos); },
        [](size_t pos, char value) { EEPROM.write(pos, value); }
    );

}