/**************************************************************************************************/
// Project: 
// File: task_cli.h
// Description: Serial Command-Line_Interface FreeRTOS task file
// Created on: 28 jul. 2019
// Last modified date: 29 jul. 2019
// Version: 1.0.0
/**************************************************************************************************/

/* Include Guard */

#ifndef SERIAL_CLI_H
#define SERIAL_CLI_H

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

extern void task_cli(void *pvParameter);

void cli_manager(Globals* Global, SimpleSPIFFS* SPIFFS);
extern int32_t cli_send_data(const char* data);
extern int32_t cli_receive_data(uint8_t* data_rx);
void cli_command_interpreter(Globals* Global, SimpleSPIFFS* SPIFFS, char* cmd_read);
int8_t cli_command_received(char* cmd_read, const uint16_t cmd_read_size);
void cli_send_help_info(void);
void cli_send_system_start_header(void);
void cli_show_device_info(void);
extern void uart0_init(void);
extern int32_t uart0_send_data(const char* data);
extern int32_t uart0_receive_data(uint8_t* data_rx);

/**************************************************************************************************/

#ifdef __cplusplus
}
#endif  // extern "C"

#endif
