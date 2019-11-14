/**************************************************************************************************/
// File: rgbleds.cpp
// Description: Library to ease RGB LEDs control.
// Created on: 16 nov. 2018
// Last modified date: 10 dec. 2018
// Version: 1.0.1
/**************************************************************************************************/

/* Libraries */

#include "rgbleds.h"

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

// RGBLEDs constructor, get the GPIO pins numbers that going to be used for RGB
RGBLEDs::RGBLEDs(const uint8_t pin_r, const uint8_t pin_g, const uint8_t pin_b)
{
    this_pin_r = pin_r;
    this_pin_g = pin_g;
    this_pin_b = pin_b;
    red_is_on = false;
    green_is_on = false;
    blue_is_on = false;

    MUTEX_INIT();
}

/**************************************************************************************************/

/* Public Methods */

// Initialize R, G and B GPIOs as outputs and set them to HIGH (LED off)
void RGBLEDs::init(void)
{
    MUTEX_SAFE
    (
        gpio_as_digital_output(this_pin_r);
        gpio_as_digital_output(this_pin_g);
        gpio_as_digital_output(this_pin_b);
        gpio_high(this_pin_r);
        gpio_high(this_pin_g);
        gpio_high(this_pin_b);
        red_is_on = false;
        green_is_on = false;
        blue_is_on = false;
    );
}

// Turn ON all RGB LED colors
void RGBLEDs::on(void)
{
    MUTEX_SAFE
    (
        gpio_low(this_pin_r);
        gpio_low(this_pin_g);
        gpio_low(this_pin_b);
        red_is_on = true;
        green_is_on = true;
        blue_is_on = true;
    );
}

// Turn OFF all RGB LED colors
void RGBLEDs::off(void)
{
    MUTEX_SAFE
    (
        gpio_high(this_pin_r);
        gpio_high(this_pin_g);
        gpio_high(this_pin_b);
        red_is_on = false;
        green_is_on = false;
        blue_is_on = false;
    );
}

// Turn ON the selected LED color (R, G or B)
void RGBLEDs::on(const rgbleds_led led, const bool shutdown_others)
{
    MUTEX_SAFE
    (
        // Do nothing if the provided LED is already on
        bool do_nothing = false;
        if((led == RGB_RED) && (red_is_on))
            do_nothing = true;
        if((led == RGB_GREEN) && (green_is_on))
            do_nothing = true;
        if((led == RGB_BLUE) && (blue_is_on))
            do_nothing = true;

        if(!do_nothing)
        {
            if(!shutdown_others)
            {
                if(led == RGB_RED)
                {
                    gpio_low(this_pin_r);
                    red_is_on = true;
                }
                else if(led == RGB_GREEN)
                {
                    gpio_low(this_pin_g);
                    green_is_on = true;
                }
                else if(led == RGB_BLUE)
                {
                    gpio_low(this_pin_b);
                    blue_is_on = true;
                }
            }
            else
            {
                if(led == RGB_RED)
                {
                    gpio_low(this_pin_r);
                    gpio_high(this_pin_g);
                    gpio_high(this_pin_b);
                    red_is_on = true;
                    green_is_on = false;
                    blue_is_on = false;
                }
                else if(led == RGB_GREEN)
                {
                    gpio_high(this_pin_r);
                    gpio_low(this_pin_g);
                    gpio_high(this_pin_b);
                    red_is_on = false;
                    green_is_on = true;
                    blue_is_on = false;
                }
                else if(led == RGB_BLUE)
                {
                    gpio_high(this_pin_r);
                    gpio_high(this_pin_g);
                    gpio_low(this_pin_b);
                    red_is_on = false;
                    green_is_on = false;
                    blue_is_on = true;
                }
            }
        }
    );
}

// Turn OFF the selected LED color (R, G or B)
void RGBLEDs::off(const rgbleds_led led)
{    
    MUTEX_SAFE
    (
        // Do nothing if the provided LED is already off
        bool do_nothing = false;
        if((led == RGB_RED) && (!red_is_on))
            do_nothing = true;
        if((led == RGB_GREEN) && (!green_is_on))
            do_nothing = true;
        if((led == RGB_BLUE) && (!blue_is_on))
            do_nothing = true;
        
        if(!do_nothing)
        {
            if(led == RGB_RED)
            {
                gpio_high(this_pin_r);
                red_is_on = false;
            }
            else if(led == RGB_GREEN)
            {
                gpio_high(this_pin_g);
                green_is_on = false;
            }
            else if(led == RGB_BLUE)
            {
                gpio_high(this_pin_b);
                blue_is_on = false;
            }
        }
    );
}

// Toggle the selected LED color (R, G or B) or toggle all
void RGBLEDs::toggle(const rgbleds_led led, const bool toggle_others)
{
    MUTEX_SAFE
    (
        if(!toggle_others)
        {
            if(led == RGB_RED)
            {
                gpio_toggle(this_pin_r);
                red_is_on = !red_is_on;
            }
            else if(led == RGB_GREEN)
            {
                gpio_toggle(this_pin_g);
                green_is_on = !green_is_on;
            }
            else if(led == RGB_BLUE)
            {
                gpio_toggle(this_pin_b);
                blue_is_on = !blue_is_on;
            }
        }
        else
        {
            gpio_toggle(this_pin_r);
            gpio_toggle(this_pin_g);
            gpio_toggle(this_pin_b);
            red_is_on = !red_is_on;
            green_is_on = !green_is_on;
            blue_is_on = !blue_is_on;
        }
    );
}

/**************************************************************************************************/

/* Private Methods (Specific device HAL functions) */

// Set the provided GPIO as Output
void RGBLEDs::gpio_as_digital_output(const uint8_t gpio)
{
    gpio_pad_select_gpio((gpio_num_t)gpio);
    gpio_set_direction((gpio_num_t)gpio, GPIO_MODE_OUTPUT);
}

// Set the provided output GPIO to LOW
void RGBLEDs::gpio_low(const uint8_t gpio)
{
    gpio_set_level((gpio_num_t)gpio, 0);
}

// Set the provided output GPIO to LOW
void RGBLEDs::gpio_high(const uint8_t gpio)
{
    gpio_set_level((gpio_num_t)gpio, 1);
}

// Toggle the provided output GPIO
void RGBLEDs::gpio_toggle(const uint8_t gpio)
{
    gpio_set_level((gpio_num_t)gpio, 1 - (GPIO.out >> gpio & 0x1));
}
