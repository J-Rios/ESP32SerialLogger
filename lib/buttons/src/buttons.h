/**************************************************************************************************/
// File: buttons.h
// Description: HAL library to ease Buttons control.
// Created on: 06 dec. 2018
// Last modified date: 06 dec. 2018
// Version: 1.0.0
/**************************************************************************************************/

/* Include Guard */

#ifndef BUTTONS_H_
#define BUTTONS_H_

/**************************************************************************************************/

/* Defines & Macros */

// Set to true or false to enable/disable FreeRTOS safe use of the input pin through multiples Tasks
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
    NORMAL   = 0,
    PULLUP = 1,
    PULLDOWN  = 2,
} gpi_mode;

/**************************************************************************************************/

class Buttons
{
    public:
        Buttons(const uint8_t pin);
        void mode(const gpi_mode mode=NORMAL);
        uint8_t read(void);

    private:
        uint8_t this_pin;
        #if FREERTOS_MUTEX
            SemaphoreHandle_t this_mutex;
        #endif

        void gpio_as_digital_input(const uint8_t gpio, const gpi_mode mode);
        uint8_t gpio_digital_read(const uint8_t gpio);
};

/**************************************************************************************************/

#endif
