/**************************************************************************************************/
// Project: 
// File: task_webserver.cpp
// Description: Device provision-configuration through AP and WebServer, FreeRTOS task file
// Created on: 19 jan. 2019
// Last modified date: 10 nov. 2019
// Version: 1.0.0
/**************************************************************************************************/

/* Libraries */

#include "task_webserver.h"

#include "commons.h"

/**************************************************************************************************/

/* HTML Builtin */

#define TAG "HTTPS WebServer"
#define WEBSERVER_RX_BUFFER_LEN 1024

#define HTTP_RES_HTML_HEADER \
    "HTTP/1.1 200 OK\r\n" \
    "Connection: close\r\n" \
    "Content-Type: text/html; charset=utf-8\r\n" \
    "Content-Length: %d\r\n" \
    "\r\n" \
    "%s\r\n" \
    "\r\n"

/*extern const uint8_t html_root[] asm("_binary_webserver_root_html_start");
extern const uint8_t html_root_end[] asm("_binary_webserver_root_html_end");
extern const uint8_t html_app_uarts[] asm("_binary_webserver_app_uarts_html_start");
extern const uint8_t html_app_uarts_end[] asm("_binary_webserver_app_uarts_html_end");*/

//const size_t html_root_size = html_root_end-html_root;
//const size_t html_app_uarts_size = html_app_uarts_end-html_app_uarts;

//static char html_response[MAX_LENGHT_HTTP_RES];
//static char html_response_body[MAX_LENGHT_HTTP_RES_BODY];

Globals* Global_for_callback;
uint8_t ws_clients_connected[MAX_WS_CLIENTS];
uint32_t last_login_token;

/**************************************************************************************************/

/* Tasks */

// Check for device configuration from user through an WiFi AP and secure WebServer
void task_webserver(void *pvParameter)
{
    bool wifi_ap_is_up = false;
    bool wifi_connected = false;
    bool wifi_has_ip = false;
    //bool webserver_started = false;

    // Get provided parameters
    tasks_argv* task_argv = (tasks_argv*)pvParameter;
    Globals* Global = task_argv->Global;

    debug("\nWebServer task initialized.\n");

    // Create WebSocket Task
    if(xTaskCreate(&task_websocket, "task_websocket", TASK_WEBSOCKET_STACK, 
                   pvParameter, tskIDLE_PRIORITY+5, NULL) != pdPASS)
    {
        debug("\nError - Can't create WebSocket task (not enough memory?)");
        esp_reboot();
    }

    while(1)
    {
        // Check for actual WiFi status
        Global->get_wifi_ap_is_up(wifi_ap_is_up);
        //Global->get_wifi_connected(wifi_connected);
        //Global->get_wifi_has_ip(wifi_has_ip);

        if(wifi_ap_is_up)
            web_server_start();

        //if(wifi_connected && wifi_has_ip)
        //{
            /*if(!webserver_started)
            {
                webserver_started = true;

                // Launch and handle Web Server
                start_https_web_server(Global);
            }*/

            // Launch and handle Web Server
            //start_https_web_server(Global);
            
            //web_server_start();
        //}
        /*else
        {
            if(webserver_started)
            {
                webserver_started = false;

                // Launch Web Server
                stop_https_web_server(Global);
            }
        }*/

        // Task CPU release
        delay(100);
    }
}

