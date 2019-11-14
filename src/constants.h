/**************************************************************************************************/
// Project: 
// File: constants.h
// Description: Project constants file
// Created on: 17 nov. 2018
// Last modified date: 22 dec. 2018
// Version: 1.0.0
/**************************************************************************************************/

/* Include Guard */

#ifndef CONSTANTS_H
#define CONSTANTS_H

/**************************************************************************************************/

/* Libraries */

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/**************************************************************************************************/

/* Commons Defines & Constants */

// Set DEBUG to true or false to enable/disable printf() debugging statement
#define DEBUG                    true

// I/O pins defines
#define UART0_TXD_PIN            GPIO_NUM_1
#define UART0_RXD_PIN            GPIO_NUM_3
#define P_O_RGBLED_R             GPIO_NUM_25
#define P_O_RGBLED_G             GPIO_NUM_32
#define P_O_RGBLED_B             GPIO_NUM_33
#define P_I_BTN_OTA              GPIO_NUM_34
#define P_I_BTN_AP_CONF          GPIO_NUM_35

// Tasks Stack size
#define TASK_MANAGE_WIFI_STACK   configMINIMAL_STACK_SIZE+4096
#define TASK_CLI_STACK           configMINIMAL_STACK_SIZE+4096
#define TASK_WIFI_STATUS_STACK   configMINIMAL_STACK_SIZE+4096
#define TASK_INTERNET_STACK      configMINIMAL_STACK_SIZE+1024
#define TASK_WEBSERVER_STACK     configMINIMAL_STACK_SIZE+12288
#define TASK_WEBSOCKET_STACK     configMINIMAL_STACK_SIZE+4096
#define TASK_UARTS_STACK         configMINIMAL_STACK_SIZE+4096
#define TASK_OTA_STACK           configMINIMAL_STACK_SIZE+7424

// Arrays and strings maximum length
#define MAX_LENGTH_VERSION       11
#define MAX_LENGTH_WIFI_SSID     31
#define MAX_LENGTH_WIFI_PASS     63
#define MAX_LENGTH_MAC_ADDR      17
#define MAX_LENGTH_IPV4          15
#define MAX_LENGHT_JSON_VALUE    128
#define MAX_LENGHT_HTTP_RES_HEAD 128
#define MAX_LENGHT_HTTP_RES_BODY 32768
#define MAX_LENGHT_HTTP_RES      MAX_LENGHT_HTTP_RES_HEAD + MAX_LENGHT_HTTP_RES_BODY

// Default device parameters values
#define DEFAULT_WIFI_AP_SSID     "SerialLogger"
#define DEFAULT_WIFI_AP_PASS     "checkserial"
#define DEFAULT_WIFI_SSID        "MyNet_23"
#define DEFAULT_WIFI_PASS        "pass1234"
#define DEFAULT_DEVICE_IPV4      "255.255.255.255"
#define DEFAULT_FIRMWARE_VERSION "1.0.0"
#define DEFAULT_FIRST_BOOT_PROV  "true"
#define MAX_WIFI_CONN_RETRIES    50
#define SPIFFS_CONFIG_FILE       "cfg.json"
#define HTTPS_PORT               443
#define WEBSERVER_USER           "admin"
#define WEBSERVER_PASS           "1234"

/**************************************************************************************************/

/* FreeRTOS Configs */

// Serial to WebSocket Queue length
#define Q_UART2WS_LEN 32
#define MSG_UART2WS_LEN 256

/**************************************************************************************************/

/* Task Serial Command-Line-Interface */

// Serial RX Buffer size
#define UART_RX_BUF_SIZE      256

// CLI Expected Commands
#define SHELL_CMD_HELP        "help"
#define SHELL_CMD_INFO        "device info"
#define SHELL_CMD_FREE_RAM    "free"
#define SHELL_CMD_CONFIG      "config"
#define SHELL_CMD_IFCONFIG    "ifconfig"
#define SHELL_CMD_WIFI_SSID   "wifi ssid"
#define SHELL_CMD_WIFI_PASS   "wifi pass"
#define SHELL_CMD_VERSION     "version"
#define SHELL_CMD_OTA_CHECK   "upgrade"
#define SHELL_CMD_REBOOT      "reboot"

/**************************************************************************************************/

/* Task OTA Defines & Constants */

// Over The Air Server last version file and firmware, and server connection port
#define OTA_SERVER_VERSION_FILE  "https://192.168.0.130/version.txt"
#define OTA_SERVER_FIRMWARE_FILE "https://192.168.0.130/firmware.bin"

/**************************************************************************************************/

/* Task Internet Status Defines & Constants */

// Ping remote server and properties
#define PING_TO_URL              "8.8.8.8"
const uint32_t PING_NUM_SENT   = 4;
const uint32_t PING_TIMEOUT_MS = 5000;
const uint32_t PING_DELAY_MS   = 500;

// Time between each Internet status checks (100ms for 300 times = 30s)
#define T_INTERNET_CHECKS        300

/**************************************************************************************************/

/* Task WebServer Defines & Constants */

// Max number of WebSockets clients connected 
#define MAX_WS_CLIENTS           2

/**************************************************************************************************/

#endif
