/**************************************************************************************************/
// Project: 
// File: task_wifistatus.cpp
// Description: WiFi status FreeRTOS task file
// Created on: 17 nov. 2018
// Last modified date: 01 jan. 2019
// Version: 1.0.1
/**************************************************************************************************/

/* Libraries */

#include "task_wifi_status.h"

/**************************************************************************************************/

/* Task */

// Check for WiFi network changes (connection/disconnection/get IP/ lost IP) and show that status 
// through RGB LED colors
void task_wifi_status(void *pvParameter)
{
    bool wifi_connected = false;
    bool wifi_has_ip = false;
    bool internet_conn = false;
    bool first_boot = true;

    // Get provided parameters
    tasks_argv* task_argv = (tasks_argv*)pvParameter;
    Globals* Global = task_argv->Global;
    SimpleSPIFFS* SPIFFS = task_argv->SPIFFS;
    RGBLEDs* LED_RGB = task_argv->LED_RGB;

    debug("\nWiFi status task initialized.\n");

    // Tur Red the RGB LED
    LED_RGB->on(RGB_RED);

    while(1)
    {
        // Check for actual WiFi status
        Global->get_wifi_connected(wifi_connected);
        Global->get_wifi_has_ip(wifi_has_ip);
        Global->get_internet_connection(internet_conn);
        Global->get_first_boot_provision(first_boot);

        // Show the actual WiFi status using the RGB LED
        if(!wifi_connected && !wifi_has_ip)
            LED_RGB->on(RGB_RED);
        else if(wifi_connected && !wifi_has_ip)
        {
            LED_RGB->toggle(RGB_RED);
            delay(400);
        }
        else if(wifi_connected && wifi_has_ip)
        {
            if(!internet_conn)
                LED_RGB->on(RGB_GREEN);
            else
                LED_RGB->on(RGB_BLUE);
            
            // Device connected and has IP, lets disable first_boot flag
            if(first_boot)
            {
                first_boot = false;
                Global->set_first_boot_provision(first_boot);
                device_config_update(SPIFFS, Global);
            }
        }

        // Task CPU release
        delay(100);
    }
}

/**************************************************************************************************/
