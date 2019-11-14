/**************************************************************************************************/
// Project: 
// File: task_webserver.h
// Description: App WebServer, FreeRTOS task file
// Created on: 15 jul. 2019
// Last modified date: 15 jul. 2019
// Version: 1.0.0
/**************************************************************************************************/

/* Include Guard */

#ifndef WEBSERVER_H
#define WEBSERVER_H

/**************************************************************************************************/

/* C++ compiler compatibility */

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************/

/* Libraries */

// Standard C/C++ libraries
#include <string.h>

// Device libraries (ESP-IDF)
#include <esp_log.h>
#include <nvs_flash.h>
#include <openssl/ssl.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>

// Custom libraries
#include "constants.h"
#include "globals.h"
//#include "commons.h"
#include "rgbleds.h"
#include "websocket_server.h"

/**************************************************************************************************/

/* HTTPS Certificates locates in internal Blob memory */

//extern const uint8_t server_cert_start[] asm("_binary_webserver_certs_esp_cert_pem_start");
//extern const uint8_t server_cert_end[] asm("_binary_webserver_certs_esp_cert_pem_end");
//extern const uint8_t server_key_start[] asm("_binary_webserver_certs_esp_key_pem_start");
//extern const uint8_t server_key_end[] asm("_binary_webserver_certs_esp_key_pem_end");


/**************************************************************************************************/

/* Functions */

extern void task_webserver(void *pvParameter);
extern void task_websocket(void *pvParameter);

extern void start_https_web_server(Globals* Global);
int8_t http_send_response(SSL* ssl, const char* header, const char* body);
extern int8_t http_get_body(const char* http_req, const size_t http_req_len, size_t* body_pos);

extern void web_server_start(void);
extern void http_serve(struct netconn *conn);
extern void websocket_callback(uint8_t num,WEBSOCKET_TYPE_t type,char* msg,uint64_t len);

extern void str_fix_duplicate_eol(char* text, size_t text_len);

/**************************************************************************************************/

#ifdef __cplusplus
}
#endif  // extern "C"

#endif
