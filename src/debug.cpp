/*

DEBUG MODULE

Copyright (C) 2019 by Shaeed Khan

*/

#include "debug.h"
void _debugSendSerial(const char* data) {
    DEBUG_PORT.print(data);
}

void _debugSend(const char * message) {
    //if(isMqttConnected()) {
    //    mqttSend(MQTT_TOPIC_DEBUG, message);
    //} else {
        _debugSendSerial(message);
    //}
}

// -----------------------------------------------------------------------------

void debugSend(const char * format, ...) {
    /*va_list args;
    va_start(args, format);
    char test[1];
    int len = ets_vsnprintf(test, 1, format, args) + 1;
    char * buffer = new char[len];
    ets_vsnprintf(buffer, len, format, args);
    va_end(args);

    _debugSend(buffer);

    delete[] buffer;*/
}

void debugSend_P(PGM_P format_P, ...) {
    /*char format[strlen_P(format_P)+1];
    memcpy_P(format, format_P, sizeof(format));

    va_list args;
    va_start(args, format_P);
    char test[1];
    int len = ets_vsnprintf(test, 1, format, args) + 1;
    char * buffer = new char[len];
    ets_vsnprintf(buffer, len, format, args);
    va_end(args);

    _debugSend(buffer);

    delete[] buffer;*/
}

// -----------------------------------------------------------------------------

void debugSetup() {
        DEBUG_PORT.begin(SERIAL_BAUDRATE);
}
