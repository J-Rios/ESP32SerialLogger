/**************************************************************************************************/
// Project: 
// File: task_cli.cpp
// Description: Serial Command-Line_Interface FreeRTOS task file
// Created on: 28 jul. 2019
// Last modified date: 29 jul. 2019
// Version: 1.0.0
/**************************************************************************************************/

/* Libraries */

#include "task_cli.h"

#include "commons.h"
#include <esp_heap_caps.h>

/**************************************************************************************************/

/* Data Types */

Globals* _Global;

/**************************************************************************************************/

/* Task */

// Check for device state to setup and configure WiFi network (create station, access point or both)
void task_cli(void *pvParameter)
{
    // Get provided parameters
    tasks_argv* task_argv = (tasks_argv*)pvParameter;
    SimpleSPIFFS* SPIFFS = task_argv->SPIFFS;
    Globals* Global = task_argv->Global;
    _Global = Global;

    debug("\nSerial Command-Line-Interface task initialized.\n");

    esp_log_level_set("CLI", ESP_LOG_INFO);

    while(1)
    {
        cli_manager(Global, SPIFFS);

        // Task CPU release
        delay(10);
    }
}

/**************************************************************************************************/

/* CLI Functions */

void cli_send_system_start_header(void)
{
    cli_send_data("\n\n");
    cli_send_data(" Advance Serial Logger\n");
    cli_send_data("---------------------\n");
    cli_send_data("Command Line Interface started and ready to receive commands.\n\n");
}

void cli_send_help_info(void)
{
    cli_send_data("Available commands:\n");
    cli_send_data("  help             - Show this message.\n");
    cli_send_data("  device info      - Show device characteristics info.\n");
    cli_send_data("  free             - Print actual RAM memory status.\n");
    cli_send_data("  config           - Show system parameters configurations.\n");
    cli_send_data("  ifconfig         - Show actual device IP address.\n");
    cli_send_data("  wifi ssid <SSID> - Set new WiFi SSID to connect.\n");
    cli_send_data("  wifi pass <PASS> - Set new WiFi Password to connect.\n");
    cli_send_data("  upgrade          - Update firmware to lastest version.\n");
    cli_send_data("  version          - Show device firmware version.\n");
    cli_send_data("  reboot           - Reboot the system.\n");
    cli_send_data("\n");
}

// Read device info and show it through serial
void cli_show_device_info(void)
{
    esp_chip_info_t chip_info;
    char* mac;
    char cad[64];

    esp_chip_info(&chip_info);
    mac = esp_get_base_mac();

    cli_send_data("\nDevice Info:\n");
    cli_send_data("---------------\n");
    if(chip_info.model == CHIP_ESP32)
        cli_send_data("Chip Model: ESP32\n");
    else
        cli_send_data("Chip Model: Unknown\n");
    snprintf(cad, 64, "Chip Revision: %d\n", chip_info.revision);
    cli_send_data(cad);
    snprintf(cad, 64, "CPU Cores: %d\n", chip_info.cores);
    cli_send_data(cad);
    snprintf(cad, 64, "Flash Memory: %dMB\n", spi_flash_get_chip_size()/(1024*1024));
    cli_send_data(cad);
    if(chip_info.features & CHIP_FEATURE_WIFI_BGN)
    {
        snprintf(cad, 64, "Base MAC: %s\n", mac);
        cli_send_data(cad);
    }
    snprintf(cad, 64, "ESP-IDF version: %s\n", esp_get_idf_version());
    cli_send_data(cad);
    if((chip_info.features & CHIP_FEATURE_WIFI_BGN) || (chip_info.features & CHIP_FEATURE_BT) ||
       (chip_info.features & CHIP_FEATURE_BLE) || (chip_info.features & CHIP_FEATURE_EMB_FLASH))
    {
        cli_send_data("Characteristics:\n");
        if(chip_info.features & CHIP_FEATURE_WIFI_BGN)
            cli_send_data("    WiFi 2.4GHz\n");
        if(chip_info.features & CHIP_FEATURE_BT)
            cli_send_data("    Bluetooth Classic\n");
        if(chip_info.features & CHIP_FEATURE_BLE)
            cli_send_data("    Bluetooth Low Energy\n");
        if(chip_info.features & CHIP_FEATURE_EMB_FLASH)
            cli_send_data("    Embedded Flash memory\n");
        else
            cli_send_data("    External Flash memory\n");
    }
    cli_send_data("\n\n");
}

