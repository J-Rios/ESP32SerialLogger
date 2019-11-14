/**************************************************************************************************/
// Project: 
// File: task_manage_wifi.h
// Description: Device network (WiFi) management FreeRTOS task file
// Created on: 24 jan. 2019
// Last modified date: 24 jan. 2019
// Version: 1.0.0
/**************************************************************************************************/

/* Include Guard */

#ifndef MANAGE_WIFI_H
#define MANAGE_WIFI_H

/**************************************************************************************************/

/* C++ compiler compatibility */

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************/

/* Libraries */

// Standard C/C++ libraries
/*#include <string.h>

// FreeRTOS libraries
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>*/

// Device libraries (ESP-IDF)
#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_event_loop.h>

// Custom libraries
#include "constants.h"
#include "globals.h"
#include "commons.h"
//#include "rgbleds.h"

/**************************************************************************************************/

/* Functions */

extern void task_manage_wifi(void *pvParameter);

extern void wifi_init(Globals* Global);
extern void wifi_start_ap(const char* ssid, const char* pass);
extern void wifi_start_stat(Globals* Global);

/**************************************************************************************************/

#ifdef __cplusplus
}
#endif  // extern "C"

#endif