// Check for device configuration from user through an WiFi AP and secure WebServer
void task_websocket(void *pvParameter)
{
    // Get provided parameters
    tasks_argv* task_argv = (tasks_argv*)pvParameter;
    Globals* Global = task_argv->Global;
    Global_for_callback = Global;
    queue_uart_msg q_msg;
    static char msg[MSG_UART2WS_LEN];
    uint8_t any_received;
    QueueHandle_t* xQueue_uart0_to_ws = Global->get_xQueue_uart0_to_ws();
    QueueHandle_t* xQueue_uart1_to_ws = Global->get_xQueue_uart1_to_ws();
    QueueHandle_t* xQueue_uart2_to_ws = Global->get_xQueue_uart2_to_ws();

    memset(ws_clients_connected, 0, MAX_WS_CLIENTS);

    debug("\nWebSocket task initialized.\n");

    if(ws_server_start() != 1)
        debug("\nWarning: WebSocket internal task can't be initialized.\n");
    
    while(1)
    {
        any_received = 0;

        // Check for incoming uarts messages
        if(xQueueReceive(*xQueue_uart0_to_ws, &q_msg, (TickType_t)0) == pdTRUE)
        {
            snprintf(msg, MSG_UART2WS_LEN, "uart%" PRIu8 ":%s", q_msg.uart_num, q_msg.data);
            ws_server_send_text_all(msg, strlen(msg));
            any_received = 1;
        }
        if(xQueueReceive(*xQueue_uart1_to_ws, &q_msg, (TickType_t)0) == pdTRUE)
        {
            snprintf(msg, MSG_UART2WS_LEN, "uart%" PRIu8 ":%s", q_msg.uart_num, q_msg.data);
            ws_server_send_bin_all(msg, strlen(msg));
            any_received = 1;
        }
        if(xQueueReceive(*xQueue_uart2_to_ws, &q_msg, (TickType_t)0) == pdTRUE)
        {
            snprintf(msg, MSG_UART2WS_LEN, "uart%" PRIu8 ":%s", q_msg.uart_num, q_msg.data);
            ws_server_send_bin_all(msg, strlen(msg));
            any_received = 1;
        }
        
        // Task CPU release
        if(!any_received)
            delay(10);
    }
}

/**************************************************************************************************/

