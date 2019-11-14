/**************************************************************************************************/
// Project: 
// File: task_internetstatus.cpp
// Description: Internet status FreeRTOS task file
// Created on: 21 dec. 2018
// Last modified date: 22 dec. 2018
// Version: 1.0.0
/**************************************************************************************************/

/* Libraries */

#include "task_internet_status.h"

/**************************************************************************************************/

volatile bool pinging = false;
volatile bool ping_success = false;
volatile uint8_t num_pings = 0;

/**************************************************************************************************/

/* Task */

// Check for Internet connection by periodic ICMP ping to Google DNS (8.8.8.8)
void task_internet_status(void *pvParameter)
{
    bool wifi_connected = false;
    bool wifi_has_ip = false;

    // Get provided parameters
    tasks_argv* task_argv = (tasks_argv*)pvParameter;
    Globals* Global = task_argv->Global;

    debug("\nInternet status task initialized.\n");

    // Initialize and configure Ping properties
    ping_setup();

    while(1)
    {
        // Check for actual WiFi status
        Global->get_wifi_connected(wifi_connected);
        Global->get_wifi_has_ip(wifi_has_ip);
        if(wifi_connected && wifi_has_ip)
        {
            // Ping and wait response
            if(!pinging)
            {
                // Launch ping
                //debug("Launching ping to %s\n", PING_TO_URL);
                ping_run(PING_TO_URL);

                // Block until ping result
                //debug("Waiting Ping Result...\n");
                while(pinging)
                {
                    // Check if connection has been lost
                    Global->get_wifi_connected(wifi_connected);
                    Global->get_wifi_has_ip(wifi_has_ip);
                    if(!wifi_connected || !wifi_has_ip)
                        ping_success = false;

                    delay(100);
                }

                // Set internet connection from pings result
                if(ping_success)
                {
                    //debug("Ping OK - Internet connection.\n");
                    Global->set_internet_connection(true);
                }
                else
                {
                    //debug("Ping Fail - No Internet connection.\n");
                    Global->set_internet_connection(false);
                }

                // Wait T_INTERNET_CHECKS*100 milliseconds (30s) until next internet check
                for(uint16_t i = 0; i < T_INTERNET_CHECKS; i++)
                {
                    // Stop waiting if connections has been lost
                    Global->get_wifi_connected(wifi_connected);
                    Global->get_wifi_has_ip(wifi_has_ip);
                    if(!wifi_connected || !wifi_has_ip)
                    {
                        Global->set_internet_connection(false);
                        break;
                    }
                    
                    delay(100);
                }
            }
        }
        else
            Global->set_internet_connection(false);

        // Task CPU release
        delay(100);
    }
}

/**************************************************************************************************/

/* Ping Process Completed Callback */

esp_err_t ping_result(ping_target_id_t msg_type, esp_ping_found* found)
{
    /*debug("\nAvgTime:%.1fmS Sent:%d Rec:%d Err:%d min(mS):%d max(mS):%d \n", 
           (float)found->total_time/found->recv_count, found->send_count, found->recv_count, 
           found->err_count, found->min_time, found->max_time);
	debug("Resp(mS):%d Timeouts:%d Total Time:%d\n",found->resp_time, found->timeout_count, 
           found->total_time);*/
    
    num_pings = num_pings + 1;
    //debug("Ping iteration %d\n", num_pings);
    if(num_pings == PING_NUM_SENT+1)
    {
        if(found->recv_count > 0)
            ping_success = true;

        pinging = false;
        num_pings = 0;
    }

	return ESP_OK;
}

/**************************************************************************************************/

/* Functions */

// Setup and configure Ping properties
void ping_setup(void)
{
    esp_ping_set_target(PING_TARGET_IP_ADDRESS_COUNT, (void*)&PING_NUM_SENT, sizeof(uint32_t));
    esp_ping_set_target(PING_TARGET_RCV_TIMEO, (void*)&PING_TIMEOUT_MS, sizeof(uint32_t));
    esp_ping_set_target(PING_TARGET_DELAY_TIME, (void*)&PING_DELAY_MS, sizeof(uint32_t));
    esp_ping_set_target(PING_TARGET_RES_FN, (void*)&ping_result, sizeof(&ping_result));
}

// Launch ping process
void ping_run(const char* ip_address)
{
    ip4_addr_t remote_host;
    inet_pton(AF_INET, ip_address, &remote_host);
    esp_ping_set_target(PING_TARGET_IP_ADDRESS, &remote_host.addr, sizeof(uint32_t));

    ping_success = false;
    pinging = true;
    ping_init();
}
