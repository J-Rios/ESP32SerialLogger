/**************************************************************************************************/
// File: globals.h
// Description: Library to thread-safe access encapsulated globals elements from FreeRTOS Tasks.
// Created on: 17 nov. 2018
// Last modified date: 22 dec. 2018
// Version: 1.0.0
/**************************************************************************************************/

/* Libraries */

#include "globals.h"

/**************************************************************************************************/

/* Globals Access Mutex Macro */

#define MUTEX_INIT() do { this_mutex = xSemaphoreCreateMutex(); } while (0)
#define SAFE(x) do \
{ \
    if(xSemaphoreTake(this_mutex, (portTickType)10)==pdTRUE) \
    { \
        x; \
        xSemaphoreGive(this_mutex); \
        return 1; \
    } \
    return 0; \
} while (0)

/**************************************************************************************************/

/* Constructor */

// Globals constructor, initialize globals parameters with default values
Globals::Globals(void)
{
    // Initialize mutex
    MUTEX_INIT();

    // Boolean data initialization
    data.wifi_ap_is_up = false;
    data.wifi_connected = false;
    data.wifi_has_ip = false;
    data.internet_connection = false;
    data.ota_update = false;
    data.first_boot_provision = true;
    data.any_ws_client_connected = 0;

    // Arrays data initialization
    snprintf(data.firmware_version, MAX_LENGTH_VERSION+1, "%s", DEFAULT_FIRMWARE_VERSION);
    memset(data.wifi_ssid, '\0', MAX_LENGTH_WIFI_SSID+1);
    memset(data.wifi_pass, '\0', MAX_LENGTH_WIFI_PASS+1);
    memset(data.internet_check_url, '\0', MAX_LENGTH_IPV4+1);
    memset(data.wifi_ip, '\0', MAX_LENGTH_IPV4+1);
    memcpy(data.wifi_ssid, DEFAULT_WIFI_SSID, MAX_LENGTH_WIFI_SSID+1);
    memcpy(data.wifi_pass, DEFAULT_WIFI_PASS, MAX_LENGTH_WIFI_PASS+1);
    memcpy(data.internet_check_url, PING_TO_URL, MAX_LENGTH_IPV4+1);
    memcpy(data.wifi_ip, DEFAULT_DEVICE_IPV4, MAX_LENGTH_IPV4+1);
    memset(data.ws_clients_connected, 0, MAX_WS_CLIENTS);
}

/**************************************************************************************************/

bool Globals::get_firmware_version(char* to_get)
{
    SAFE( memcpy(to_get, data.firmware_version, MAX_LENGTH_VERSION+1) );
}

bool Globals::set_firmware_version(const char* to_set)
{
    SAFE( memcpy(data.firmware_version, to_set, MAX_LENGTH_VERSION+1) );
}

/**************************************************************************************************/

bool Globals::get_wifi_ssid(char* to_get)
{
    SAFE( memcpy(to_get, data.wifi_ssid, MAX_LENGTH_WIFI_SSID+1) );
}

bool Globals::set_wifi_ssid(const char* to_set)
{
    SAFE( memcpy(data.wifi_ssid, to_set, MAX_LENGTH_WIFI_SSID+1) );
}

/**************************************************************************************************/

bool Globals::get_wifi_pass(char* to_get)
{
    SAFE( memcpy(to_get, data.wifi_pass, MAX_LENGTH_WIFI_PASS+1) );
}

bool Globals::set_wifi_pass(const char* to_set)
{
    SAFE( memcpy(data.wifi_pass, to_set, MAX_LENGTH_WIFI_PASS+1) );
}

/**************************************************************************************************/

bool Globals::get_internet_check_url(char* to_get)
{
    SAFE( memcpy(to_get, data.internet_check_url, MAX_LENGTH_IPV4+1) );
}

bool Globals::set_internet_check_url(const char* to_set)
{
    SAFE( memcpy(data.internet_check_url, to_set, MAX_LENGTH_IPV4+1) );
}

