/**************************************************************************************************/
// Project: WiFiStatusLight
// File: main.cpp
// Description: Project main file
// Created on: 16 nov. 2018
// Last modified date: 01 jan. 2019
// Version: 1.0.0
/**************************************************************************************************/

/* Libraries */

// Standard C/C++ libraries


// FreeRTOS libraries
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

// Device libraries (ESP-IDF)
#include "sdkconfig.h"
#include <nvs_flash.h>
//#include "esp_heap_trace.h"

// Tasks implementations
#include "task_manage_wifi.h"
#include "task_cli.h"
#include "task_wifi_status.h"
#include "task_internet_status.h"
#include "task_webserver.h"
#include "task_uarts.h"
#include "task_ota.h"

// Custom libraries
#include "constants.h"
#include "globals.h"
#include "commons.h"
#include "configuration.h"
#include "simplespiffs.h"
#include "buttons.h"
#include "rgbleds.h"

/**************************************************************************************************/

/* Functions Prototypes */

extern "C" { void app_main(void); }
void system_start(Globals* Global, SimpleSPIFFS* SPIFFS, Buttons* Btn_AP_Conf, 
                  Buttons* Btn_OTA_Update, RGBLEDs* LED_RGB);
void nvs_init(void);
void create_queues(Globals* Global);
void task_creation(Globals* Global, SimpleSPIFFS* SPIFFS, Buttons* Btn_AP_Conf, 
                   Buttons* Btn_OTA_Update, RGBLEDs* LED_RGB);

/**************************************************************************************************/

/* Main Function */

void app_main(void)
{
    // Elements that will exists during all system live time
    Globals Global;
    SimpleSPIFFS SPIFFS;
    //Buttons Btn_OTA_Update(P_I_BTN_OTA);
    //Buttons Btn_AP_Conf(P_I_BTN_AP_CONF);
    RGBLEDs LED_RGB(P_O_RGBLED_R, P_O_RGBLED_G, P_O_RGBLED_B);

    // System start and FreeRTOS task creation functions
    //system_start(&Global, &SPIFFS, &Btn_AP_Conf, &Btn_OTA_Update, &LED_RGB);
    //task_creation(&Global,&SPIFFS, &Btn_AP_Conf, &Btn_OTA_Update, &LED_RGB);
    system_start(&Global, &SPIFFS, NULL, NULL, &LED_RGB);
    task_creation(&Global, &SPIFFS, NULL, NULL, &LED_RGB);
    
    // Keep Main "Task" running to avoid lost local scope data that has been passed to Tasks
    while(1)
        delay(1000); // vTaskDelay() Macro (See commons.h)
}

/**************************************************************************************************/

/* Functions */

// Initial system start
void system_start(Globals* Global, SimpleSPIFFS* SPIFFS, Buttons* Btn_AP_Conf, 
                  Buttons* Btn_OTA_Update, RGBLEDs* LED_RGB)
{
    debug("\n-------------------------------------------------------------------------------\n");
    debug("\nSystem start.\n\n");

    show_device_info();

    // Non-Volatile-Storage
    nvs_init();

    // Mount SPIFFS and create/load persistent config file
    debug("Mounting SPIFFS FileSystem...\n");
    if(SPIFFS->mount())
    {
        debug("[OK] SPIFFS successfully mounted.\n");
        device_config_init(SPIFFS, Global);
    }

    // Create FreeRTOS Queues
    create_queues(Global);

    // Initialize Heap trace module (to get free ram functions)
    //heap_caps_init();

    /* Hardware Initialization */

    //Btn_OTA_Update->mode(NORMAL);
    //debug("Button OTA initialized.\n");

    //Btn_AP_Conf->mode(NORMAL);
    //debug("Button AP Configuration provisioning initialized.\n");

    LED_RGB->init();
    debug("RGB LED initialized.\n");
}

