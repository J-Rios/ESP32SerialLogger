/**************************************************************************************************/
// Project: 
// File: task_internetstatus.cpp
// Description: Internet status FreeRTOS task file
// Created on: 21 dec. 2018
// Last modified date: 22 dec. 2018
// Version: 1.0.0
/**************************************************************************************************/

/* Include Guard */

#ifndef INTERNETSTATUS_H
#define INTERNETSTATUS_H

/**************************************************************************************************/

/* C++ compiler compatibility */

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************/

/* Libraries */

// Standard C/C++ libraries
#include <string.h>

// Device libraries (ESP-IDF)
#include <lwip/sockets.h>
#include <ping/ping.h>
#include <esp_ping.h>

// Custom libraries
#include "constants.h"
#include "globals.h"
#include "commons.h"

/**************************************************************************************************/

/* Functions */

extern void task_internet_status(void *pvParameter);

extern void ping_setup(void);
extern void ping_run(const char* ip_address);
extern esp_err_t ping_result(ping_target_id_t msg_type, esp_ping_found* found);

/**************************************************************************************************/

#ifdef __cplusplus
}
#endif  // extern "C"

#endif
