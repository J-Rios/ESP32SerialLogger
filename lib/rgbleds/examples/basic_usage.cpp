/**************************************************************************************************/
// File: basic_usage.cpp
// Description: RGBLEDs library basic usage example
// Created on: 16 nov. 2018
// Last modified date: 16 nov. 2018
// Version: 1.0.0
/**************************************************************************************************/

/* Libraries */

// Standard C/C++ libraries
#include <stdio.h>

// FreeRTOS libraries
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Device libraries (ESP-IDF)
#include "sdkconfig.h"

// Custom libraries
#include "rgbleds.h"

/**************************************************************************************************/

/* Defines, Macros, Constants and Types */

// I/O pins defines
#define P_O_RGBLED_R GPIO_NUM_12
#define P_O_RGBLED_G GPIO_NUM_13
#define P_O_RGBLED_B GPIO_NUM_14

// Macros
#define delay(x) do { vTaskDelay(x/portTICK_PERIOD_MS); } while(0)

/**************************************************************************************************/

/* Functions Prototypes */

// Main Function
extern "C" { void app_main(void); }

/**************************************************************************************************/

/* Main Function */

void app_main(void)
{
    RGBLEDs LED_RGB(P_O_RGBLED_R, P_O_RGBLED_G, P_O_RGBLED_B);

    LED_RGB.init();

    while(1)
    {
        LED_RGB.on(RGB_RED);
        delay(1000);
        LED_RGB.on(RGB_GREEN);
        delay(1000);
        LED_RGB.on(RGB_BLUE);
        delay(1000);
        LED_RGB.off(RGB_BLUE);
        delay(1000);
        LED_RGB.on(RGB_RED, false);
        delay(1000);
        LED_RGB.on(RGB_GREEN, false);
        delay(1000);
        LED_RGB.on(RGB_BLUE, false);
        delay(1000);
        LED_RGB.off();
        delay(1000);
        LED_RGB.on();
        delay(1000);
    }
}