void cli_manager(Globals* Global, SimpleSPIFFS* SPIFFS)
{
    static char uart_data_rx[UART_RX_BUF_SIZE];
    queue_uart_msg q_msg;
    static uint8_t first_run = 1;
    uint8_t any_ws_clients_connected;
    QueueHandle_t* xQueue_ws_to_uart0 = Global->xQueue_ws_to_uart0();
    
    // If the function is called for the first time
    if(first_run)
    {
        // Initialize received UART data
        memset(uart_data_rx, '\0', UART_RX_BUF_SIZE);

        // Initialize UART
        uart0_init();

        // Device start debug header
        cli_send_system_start_header();
        cli_send_help_info();

        first_run = 0;
    }
    
    // If a command has been received from the UART Command-Line-Interface
    if(cli_command_received(uart_data_rx, UART_RX_BUF_SIZE))
    {
        // Interprete, check and handle received command
        cli_command_interpreter(Global, SPIFFS, uart_data_rx);
        
        // Reset received UART data
        memset(uart_data_rx, '\0', UART_RX_BUF_SIZE);
    }

    // If a command has been received from the WebSocket
    Global->get_any_ws_clients_connected(any_ws_clients_connected);
    if(any_ws_clients_connected)
    {
        if(xQueueReceive(*xQueue_ws_to_uart0, &q_msg, (TickType_t)0))
        {
            snprintf(uart_data_rx, UART_RX_BUF_SIZE, "%s", q_msg.data);

            // Interprete, check and handle received command
            cli_command_interpreter(Global, SPIFFS, uart_data_rx);
            
            // Reset received UART data
            memset(uart_data_rx, '\0', UART_RX_BUF_SIZE);
        }
    }
}


int8_t cli_command_received(char* cmd_read, const uint16_t cmd_read_size)
{
    // Read from UART RX buffer
    if(cli_receive_data((uint8_t*)cmd_read))
    {
        // Check if read has an EOL (it is a command)
        for(uint16_t i = 0; i < strlen(cmd_read); i++)
        {
            if((cmd_read[i] == '\r') || (cmd_read[i] == '\n'))
            {
                // Remove EOL
                cmd_read[i] = '\0';

                return 1;
            }
        }
    }
    
    return 0;
}

