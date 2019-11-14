/**************************************************************************************************/
// File: globals.h
// Description: Library to thread-safe access encapsulated globals elements from FreeRTOS Tasks.
// Created on: 17 nov. 2018
// Last modified date: 22 dec. 2018
// Version: 1.0.0
/**************************************************************************************************/

/* Include Guard */

#ifndef GLOBALS_H_
#define GLOBALS_H_

/**************************************************************************************************/

/* Libraries */

// Standard C/C++ libraries
#include <string.h> // memset(), memcpy()
#include <stdio.h> // sprintf

// Include SDK and Freertos
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Custom libraries
#include "constants.h"

/**************************************************************************************************/

/* Globals Elements Data Type */

typedef struct
{
    // Persistent Parameters
    char wifi_ssid[MAX_LENGTH_WIFI_SSID+1];
    char wifi_pass[MAX_LENGTH_WIFI_PASS+1];
    char internet_check_url[MAX_LENGTH_IPV4+1];
    char firmware_version[MAX_LENGTH_VERSION+1];
    bool first_boot_provision;

    // Volatile Parameters
    bool wifi_ap_is_up;
    bool wifi_connected;
    bool wifi_has_ip;
    bool internet_connection;
    bool ota_update;
    char wifi_ip[MAX_LENGTH_IPV4+1];
    uint8_t any_ws_client_connected;
    uint8_t ws_clients_connected[MAX_WS_CLIENTS];

    // FreeRTOS Queues
    QueueHandle_t xQueue_uart0_to_ws;
    QueueHandle_t xQueue_uart1_to_ws;
    QueueHandle_t xQueue_uart2_to_ws;
    QueueHandle_t xQueue_ws_to_uart0;
    QueueHandle_t xQueue_ws_to_uart1;
    QueueHandle_t xQueue_ws_to_uart2;
} gdata;

/**************************************************************************************************/

class Globals
{
    public:
        Globals(void);

        bool get_firmware_version(char* to_get);
        bool set_firmware_version(const char* to_set);
        
        bool get_wifi_ssid(char* to_get);
        bool set_wifi_ssid(const char* to_set);

        bool get_wifi_pass(char* to_get);
        bool set_wifi_pass(const char* to_set);

        bool get_internet_check_url(char* to_get);
        bool set_internet_check_url(const char* to_set);

        bool get_first_boot_provision(bool& to_get);
        bool set_first_boot_provision(const bool to_set);

        bool get_wifi_ap_is_up(bool& to_get);
        bool set_wifi_ap_is_up(const bool to_set);

        bool get_wifi_connected(bool& to_get);
        bool set_wifi_connected(const bool to_set);

        bool get_wifi_has_ip(bool& to_get);
        bool set_wifi_has_ip(const bool to_set);

        bool get_internet_connection(bool& to_get);
        bool set_internet_connection(const bool to_set);

        bool get_ota_update(bool& to_get);
        bool set_ota_update(const bool to_set);

        bool get_wifi_ip(char* to_get);
        bool set_wifi_ip(const char* to_set);

        bool get_any_ws_clients_connected(uint8_t& to_get);
        bool increase_any_ws_clients_connected(void);
        bool decrease_any_ws_clients_connected(void);

        bool get_ws_clients_connected(const uint8_t num, uint8_t& to_get);
        bool set_ws_clients_connected(const uint8_t num, uint8_t value);

        QueueHandle_t* get_xQueue_uart0_to_ws(void);
        QueueHandle_t* get_xQueue_uart1_to_ws(void);
        QueueHandle_t* get_xQueue_uart2_to_ws(void);
        QueueHandle_t* xQueue_ws_to_uart0(void);
        QueueHandle_t* xQueue_ws_to_uart1(void);
        QueueHandle_t* xQueue_ws_to_uart2(void);

    private:
        // Mutex
        SemaphoreHandle_t this_mutex;

        // Globals elements
        gdata data;
};

/**************************************************************************************************/

#endif
