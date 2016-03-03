/**
 * @file	RESTapiHandler.c
 * @brief	HTTP Server - REST API handler
 * @version 1.0
 * @date	2016/03
 * @par Revision
 *			2016/03 - 1.0 Release
 * @author	
 * \n\n @par Copyright (C) 1998 - 2016 WIZnet. All rights reserved.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "wizchip_conf.h"
#include "common.h"
#include "ConfigData.h"
#include "W7500x_board.h"
#include "timerHandler.h"
#include "gpioHandler.h"

#include "RESTapiHandler.h"
#include "frozen.h" // Frozen: JSON parser and generator for C/C++

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
#define RESTAPI_STR_ID          "id"
#define RESTAPI_STR_TYPE        "type"       // analog or digital
#define RESTAPI_STR_DIR         "direction"  // input or output
#define RESTAPI_STR_ANALOG      "analog"
#define RESTAPI_STR_DIGITAL     "digital"
#define RESTAPI_STR_INPUT       "input"
#define RESTAPI_STR_OUTPUT      "output"
 
// HTTP CRUD functions
// Create / Read / Update / Delete
static int16_t restapi_read_index(char* buf);                // [GET] Index: list of resources
static int16_t restapi_read_uptime(char* buf);               // [GET] Device Uptime
static int16_t restapi_read_netinfo(char* buf);              // [GET] Network information; IP address, Gateway address, Subnet mask, DHCP enable...
static int16_t restapi_read_userio(char* buf);
static int16_t restapi_read_userio_id(char* buf);
static int16_t restapi_read_userio_info(char* buf);
static int16_t restapi_create_userio_id(char* buf);
static int16_t restapi_update_userio_id(char* buf);
static int16_t restapi_update_userio_info(char* buf);
static int16_t restapi_delete_userio_id(char* buf);
static int8_t find_matched_userio_id(uint8_t * req_id);
	
const struct st_http_resource uri_table[] = 
{
	{ HTTP_REQ_METHOD_GET,    "index",           restapi_read_index,          "index page"  },
	{ HTTP_REQ_METHOD_GET,    "uptime",          restapi_read_uptime,         "uptime" },
	{ HTTP_REQ_METHOD_GET,    "netinfo",         restapi_read_netinfo,        "network configration" },
	{ HTTP_REQ_METHOD_GET,    "userio",          restapi_read_userio,         "enabled io list"},
	{ HTTP_REQ_METHOD_GET,    "userio/:id",      restapi_read_userio_id,      "get io status or value"},
	{ HTTP_REQ_METHOD_POST,   "userio/:id",      restapi_create_userio_id,    "enable new io pin" },
	{ HTTP_REQ_METHOD_PUT,    "userio/:id",      restapi_update_userio_id,    "set the io status (digital output only)" },
	{ HTTP_REQ_METHOD_DELETE, "userio/:id",      restapi_delete_userio_id,    "disable the io pin" },
	{ HTTP_REQ_METHOD_GET,    "userio/:id/info", restapi_read_userio_info,    "get the io configuration, type and direction"},
	{ HTTP_REQ_METHOD_PUT,    "userio/:id/info", restapi_update_userio_info,  "set the io configuration, type and direction"},
	
	{ NULL, NULL, NULL, NULL } // Last item should be set to NULL
};

uint8_t req_resource_ID[MAX_RESOURCE_ID_SIZE];

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/
//uri_table: method / uri / process
int8_t search_http_resources(uint8_t method, uint8_t * uri)
{
	int8_t ret = RESTAPI_ERROR_RESOURCE_NOT_FOUND;
	uint8_t i, j;
	
	char * tok_ptr;
	uint8_t uri_matched = 0;
	uint8_t uri_matched_count = 0;
	uint8_t req_uri_depth = 0;
	
	// Requested URI buf and token ptr
	char uri_buf[MAX_URI_SIZE/2];
	char * uri_tok[MAX_URI_DEPTH];
	
	// server resource string buf
	char resource_buf[MAX_URI_SIZE/2];
	
	// Parse the requested URI
	strcpy(uri_buf, (char *)uri);
	tok_ptr = strtok(uri_buf, "/");
	uri_tok[0] = tok_ptr;
	
	if(uri_tok[0] == NULL)
	{
#ifdef _RESTAPI_DEBUG_
		printf("  [Error] URI path NULL\r\n");
#endif
		return RESTAPI_ERROR_RESOURCE_NOT_FOUND; // Parse failed
	}
	
	req_uri_depth++;
	
	for(i = 1; (tok_ptr = strtok(NULL, "/")) != NULL; i++)
	{
		if(i >= MAX_URI_DEPTH)
		{
#ifdef _RESTAPI_DEBUG_
			printf("  [Error] URI path depth exceeded [max: %d]\r\n", MAX_URI_DEPTH);
#endif
			req_uri_depth = 0;
			return RESTAPI_ERROR_RESOURCE_NOT_FOUND; // Parse failed
		}
		uri_tok[i] = tok_ptr;
		req_uri_depth++;
	}
	
	
#ifdef _RESTAPI_DEBUG_
	printf("  [Debug] req_uri_depth: %d\r\n", req_uri_depth);
	for(i = 0; i < MAX_URI_DEPTH; i++)
	{
		if(uri_tok[i] != NULL) printf("  [Debug] uri_tok[%d]: %s\r\n", i, uri_tok[i]);
		else printf("  [Debug] uri_tok[%d]: %s\r\n", i, "NULL");
	}
#endif
	
	// Find the requested resource
	for(i = 0; uri_table[i].method != NULL; i++)
	{
		// Parse the registered resource string
		strcpy(resource_buf, (char *)uri_table[i].uri);
		tok_ptr = strtok(resource_buf, "/");
		
		if(strcmp(uri_tok[0], tok_ptr) == 0) // token matched
		{
			uri_matched_count++;
			for(j = 1; ((tok_ptr = strtok(NULL, "/")) != NULL); j++)
			{
				if((uri_tok[j] != NULL) && (strcmp(RESOURCE_ID_MARK, tok_ptr) == 0))
				{
					// Token replacement: When the resource token is the ":ID"
					if(strlen(uri_tok[j]) <= MAX_RESOURCE_ID_SIZE)
					{
						strcpy((char *)req_resource_ID, uri_tok[j]);
						tok_ptr = uri_tok[j];
					}
				}
				
				if((j >= req_uri_depth) || (strcmp(uri_tok[j], tok_ptr) != 0))
				{
					// failed: URI depth or URI token unmatched
					uri_matched_count = 0;
					break;
				}
				else // URI matches succeeded in depth
				{
					uri_matched_count++;
				}
			}
		}
		
		// string compare
		if(req_uri_depth == uri_matched_count)
		{
			uri_matched = 1;
			if(uri_table[i].method == method)
			{
#ifdef _RESTAPI_DEBUG_
				printf("  [Debug] Requested URI - resource table num %d matched\r\n", i);
#endif
				ret = i;
				break;
			}
		}
		
		uri_matched_count = 0;
	}
	
	if((uri_matched == 1) && (ret < 0)) ret = RESTAPI_ERROR_METHOD_NOT_ALLOWED;
	
	return ret;
}

int16_t http_resources_handler(st_http_request * p_http_request, uint8_t * buf, uint8_t table_num, uint16_t http_status)
{
	uint16_t len = 0;
	
	// Todo: PUT update
	/*
	uint8_t* ptr = NULL;
	
	if(p_http_request->METHOD == HTTP_REQ_METHOD_PUT)
	{
		printf("  [Debug] HTTP PUT Request: body len = %d\r\n", p_http_request->BODY_LEN);
		printf("  [Debug] HTTP PUT Request: body\r\n");
		printf("%s\r\n", p_http_request->BODY);
	
		len = uri_table[table_num].process((char* )buf);
	}
	*/
	
	len = uri_table[table_num].process((char* )buf);
	
	return len;
}

