/*

EEPROM MODULE

Copyright (C) 2019 by Shaeed Khan

*/

#include "eeprom_s.h"


const size_t EEPROM_SIZE = E2END + 1;

void setup() 
{
    // Create a key-value Dictionary in EEPROM
    Embedis::dictionary( "EEPROM",
        EEPROM_SIZE,
        [](size_t pos) -> char { return EEPROM.read(pos); },
        [](size_t pos, char value) { EEPROM.write(pos, value); }
    ); 
}