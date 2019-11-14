/**************************************************************************************************/
// Project: 
// File: configuration.h
// Description: Device configuration and persistent parameters save/load functions file
// Created on: 25 dec. 2018
// Last modified date: 31 dec. 2018
// Version: 0.0.1
/**************************************************************************************************/

/* Libraries */

#include "configuration.h"

/**************************************************************************************************/

/* Data Types */


/**************************************************************************************************/

/* Functions */

// Create/Load persistent SPIFFS config file from/to system parameters
void device_config_init(SimpleSPIFFS* SPIFFS, Globals* Global)
{
    // If config file doesnt exists in SPIFFS
    if(!SPIFFS->file_exists(SPIFFS_CONFIG_FILE))
    {
        debug("Persistent config file doesn't found. Creating a new one with default values...\n");
        device_config_create_default(SPIFFS);
        return;
    }

    // The config file exists
    else
    {
        bool save_to_spiffs = false;
        bool missing_first_boot_prov = true;
        bool missing_wifi_ssid = true;
        bool missing_wifi_pass = true;
        bool missing_fw_ver = true;
        char first_boot_prov_val[MAX_LENGHT_JSON_VALUE];
        char wifi_ssid_val[MAX_LENGTH_WIFI_SSID+1];
        char wifi_pass_val[MAX_LENGTH_WIFI_PASS+1];
        char fw_ver_val[MAX_LENGHT_JSON_VALUE];
        char cstr_params_config[MAX_SPIFFS_FILE_CONTENT+1];

        // SPIFFS has a config file, lets read it
        char cstr_actual_config[MAX_SPIFFS_FILE_CONTENT];
        memset(cstr_actual_config, '\0', MAX_SPIFFS_FILE_CONTENT);
        debug("Persistent config file found. Loading data...\n");
        if(!SPIFFS->file_read(SPIFFS_CONFIG_FILE, cstr_actual_config, MAX_SPIFFS_FILE_CONTENT))
        {
            debug("SPIFFS config file can't be read.");
            esp_reboot();
        }

        // Parse JSON object of actual device configuration
        jsmn_parser parser;
        jsmntok_t tokens[128];
        int r;

        jsmn_init(&parser);
        r = jsmn_parse(&parser, cstr_actual_config, strlen(cstr_actual_config), tokens, 
            sizeof(tokens)/sizeof(tokens[0]));
        if(r < 0)
        {
            debug("Can't parse JSON data from actual config C string.");
            esp_reboot();
        }
        if(r < 1 || tokens[0].type != JSMN_OBJECT)
        {
            debug("Root token of parsed actual config is not a valid JSON element.\n");
            debug("Restoring config file from default values.");
            device_config_create_default(SPIFFS);
            esp_reboot();
        }

        // Load actual device configuration to system
        for(int i = 1; i < r; i++)
        {
            if(jsoneq(cstr_actual_config, &tokens[i], "first_boot_provision") == 0)
            {
                snprintf(first_boot_prov_val, MAX_LENGHT_JSON_VALUE, "%.*s", 
                    tokens[i+1].end - tokens[i+1].start, cstr_actual_config + tokens[i+1].start);
                
                if((strncmp(first_boot_prov_val, "false", MAX_LENGHT_JSON_VALUE) == 0) || 
                    (strncmp(first_boot_prov_val, "False", MAX_LENGHT_JSON_VALUE) == 0) || 
                    (strncmp(first_boot_prov_val, "0", MAX_LENGHT_JSON_VALUE) == 0))
                {
                    snprintf(first_boot_prov_val, MAX_LENGHT_JSON_VALUE, "%s", "false");
                    missing_first_boot_prov = false;
                }
                else if((strncmp(first_boot_prov_val, "true", MAX_LENGHT_JSON_VALUE) == 0) || 
                    (strncmp(first_boot_prov_val, "True", MAX_LENGHT_JSON_VALUE) == 0) || 
                    (strncmp(first_boot_prov_val, "1", MAX_LENGHT_JSON_VALUE) == 0))
                {
                    snprintf(first_boot_prov_val, MAX_LENGHT_JSON_VALUE, "%s", "true");
                    missing_first_boot_prov = false;
                }
                else
                {
                    debug("Detected an unexpected value for \"first_boot_provision\" parameter. ");
                    debug("It will be restored to default.");
                }

                i = i + 1;
            }
            else if(jsoneq(cstr_actual_config, &tokens[i], "wifi_ssid") == 0)
            {
                snprintf(wifi_ssid_val, MAX_LENGTH_WIFI_SSID+1, "%.*s", 
                    tokens[i+1].end - tokens[i+1].start, cstr_actual_config + tokens[i+1].start);

                missing_wifi_ssid = false;
                i = i + 1;
            }
            else if(jsoneq(cstr_actual_config, &tokens[i], "wifi_pass") == 0)
            {
                snprintf(wifi_pass_val, MAX_LENGTH_WIFI_PASS+1, "%.*s", 
                    tokens[i+1].end - tokens[i+1].start, cstr_actual_config + tokens[i+1].start);

                missing_wifi_pass = false;
                i = i + 1;
            }
            else if(jsoneq(cstr_actual_config, &tokens[i], "firmware_ver") == 0)
            {
                snprintf(fw_ver_val, MAX_LENGHT_JSON_VALUE, "%.*s", 
                    tokens[i+1].end - tokens[i+1].start, cstr_actual_config + tokens[i+1].start);

                missing_fw_ver = false;
                i = i + 1;
            }
        }

        // Check if any param was missing
        if((missing_first_boot_prov|missing_wifi_ssid|missing_wifi_pass|missing_fw_ver) == true)
        {
            // Restore parameter with default value
            if(missing_first_boot_prov)
            {
                debug("Wrong/missing \"first_boot_provision\" parameter in persistent config " \
                    "file, regenerating with default value...\n");
                snprintf(first_boot_prov_val, MAX_LENGHT_JSON_VALUE, "%s", 
                    DEFAULT_FIRST_BOOT_PROV);
            }
            if(missing_wifi_ssid)
            {
                debug("Wrong/missing \"wifi_ssid\" parameter in persistent config " \
                    "file, regenerating with default value...\n");
                snprintf(wifi_ssid_val, MAX_LENGTH_WIFI_SSID+1, "%s", DEFAULT_WIFI_SSID);
            }
            if(missing_wifi_pass)
            {
                debug("Wrong/missing \"wifi_pass\" parameter in persistent config " \
                    "file, regenerating with default value...\n");
                snprintf(wifi_pass_val, MAX_LENGTH_WIFI_PASS+1, "%s", DEFAULT_WIFI_PASS);
            }
            if(missing_fw_ver)
            {
                debug("Wrong/missing \"firmware_ver\" parameter in persistent config " \
                    "file, regenerating with default value...\n");
                snprintf(fw_ver_val, MAX_LENGHT_JSON_VALUE, "%s", DEFAULT_FIRMWARE_VERSION);
            }

            // Set save to spiffs flag
            save_to_spiffs = true;
        }
        
        // Generate device config in JSON format
        memset(cstr_params_config, '\0', MAX_SPIFFS_FILE_CONTENT+1);
        snprintf(cstr_params_config, MAX_SPIFFS_FILE_CONTENT+1, \
            "{\n" \
            "    \"first_boot_provision\": \"%s\",\n" \
            "    \"wifi_ssid\": \"%s\",\n" \
            "    \"wifi_pass\": \"%s\",\n" \
            "    \"firmware_ver\": \"%s\"\n" \
            "}", 
            first_boot_prov_val,
            wifi_ssid_val, 
            wifi_pass_val, //"<HIDDEN>", // It is better to hide WiFi password through serial
            fw_ver_val
        );

        // If there was any wrong/missing parameter, overwrite config file with regenerated ones
        if(save_to_spiffs)
        {
            debug("Regenerating SPIFFS config file...\n");
            if(!SPIFFS->file_write(SPIFFS_CONFIG_FILE, cstr_params_config))
            {
                debug("SPIFFS config file can't be created.");
                esp_reboot();
            }
            debug("SPIFFS config file successfully created/modified.\n");
        }

        // Apply loaded parameters values
         if((strncmp(first_boot_prov_val, "true", MAX_LENGHT_JSON_VALUE) == 0))
            Global->set_first_boot_provision(true);
        else
            Global->set_first_boot_provision(false);
        Global->set_wifi_ssid(wifi_ssid_val);
        Global->set_wifi_pass(wifi_pass_val);
        Global->set_firmware_version(fw_ver_val);
        
        debug("\nDevice Configuration Parameters Loaded:\n");
        debug("----------------------------------------\n");
        debug("%s\n\n", cstr_params_config);
    }
}

