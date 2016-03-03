#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "socket.h"
#include "wizchip_conf.h"

#include "common.h"
#include "httpServer_rest.h"
#include "httpParser_rest.h"
#include "RESTapiHandler.h"


#ifndef DATA_BUF_SIZE
	#define DATA_BUF_SIZE		2048
#endif

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
static uint8_t httpsock_num[_WIZCHIP_SOCK_NUM_] = {0, };
static st_http_request * http_request;				/**< Pointer to received HTTP request */
static st_http_request * parsed_http_request;		/**< Pointer to parsed HTTP request */
static uint8_t http_req[MAX_URI_SIZE+16];			/**< Pointer to parsed HTTP request */

static uint8_t * http_response;						/**< Pointer to HTTP response header*/
static uint8_t * http_response_body;				/**< Pointer to HTTP response body*/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/
struct st_http_info httpserver;
st_http_socket HTTPSock[_WIZCHIP_SOCK_NUM_] = { {STATE_HTTP_IDLE, }, };

volatile uint32_t httpServer_tick_1s = 0;

/*****************************************************************************
 * Private functions
 ****************************************************************************/
static void http_process_handler(uint8_t sock, st_http_request * p_http_request);
static void send_http_response_header(uint8_t sock, uint8_t * buf, uint8_t content_type, uint32_t body_len, uint16_t http_status);
static void send_http_response_body(uint8_t sock, uint8_t * buf, uint16_t content_len);
//static void send_http_response_header(uint8_t sock, uint8_t content_type, uint32_t body_len, uint16_t http_status);
//static void send_http_response_body(uint8_t sock, uint8_t * uri_name, uint8_t * buf, uint32_t start_addr, uint32_t file_len);

static int8_t  getAvailableHTTPSocketNum(void);
static uint8_t getHTTPSocketNum(uint8_t seqnum);
static int8_t  getHTTPSequenceNum(uint8_t sock);
static int8_t  http_disconnect(uint8_t sock);

/*****************************************************************************
 * Public functions
 ****************************************************************************/
// Callback functions definition: MCU Reset / WDT Reset
void default_mcu_reset(void) {;}
void default_wdt_reset(void) {;}
void (*HTTPServer_ReStart)(void) = default_mcu_reset;
void (*HTTPServer_WDT_Reset)(void) = default_wdt_reset;


/* Register the call back functions for HTTP Server */
void reg_httpServer_cbfunc(void(*mcu_reset)(void), void(*wdt_reset)(void))
{
	// Callback: HW Reset and WDT reset function for each MCU platforms
	if(mcu_reset) HTTPServer_ReStart = mcu_reset;
	if(wdt_reset) HTTPServer_WDT_Reset = wdt_reset;
}
	

/* HTTP Server Initialize */
void httpServer_init(uint8_t * tx_buf, uint8_t * rx_buf, uint8_t sock_cnt, uint8_t * sock_list)
{
	uint8_t i;
	
	// Socket initialize: HW socket for HTTP server
	if(sock_cnt > _WIZCHIP_SOCK_NUM_)
		httpserver.sock_cnt = _WIZCHIP_SOCK_NUM_;
	else 
		httpserver.sock_cnt = sock_cnt;
	
	httpserver.sock_list = sock_list;
	
	// User's shared buffer
	httpserver.sendbuf = tx_buf;
	httpserver.recvbuf = rx_buf;
	
	// H/W Socket number mapping
	for(i = 0; i < sock_cnt; i++)
	{
		// Mapping the H/W socket numbers to the sequential index numbers
		httpsock_num[i] = sock_list[i];
	}
}

