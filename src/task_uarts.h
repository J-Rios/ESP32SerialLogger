/**************************************************************************************************/
// Project: 
// File: task_cli.h
// Description: Serial Command-Line_Interface FreeRTOS task file
// Created on: 28 jul. 2019
// Last modified date: 29 jul. 2019
// Version: 1.0.0
/**************************************************************************************************/

/* Include Guard */

#ifndef UARTS_SERIAL_H
#define UARTS_SERIAL_H

/**************************************************************************************************/

/* C++ compiler compatibility */

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************/

/* Libraries */

// Standard C/C++ libraries
#include "string.h"

// FreeRTOS libraries


// Device libraries (ESP-IDF)
#include <esp_system.h>
#include "esp_log.h"
#include "driver/uart.h"
#include "soc/uart_struct.h"

// Custom libraries
#include "constants.h"
#include "globals.h"
//#include "commons.h"
#include "configuration.h"
#include "simplespiffs.h"
#include "rgbleds.h"

/**************************************************************************************************/

/* Functions */

extern void task_uarts(void *pvParameter);

extern void uart_init(const uart_port_t uart_num, const uint32_t baud_rate=115200, 
        const uint8_t pin_rx=UART_PIN_NO_CHANGE, const uint8_t pin_tx=UART_PIN_NO_CHANGE,
        const uart_word_length_t data_bits=UART_DATA_8_BITS, 
        const uart_stop_bits_t stop_bits=UART_STOP_BITS_1, 
        const uart_parity_t parity=UART_PARITY_DISABLE);
extern int32_t uart_send_data(const uart_port_t uart_num, const char* data_tx);
extern int32_t uart_receive_data(const uart_port_t uart_num, uint8_t* data_rx);

/**************************************************************************************************/

#ifdef __cplusplus
}
#endif  // extern "C"

#endif
