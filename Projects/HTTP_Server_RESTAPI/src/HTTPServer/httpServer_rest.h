/**
 @file		httpServer.h
 @brief 	Define constants and functions related HTTP Web server.
 */

#ifndef	__HTTPSERVER_H__
#define	__HTTPSERVER_H__

#include <stdint.h>
#include "W7500x_wztoe.h"

// HTTP Server debug message enable
#define _HTTPSERVER_DEBUG_

#define INITIAL_WEBPAGE				"index.html"
#define INITIAL_RESOURCE			"index"

/* Not supported: Web Server Content Storage Select */
/*
//#define _USE_SDCARD_
#ifndef _USE_SDCARD_
	//#define _USE_FLASH_
#endif
*/

/* Watchdog timer */
//#define _USE_WATCHDOG_

/*********************************************
* HTTP Process states list
*********************************************/
#define STATE_HTTP_IDLE             0        /* IDLE, Waiting for data received (TCP established) */
#define STATE_HTTP_REQ_INPROC  		1        /* Received HTTP request from HTTP client */
#define STATE_HTTP_REQ_DONE    		2        /* The end of HTTP request parse */
#define STATE_HTTP_RES_INPROC  		3        /* Sending the HTTP response to HTTP client (in progress) */
#define STATE_HTTP_RES_DONE    		4        /* The end of HTTP response send (HTTP transaction ended) */

/*********************************************
* HTTP Simple Return Value
*********************************************/
#define HTTP_FAILED					0
#define HTTP_OK						1
#define HTTP_RESET					2


/*********************************************
* HTTP Content NAME length
*********************************************/
#define MAX_CONTENT_NAME_LEN		128

/*********************************************
* HTTP Timeout
*********************************************/
#define HTTP_MAX_TIMEOUT_SEC		3 // Sec.

typedef enum
{
	NONE,		///< Web storage none
	CODEFLASH,	///< Code flash memory
	SDCARD,		///< SD card
	DATAFLASH	///< External data flash memory
} StorageType;

struct st_http_info
{
	uint8_t  sock_cnt;
	uint8_t* sock_list;
	uint8_t* sendbuf;
	uint8_t* recvbuf;
	uint16_t port;
	uint8_t  storage_type;
};

typedef struct _st_http_socket
{
	uint8_t  status;
	uint8_t  file_name[MAX_CONTENT_NAME_LEN];
	uint32_t file_start;
	uint32_t file_len;
	uint32_t file_offset; // (start addr + sent size...)
} st_http_socket;

void reg_httpServer_cbfunc(void(*mcu_reset)(void), void(*wdt_reset)(void));

void httpServer_init(uint8_t * tx_buf, uint8_t * rx_buf, uint8_t sock_cnt, uint8_t * sock_list);
void httpServer_run(uint16_t server_port);

/*
 * @brief HTTP Server 1sec Tick Timer handler
 * @note SHOULD BE register to your system 1s Tick timer handler
 */
void httpServer_time_handler(void);
uint32_t get_httpServer_timecount(void);

#endif