void cli_command_interpreter(Globals* Global, SimpleSPIFFS* SPIFFS, char* cmd_read)
{
    cli_send_data("\n> "); cli_send_data(cmd_read); cli_send_data("\n\n");
    
    /* Check and execute commands */

    // Command: "help"
    if(strncmp(cmd_read, SHELL_CMD_HELP, UART_RX_BUF_SIZE)  == 0)
    {
        cli_send_help_info();
    }

    // Command: "device info"
    else if(strncmp(cmd_read, SHELL_CMD_INFO, strlen(SHELL_CMD_INFO))  == 0)
    {
        cli_show_device_info();
    }

    // Command: "free"
    else if(strncmp(cmd_read, SHELL_CMD_FREE_RAM, strlen(SHELL_CMD_FREE_RAM))  == 0)
    {
//        heap_caps_print_heap_info(MALLOC_CAP_32BIT|MALLOC_CAP_8BIT|MALLOC_CAP_DMA|MALLOC_CAP_PID2| 
//            MALLOC_CAP_PID3|MALLOC_CAP_PID4|MALLOC_CAP_PID5|MALLOC_CAP_PID6|MALLOC_CAP_PID7| 
//            MALLOC_CAP_SPIRAM|MALLOC_CAP_INTERNAL|MALLOC_CAP_DEFAULT|MALLOC_CAP_INVALID);
        heap_caps_print_heap_info(MALLOC_CAP_32BIT);
        heap_caps_print_heap_info(MALLOC_CAP_8BIT);
        heap_caps_print_heap_info(MALLOC_CAP_DMA);
        heap_caps_print_heap_info(MALLOC_CAP_PID2);
        heap_caps_print_heap_info(MALLOC_CAP_PID3);
        heap_caps_print_heap_info(MALLOC_CAP_PID4);
        heap_caps_print_heap_info(MALLOC_CAP_PID5);
        heap_caps_print_heap_info(MALLOC_CAP_PID6);
        heap_caps_print_heap_info(MALLOC_CAP_PID7);
        heap_caps_print_heap_info(MALLOC_CAP_SPIRAM);
        heap_caps_print_heap_info(MALLOC_CAP_INTERNAL);
        heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);
        heap_caps_print_heap_info(MALLOC_CAP_INVALID);
        printf("\nRAM left %d", esp_get_free_heap_size());
        cli_send_data("\n");
    }
    
    // Command: "config"
    else if(strncmp(cmd_read, SHELL_CMD_CONFIG, UART_RX_BUF_SIZE)  == 0)
    {
        char cstr_default_config[MAX_SPIFFS_FILE_CONTENT+1];
        bool first_boot_prov_val;
        char first_boot_prov[MAX_LENGHT_JSON_VALUE];
        char wifi_ssid[MAX_LENGTH_WIFI_SSID+1];
        char wifi_pass[MAX_LENGTH_WIFI_PASS+1];
        char firmware_version[MAX_LENGTH_VERSION+1];

        Global->get_first_boot_provision(first_boot_prov_val);
        Global->get_wifi_ssid(wifi_ssid);
        Global->get_wifi_pass(wifi_pass);
        Global->get_firmware_version(firmware_version);

        if(first_boot_prov_val)
            snprintf(first_boot_prov, MAX_LENGHT_JSON_VALUE, "%s", "true");
        else
            snprintf(first_boot_prov, MAX_LENGHT_JSON_VALUE, "%s", "false");
        
        // Generate default device config in JSON format
        snprintf(cstr_default_config, MAX_SPIFFS_FILE_CONTENT+1, \
            "{\n" \
            "    \"first_boot_provision\": \"%s\",\n" \
            "    \"wifi_ssid\": \"%s\",\n" \
            "    \"wifi_pass\": \"%s\",\n" \
            "    \"firmware_ver\": \"%s\"\n" \
            "}", 
            first_boot_prov,
            wifi_ssid, 
            wifi_pass, 
            firmware_version
        );

        cli_send_data(cstr_default_config);
        cli_send_data("\n\n");
    }

    // Command: "ifconfig"
    else if(strncmp(cmd_read, SHELL_CMD_IFCONFIG, UART_RX_BUF_SIZE)  == 0)
    {
        char ip[MAX_LENGTH_IPV4+1];
        char* mac = esp_get_base_mac();
        Global->get_wifi_ip(ip);
        cli_send_data("IP Address: ");
        cli_send_data(ip);
        cli_send_data("\n");
        cli_send_data("MAC address: ");
        cli_send_data(mac);
        cli_send_data("\n\n");
    }

    // Command: "wifi ssid"
    else if(strncmp(cmd_read, SHELL_CMD_WIFI_SSID, strlen(SHELL_CMD_WIFI_SSID))  == 0)
    {
        char wifi_ssid[MAX_LENGTH_WIFI_SSID+1];

        // Safe check
        if(strlen(SHELL_CMD_WIFI_SSID)+1 > UART_RX_BUF_SIZE-1)
            return;

        // If command has an argument (length is large than command base)
        if(strlen(cmd_read) > strlen(SHELL_CMD_WIFI_SSID)+1)
        {
            char* cmd_arg = cmd_read;
            cmd_arg = cmd_arg + strlen(SHELL_CMD_WIFI_SSID)+1;

            Global->set_wifi_ssid(cmd_arg);
            device_config_update(SPIFFS, Global);
            cli_send_data("New WiFi SSID configured.\n");
        }

        // Show configured Wifi SSID connection
        Global->get_wifi_ssid(wifi_ssid);
        cli_send_data("Actual WiFi SSID to connect: ");
        cli_send_data(wifi_ssid);
        cli_send_data("\n\n");
    }

    // Command: "wifi pass"
    else if(strncmp(cmd_read, SHELL_CMD_WIFI_PASS, strlen(SHELL_CMD_WIFI_PASS))  == 0)
    {
        char wifi_pass[MAX_LENGTH_WIFI_PASS+1];

        // Safe check
        if(strlen(SHELL_CMD_WIFI_PASS)+1 > UART_RX_BUF_SIZE-1)
            return;

        // If command has an argument (length is large than command base)
        if(strlen(cmd_read) > strlen(SHELL_CMD_WIFI_PASS)+1)
        {
            char* cmd_arg = cmd_read;
            cmd_arg = cmd_arg + strlen(SHELL_CMD_WIFI_PASS)+1;

            Global->set_wifi_pass(cmd_arg);
            device_config_update(SPIFFS, Global);
            cli_send_data("New WiFi password configured.\n");
        }

        // Show configured Wifi Password
        Global->get_wifi_pass(wifi_pass);
        cli_send_data("Actual WiFi password to connect: ");
        cli_send_data(wifi_pass);
        cli_send_data("\n\n");
    }

    // Command: "version"
    else if(strncmp(cmd_read, SHELL_CMD_VERSION, UART_RX_BUF_SIZE)  == 0)
    {
        char fw_ver[MAX_LENGTH_VERSION+1];
        Global->get_firmware_version(fw_ver);
        cli_send_data("Firmware version: ");
        cli_send_data(fw_ver);
        cli_send_data("\n\n");
    }

    // Command: "upgrade"
    else if(strncmp(cmd_read, SHELL_CMD_OTA_CHECK, UART_RX_BUF_SIZE)  == 0)
    {
        cli_send_data("Set check for new firmware version.\n\n");
        Global->set_ota_update(true);
    }

    // Command: "reboot"
    else if(strncmp(cmd_read, SHELL_CMD_REBOOT, UART_RX_BUF_SIZE)  == 0)
    {
        esp_reboot();
    }

    // Unexpected Command
    else
    {
        cli_send_data("Unknown command.\n");
        cli_send_data("Use \"help\" command to check available commands.\n\n");
    }
}