/**************************************************************************************************/

bool Globals::get_first_boot_provision(bool& to_get)
{
    SAFE( to_get = data.first_boot_provision );
}

bool Globals::set_first_boot_provision(const bool to_set)
{
    SAFE( data.first_boot_provision = to_set );
}

/**************************************************************************************************/

bool Globals::get_wifi_ap_is_up(bool& to_get)
{
    SAFE( to_get = data.wifi_ap_is_up );
}

bool Globals::set_wifi_ap_is_up(const bool to_set)
{
    SAFE( data.wifi_ap_is_up = to_set );
}

/**************************************************************************************************/

bool Globals::get_wifi_connected(bool& to_get)
{
    SAFE( to_get = data.wifi_connected );
}

bool Globals::set_wifi_connected(const bool to_set)
{
    SAFE( data.wifi_connected = to_set );
}

/**************************************************************************************************/

bool Globals::get_wifi_has_ip(bool& to_get)
{
    SAFE( to_get = data.wifi_has_ip );
}

bool Globals::set_wifi_has_ip(const bool to_set)
{
    SAFE( data.wifi_has_ip = to_set );
}

/**************************************************************************************************/

bool Globals::get_internet_connection(bool& to_get)
{
    SAFE( to_get = data.internet_connection );
}

bool Globals::set_internet_connection(const bool to_set)
{
    SAFE( data.internet_connection = to_set );
}

/**************************************************************************************************/

bool Globals::get_ota_update(bool& to_get)
{
    SAFE( to_get = data.ota_update );
}

bool Globals::set_ota_update(const bool to_set)
{
    SAFE( data.ota_update = to_set );
}

/**************************************************************************************************/

bool Globals::get_wifi_ip(char* to_get)
{
    SAFE( memcpy(to_get, data.wifi_ip, MAX_LENGTH_IPV4+1) );
}

bool Globals::set_wifi_ip(const char* to_set)
{
    SAFE( memcpy(data.wifi_ip, to_set, MAX_LENGTH_IPV4+1) );
}

/**************************************************************************************************/

bool Globals::get_any_ws_clients_connected(uint8_t& to_get)
{
    SAFE( to_get = data.any_ws_client_connected );
}

bool Globals::increase_any_ws_clients_connected(void)
{
    SAFE(
        if(data.any_ws_client_connected < MAX_WS_CLIENTS)
            data.any_ws_client_connected = data.any_ws_client_connected + 1
    );
}

bool Globals::decrease_any_ws_clients_connected(void)
{
    SAFE(
        if(data.any_ws_client_connected > 0)
            data.any_ws_client_connected = data.any_ws_client_connected - 1
    );
}

/**************************************************************************************************/

bool Globals::get_ws_clients_connected(const uint8_t num, uint8_t& to_get)
{
    SAFE( to_get = data.ws_clients_connected[num] );
}

bool Globals::set_ws_clients_connected(const uint8_t num, uint8_t value)
{
    SAFE( data.ws_clients_connected[num] = value );
}

/**************************************************************************************************/

/* Get FreeRTOS Queues */

QueueHandle_t* Globals::get_xQueue_uart0_to_ws(void)
{
    return &(data.xQueue_uart0_to_ws);
}

QueueHandle_t* Globals::get_xQueue_uart1_to_ws(void)
{
    return &(data.xQueue_uart1_to_ws);
}

QueueHandle_t* Globals::get_xQueue_uart2_to_ws(void)
{
    return &(data.xQueue_uart2_to_ws);
}

QueueHandle_t* Globals::xQueue_ws_to_uart0(void)
{
    return &(data.xQueue_ws_to_uart0);
}

QueueHandle_t* Globals::xQueue_ws_to_uart1(void)
{
    return &(data.xQueue_ws_to_uart1);
}

QueueHandle_t* Globals::xQueue_ws_to_uart2(void)
{
    return &(data.xQueue_ws_to_uart2);
}
