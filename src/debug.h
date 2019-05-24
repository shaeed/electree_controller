/*

DEBUG HEADER MODULE

Copyright (C) 2019 by Shaeed Khan

*/

#ifndef DEBUG_H
#define DEBUG_H

#include <avr/pgmspace.h>
#include <Arduino.h>
#include "prototypes.h"

#define DEBUG_MSG(...) debugSend(__VA_ARGS__)
#define DEBUG_MSG_P(...) debugSend_P(__VA_ARGS__)

#ifndef DEBUG_PORT
#define DEBUG_PORT              Serial          // Default debugging port
#endif

#ifndef SERIAL_BAUDRATE
#define SERIAL_BAUDRATE         115200          // Default baudrate
#endif

void debugSend(const char * format, ...);
void debugSend_P(PGM_P format, ...);
void debugSetup();

#endif