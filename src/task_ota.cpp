/**************************************************************************************************/
// Project: 
// File: task_ota.cpp
// Description: System Over-The-Air (OTA) update, through secure HTTPS, FreeRTOS task file
// Created on: 20 nov. 2018
// Last modified date: 09 dec. 2018
// Version: 1.0.0
/**************************************************************************************************/

/* Libraries */

#include "task_ota.h"

/**************************************************************************************************/

/* OTA Events Handler Prototype */

static esp_err_t _http_check_version_event_handler(esp_http_client_event_t *evt);
static esp_err_t _http_ota_update_event_handler(esp_http_client_event_t *evt);

char https_received_data[512];

/**************************************************************************************************/

/* Task */

// Check for new firmware version avalible in a remote server and update system through secure OTA.
// Check server firmware version each hour and when OTA button is pressed
void task_ota(void *pvParameter)
{
    // Get provided parameters
    tasks_argv* task_argv = (tasks_argv*)pvParameter;
    Globals* Global = task_argv->Global;
    Buttons* Btn_OTA_Update = task_argv->Btn_OTA_Update;
    RGBLEDs* LED_RGB = task_argv->LED_RGB;

    memset(https_received_data, '\0', 512);
    
    debug("\nOTA task initialized.\n");

    // Initialize and set OTA HTTPS client config (except URL)
    esp_http_client_config_t config;
    config.port = HTTPS_PORT;
    config.cert_pem = (const char*)ota_server_cert_start;
    config.transport_type = HTTP_TRANSPORT_OVER_SSL;

    bool ota_update = false;
    uint32_t num_iterations = 0;
    while(1)
    {
        // Check for OTA Update
        Global->get_ota_update(ota_update);
        
        // When OTA check for update process is not fired
        if(!ota_update)
        {
            // If check OTA last firmware update button is pressed
            if(Btn_OTA_Update->read() == 0)
            {
                // Activate OTA check for update process
                debug("Button OTA pressed.\n");
                ota_update = true;
                Global->set_ota_update(ota_update);
                delay(100);
                num_iterations = num_iterations + 1;
            }

            // If while iterations has reach value according to 1 hour for each 100ms delay
            // [iterations_in_a_hour = ((60*60)*1000)/100 = 36000]
            if(num_iterations >= 36000)
            {
                // Activate OTA check for update process
                debug("An hour has passed, checking for new firmware.\n");
                ota_update = true;
                Global->set_ota_update(ota_update);
                num_iterations = 0;
            }
        }

        if(ota_update)
        {
            // Check for actual WiFi and OTA check status
            bool wifi_connected, wifi_has_ip;
            Global->get_wifi_connected(wifi_connected);
            Global->get_wifi_has_ip(wifi_has_ip);

            printf("An OTA update request has been set.\n");
            
            if(wifi_connected && wifi_has_ip)
            {
                bool new_firmware = false;

                // Connect to the Server that has OTA last firmware version file
                debug("Checking last firmware version in the server...\n");
                config.url = OTA_SERVER_VERSION_FILE;
                config.event_handler = _http_check_version_event_handler;
                debug("Connecting to %s...\n", config.url);
                esp_http_client_handle_t client = esp_http_client_init(&config);
                esp_err_t ret = esp_http_client_perform(client);
                if (ret == ESP_OK)
                {
                    debug("Secure connection to server established.\n");
                    debug("Received data from server: %s\n", https_received_data);
                    uint8_t last_server_version_digits[3];
                    if(get_version_nums_from_str(https_received_data, strlen(https_received_data), 
                                                 last_server_version_digits) == 0)
                    {
                        char actual_firmware_version[MAX_LENGTH_VERSION+1];
                        Global->get_firmware_version(actual_firmware_version);
                        uint8_t actual_version_digits[3];
                        if(get_version_nums_from_str(actual_firmware_version, 
                                                     strlen(actual_firmware_version), 
                                                     actual_version_digits) == 0)
                        {
                            debug("Device actual firmware version: %d.%d.%d\n", 
                                  actual_version_digits[0], actual_version_digits[1],
                                  actual_version_digits[2]);
                            debug("Firmware version in server: %d.%d.%d\n", 
                                  last_server_version_digits[0], last_server_version_digits[1],
                                  last_server_version_digits[2]);
                            
                            // Check if device version is low or equal to server firmware version
                            bool already_in_last_ver = false;
                            if(last_server_version_digits[0] > actual_version_digits[0])
                                new_firmware = true;
                            else if(last_server_version_digits[0] == actual_version_digits[0])
                            {
                                if(last_server_version_digits[1] > actual_version_digits[1])
                                    new_firmware = true;
                                else if(last_server_version_digits[1] == actual_version_digits[1])
                                {
                                    if(last_server_version_digits[2] > actual_version_digits[2])
                                        new_firmware = true;
                                    else if(last_server_version_digits[2] == 
                                            actual_version_digits[2])
                                        already_in_last_ver = true;
                                }
                            }

                            if(new_firmware)
                                debug("A new firmware version is available in the server.\n");
                            else
                            {
                                if(already_in_last_ver)
                                    debug("Device firmware version is the lastest one, there is " \
                                          "nothing to update.\n");
                                else
                                    debug("Device firmware version is newer than server one!!!\n");
                            }
                        }
                    }
                    else
                        debug("Downloaded last firmware version file is empty or version string " \
                              "has invalid format!\n");
                }
                else
                {
                    debug("Firmware Upgrades Failed.\n");
                    if(ret == ESP_ERR_NO_MEM)
                        debug("Not enough memory available for OTA download.");
                    else
                        ESP_ERROR_CHECK(ret);
                }
                esp_http_client_cleanup(client);
                memset(https_received_data, '\0', 512);

                if(new_firmware)
                {
                    printf("Updating firmware to last version, please wait...\n");
                    config.url = OTA_SERVER_FIRMWARE_FILE;
                    config.event_handler = _http_ota_update_event_handler;

                    // Show OTA updating status through RGB LED
                    LED_RGB->on(RGB_RED, false);

                    // Connect to the Server that has the OTA firmware and download-flash it
                    esp_err_t ret = esp_https_ota(&config);
                    if (ret == ESP_OK)
                    {
                        debug("\nFirmware successfully upgraded.");
                        esp_reboot();
                    }
                    else
                    {
                        debug("Firmware upgrade fail.\n");
                        if(ret == ESP_ERR_NO_MEM)
                            debug("Not enough memory available for OTA download.");
                        else
                            ESP_ERROR_CHECK(ret);
                        
                        // Just turn Red to off, if the error is not from lost network connection
                        Global->get_wifi_connected(wifi_connected);
                        if(wifi_connected)
                            LED_RGB->off(RGB_RED);
                    }
                }
            }
            else
                printf("The update can't be done cause the system has not Internet access.\n");
            
            ota_update = false;
            Global->set_ota_update(ota_update);
        }

        // Increase while iterations counter
        num_iterations = num_iterations + 1;

        // Task CPU release
        delay(100);
    }
}

