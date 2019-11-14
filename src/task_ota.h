/**************************************************************************************************/
// Project: 
// File: task_ota.cpp
// Description: System Over-The-Air (OTA) update, through secure HTTPS, FreeRTOS task file
// Created on: 20 nov. 2018
// Last modified date: 09 dec. 2018
// Version: 1.0.0
/**************************************************************************************************/

/* Include Guard */

#ifndef OTA_H
#define OTA_H

/**************************************************************************************************/

/* C++ compiler compatibility */

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************/

/* Libraries */

// Standard C/C++ libraries
#include <string.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

// FreeRTOS libraries
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

// Device libraries (ESP-IDF)
#include <esp_system.h>
#include <esp_event_loop.h>
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"

// Custom libraries
#include "constants.h"
#include "globals.h"
#include "commons.h"
#include "buttons.h"
#include "rgbleds.h"

/**************************************************************************************************/

/* HTTPS Certificates locates in internal Blob memory */

extern const uint8_t ota_server_cert_start[] asm("_binary_otawebserver_certs_ota_cert_pem_start");
extern const uint8_t ota_server_cert_end[] asm("_binary_otawebserver_certs_ota_cert_pem_end");

/**************************************************************************************************/

/* Functions */

extern void task_ota(void *pvParameter);

extern uint8_t get_version_nums_from_str(const char* str, const uint16_t str_len, uint8_t* ver);
extern uint16_t cstr_count_char(const char* str, const uint16_t str_len, const char c);
extern int32_t cstr_get_index_char_between(const char* str, const uint16_t str_len, 
                                           const uint16_t start_from, const char c);
extern bool cstr_read_between_idx(const char* str, const uint16_t str_len, const uint16_t idx_start,
                                  const uint16_t idx_end, char* readed, const uint16_t readed_len);

/**************************************************************************************************/

#ifdef __cplusplus
}
#endif  // extern "C"

#endif
