/*

ELECTREE MAIN MODULE

Copyright (C) 2019 by Shaeed Khan

*/

#include <Arduino.h>

#include "prototypes.h"
#include "debug.h"
#include "settings.h"
#include "utils.h"
#include "uartmqtt.h"
#include "Vector.h"

Vector<void (*)()> _loop_callbacks;
Vector<void (*)()> _reload_callbacks;

void espurnaRegisterLoop(void (*callback)()) {
    _loop_callbacks.push_back(callback);
}

void espurnaRegisterReload(void (*callback)()) {
    _reload_callbacks.push_back(callback);
}

void espurnaReload() {
    for (unsigned char i = 0; i < _reload_callbacks.size(); i++) {
        (_reload_callbacks[i])();
    }
}

void setup() {

  debugSetup();

  settingsSetup();

  uartmqttSetup();
}

void loop() {
  // Call registered loop callbacks
  for (unsigned char i = 0; i < _loop_callbacks.size(); i++) {
    (_loop_callbacks[i])();
  }
  
  _uartmqttLoop();
  //_relayLoop();
}