/**************************************************************************************************/

/* OTA Events Handler */

static esp_err_t _http_check_version_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id)
    {
        case HTTP_EVENT_ERROR:
            debug("HTTP_EVENT_ERROR\n");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            debug("Connected.\n");
            break;
        case HTTP_EVENT_HEADER_SENT:
            debug("Client Header sent.\n");
            break;
        case HTTP_EVENT_ON_HEADER:
            debug("Received Server Header data:\n  key=%s\n  value=%s\n", evt->header_key, 
                  evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            debug("Received Server Content data (%d bytes).\n", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client))
            {
                strncat(https_received_data, (char*)evt->data, 512-strlen(https_received_data)-1);
                if(evt->data_len < 512)
                    https_received_data[evt->data_len] = '\0';
                else
                    https_received_data[511] = '\0';
                debug("Data: %s\n\n", https_received_data);
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            debug("HTTP_EVENT_ON_FINISH\n");
            break;
        case HTTP_EVENT_DISCONNECTED:
            debug("HTTP/S Disconnected.\n");
            break;
    }
    return ESP_OK;
}

static esp_err_t _http_ota_update_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id)
    {
        case HTTP_EVENT_ERROR:
            break;
        case HTTP_EVENT_ON_CONNECTED:
            break;
        case HTTP_EVENT_HEADER_SENT:
            break;
        case HTTP_EVENT_ON_HEADER:
            break;
        case HTTP_EVENT_ON_DATA:
            break;
        case HTTP_EVENT_ON_FINISH:
            break;
        case HTTP_EVENT_DISCONNECTED:
            break;
    }
    return ESP_OK;
}