// handles websocket events
void websocket_callback(uint8_t num, WEBSOCKET_TYPE_t type, char* msg, uint64_t len)
{
    queue_uart_msg q_msg;
    char to_send_msg[32] = "";
    uint32_t login_token = 0;

    switch(type)
    {
        case WEBSOCKET_CONNECT:
            ESP_LOGI(TAG, "client %i connected!", num);
            Global_for_callback->set_ws_clients_connected(num, 1);
            Global_for_callback->increase_any_ws_clients_connected();
            break;

        case WEBSOCKET_DISCONNECT_EXTERNAL:
            ESP_LOGI(TAG, "client %i sent a disconnect message", num);
            Global_for_callback->set_ws_clients_connected(num, 0);
            Global_for_callback->decrease_any_ws_clients_connected();
            break;

        case WEBSOCKET_DISCONNECT_INTERNAL:
            ESP_LOGI(TAG, "client %i was disconnected", num);
            Global_for_callback->set_ws_clients_connected(num, 0);
            Global_for_callback->decrease_any_ws_clients_connected();
            break;

        case WEBSOCKET_DISCONNECT_ERROR:
            ESP_LOGI(TAG, "client %i was disconnected due to an error", num);
            Global_for_callback->set_ws_clients_connected(num, 0);
            Global_for_callback->decrease_any_ws_clients_connected();
            break;

        case WEBSOCKET_TEXT:
            printf("Websocket data from client:\n%s\n\n", msg);
            if(len)
            {
                if(strncmp(msg, "login:", strlen("login:")) == 0)
                {
                    char web_user[32] = "";
                    char web_passw[32] = "";

                    printf("Web login detected.\n");

                    if(sscanf(msg, (char*)"login:%s %s", web_user, web_passw) != 2)
                    {
                        printf("Login fail. Can't get user and password data.\n");
                        printf("  Received web user: %s\n", web_user);
                        printf("  Received web password: %s\n\n", web_passw);
                        ws_server_send_text_client_from_callback(num, (char*)"login_bad", strlen("login_bad"));
                        break;
                    }
                    printf("  Received web user: %s\n", web_user);
                    printf("  Received web password: %s\n\n", web_passw);
                    
                    if((strcmp(web_user, WEBSERVER_USER) == 0) && (strcmp(web_passw, WEBSERVER_PASS) == 0))
                    {
                        printf("Login success. Valid credentials.\n");
                        login_token = esp_random();
                        last_login_token = login_token;
                        snprintf(to_send_msg, 32, "login_ok:%" PRIu32, login_token);
                        ws_server_send_text_client_from_callback(num, to_send_msg, strlen(to_send_msg));
                    }
                    else
                    {
                        printf("Login fail. Invalid credentials.\n");
                        ws_server_send_text_client_from_callback(num, (char*)"login_bad", strlen("login_bad"));
                    }
                }
                else if(strncmp(msg, "cfg_bauds_uart", strlen("cfg_bauds_uart")) == 0)
                {
                    uint16_t uart_num;
                    uint32_t uart_bauds;
                    QueueHandle_t* xQueue_ws_to_uart0 = Global_for_callback->xQueue_ws_to_uart0();
                    QueueHandle_t* xQueue_ws_to_uart1 = Global_for_callback->xQueue_ws_to_uart1();
                    QueueHandle_t* xQueue_ws_to_uart2 = Global_for_callback->xQueue_ws_to_uart2();

                    if(sscanf(msg, (char*)"cfg_bauds_uart%" SCNu16 ":%" SCNu32, &uart_num, &uart_bauds) != 2)
                    {
                        printf("WebServer error when parsing an unexpected uart data format.\n");
                        break;
                    }

                    printf("  Received uart number: %" PRIu16 "\n", uart_num);
                    printf("  Received uart cfg bauds: %" PRIu32 "\n\n", uart_bauds);
                    q_msg.uart_num = uart_num;
                    q_msg.uart_bauds = uart_bauds;
                    q_msg.data[0] = '\0';
                    if(uart_num == 0)
                        xQueueSend(*xQueue_ws_to_uart0, &q_msg, (TickType_t)0);
                    else if(uart_num == 1)
                        xQueueSend(*xQueue_ws_to_uart1, &q_msg, (TickType_t)0);
                    else if(uart_num == 2)
                        xQueueSend(*xQueue_ws_to_uart2, &q_msg, (TickType_t)0);
                }
                else if(strncmp(msg, "uart", strlen("uart")) == 0)
                {
                    uint16_t uart_num;
                    char uart_text[MSG_UART2WS_LEN];
                    QueueHandle_t* xQueue_ws_to_uart0 = Global_for_callback->xQueue_ws_to_uart0();
                    QueueHandle_t* xQueue_ws_to_uart1 = Global_for_callback->xQueue_ws_to_uart1();
                    QueueHandle_t* xQueue_ws_to_uart2 = Global_for_callback->xQueue_ws_to_uart2();

                    memset(uart_text, '\0', MSG_UART2WS_LEN);
                    if(sscanf(msg, (char*)"uart%" SCNu16 ":%255c", &uart_num, uart_text) != 2)
                    {
                        printf("WebServer error when parsing an unexpected uart data format.\n");
                        break;
                    }
                    uart_text[MSG_UART2WS_LEN-1] = '\0'; // Safe close string (cause %255c doesn't append \0)
                    str_fix_duplicate_eol(uart_text, strlen(uart_text));

                    printf("  Received uart number: %" PRIu16 "\n", uart_num);
                    printf("  Received uart text: %s\n\n", uart_text);
                    q_msg.uart_num = uart_num;
                    q_msg.uart_bauds = 0;
                    snprintf(q_msg.data, MSG_UART2WS_LEN, "%s", uart_text);
                    if(uart_num == 0)
                        xQueueSend(*xQueue_ws_to_uart0, &q_msg, (TickType_t)0);
                    else if(uart_num == 1)
                        xQueueSend(*xQueue_ws_to_uart1, &q_msg, (TickType_t)0);
                    else if(uart_num == 2)
                        xQueueSend(*xQueue_ws_to_uart2, &q_msg, (TickType_t)0);
                }
            }
            break;

        case WEBSOCKET_BIN:
            ESP_LOGI(TAG, "client %i sent binary message of size %i:\n%s", num, (uint32_t)len, msg);
            break;

        case WEBSOCKET_PING:
            ESP_LOGI(TAG, "client %i pinged us with message of size %i:\n%s", num, (uint32_t)len, msg);
            break;

        case WEBSOCKET_PONG:
            ESP_LOGI(TAG, "client %i responded to the ping\n", num);
            break;

        default:
            printf("WS DEFAULT\n");
    }
}


void web_server_start(void)
{
    struct netconn *conn, *newconn;
    static err_t err;

    conn = netconn_new(NETCONN_TCP);
    netconn_bind(conn, NULL, 80);
    netconn_listen(conn);
    ESP_LOGI(TAG, "server listening");
    do
    {
        err = netconn_accept(conn, &newconn);
        ESP_LOGI(TAG, "new client connected");
        if(err == ERR_OK)
        {
            http_serve(newconn);
        }

        // Task CPU release
        //delay(10);
    } while(err == ERR_OK);

    netconn_close(conn);
    netconn_delete(conn);
}

