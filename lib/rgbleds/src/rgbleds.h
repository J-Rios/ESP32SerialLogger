/**************************************************************************************************/
// File: rgbleds.h
// Description: Library to ease RGB LEDs control.
// Created on: 16 nov. 2018
// Last modified date: 10 dec. 2018
// Version: 1.0.1
/**************************************************************************************************/

/* Include Guard */

#ifndef RGBLEDS_H_
#define RGBLEDS_H_

/**************************************************************************************************/

/* Defines & Macros */

// Set to true or false to enable/disable FreeRTOS safe use of the RGB LED through multiples Tasks
#define FREERTOS_MUTEX true

/**************************************************************************************************/

/* Libraries */

#if FREERTOS_MUTEX
    // Include SDK and Freertos
    #include <freertos/FreeRTOS.h>
    #include <freertos/semphr.h>
#endif

// Device libraries (ESP-IDF)
#include <driver/gpio.h>

/**************************************************************************************************/

/* Library Data Types */

// RGB LED component data type
typedef enum
{
    RGB_RED   = 0,
    RGB_GREEN = 1,
    RGB_BLUE  = 2,
} rgbleds_led;

/**************************************************************************************************/

class RGBLEDs
{
    public:
        RGBLEDs(const uint8_t pin_r, const uint8_t pin_g, const uint8_t pin_b);
        void init(void);
        void on(void);
        void off(void);
        void on(const rgbleds_led led, const bool shutdown_others=true);
        void off(const rgbleds_led led);
        void toggle(const rgbleds_led led, const bool toggle_others=false);

    private:
        uint8_t this_pin_r, this_pin_g, this_pin_b;
        bool red_is_on, green_is_on, blue_is_on;
        #if FREERTOS_MUTEX
            SemaphoreHandle_t this_mutex;
        #endif

        void gpio_as_digital_output(const uint8_t gpio);
        void gpio_low(const uint8_t gpio);
        void gpio_high(const uint8_t gpio);
        void gpio_toggle(const uint8_t gpio);
};

/**************************************************************************************************/

#endif
