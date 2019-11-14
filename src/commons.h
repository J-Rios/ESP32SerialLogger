/**************************************************************************************************/
// Project: 
// File: commons.h
// Description: Commons functions file
// Created on: 17 nov. 2018
// Last modified date: 18 nov. 2018
// Version: 1.0.0
/**************************************************************************************************/

/* Include Guard */

#ifndef COMMONS_H
#define COMMONS_H

/**************************************************************************************************/

/* C++ compiler compatibility */

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************/

/* Libraries */

// FreeRTOS libraries
//#include <freertos/FreeRTOS.h>

// Device libraries (ESP-IDF)
#include <esp_system.h>
#include <esp_spi_flash.h>

// Custom libraries
#include "constants.h"
#include "globals.h"
#include "simplespiffs.h"
#include "buttons.h"
#include "rgbleds.h"

/**************************************************************************************************/

/* Macros */

// Debug macro
#define debug(...) do { if(DEBUG) printf(__VA_ARGS__); } while (0)

// FreeRTOS to Arduino like delay() macro
#define delay(x) do { vTaskDelay(x/portTICK_PERIOD_MS); } while(0)

/**************************************************************************************************/

/* FreeRTOS Global Elements */

// Tasks arguments struct
typedef struct 
{
    Globals* Global;
    SimpleSPIFFS* SPIFFS;
    Buttons* Btn_AP_Conf;
    Buttons* Btn_OTA_Update;
    RGBLEDs* LED_RGB;
} tasks_argv;

// Serial to websocket share data queue
typedef struct
{
    uint8_t uart_num;
    uint32_t uart_bauds;
    char data[MSG_UART2WS_LEN];
} queue_uart_msg;

/**************************************************************************************************/

/* Functions */

extern void show_device_info(void);
extern char* esp_get_base_mac(esp_mac_type_t interface=ESP_MAC_WIFI_STA, uint8_t dots=1);
void esp_reboot(void);

/**************************************************************************************************/

#ifdef __cplusplus
}
#endif  // extern "C"

#endif