// serves any clients
void http_serve(struct netconn *conn)
{
  const static char HTML_HEADER[] = "HTTP/1.1 200 OK\nContent-type: text/html\n\n";
  const static char ERROR_HEADER[] = "HTTP/1.1 404 Not Found\nContent-type: text/html\n\n";
  const static char JS_HEADER[] = "HTTP/1.1 200 OK\nContent-type: text/javascript\n\n";
  const static char CSS_HEADER[] = "HTTP/1.1 200 OK\nContent-type: text/css\n\n";
  //const static char PNG_HEADER[] = "HTTP/1.1 200 OK\nContent-type: image/png\n\n";
  const static char ICO_HEADER[] = "HTTP/1.1 200 OK\nContent-type: image/x-icon\n\n";
  //const static char PDF_HEADER[] = "HTTP/1.1 200 OK\nContent-type: application/pdf\n\n";
  //const static char EVENT_HEADER[] = "HTTP/1.1 200 OK\nContent-Type: text/event-stream\nCache-Control: no-cache\nretry: 3000\n\n";
  struct netbuf* inbuf;
  static char* buf;
  static uint16_t buflen;
  static err_t err;

  // root page
  extern const uint8_t root_html_start[] asm("_binary_webserver_root_html_start");
  extern const uint8_t root_html_end[] asm("_binary_webserver_root_html_end");
  const uint32_t root_html_len = root_html_end - root_html_start;

  // app_uarts page
  extern const uint8_t appuarts_html_start[] asm("_binary_webserver_app_uarts_html_start");
  extern const uint8_t app_uarts_html_end[] asm("_binary_webserver_app_uarts_html_end");
  const uint32_t appuarts_html_len = app_uarts_html_end - appuarts_html_start - 1;

  // not_found page
  extern const uint8_t notfound_html_start[] asm("_binary_webserver_not_found_html_start");
  extern const uint8_t notfound_html_end[] asm("_binary_webserver_not_found_html_end");
  const uint32_t notfound_html_len = notfound_html_end - notfound_html_start - 1;

  // script.js
  extern const uint8_t script_js_start[] asm("_binary_webserver_script_js_start");
  extern const uint8_t script_js_end[] asm("_binary_webserver_script_js_end");
  const uint32_t script_js_len = script_js_end - script_js_start - 1;

  // styles.css
  extern const uint8_t styles_css_start[] asm("_binary_webserver_styles_css_start");
  extern const uint8_t styles_css_end[] asm("_binary_webserver_styles_css_end");
  const uint32_t styles_css_len = styles_css_end - styles_css_start - 1;

  // favicon.ico
  extern const uint8_t favicon_ico_start[] asm("_binary_webserver_favicon_ico_start");
  extern const uint8_t favicon_ico_end[] asm("_binary_webserver_favicon_ico_end");
  const uint32_t favicon_ico_len = favicon_ico_end - favicon_ico_start;

  netconn_set_recvtimeout(conn, 2000); // allow a connection timeout of 2 second
  ESP_LOGI(TAG, "reading from client...");
  err = netconn_recv(conn, &inbuf);
  ESP_LOGI(TAG, "read from client");
  if(err == ERR_OK)
  {
    netbuf_data(inbuf, (void**)&buf, &buflen);
    if(buf)
    {
      // default page
      if(strstr(buf, "GET / ") && !strstr(buf, "Upgrade: websocket"))
      {
        ESP_LOGI(TAG, "Sending /");
        netconn_write(conn, HTML_HEADER, sizeof(HTML_HEADER)-1, NETCONN_NOCOPY);
        netconn_write(conn, root_html_start, root_html_len, NETCONN_NOCOPY);
        netconn_close(conn);
        netconn_delete(conn);
        netbuf_delete(inbuf);
      }

      // default page websocket
      else if(strstr(buf, "GET / ") && strstr(buf, "Upgrade: websocket"))
      {
        ESP_LOGI(TAG, "Requesting websocket on /");
        ws_server_add_client(conn, buf, buflen, (char*)"/", websocket_callback);
        netbuf_delete(inbuf);
      }

      else if(strstr(buf,"GET /script.js "))
      {
        ESP_LOGI(TAG, "Sending /script.js");
        netconn_write(conn, JS_HEADER, sizeof(JS_HEADER)-1, NETCONN_NOCOPY);
        netconn_write(conn, script_js_start, script_js_len, NETCONN_NOCOPY);
        netconn_close(conn);
        netconn_delete(conn);
        netbuf_delete(inbuf);
      }

      else if(strstr(buf,"GET /styles.css "))
      {
        ESP_LOGI(TAG, "Sending /styles.css");
        netconn_write(conn, CSS_HEADER, sizeof(CSS_HEADER)-1, NETCONN_NOCOPY);
        netconn_write(conn, styles_css_start, styles_css_len, NETCONN_NOCOPY);
        netconn_close(conn);
        netconn_delete(conn);
        netbuf_delete(inbuf);
      }

      else if(strstr(buf, "GET /favicon.ico "))
      {
        ESP_LOGI(TAG, "Sending favicon.ico");
        netconn_write(conn, ICO_HEADER, sizeof(ICO_HEADER)-1, NETCONN_NOCOPY);
        netconn_write(conn, favicon_ico_start, favicon_ico_len, NETCONN_NOCOPY);
        netconn_close(conn);
        netconn_delete(conn);
        netbuf_delete(inbuf);
      }

      else if(strstr(buf,"GET /"))
      {
        ESP_LOGI(TAG, "Unknown request, sending not_found page: %s", buf);
        netconn_write(conn, ERROR_HEADER, sizeof(ERROR_HEADER)-1, NETCONN_NOCOPY);
        netconn_write(conn, notfound_html_start, notfound_html_len, NETCONN_NOCOPY);
        netconn_close(conn);
        netconn_delete(conn);
        netbuf_delete(inbuf);
      }

      else if(strstr(buf, "POST /app_uarts "))
      {
        char expect_data[32];

        ESP_LOGI(TAG, "Sending app_uarts.html");

        snprintf(expect_data, 32, "token=%" PRIu32, last_login_token);
        if(strstr(buf, expect_data))
        {
            netconn_write(conn, HTML_HEADER, sizeof(HTML_HEADER)-1, NETCONN_NOCOPY);
            netconn_write(conn, appuarts_html_start, appuarts_html_len, NETCONN_NOCOPY);
            netconn_close(conn);
            netconn_delete(conn);
            netbuf_delete(inbuf);
            // Renew login token (avoid captured POST login replication)
            last_login_token = esp_random();
        }
      }

      else
      {
        ESP_LOGI(TAG, "Unknown request");
        printf("%s\n", buf);
        netconn_close(conn);
        netconn_delete(conn);
        netbuf_delete(inbuf);
      }
    }
    else
    {
      ESP_LOGI(TAG, "Unknown request (empty?...)");
      netconn_close(conn);
      netconn_delete(conn);
      netbuf_delete(inbuf);
    }
  }
  else
  { // if err==ERR_OK
    ESP_LOGI(TAG, "error on read, closing connection");
    netconn_close(conn);
    netconn_delete(conn);
    netbuf_delete(inbuf);
  }
}