// FreeRTOS Tasks creation
void task_creation(Globals* Global, SimpleSPIFFS* SPIFFS, Buttons* Btn_AP_Conf, Buttons* Btn_OTA_Update, RGBLEDs* LED_RGB)
{
    // Prepare parameters to pass
    static tasks_argv task_argv;
    task_argv.Global = Global;
    task_argv.SPIFFS = SPIFFS;
    task_argv.Btn_AP_Conf = Btn_AP_Conf;
    task_argv.Btn_OTA_Update = Btn_OTA_Update;
    task_argv.LED_RGB = LED_RGB;

    // Create CLI Task
    if(xTaskCreate(&task_cli, "task_cli", TASK_CLI_STACK, 
                   (void*)&task_argv, tskIDLE_PRIORITY+5, NULL) != pdPASS)
    {
        debug("\nError - Can't create Command-Line-Interface task (not enough memory?)");
        esp_reboot();
    }
    
    // Create Network (WiFi) manager Task
    if(xTaskCreate(&task_manage_wifi, "task_manage_wifi", TASK_MANAGE_WIFI_STACK, 
                   (void*)&task_argv, tskIDLE_PRIORITY+5, NULL) != pdPASS)
    {
        debug("\nError - Can't create WiFi status task (not enough memory?)");
        esp_reboot();
    }

    // Create WiFi Status Task
    if(xTaskCreate(&task_wifi_status, "task_wifi_status", TASK_WIFI_STATUS_STACK, 
                   (void*)&task_argv, tskIDLE_PRIORITY+5, NULL) != pdPASS)
    {
        debug("\nError - Can't create WiFi status task (not enough memory?)");
        esp_reboot();
    }

    // Create Internet Status Task
    if(xTaskCreate(&task_internet_status, "task_internet_status", TASK_INTERNET_STACK, 
                   (void*)&task_argv, tskIDLE_PRIORITY+5, NULL) != pdPASS)
    {
        debug("\nError - Can't create Internet status task (not enough memory?)");
        esp_reboot();
    }

    // Create Web Server Task
    if(xTaskCreate(&task_webserver, "task_webserver", TASK_WEBSERVER_STACK, (void*)&task_argv, 
                   tskIDLE_PRIORITY+5, NULL) != pdPASS)
    {
        debug("\nError - Can't create OTA task (not enough memory?)");
        esp_reboot();
    }

    // Create UARTs Serial Task
    if(xTaskCreate(&task_uarts, "task_uarts", TASK_UARTS_STACK, 
                   (void*)&task_argv, tskIDLE_PRIORITY+5, NULL) != pdPASS)
    {
        debug("\nError - Can't create UARTs task (not enough memory?)");
        esp_reboot();
    }

    // Create OTA Task
    /*if(xTaskCreate(&task_ota, "task_ota", TASK_OTA_STACK, (void*)&task_argv, 
                   tskIDLE_PRIORITY+5, NULL) != pdPASS)
    {
        debug("\nError - Can't create OTA task (not enough memory?)");
        esp_reboot();
    }*/
}

// Initialize Non-Volatile-Storage
void nvs_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret != ESP_OK)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

// FreeRTOS Queues creation
void create_queues(Globals* Global)
{
    QueueHandle_t* xQueue_uart0_to_ws;
    QueueHandle_t* xQueue_uart1_to_ws;
    QueueHandle_t* xQueue_uart2_to_ws;
    QueueHandle_t* xQueue_ws_to_uart0;
    QueueHandle_t* xQueue_ws_to_uart1;
    QueueHandle_t* xQueue_ws_to_uart2;

    // Get queues from Globals
    xQueue_uart0_to_ws = Global->get_xQueue_uart0_to_ws();
    xQueue_uart1_to_ws = Global->get_xQueue_uart1_to_ws();
    xQueue_uart2_to_ws = Global->get_xQueue_uart2_to_ws();
    xQueue_ws_to_uart0 = Global->xQueue_ws_to_uart0();
    xQueue_ws_to_uart1 = Global->xQueue_ws_to_uart1();
    xQueue_ws_to_uart2 = Global->xQueue_ws_to_uart2();

    // Create UART0 to WebSocket Queue
    *xQueue_uart0_to_ws = xQueueCreate(Q_UART2WS_LEN, sizeof(queue_uart_msg));
	if(*xQueue_uart0_to_ws == 0)
    {
        debug("\nError - Can't create UART to WebSocket queue (not enough memory?)");
        esp_reboot();
    }
    // Create WebSocket to UART0 Queue
    *xQueue_ws_to_uart0 = xQueueCreate(Q_UART2WS_LEN, sizeof(queue_uart_msg));
	if(*xQueue_ws_to_uart0 == 0)
    {
        debug("\nError - Can't create WebSocket to UART0 queue (not enough memory?)");
        esp_reboot();
    }

    // Create UART1 to WebSocket Queue
    *xQueue_uart1_to_ws = xQueueCreate(Q_UART2WS_LEN, sizeof(queue_uart_msg));
	if(*xQueue_uart1_to_ws == 0)
    {
        debug("\nError - Can't create UART1 to WebSocket queue (not enough memory?)");
        esp_reboot();
    }
    // Create WebSocket to UART1 Queue
    *xQueue_ws_to_uart1 = xQueueCreate(Q_UART2WS_LEN, sizeof(queue_uart_msg));
	if(*xQueue_ws_to_uart1 == 0)
    {
        debug("\nError - Can't create WebSocket to UART1 queue (not enough memory?)");
        esp_reboot();
    }

    // Create UART2 to WebSocket Queue
    *xQueue_uart2_to_ws = xQueueCreate(Q_UART2WS_LEN, sizeof(queue_uart_msg));
	if(*xQueue_uart2_to_ws == 0)
    {
        debug("\nError - Can't create UART2 to WebSocket queue (not enough memory?)");
        esp_reboot();
    }
    // Create WebSocket to UART2 Queue
    *xQueue_ws_to_uart2 = xQueueCreate(Q_UART2WS_LEN, sizeof(queue_uart_msg));
	if(*xQueue_ws_to_uart2 == 0)
    {
        debug("\nError - Can't create WebSocket to UART2 queue (not enough memory?)");
        esp_reboot();
    }
}