/* HTTP Server Run */
void httpServer_run(uint16_t server_port)
{
	int8_t sock; 			// HW socket number
	uint8_t sock_status;	// HW socket status
	int8_t seqnum; 			// Sequence number
	int16_t len;
	
#ifdef _HTTPSERVER_DEBUG_
	uint8_t destip[4] = {0, };	// Destination IP address
	uint16_t destport = 0;		// Destination Port number
#endif
	
	sock = getAvailableHTTPSocketNum(); // Get the H/W socket number
	if(sock < 0) return; // HW socket allocation failed
	
	seqnum = getHTTPSequenceNum(sock);
	
	http_request = (st_http_request *)httpserver.recvbuf;		// HTTP Request Structure
	parsed_http_request = (st_http_request *)http_req;
	//parsed_http_request = (st_http_request *)httpserver.sendbuf; // old
	
	/* Web Service Start */
	sock_status = getSn_SR(sock);
	switch(sock_status)
	{
		case SOCK_ESTABLISHED:
			// Interrupt clear
			if(getSn_IR(sock) & Sn_IR_CON)
			{
				setSn_IR(sock, Sn_IR_CON);
			}

			// HTTP Process states
			switch(HTTPSock[seqnum].status)
			{
				case STATE_HTTP_IDLE :
					if ((len = getSn_RX_RSR(sock)) > 0)
					{
						if (len > DATA_BUF_SIZE) len = DATA_BUF_SIZE;
						if ((len = recv(sock, (uint8_t *)http_request, len)) < 0) break;	// Exception handler
						
						*(((uint8_t *)http_request) + len) = '\0';	// End of string (EOS) marker
						
						parse_http_request(parsed_http_request, (uint8_t *)http_request);

#ifdef _HTTPSERVER_DEBUG_
						printf("> HTTP Request START ==========\r\n");
						printf("%s", (uint8_t *)http_request);
						printf("> HTTP Request END ============\r\n");
#endif
						
#ifdef _HTTPSERVER_DEBUG_
						getSn_DIPR(sock, destip);
						destport = getSn_DPORT(sock);
						printf("\r\n");
						printf("> HTTPSocket[%d] : HTTP Request received ", sock);
						printf("from %d.%d.%d.%d : %d\r\n", destip[0], destip[1], destip[2], destip[3], destport);
#endif

#ifdef _HTTPSERVER_DEBUG_
						printf("> HTTPSocket[%d] : [State] STATE_HTTP_REQ_DONE\r\n", sock);
#endif
						// HTTP 'response' handler; includes send_http_response_header / body function
						http_process_handler(sock, parsed_http_request);

						if(HTTPSock[seqnum].file_len > 0) HTTPSock[seqnum].status = STATE_HTTP_RES_INPROC;
						else HTTPSock[seqnum].status = STATE_HTTP_RES_DONE; // Send the 'HTTP response' end
						
					}
					break;


				case STATE_HTTP_RES_INPROC :
					/* Repeat: Send the remain parts of HTTP responses */
#ifdef _HTTPSERVER_DEBUG_
					printf("> HTTPSocket[%d] : [State] STATE_HTTP_RES_INPROC\r\n", sock);
#endif
					// Repeatedly send remaining data to client
					send_http_response_body(sock, http_response, 0);

					if(HTTPSock[seqnum].file_len == 0) HTTPSock[seqnum].status = STATE_HTTP_RES_DONE;
					break;

				case STATE_HTTP_RES_DONE :
#ifdef _HTTPSERVER_DEBUG_
					printf("> HTTPSocket[%d] : [State] STATE_HTTP_RES_DONE\r\n", sock);
#endif
					// Socket file info structure re-initialize
					HTTPSock[seqnum].file_len = 0;
					HTTPSock[seqnum].file_offset = 0;
					HTTPSock[seqnum].file_start = 0;
					HTTPSock[seqnum].status = STATE_HTTP_IDLE;
					
#ifdef _USE_WATCHDOG_
					HTTPServer_WDT_Reset();
#endif
					http_disconnect(sock);
					break;

				default :
					break;
			}
			break;

		case SOCK_CLOSE_WAIT:
#ifdef _HTTPSERVER_DEBUG_
			printf("> HTTPSocket[%d] : ClOSE_WAIT\r\n", sock);	// if a peer requests to close the current connection
#endif
			// Socket file info structure re-initialize: HTTP connection 'close'
			HTTPSock[seqnum].file_len = 0;
			HTTPSock[seqnum].file_offset = 0;
			HTTPSock[seqnum].file_start = 0;
			HTTPSock[seqnum].status = STATE_HTTP_IDLE;
			
			http_disconnect(sock);
			break;

		case SOCK_INIT:
			listen(sock);
			break;

		case SOCK_LISTEN:
			break;

		case SOCK_SYNSENT:
		//case SOCK_SYNSENT_M:
		case SOCK_SYNRECV:
		//case SOCK_SYNRECV_M:
			break;

		case SOCK_CLOSED:
#ifdef _HTTPSERVER_DEBUG_
			//printf("> HTTPSocket[%d] : CLOSED\r\n", sock);
#endif
			if(server_port == 0) server_port = HTTP_SERVER_PORT;
			if(socket(sock, Sn_MR_TCP, server_port, 0x00) == sock) // Init / Reinitialize the socket
			{
#ifdef _HTTPSERVER_DEBUG_
				printf("> HTTPSocket[%d] : SERVER OPEN, Port: %d\r\n", sock, server_port);
#endif
				;
			}
			break;

		default :
			break;
	} // end of switch

#ifdef _USE_WATCHDOG_
	HTTPServer_WDT_Reset();
#endif
}