// Fix EOL two bytes ('\'+'n' instead '\n'; also for \r)
void str_fix_duplicate_eol(char* text, size_t text_len)
{
    if(text[text_len-2] == '\\')
    {
        if(text[text_len-1] == 'n')
        {
            text[text_len-2] = '\n';
            text[text_len-1] = '\0';
        }
        else if(text[text_len-1] == 'r')
        {
            text[text_len-2] = '\r';
            text[text_len-1] = '\0';
        }
    }
    text_len = strlen(text);
    if((text[text_len-3] == '\\') && (text[text_len-2] == 'r'))
    {
        text[text_len-3] = '\r';
        text[text_len-2] = text[text_len-1];
        text[text_len-1] = '\0';
    }
}

/**************************************************************************************************/

/* HTTPS Web Server Functions */

// Start HTTPS Web Server
/*void start_https_web_server(Globals* Global)
{
    int ret;

    SSL_CTX *ctx;
    SSL *ssl;

    int sockfd, new_sockfd;
    socklen_t addr_len;
    struct sockaddr_in sock_addr;

    char fw_ver[MAX_LENGTH_VERSION+1];
    char ip[MAX_LENGTH_IPV4+1];
    char* mac = esp_get_base_mac();

    static char recv_buf[WEBSERVER_RX_BUFFER_LEN];

    ESP_LOGI(TAG, "SSL server context create ......");
    // For security reasons, it is best if you can use
    //   TLSv1_2_server_method() here instead of TLS_server_method().
    //   However some old browsers may not support TLS v1.2.
    ctx = SSL_CTX_new(TLS_server_method());
    if(!ctx)
    {
        ESP_LOGI(TAG, "failed");
        goto failed1;
    }
    ESP_LOGI(TAG, "OK");

    ESP_LOGI(TAG, "SSL server context set own certification......");
    ret = SSL_CTX_use_certificate_ASN1(ctx, server_cert_end-server_cert_start, server_cert_start);
    if(!ret)
    {
        ESP_LOGI(TAG, "failed");
        goto failed2;
    }
    ESP_LOGI(TAG, "OK");

    ESP_LOGI(TAG, "SSL server context set private key......");
    ret = SSL_CTX_use_PrivateKey_ASN1(0, ctx, server_key_start, server_key_end-server_key_start);
    if(!ret)
    {
        ESP_LOGI(TAG, "failed");
        goto failed2;
    }
    ESP_LOGI(TAG, "OK");

    ESP_LOGI(TAG, "SSL server create socket ......");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        ESP_LOGI(TAG, "failed");
        goto failed2;
    }
    ESP_LOGI(TAG, "OK");

    ESP_LOGI(TAG, "SSL server socket bind ......");
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = 0;
    sock_addr.sin_port = htons(HTTPS_PORT);
    ret = bind(sockfd, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
    if(ret)
    {
        ESP_LOGI(TAG, "failed");
        goto failed3;
    }
    ESP_LOGI(TAG, "OK");

    ESP_LOGI(TAG, "SSL server socket listen ......");
    ret = listen(sockfd, 32);
    if(ret)
    {
        ESP_LOGI(TAG, "failed");
        goto failed3;
    }
    ESP_LOGI(TAG, "OK");

reconnect:
    ESP_LOGI(TAG, "SSL server create ......");
    ssl = SSL_new(ctx);
    if(!ssl)
    {
        ESP_LOGI(TAG, "failed");
        goto failed3;
    }
    ESP_LOGI(TAG, "OK");

    ESP_LOGI(TAG, "SSL server socket accept client ......");
    new_sockfd = accept(sockfd, (struct sockaddr *)&sock_addr, &addr_len);
    if(new_sockfd < 0)
    {
        ESP_LOGI(TAG, "failed" );
        goto failed4;
    }
    ESP_LOGI(TAG, "OK");

    SSL_set_fd(ssl, new_sockfd);

    ESP_LOGI(TAG, "SSL server accept client ......");
    ret = SSL_accept(ssl);
    if(!ret)
    {
        ESP_LOGI(TAG, "failed");
        goto failed5;
    }
    ESP_LOGI(TAG, "OK");

    // Server handle connections
    ESP_LOGI(TAG, "SSL server read message ......");
    do
    {
        memset(recv_buf, 0, WEBSERVER_RX_BUFFER_LEN);
        ret = SSL_read(ssl, recv_buf, WEBSERVER_RX_BUFFER_LEN - 1);
        if (ret <= 0)
            break;

        ESP_LOGI(TAG, "SSL read: %s", recv_buf);

        // For any GET Request, just send root page
        if(strstr(recv_buf, "GET "))
        {
            ESP_LOGI(TAG, "GET Request received");
            http_send_response(ssl, HTTP_RES_HTML_HEADER, (const char*)html_root);
            break;
        }
        else if(strstr(recv_buf, "POST /setup-info"))
        {
            ESP_LOGI(TAG, "POST /setup-info Request received");

            // If expected Body fields exists
            if((strstr(recv_buf, "action=login")) && (strstr(recv_buf, "user=")) && (strstr(recv_buf, "password="))) 
            {
                static char user[64];
                static char pass[128];
                
                snprintf(user, 64, "user=%s", WEBSERVER_USER);
                snprintf(pass, 128, "password=%s", WEBSERVER_PASS);

                // If correct credentials provided
                if((strstr(recv_buf, user)) && (strstr(recv_buf, pass)))
                {
                    ESP_LOGI(TAG, "Correct Login Credentials");

                    // Send device info page                    
                    Global->get_wifi_ip(ip);
                    Global->get_firmware_version(fw_ver);
                    snprintf(html_response_body, MAX_LENGHT_HTTP_RES_BODY, (const char*)html_app_uarts, "WiFi Status Light", ip, mac, fw_ver);
                    http_send_response(ssl, HTTP_RES_HTML_HEADER, html_response_body);
                    break;
                }
            }

            ESP_LOGI(TAG, "Bad Login Credentials");
            snprintf(html_response_body, MAX_LENGHT_HTTP_RES_BODY, (const char*)html_root, "Invalid Credentials");
            http_send_response(ssl, HTTP_RES_HTML_HEADER, html_response_body);
            break;
        }
        else if(strstr(recv_buf, "POST /setup-config"))
        {
            ESP_LOGI(TAG, "POST /setup-config Request received");

            // Send device config page
            // TODO - Send proper Page (Now sending info page)
            Global->get_wifi_ip(ip);
            Global->get_firmware_version(fw_ver);
            snprintf(html_response_body, MAX_LENGHT_HTTP_RES_BODY, (const char*)html_app_uarts, "WiFi Status Light", ip, mac, fw_ver);
            http_send_response(ssl, HTTP_RES_HTML_HEADER, html_response_body);
            break;
        }
        else if(strstr(recv_buf, "POST /setup-update"))
        {
            ESP_LOGI(TAG, "POST /setup-update Request received");

            // Send device update page
            // TODO - Send proper Page (Now sending info page)
            Global->get_wifi_ip(ip);
            Global->get_firmware_version(fw_ver);
            snprintf(html_response_body, MAX_LENGHT_HTTP_RES_BODY, (const char*)html_app_uarts, "WiFi Status Light", ip, mac, fw_ver);
            http_send_response(ssl, HTTP_RES_HTML_HEADER, html_response_body);
            break;
        }
        else if(strstr(recv_buf, "POST /setup-about"))
        {
            ESP_LOGI(TAG, "POST /setup-about Request received");

            // Send device about page
            // TODO - Send proper Page (Now sending info page)
            Global->get_wifi_ip(ip);
            Global->get_firmware_version(fw_ver);
            snprintf(html_response_body, MAX_LENGHT_HTTP_RES_BODY, (const char*)html_app_uarts, "WiFi Status Light", ip, mac, fw_ver);
            http_send_response(ssl, HTTP_RES_HTML_HEADER, html_response_body);
            break;
        }
        else
        {
            ESP_LOGI(TAG, "Unexpected Request received");

            // For all unexpected requests send root page
            http_send_response(ssl, HTTP_RES_HTML_HEADER, (const char*)html_root);
            break;
        }
        
    } while (1);
    
    SSL_shutdown(ssl);
failed5:
    close(new_sockfd);
    new_sockfd = -1;
failed4:
    SSL_free(ssl);
    ssl = NULL;
    goto reconnect;
failed3:
    close(sockfd);
    sockfd = -1;
failed2:
    SSL_CTX_free(ctx);
    ctx = NULL;
failed1:
    debug("WebServer initialization fail.");
    esp_reboot();
    return;
}

int8_t http_send_response(SSL* ssl, const char* header, const char* body)
{
    int rc;

    snprintf(html_response, MAX_LENGHT_HTTP_RES, header, strlen(body), body);
    
    ESP_LOGI(TAG, "HTTP Response to send:\n%s\n", html_response);

    // Send the header
    rc = SSL_write(ssl, html_response, strlen(html_response));
    if(rc <= 0)
        ESP_LOGI(TAG, "ERROR");
        return rc;
    
    return rc;
}

// Parse HTTP request and find index position of body
// Return Code: 0 - body found; 1 - missing body
int8_t http_get_body(const char* http_req, const size_t http_req_len, size_t* body_pos)
{
	int8_t rc = -1;

	// Check if request pointer is NULL
	if(http_req == NULL)
	{
		rc = -2;
		return rc;
	}
	
	// Check using Standard CRLFCRLF
	for(size_t i = 0; i < http_req_len; i++)
	{
		// If next char is outside array (end of array reach), break the loop
		if(i+3 >= http_req_len)
			break;
		
		if((http_req[i] == '\r') && (http_req[i+1] == '\n') && (http_req[i+2] == '\r') && (http_req[i+3] == '\n'))
		{
			*body_pos = i + 4;
            rc = 0;
            return rc;
		}
	}
	
	// Lets check using just LFLF (Support for non-standar requests)
	for(size_t i = 0; i < http_req_len; i++)
	{
		// If next char is outside array (end of array reach), break the loop
		if(i+1 >= http_req_len)
			break;
		
		if((http_req[i] == '\n') && (http_req[i+1] == '\n'))
		{
			// If there is no body (not '\0' in array or bad request lenght provided)
			if(i+1 >= http_req_len)
				break;
			
			// Body found
			*body_pos = i + 2;
            rc = 0;
            return rc;
		}
	}
	
	return rc;
}*/