/**************************************************************************************************/

int32_t cli_send_data(const char* data_tx)
{
    int32_t sent_bytes = uart0_send_data(data_tx);
    queue_uart_msg q_msg;
    uint8_t any_ws_clients_connected;
    static QueueHandle_t* xQueue_uart0_to_ws = _Global->get_xQueue_uart0_to_ws();

    // If something was sent
    if(sent_bytes)
    {
        _Global->get_any_ws_clients_connected(any_ws_clients_connected);
        if(any_ws_clients_connected)
        {
            // Send the sent data to websocket task
            q_msg.uart_num = 0;
            snprintf(q_msg.data, MSG_UART2WS_LEN, "%s", data_tx);
            if(xQueueSend(*xQueue_uart0_to_ws, &q_msg, (TickType_t)0) != pdPASS)
                printf("\nQueue send fail in cli_send_data()\n\n");
        }
    }

    return sent_bytes;
}

int32_t cli_receive_data(uint8_t* data_rx)
{
    int32_t received_bytes = uart0_receive_data(data_rx);
    queue_uart_msg q_msg;
    uint8_t any_ws_clients_connected;
    static QueueHandle_t* xQueue_uart0_to_ws = _Global->get_xQueue_uart0_to_ws();

    // If something was received
    if(received_bytes)
    {
        _Global->get_any_ws_clients_connected(any_ws_clients_connected);
        if(any_ws_clients_connected)
        {
            // Send the received data to websocket task
            q_msg.uart_num = 0;
            snprintf(q_msg.data, MSG_UART2WS_LEN, "%s", data_rx);
            if(xQueueSend(*xQueue_uart0_to_ws, &q_msg, (TickType_t)0) != pdPASS)
                printf("\nQueue send fail in cli_receive_data()\n\n");
        }
    }

    return received_bytes;
}

/**************************************************************************************************/

/* UART Functions */

// UART initialization
void uart0_init(void)
{
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
        .use_ref_tick = false
    };

    uart_param_config(UART_NUM_0, &uart_config);
    uart_set_pin(UART_NUM_0, UART0_TXD_PIN, UART0_RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM_0, UART_RX_BUF_SIZE*2, 0, 0, NULL, 0);
}

// UART Send data
int32_t uart0_send_data(const char* data_tx)
{
    const int len = strlen(data_tx);
    const int tx_bytes = uart_write_bytes(UART_NUM_0, data_tx, len);
    if(tx_bytes < len)
    {
        ESP_LOGI("CLI", "Error when sending data.\nBytes to send: %d.\nBytes sent: %d.\n", len, 
                tx_bytes);
    }
    return tx_bytes;
}

// UART Receive data
int32_t uart0_receive_data(uint8_t* data_rx)
{
    const int rx_bytes = uart_read_bytes(UART_NUM_0, data_rx, UART_RX_BUF_SIZE, 
            100/portTICK_RATE_MS);
    if(rx_bytes < 0)
        ESP_LOGI("CLI", "Error when reading data.\nBytes read: %d.\n", rx_bytes);
    else if(rx_bytes > 0)
    {
        if(rx_bytes < UART_RX_BUF_SIZE)
            data_rx[rx_bytes] = '\0';
    }

    return rx_bytes;
}
