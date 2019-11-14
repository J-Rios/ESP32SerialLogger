/**************************************************************************************************/
// Project: 
// File: configuration.h
// Description: Device configuration and persistent parameters save/load functions file
// Created on: 25 dec. 2018
// Last modified date: 30 dec. 2018
// Version: 0.0.1
/**************************************************************************************************/

/* Include Guard */

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

/**************************************************************************************************/

/* C++ compiler compatibility */

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************/

/* Libraries */

// FreeRTOS libraries


// Device libraries (ESP-IDF)


// Custom libraries
#include "constants.h"
#include "globals.h"
#include "commons.h"
#include "simplespiffs.h"
#include "jsmn.h"

/**************************************************************************************************/

/* Functions */

extern void device_config_init(SimpleSPIFFS* SPIFFS, Globals* Global);
void device_config_create_default(SimpleSPIFFS* SPIFFS);
void device_config_update(SimpleSPIFFS* SPIFFS, Globals* Global);
extern int jsoneq(const char* json, jsmntok_t* tok, const char* s);

/**************************************************************************************************/

#ifdef __cplusplus
}
#endif  // extern "C"

#endif
