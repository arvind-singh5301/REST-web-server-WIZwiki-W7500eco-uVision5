/**
 * @file	RESTapiHandler.h
 * @brief	Header File for HTTP Server - REST API Handler
 * @version 1.0
 * @date	2016/03
 * @par Revision
 *			2016/03 - 1.0 Release
 * @author	
 * \n\n @par Copyright (C) 1998 - 2016 WIZnet. All rights reserved.
 */

#ifndef	__RESTAPIHANDLER_H__
#define	__RESTAPIHANDLER__

#include <stdint.h>
#include "httpParser_rest.h"

//#define _RESTAPI_DEBUG_

#define MAX_URI_DEPTH           4
#define MAX_RESOURCE_ID_SIZE    20
#define RESOURCE_ID_MARK        ":id"

#define RESTAPI_RET_CREATED                     1
#define RESTAPI_ERROR                           0
#define RESTAPI_ERROR_RESOURCE_NOT_FOUND        (RESTAPI_ERROR - 1)
#define RESTAPI_ERROR_METHOD_NOT_ALLOWED        (RESTAPI_ERROR - 2)
#define RESTAPI_ERROR_CONFLICT                  (RESTAPI_ERROR - 3)


//{ method, uri, function }
struct st_http_resource
{
	uint8_t method;
	const char* uri;
	int16_t (*process)(char*);
	const char* description;
};

int8_t search_http_resources(uint8_t method, uint8_t * uri);
int16_t http_resources_handler(st_http_request * p_http_request, uint8_t * buf, uint8_t table_num, uint16_t http_status);
int16_t make_http_response_error_message(uint8_t* buf, uint16_t http_status);

#endif
