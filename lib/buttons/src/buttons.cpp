/**************************************************************************************************/
// File: buttons.cpp
// Description: HAL library to ease Buttons control.
// Created on: 06 dec. 2018
// Last modified date: 06 dec. 2018
// Version: 1.0.0
/**************************************************************************************************/

/* Libraries */

#include "buttons.h"

/**************************************************************************************************/

/* Resource Safe Access Mutex Macro */

#if !FREERTOS_MUTEX
    #define MUTEX_INIT() do { } while (0)
    #define MUTEX_SAFE(x) do { x; } while (0)
#else
    #define MUTEX_INIT() do { this_mutex = xSemaphoreCreateMutex(); } while (0)
    #define MUTEX_SAFE(x) do \
    { \
        if(xSemaphoreTake(this_mutex, (portTickType)10)==pdTRUE) \
        { \
            x; \
            xSemaphoreGive(this_mutex); \
        } \
    } while (0)
#endif

/**************************************************************************************************/

/* Constructor */

// Buttons constructor, get the GPIO pins numbers that going to be used for the button
Buttons::Buttons(const uint8_t pin)
{
    this_pin = pin;

    MUTEX_INIT();
}

/**************************************************************************************************/

/* Public Methods */

// Initialize Button GPIO as normal/pulldown/pullup input
void Buttons::mode(const gpi_mode mode)
{
    MUTEX_SAFE
    (
        gpio_as_digital_input(this_pin, mode);
    );
}

// Read Button value
uint8_t Buttons::read(void)
{
    uint8_t value = 1;

    MUTEX_SAFE
    (
        value = gpio_digital_read(this_pin);
    );

    return value;
}

/**************************************************************************************************/

/* Private Methods (Specific device HAL functions) */

// Set the provided GPIO as Input
void Buttons::gpio_as_digital_input(const uint8_t gpio, const gpi_mode mode)
{
    if(mode == PULLUP)
        gpio_pad_pullup((gpio_num_t)gpio);
    else if(mode == PULLDOWN)
        gpio_pad_pulldown((gpio_num_t)gpio);
    else
        gpio_pad_select_gpio((gpio_num_t)gpio);
    
    gpio_set_direction((gpio_num_t)gpio, GPIO_MODE_INPUT);
}

// Read and get digital value of input pin
uint8_t Buttons::gpio_digital_read(const uint8_t gpio)
{
    return gpio_get_level((gpio_num_t)gpio);
}
