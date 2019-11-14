/**************************************************************************************************/
// Project: 
// File: commons.cpp
// Description: Commons functions file
// Created on: 17 nov. 2018
// Last modified date: 29 jul. 2018
// Version: 1.0.0
/**************************************************************************************************/

/* Libraries */

#include "commons.h"

/**************************************************************************************************/

/* Data Types */


/**************************************************************************************************/

/* Functions */

// Read device info and show it through serial
void show_device_info(void)
{
    esp_chip_info_t chip_info;
    char* mac;

    esp_chip_info(&chip_info);
    mac = esp_get_base_mac();

    debug("\nDevice Info:\n");
    debug("---------------\n");
    if(chip_info.model == CHIP_ESP32)
        debug("Chip Model: ESP32\n");
    else
        debug("Chip Model: Unknown\n");
    debug("Chip Revision: %d\n", chip_info.revision);
    debug("CPU Cores: %d\n", chip_info.cores);
    debug("Flash Memory: %dMB\n", spi_flash_get_chip_size()/(1024*1024));
    if(chip_info.features & CHIP_FEATURE_WIFI_BGN)
        debug("Base MAC: %s\n", mac);
    debug("ESP-IDF version: %s\n", esp_get_idf_version());
    if((chip_info.features & CHIP_FEATURE_WIFI_BGN) || (chip_info.features & CHIP_FEATURE_BT) ||
       (chip_info.features & CHIP_FEATURE_BLE) || (chip_info.features & CHIP_FEATURE_EMB_FLASH))
    {
        debug("Characteristics:\n");
        if(chip_info.features & CHIP_FEATURE_WIFI_BGN)
            debug("    WiFi 2.4GHz\n");
        if(chip_info.features & CHIP_FEATURE_BT)
            debug("    Bluetooth Classic\n");
        if(chip_info.features & CHIP_FEATURE_BLE)
            debug("    Bluetooth Low Energy\n");
        if(chip_info.features & CHIP_FEATURE_EMB_FLASH)
            debug("    Embedded Flash memory\n");
        else
            debug("    External Flash memory\n");
    }
    debug("\n\n");
}

// Get ESP MAC Address
char* esp_get_base_mac(esp_mac_type_t interface, uint8_t dots)
{
    static char mac[18];
    uint8_t u8_mac[6];

	esp_read_mac(u8_mac, interface);

    if(dots)
    {
        snprintf(mac, 18, "%02X:%02X:%02X:%02X:%02X:%02X", u8_mac[0], u8_mac[1], u8_mac[2], 
                u8_mac[3], u8_mac[4], u8_mac[5]);
    }
    else
    {
        snprintf(mac, 18, "%02X%02X%02X%02X%02X%02X", u8_mac[0], u8_mac[1], u8_mac[2], 
                u8_mac[3], u8_mac[4], u8_mac[5]);
    }
    
    return mac;
}

// Reboot the system
void esp_reboot(void)
{
    debug("\nRebooting the system...");
    debug("\n\n-------------------------------------------------------------------------------\n\n");
    esp_restart();
}