int16_t make_http_response_error_message(uint8_t* buf, uint16_t http_status)
{
	uint8_t i;
	int16_t len = 0;
	char * buf_ptr = (char *)buf;
	const char * str_ptr = NULL;
	
	// Find the HTTP status code
	for(i = 0; code_table[i].code != NULL; i++)
	{
		if(http_status == code_table[i].code)
		{
			str_ptr = code_table[i].code_str;
			str_ptr += 4;
			
			len = json_emit(buf_ptr, DATA_BUF_SIZE, "{ s: ", "error");
			len += json_emit(buf_ptr+len, DATA_BUF_SIZE, "{ s: s, s: i }", "message", str_ptr, "code", http_status);
			len += json_emit(buf_ptr+len, DATA_BUF_SIZE, " }");
			
			break;
		}
	}
	
	return len;
}	

/*****************************************************************************
 * Private functions
 ****************************************************************************/
static int16_t restapi_read_index(char* buf)
{
	wiz_NetInfo gWIZNETINFO;
	
	uint8_t i, j;
	uint16_t len;
	char str_buf[50];
	
	ctlnetwork(CN_GET_NETINFO, (void*) &gWIZNETINFO);
	
	// Target board
	len = json_emit(buf, DATA_BUF_SIZE, "{ s: s,", "target", "wizwiki-7500eco");
	
	// Supported (defined) I/O list
	len += json_emit(buf+len, DATA_BUF_SIZE, "s: [", "io");
	for(i = 0; i < USER_IOn; i++)
	{
		len += json_emit(buf+len, DATA_BUF_SIZE, "{ s: s, s: s },", RESTAPI_STR_ID, USER_IO_STR[i], "pin", USER_IO_PIN_STR[i]);
	}
	len += json_emit(buf+len-1, DATA_BUF_SIZE, " ], ");
	len--;
	
	// Supported URI (resource) list
	len += json_emit(buf+len, DATA_BUF_SIZE, "s: [", "resource");
	
	for(i = 0; uri_table[i].method != NULL; i++)
	{
		for(j = 0; method_table[j].method != NULL; j++)
		{
			if(method_table[j].method == uri_table[i].method) break;
		}
		
		sprintf(str_buf, "http://%d.%d.%d.%d/%s", gWIZNETINFO.ip[0], gWIZNETINFO.ip[1], gWIZNETINFO.ip[2], gWIZNETINFO.ip[3], uri_table[i].uri);
		len += json_emit(buf+len, DATA_BUF_SIZE, "{ s: s, s: s, s: s },", "uri", str_buf, "method", method_table[j].method_str, "description", uri_table[i].description);
	}
	
	len += json_emit(buf+len-1, DATA_BUF_SIZE, " ] }");
	len--;
	
	return len;
}