// Create config file with default values
void device_config_create_default(SimpleSPIFFS* SPIFFS)
{
    char cstr_default_config[MAX_SPIFFS_FILE_CONTENT+1];

    // Generate default device config in JSON format
    memset(cstr_default_config, '\0', MAX_SPIFFS_FILE_CONTENT+1);
    snprintf(cstr_default_config, MAX_SPIFFS_FILE_CONTENT+1, \
        "{\n" \
        "    \"first_boot_provision\": \"%s\",\n" \
        "    \"wifi_ssid\": \"%s\",\n" \
        "    \"wifi_pass\": \"%s\",\n" \
        "    \"firmware_ver\": \"%s\"\n" \
        "}", 
        DEFAULT_FIRST_BOOT_PROV,
        DEFAULT_WIFI_SSID, 
        DEFAULT_WIFI_PASS, 
        DEFAULT_FIRMWARE_VERSION
    );
    
    if(!SPIFFS->file_write(SPIFFS_CONFIG_FILE, cstr_default_config))
    {
        debug("SPIFFS config file can't be created.");
        esp_reboot();
    }
    debug("Config file successfully created.\n");
}

// Modify config file with actual system values
void device_config_update(SimpleSPIFFS* SPIFFS, Globals* Global)
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
    memset(cstr_default_config, '\0', MAX_SPIFFS_FILE_CONTENT+1);
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
    
    if(!SPIFFS->file_write(SPIFFS_CONFIG_FILE, cstr_default_config))
    {
        debug("SPIFFS config file can't be updated.");
        esp_reboot();
    }
    debug("Config file successfully updated.\n");
}

// JSON safe string-token comparison
int jsoneq(const char* json, jsmntok_t* tok, const char* s)
{
    if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
        strncmp(json + tok->start, s, tok->end - tok->start) == 0)
    {
        return 0;
    }
    return -1;
}
