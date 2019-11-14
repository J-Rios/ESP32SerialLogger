/**************************************************************************************************/
// Project: WiFiStatusLight
// File: task_wifistatus.cpp
// Description: WiFi status FreeRTOS task file
// Created on: 17 nov. 2018
// Last modified date: 01 jan. 2019
// Version: 1.0.1
/**************************************************************************************************/

/* Include Guard */

#ifndef WIFISTATUS_H
#define WIFISTATUS_H

/**************************************************************************************************/

/* C++ compiler compatibility */

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************/

/* Libraries */

// Standard C/C++ libraries
#include <string.h>

// FreeRTOS libraries
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

// Device libraries (ESP-IDF)
#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_event_loop.h>

// Custom libraries
#include "constants.h"
#include "globals.h"
#include "commons.h"
#include "configuration.h"
#include "rgbleds.h"

/**************************************************************************************************/

/* Functions */

extern void task_wifi_status(void *pvParameter);

extern void wifi_start_stat(Globals* Global);

/**************************************************************************************************/

#ifdef __cplusplus
}
#endif  // extern "C"

#endif