static int16_t restapi_read_uptime(char* buf)
{
	uint16_t len;
	
	len = json_emit(buf, DATA_BUF_SIZE, "{ s: ", "uptime");
	len += json_emit(buf+len, DATA_BUF_SIZE, "{ s: i, s: i, s: i, s: i }", "hour", getDeviceUptime_hour(), "min", getDeviceUptime_min(), "sec", getDeviceUptime_sec(), "msec", getDeviceUptime_msec());
	len += json_emit(buf+len, DATA_BUF_SIZE, " }");
	
	return len;
}

static int16_t restapi_read_netinfo(char* buf)
{
	wiz_NetInfo gWIZNETINFO;
	
	uint16_t len = 0;
	char str_buf[18] = {0, };
	
	ctlnetwork(CN_GET_NETINFO, (void*) &gWIZNETINFO);

	len = json_emit(buf, DATA_BUF_SIZE, "{ s: { ", "netinfo");
		sprintf(str_buf, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", gWIZNETINFO.mac[0], gWIZNETINFO.mac[1], gWIZNETINFO.mac[2], gWIZNETINFO.mac[3], gWIZNETINFO.mac[4], gWIZNETINFO.mac[5]);
	len += json_emit(buf+len, DATA_BUF_SIZE, "s: s,", "mac", str_buf);
		sprintf(str_buf, "%d.%d.%d.%d", gWIZNETINFO.ip[0], gWIZNETINFO.ip[1], gWIZNETINFO.ip[2], gWIZNETINFO.ip[3]);
	len += json_emit(buf+len, DATA_BUF_SIZE, "s: s,", "ip", str_buf);
		sprintf(str_buf, "%d.%d.%d.%d", gWIZNETINFO.gw[0], gWIZNETINFO.gw[1], gWIZNETINFO.gw[2], gWIZNETINFO.gw[3]);
	len += json_emit(buf+len, DATA_BUF_SIZE, "s: s,", "gw", str_buf);
		sprintf(str_buf, "%d.%d.%d.%d", gWIZNETINFO.sn[0], gWIZNETINFO.sn[1], gWIZNETINFO.sn[2], gWIZNETINFO.sn[3]);
	len += json_emit(buf+len, DATA_BUF_SIZE, "s: s,", "sn", str_buf);
		sprintf(str_buf, "%d.%d.%d.%d", gWIZNETINFO.dns[0], gWIZNETINFO.dns[1], gWIZNETINFO.dns[2], gWIZNETINFO.dns[3]);
	len += json_emit(buf+len, DATA_BUF_SIZE, "s: s,", "dns", str_buf);
	len += json_emit(buf+len, DATA_BUF_SIZE, "s: s", "dhcp", (gWIZNETINFO.dhcp == NETINFO_DHCP)?"enabled":"disabled");
	len += json_emit(buf+len, DATA_BUF_SIZE, " } }");
	
	return len;
}

static int16_t restapi_read_userio(char* buf)
{
	struct __user_io_info *user_io_info = (struct __user_io_info *)&(get_DevConfig_pointer()->user_io_info);
	
	uint8_t i;
	uint16_t len = 0;
	char* type; 
	char* dir;
	
	if(user_io_info->user_io_enable == 0)
	{
		len = json_emit(buf, DATA_BUF_SIZE, "{ s: s }", "userio", "NULL");
	}
	else
	{
		len = json_emit(buf, DATA_BUF_SIZE, "{ s: [", "userio");
		
		for(i = 0; i < USER_IOn; i++)
		{
			if(get_user_io_enabled(USER_IO_SEL[i]) == IO_ENABLE)
			{
				if(get_user_io_type(USER_IO_SEL[i]) == IO_ANALOG_IN) 
					type = RESTAPI_STR_ANALOG;
				else
					type = RESTAPI_STR_DIGITAL;
				
				if(get_user_io_direction(USER_IO_SEL[i]) == IO_OUTPUT)
					dir = RESTAPI_STR_OUTPUT;
				else
					dir = RESTAPI_STR_INPUT;
				
				len += json_emit(buf+len, DATA_BUF_SIZE, "{ s: s, s: s, s: s },", RESTAPI_STR_ID, USER_IO_STR[i], RESTAPI_STR_TYPE, type, RESTAPI_STR_DIR, dir);
			}
		}
		len += json_emit(buf+len-1, DATA_BUF_SIZE, " ] }");
		len--; // Remove the last comma.
	}
	return len;
}

static int16_t restapi_read_userio_id(char* buf)
{
	int8_t id_num;
	uint16_t len = 0;
	uint16_t val = 0;
	
	id_num = find_matched_userio_id(req_resource_ID);
	
	if(get_user_io_enabled(USER_IO_SEL[id_num]) == IO_DISABLE)
	{
		return RESTAPI_ERROR_RESOURCE_NOT_FOUND;
	}
	
	if(id_num >= 0)
	{
		// IO control: Read the io status (digital) or value (analog)
		get_user_io_val(USER_IO_SEL[id_num], &val);
		
		// Generating JSON string
		len = json_emit(buf, DATA_BUF_SIZE, "{ s: i }", req_resource_ID, val);
	}
	else
	{
		return RESTAPI_ERROR_RESOURCE_NOT_FOUND;
	}
	
	return len;
}

static int16_t restapi_read_userio_info(char* buf)
{
	int8_t id_num;
	uint16_t len = 0;
	char* type; 
	char* dir;
	
	id_num = find_matched_userio_id(req_resource_ID);
	
	if(get_user_io_enabled(USER_IO_SEL[id_num]) == IO_DISABLE)
	{
		return RESTAPI_ERROR_RESOURCE_NOT_FOUND;
	}
	
	if(id_num >= 0)
	{
		if(get_user_io_type(USER_IO_SEL[id_num]) == IO_ANALOG_IN) 
		{
			type = RESTAPI_STR_ANALOG;
		}
		else
		{
			type = RESTAPI_STR_DIGITAL;
		}
		
		if(get_user_io_direction(USER_IO_SEL[id_num]) == IO_OUTPUT)
		{
			dir = RESTAPI_STR_OUTPUT;
		}
		else
		{
			dir = RESTAPI_STR_INPUT;
		}
		
		len = json_emit(buf, DATA_BUF_SIZE, "{ s: s, s: s, s: s },", RESTAPI_STR_ID, USER_IO_STR[id_num], RESTAPI_STR_TYPE, type, RESTAPI_STR_DIR, dir);
	}
	else
	{
		return RESTAPI_ERROR_RESOURCE_NOT_FOUND;
	}
	
	return len;
}

static int16_t restapi_create_userio_id(char* buf)
{
	int8_t id_num;
	
	id_num = find_matched_userio_id(req_resource_ID);
	
	if(get_user_io_enabled(USER_IO_SEL[id_num]) == IO_ENABLE)
	{
		return RESTAPI_ERROR_CONFLICT;
	}
	
	if(id_num >= 0)
	{
		set_user_io_enable(USER_IO_SEL[id_num], ENABLE);
		return RESTAPI_RET_CREATED;
	}
	return RESTAPI_ERROR_RESOURCE_NOT_FOUND;
}

// IO on/off settings
static int16_t restapi_update_userio_id(char* buf)
{
	uint16_t len = 0;
	
	// Todo: PUT update - IO status changes (Digital output only)
	// buf: HTTP request structure
	// return: len = 0 (204 No Content)
	
	return len;
}

// IO type/direction settings
static int16_t restapi_update_userio_info(char* buf)
{
	uint16_t len = 0;
	
	// Todo: PUT update - IO type and direction changes
	// buf: HTTP request structure
	// return: len = 0 (204 No Content)
	
	return len;
}


static int16_t restapi_delete_userio_id(char* buf)
{
	int8_t id_num;
	uint16_t len = 0;
	
	id_num = find_matched_userio_id(req_resource_ID);
	
	if(get_user_io_enabled(USER_IO_SEL[id_num]) == IO_DISABLE)
	{
		return RESTAPI_ERROR_RESOURCE_NOT_FOUND;
	}
	
	if(id_num >= 0)
	{
		set_user_io_enable(USER_IO_SEL[id_num], DISABLE);
		len = 0;
	}
	else
	{
		return RESTAPI_ERROR_RESOURCE_NOT_FOUND;
	}
	
	return len;
}

static int8_t find_matched_userio_id(uint8_t * req_id)
{
	int8_t ret = -1;
	uint8_t i;
	
	for(i = 0; i < USER_IOn; i++)
	{
		// Search the requested I/O pin
		if(strcmp((const char *)req_id, USER_IO_STR[i]) == 0)
		{
#ifdef _RESTAPI_DEBUG_
			printf("  [Debug] Request USER_IO : %s (num: %d, code: %.4x)\r\n", USER_IO_STR[i], i, USER_IO_SEL[i]);
#endif
			ret = i;
			break;
		}
	}
	return ret;
}


