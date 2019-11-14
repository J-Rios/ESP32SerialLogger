/**************************************************************************************************/
// Project: 
// File: task_uarts.cpp
// Description: 
// Created on: 27 oct. 2019
// Last modified date: 10 nov. 2019
// Version: 1.0.0
/**************************************************************************************************/

/* Libraries */

#include "task_uarts.h"

#include "commons.h"
#include <esp_heap_caps.h>

/**************************************************************************************************/

/* Task */

// 
void task_uarts(void *pvParameter)
{
    // Get provided parameters
    tasks_argv* task_argv = (tasks_argv*)pvParameter;
    Globals* Global = task_argv->Global;
    QueueHandle_t* xQueue_ws_to_uart1 = Global->xQueue_ws_to_uart1();
    QueueHandle_t* xQueue_ws_to_uart2 = Global->xQueue_ws_to_uart2();
    QueueHandle_t* xQueue_uart1_to_ws = Global->get_xQueue_uart1_to_ws();
    QueueHandle_t* xQueue_uart2_to_ws = Global->get_xQueue_uart2_to_ws();
    queue_uart_msg q_msg;
    static uint8_t uart_data_rx[UART_RX_BUF_SIZE];
    uint8_t any_ws_clients_connected;
    int32_t received_bytes = 0;

    debug("\nUARTs task initialized.\n");

    esp_log_level_set("UARTS", ESP_LOG_INFO);

    uart_init(UART_NUM_1, 9600, GPIO_NUM_18, GPIO_NUM_19);
    uart_init(UART_NUM_2, 19200, GPIO_NUM_16, GPIO_NUM_17);

    while(1)
    {
        /* Check for incoming websocket to uarts messages */

        // UART1
        if(xQueueReceive(*xQueue_ws_to_uart1, &q_msg, (TickType_t)0) == pdTRUE)
        {
            if(q_msg.uart_bauds != 0)
                uart_set_baudrate(UART_NUM_1, q_msg.uart_bauds);
            else
                uart_send_data(UART_NUM_1, q_msg.data);
        }
        // UART2
        if(xQueueReceive(*xQueue_ws_to_uart2, &q_msg, (TickType_t)0) == pdTRUE)
        {
            if(q_msg.uart_bauds != 0)
                uart_set_baudrate(UART_NUM_2, q_msg.uart_bauds);
            else
                uart_send_data(UART_NUM_2, q_msg.data);
        }

        /******************************************************************************************/

        /* Check for incomming uarts to websocket messages */

        // UART1
        received_bytes = uart_receive_data(UART_NUM_1, uart_data_rx);
        if(received_bytes > 0)
        {
            //printf("UART1 read: %s\n", uart_data_rx);
            Global->get_any_ws_clients_connected(any_ws_clients_connected);
            if(any_ws_clients_connected)
            {
                // Send the received data to websocket task
                q_msg.uart_num = 1;
                snprintf(q_msg.data, MSG_UART2WS_LEN, "%s", uart_data_rx);
                if(xQueueSend(*xQueue_uart1_to_ws, &q_msg, (TickType_t)0) != pdPASS)
                    printf("\nQueue send fail in UART1\n\n");
            }
        }
        // UART2
        received_bytes = uart_receive_data(UART_NUM_2, uart_data_rx);
        if(received_bytes > 0)
        {
            //printf("UART2 read: %s\n", uart_data_rx);
            Global->get_any_ws_clients_connected(any_ws_clients_connected);
            if(any_ws_clients_connected)
            {
                // Send the received data to websocket task
                q_msg.uart_num = 2;
                snprintf(q_msg.data, MSG_UART2WS_LEN, "%s", uart_data_rx);
                if(xQueueSend(*xQueue_uart2_to_ws, &q_msg, (TickType_t)0) != pdPASS)
                    printf("\nQueue send fail in UART2\n\n");
            }
        }

        // Task CPU release
        delay(10);
    }
}

/**************************************************************************************************/

/* UART Functions */

// UART initialization
void uart_init(const uart_port_t uart_num, const uint32_t baud_rate, const uint8_t pin_rx, 
const uint8_t pin_tx, const uart_word_length_t data_bits, const uart_stop_bits_t stop_bits, 
const uart_parity_t parity)
{
    const uart_config_t uart_config = {
        .baud_rate = (int)baud_rate,
        .data_bits = data_bits,
        .parity = parity,
        .stop_bits = stop_bits,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
        .use_ref_tick = false
    };

    uart_driver_delete(uart_num);
    uart_param_config(uart_num, &uart_config);
    uart_set_pin(uart_num, pin_tx, pin_rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(uart_num, UART_RX_BUF_SIZE*2, 0, 0, NULL, 0);
}

// UART Send data
int32_t uart_send_data(const uart_port_t uart_num, const char* data_tx)
{
    const int len = strlen(data_tx);
    const int tx_bytes = uart_write_bytes(uart_num, data_tx, len);
    if(tx_bytes < len)
    {
        ESP_LOGI("UARTS", "Error when sending data.\nBytes to send: %d.\nBytes sent: %d.\n", len, 
                tx_bytes);
    }
    return tx_bytes;
}

// UART Receive data
int32_t uart_receive_data(const uart_port_t uart_num, uint8_t* data_rx)
{
    const int rx_bytes = uart_read_bytes(uart_num, data_rx, UART_RX_BUF_SIZE, 100/portTICK_RATE_MS);
    if(rx_bytes < 0)
        ESP_LOGI("UARTS", "Error when reading data.\nBytes read: %d.\n", rx_bytes);
    else if(rx_bytes > 0)
    {
        if(rx_bytes < UART_RX_BUF_SIZE)
            data_rx[rx_bytes] = '\0';
        else
            data_rx[UART_RX_BUF_SIZE-1] = '\0';
    }

    return rx_bytes;
}