////////////////////////////////////////////
// Private Functions
////////////////////////////////////////////

static void http_process_handler(uint8_t sock, st_http_request * p_http_request)
{
	uint8_t * uri_name;
	uint8_t uri_buf[MAX_URI_SIZE]={0x00, };
	int32_t content_len = 0;
	uint16_t status_code = 0;
	uint16_t content_type;
	int8_t table_num;
	
	//int8_t seq_num;
	//if((seq_num = getHTTPSequenceNum(sock)) == -1) return; // exception handling; invalid number
	
	http_response = httpserver.recvbuf;
	http_response_body = httpserver.sendbuf;
	
	// method Analyze
	switch (p_http_request->METHOD)
	{
		case HTTP_REQ_METHOD_HEAD :
		case HTTP_REQ_METHOD_GET :
			get_http_uri_name(p_http_request->URI, uri_buf);
			uri_name = uri_buf;
			if (!strcmp((char *)uri_name, "/")) strcpy((char *)uri_name, INITIAL_RESOURCE);	// If URI is "/", respond by index
			find_http_uri_type(&p_http_request->TYPE, uri_name);	// Checking requested file types (HTML, TEXT, GIF, JPEG and Etc. are included)
			break;

		case HTTP_REQ_METHOD_POST :
			mid((char *)p_http_request->URI, "/", " HTTP", (char *)uri_buf);
			uri_name = uri_buf;
			find_http_uri_type(&p_http_request->TYPE, uri_name); // Check file type
			break;
		
		case HTTP_REQ_METHOD_PUT :
			mid((char *)p_http_request->URI, "/", " HTTP", (char *)uri_buf);
			uri_name = uri_buf;
			find_http_uri_type(&p_http_request->TYPE, uri_name); // Check file type
			
			break;
		
		case HTTP_REQ_METHOD_DELETE :
			mid((char *)p_http_request->URI, "/", " HTTP", (char *)uri_buf);
			uri_name = uri_buf;
			find_http_uri_type(&p_http_request->TYPE, uri_name); // Check file type
			
			break;
		
		case HTTP_REQ_METHOD_ERR :
		default :
			status_code = HTTP_RES_CODE_NOT_IMPLE;
			break;
	}
	
#ifdef _HTTPSERVER_DEBUG_
	printf("\r\n> HTTPSocket[%d] : HTTP Method = %.4x\r\n", sock, p_http_request->METHOD);
	printf("> HTTPSocket[%d] : Request Type = %d\r\n", sock, p_http_request->TYPE);
	printf("> HTTPSocket[%d] : Request URI = %s\r\n", sock, uri_name);
#endif
	
	if(p_http_request->TYPE == 0) // REST API request or Requested file type not found
	{
		table_num = search_http_resources(p_http_request->METHOD, uri_name); // get the resource table number
		
		if(table_num < 0) // HTTP resource search failed
		{
			//content_type = HTTP_RES_TYPE_TEXT;
			content_type = HTTP_RES_TYPE_JSON;
			
			if(table_num == RESTAPI_ERROR_RESOURCE_NOT_FOUND) status_code = HTTP_RES_CODE_NOT_FOUND;	// uri unmatched
			if(table_num == RESTAPI_ERROR_METHOD_NOT_ALLOWED) status_code = HTTP_RES_CODE_NOT_ALLOWED; 	// uri matched but not supported method
			else status_code = HTTP_RES_CODE_NOT_FOUND;
		}
		else // HTTP resource search success
		{
			// REST API function handler
			// If necessary, generating JSON object of HTTP response body and copy the object to send buffer(http_response_body)
			content_len = http_resources_handler(p_http_request, http_response_body, table_num, status_code);
			
			// content_len variable: content body length or API handling results (e.g., http error)
			if(content_len == 1)
			{
				content_type = HTTP_RES_TYPE_JSON;
				status_code = HTTP_RES_CODE_CREATED;
				content_len = 0;
			}
			else if(content_len > 0)
			{
				content_type = HTTP_RES_TYPE_JSON;
				status_code = HTTP_RES_CODE_OK;
			}
			else if(content_len == 0)
			{
				content_type = HTTP_RES_TYPE_JSON;
				status_code = HTTP_RES_CODE_NO_CONTENT;
			}
			else if(content_len == RESTAPI_ERROR_CONFLICT)
			{
				content_type = HTTP_RES_TYPE_JSON;
				status_code = HTTP_RES_CODE_CONFLICT;
			}
			else
			{
				content_type = HTTP_RES_TYPE_JSON;
				status_code = HTTP_RES_CODE_NOT_FOUND;
			}
			
		}
	}
	else
	{
		// GET method: find requested file in storage (not implemented), //e.g., if(search_http_storage(...))...
		// If the size of the requested file is larger than the buffer size, use the file_len / file_offset field in HTTPSock statuc
		
		// Other methods: return 'content not found'
		content_type = HTTP_RES_TYPE_JSON;
		status_code = HTTP_RES_CODE_NOT_FOUND;
	}
	
	// HTTP response error codes; 4xx or 5xx
	// Generate the JSON body {"message": "xxxxxx", "code": xxx}
	if(((status_code & HTTP_RES_CODE_BAD_REQUEST) == HTTP_RES_CODE_BAD_REQUEST) || ((status_code & HTTP_RES_CODE_INT_SERVER) == HTTP_RES_CODE_INT_SERVER))
	{
		// Generating JSON object of HTTP Error messages
		// e.g., {"errors":{ "error" : { "message":"Method not allowed", "code":404 } }
		content_len = make_http_response_error_message(http_response_body, status_code);
	}
	
	// Generate and Send the HTTP response 'header'
	send_http_response_header(sock, http_response, content_type, content_len, status_code);
	
	// If necessary, Send the HTTP response 'body'
	if(p_http_request->METHOD != HTTP_REQ_METHOD_HEAD)
	{
		if(content_len > 0) send_http_response_body(sock, http_response_body, content_len);
	}
}


