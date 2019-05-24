/*

COMMOM DEFINITIONS MODULE

Copyright (C) 2019 by Shaeed Khan

*/

#ifndef DEF_H
#define DEF_H

#define APP_NAME                "ELECTREE"

#define MANUFACTURER            "ELECTREE"
#define DEVICE                  "mega2560"
#define DEVICE_NAME             MANUFACTURER "_" DEVICE     // Concatenate both to get a unique device name

#ifndef ADMIN_PASS
#define ADMIN_PASS              "Shaeed@12"     // Default password
#endif

#ifndef USE_PASSWORD
#define USE_PASSWORD            1               // Insecurity caution! Disabling this will disable password querying completely.
#endif

#ifndef LOOP_DELAY_TIME
#define LOOP_DELAY_TIME         1               // Delay for this millis in the main loop [0-250] (see https://github.com/xoseperez/espurna/issues/1541)
#endif

#endif