/**************************************************************************************************/

/* Functions */

// Check for valid expected version string format and get version integers digits
uint8_t get_version_nums_from_str(const char* str, const uint16_t str_len, uint8_t* ver)
{
    // Check if provided string hasnt got expected number of chars (X.Y.Z - XXX.YYY.ZZZ)
    if((str_len < 5) || (str_len > MAX_LENGTH_VERSION))
        return -1;
    // Check if provided string hasnt got expected format (doesnt contains 2 dots)
    if(cstr_count_char(str, str_len, '.') != 2)
        return -2;

    // Get dots index positions
    uint16_t first_dot = cstr_get_index_char_between(str, str_len, 0, '.');
    uint16_t second_dot = cstr_get_index_char_between(str, str_len, first_dot+1, '.');

    // Check if provided string hasnt got expected format (dots were in unexpected positions)
    if((first_dot < 1) || (first_dot > 3))
        return -3;
    if((second_dot < 3) || (second_dot > 7))
        return -3;

    // Get each version digit
    char ver_digit_0[4]; memset(ver_digit_0, '\0', 4);
    char ver_digit_1[4]; memset(ver_digit_1, '\0', 4);
    char ver_digit_2[4]; memset(ver_digit_2, '\0', 4);
    if(!cstr_read_between_idx(str, str_len, 0, first_dot, ver_digit_0, 4))
        return -4;
    if(!cstr_read_between_idx(str, str_len, first_dot+1, second_dot, ver_digit_1, 4))
        return -4;
    if(!cstr_read_between_idx(str, str_len, second_dot+1, str_len+1, ver_digit_2, 4))
        return -4;

    // Convert each version string digit to uint8_t. Note: We cant sscanf directly to uint8_t, 
    // cause newlib nano formatting was enabled when SDK build
    uint16_t temp;
    sscanf(ver_digit_0, "%" SCNu16, &temp);    ver[0] = temp;
    sscanf(ver_digit_1, "%" SCNu16, &temp);    ver[1] = temp;
    sscanf(ver_digit_2, "%" SCNu16, &temp);    ver[2] = temp;

    return 0;
}

// Count number of occurrences for a given character in the provided string
uint16_t cstr_count_char(const char* str, const uint16_t str_len, const char c)
{ 
    uint16_t num_c = 0; 

    for(uint16_t i = 0; i < str_len; i++)
    {
        if(str[i] == c) 
            num_c = num_c + 1;
    }

    return num_c; 
}

// Get the first given character appearance index position for the provided string from a start 
// index position
int32_t cstr_get_index_char_between(const char* str, const uint16_t str_len, 
                                    const uint16_t start_from, const char c)
{
    int32_t pos = -1;

    for(uint16_t i = start_from; i < str_len; i++)
    {
        if(str[i] == c)
        {
            pos = i;
            break;
        }
    }

    return pos;
}

// Read a substring from provided string between given index start and end positions
// Note: Read until idx_end-1
bool cstr_read_between_idx(const char* str, const uint16_t str_len, const uint16_t idx_start, 
                           const uint16_t idx_end, char* readed, const uint16_t readed_len)
{
    if(idx_end > str_len+1)
        return false;
    if(readed_len < (idx_end-idx_start))
        return false;

    uint16_t ii = 0;
    for(uint16_t i = idx_start; i < idx_end; i++)
    {
        readed[ii] = str[i];
        ii = ii + 1;
    }

    if(ii <= readed_len-1)
        readed[ii] = '\0';
    else
        readed[readed_len-1] = '\0';

    return true;
}