static void send_http_response_header(uint8_t sock, uint8_t * buf, uint8_t content_type, uint32_t body_len, uint16_t http_status)
{
	make_http_response_header((char*)http_response, content_type, body_len, http_status);
	send(sock, http_response, strlen((char *)http_response));
	
#ifdef _HTTPSERVER_DEBUG_
	printf("> HTTPSocket[%d] : [Send] HTTP Response Header [ %d ]byte\r\n", sock, (uint16_t)strlen((char *)http_response));
	printf("> HTTPSocket[%d] : [Send] HTTP Response Header\r\n %s", sock, (char *)http_response);
#endif
}

//static void send_http_response_body(uint8_t sock, uint8_t * uri_name, uint8_t * buf, uint32_t start_addr, uint32_t file_len)
static void send_http_response_body(uint8_t sock, uint8_t * buf, uint16_t content_len)
{
	send(sock, buf, content_len);
	
/*
	int8_t get_seqnum;
	uint32_t send_len;

	uint8_t flag_datasend_end = 0;

#ifdef _USE_SDCARD_
	uint16_t blocklen;
#else
	uint32_t addr = 0;
#endif

	if((get_seqnum = getHTTPSequenceNum(s)) == -1) return; // exception handling; invalid number

	// Send the HTTP Response 'body'; requested file
	if(!HTTPSock[get_seqnum].file_len) // ### Send HTTP response body: First part ###
	{
		if (file_len > DATA_BUF_SIZE - 1)
		{
			HTTPSock[get_seqnum].file_start = start_addr;
			HTTPSock[get_seqnum].file_len = file_len;
			send_len = DATA_BUF_SIZE - 1;

/////////////////////////////////////////////////////////////////////////////////////////////////
// ## 20141219 Eric added, for 'File object structure' (fs) allocation reduced (8 -> 1)
			memset(HTTPSock[get_seqnum].file_name, 0x00, MAX_CONTENT_NAME_LEN);
			strcpy((char *)HTTPSock[get_seqnum].file_name, (char *)uri_name);
#ifdef _HTTPSERVER_DEBUG_
			printf("> HTTPSocket[%d] : HTTP Response body - file name [ %s ]\r\n", s, HTTPSock[get_seqnum].file_name);
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _HTTPSERVER_DEBUG_
			printf("> HTTPSocket[%d] : HTTP Response body - file len [ %ld ]byte\r\n", s, file_len);
#endif
		}
		else
		{
			// Send process end
			send_len = file_len;

#ifdef _HTTPSERVER_DEBUG_
			printf("> HTTPSocket[%d] : HTTP Response end - file len [ %ld ]byte\r\n", s, send_len);
#endif
		}
#ifdef _USE_FLASH_
		addr = start_addr;
#endif
	}
	else // ### Send HTTP response body: Remained parts ###
	{
#ifdef _USE_FLASH_
		addr = HTTPSock[get_seqnum].file_start + HTTPSock[get_seqnum].file_offset;
#endif
		send_len = HTTPSock[get_seqnum].file_len - HTTPSock[get_seqnum].file_offset;

		if(send_len > DATA_BUF_SIZE - 1)
		{
			send_len = DATA_BUF_SIZE - 1;
			//HTTPSock[get_seqnum].file_offset += send_len;
		}
		else
		{
#ifdef _HTTPSERVER_DEBUG_
			printf("> HTTPSocket[%d] : HTTP Response end - file len [ %ld ]byte\r\n", s, HTTPSock[get_seqnum].file_len);
#endif
			// Send process end
			flag_datasend_end = 1;
		}
#ifdef _HTTPSERVER_DEBUG_
			printf("> HTTPSocket[%d] : HTTP Response body - send len [ %ld ]byte\r\n", s, send_len);
#endif

// ## 20141219 Eric added, for 'File object structure' (fs) allocation reduced (8 -> 1)
#ifdef _USE_SDCARD_
			if((fr = f_open(&fs, (const char *)HTTPSock[get_seqnum].file_name, FA_READ)) == 0)
			{
				f_lseek(&fs, HTTPSock[get_seqnum].file_offset);
			}
			else
			{
				send_len = 0;
#ifdef _HTTPSERVER_DEBUG_
				printf("> HTTPSocket[%d] : [FatFs] Error code return: %d (File Open) / HTTP Send Failed - %s\r\n", s, fr, HTTPSock[get_seqnum].file_name);
#endif
			}
#endif
// ## 20141219 added end
	}

#ifdef _USE_SDCARD_
	// Data read from SD Card
	fr = f_read(&fs, &buf[0], send_len, (void *)&blocklen);
	if(fr != FR_OK)
	{
		send_len = 0;
#ifdef _HTTPSERVER_DEBUG_
		printf("> HTTPSocket[%d] : [FatFs] Error code return: %d (File Read) / HTTP Send Failed - %s\r\n", s, fr, HTTPSock[get_seqnum].file_name);
#endif
	}
	else
	{
		*(buf+send_len+1) = 0; // Insert '/0' for EOS marker (End of string)
	}
#else
	// Data read from external data flash memory
	read_from_flashbuf(addr, &buf[0], send_len);
	*(buf+send_len+1) = 0; // Insert '/0' for string operation
#endif

	// Requested content send to HTTP client
#ifdef _HTTPSERVER_DEBUG_
	printf("> HTTPSocket[%d] : [Send] HTTP Response body [ %ld ]byte\r\n", s, send_len);
#endif

	if(send_len) send(s, buf, send_len);
	else flag_datasend_end = 1;

	if(flag_datasend_end)
	{
		HTTPSock[get_seqnum].file_start = 0;
		HTTPSock[get_seqnum].file_len = 0;
		HTTPSock[get_seqnum].file_offset = 0;
		flag_datasend_end = 0;
	}
	else
	{
		HTTPSock[get_seqnum].file_offset += send_len;
#ifdef _HTTPSERVER_DEBUG_
		printf("> HTTPSocket[%d] : HTTP Response body - offset [ %ld ]\r\n", s, HTTPSock[get_seqnum].file_offset);
#endif
	}

// ## 20141219 Eric added, for 'File object structure' (fs) allocation reduced (8 -> 1)
#ifdef _USE_SDCARD_
	f_close(&fs);
#endif
// ## 20141219 added end
*/
}


static int8_t getAvailableHTTPSocketNum(void)
{
	int8_t sock;
	static uint8_t i = 0;
	
	if(httpserver.sock_cnt == 0) {return -1;} // Check the HTTP server init
	
	sock = getHTTPSocketNum(i++);
	if(httpserver.sock_cnt <= i) i = 0;
	
	return sock;
}


static uint8_t getHTTPSocketNum(uint8_t seqnum)
{
	// Return the 'H/W socket number' corresponding to the index number
	return httpsock_num[seqnum];
}


static int8_t getHTTPSequenceNum(uint8_t sock)
{
	uint8_t i;

	for(i = 0; i < _WIZCHIP_SOCK_NUM_; i++)
		if(httpsock_num[i] == sock) return i;

	return -1;
}


static int8_t http_disconnect(uint8_t sock)
{
	setSn_CR(sock,Sn_CR_DISCON);
	/* wait to process the command... */
	while(getSn_CR(sock));

	return SOCK_OK;
}


void httpServer_time_handler(void)
{
	httpServer_tick_1s++;
}


uint32_t get_httpServer_timecount(void)
{
	return httpServer_tick_1s;
}
