/**
 @file		httpParser_rest.h
 @brief 	Define Constants and fucntions associated with HTTP protocol.
 */

#include <stdint.h>

#ifndef	__HTTPPARSER_H__
#define	__HTTPPARSER_H__

//#define _HTTPPARSER_DEBUG_

#define HTTP_SERVER_PORT		80		/**< HTTP server well-known port number */

/* HTTP Methods */
#define HTTP_REQ_METHOD_ERR       0x00                   /**< Method Error   */
#define HTTP_REQ_METHOD_GET       (uint8_t)(0x01 <<  0)  /**< GET Method     */
#define HTTP_REQ_METHOD_HEAD      (uint8_t)(0x01 <<  1)  /**< HEAD Method    */
#define HTTP_REQ_METHOD_POST      (uint8_t)(0x01 <<  2)  /**< POST Method    */
#define HTTP_REQ_METHOD_PUT       (uint8_t)(0x01 <<  3)  /**< PUT Method     */
#define HTTP_REQ_METHOD_DELETE    (uint8_t)(0x01 <<  4)  /**< DELETE Method  */
// These methods are not implemented; 'OPTION', 'TRACE', 'CONNECT'

// Method names are case-sensitive, and all registered methods are all upper-case.
#define HTTP_REQ_STR_GET          "GET"
#define HTTP_REQ_STR_HEAD         "HEAD"
#define HTTP_REQ_STR_POST         "POST"
#define HTTP_REQ_STR_PUT          "PUT"
#define HTTP_REQ_STR_DELETE       "DELETE"

/* HTTP Version */
#define HTTP_VERSION_STR          "HTTP/1.1"

/* HTTP Status Codes */
#define HTTP_RES_CODE_OK          200    // The request has succeeded (OK + entity, GET/HEAD/POST, entity containing the requested resource(GET) or the result of the action(POST))
#define HTTP_RES_CODE_CREATED     201    // The request has been fulfilled and resulted in a new resource being created (POST-resource create- OK)
#define HTTP_RES_CODE_NO_CONTENT  204    // The server has fulfilled the request but does not need to return an entity-body, and might want to return updated metainformation (OK + no entity)
#define HTTP_RES_CODE_BAD_REQUEST 400    // The request could not be understood by the server due to malformed syntax
#define HTTP_RES_CODE_FORBIDDEN   403    // The server understood the request, but is refusing to fulfill it
#define HTTP_RES_CODE_NOT_FOUND   404    // The server has not found anything matching the Request-URI
#define HTTP_RES_CODE_NOT_ALLOWED 405    // The method specified in the Request-Line is not allowed for the resource identified by the Request-URI
#define HTTP_RES_CODE_CONFLICT    409    // The request could not be completed due to a conflict with the current state of the resource
#define HTTP_RES_CODE_INT_SERVER  500    // The server encountered an unexpected condition which prevented it from fulfilling the request
#define HTTP_RES_CODE_NOT_IMPLE   501    // The server does not support the functionality required to fulfill the request

#define HTTP_RES_STR_OK           "200 OK"                  
#define HTTP_RES_STR_CREATED      "201 Created"             
#define HTTP_RES_STR_NO_CONTENT   "204 No Content"          
#define HTTP_RES_STR_BAD_REQUEST  "400 Bad Request"         
#define HTTP_RES_STR_FORBIDDEN    "403 Forbidden"           
#define HTTP_RES_STR_NOT_FOUND    "404 Not Found"           
#define HTTP_RES_STR_NOT_ALLOWED  "405 Method Not Allowed"  
#define HTTP_RES_STR_CONFLICT     "409 Conflict"            
#define HTTP_RES_STR_INT_SERVER   "500 Internal Server Error"
#define HTTP_RES_STR_NOT_IMPLE    "501 Not Implemented" 

/* HTTP Header fields */
#define HTTP_RES_HEADER_TYPE      "Content-Type: "    // HTTP response content type 
#define HTTP_RES_HEADER_LEN       "Content-Length: "  // Byte length of entity
#define HTTP_RES_HEADER_CONN      "Connection: "      // 'close' or 'keep-alive'

/* HTTP Content Types (MIME) */
// ERROR
#define HTTP_RES_TYPE_ERR         0
// Web standards model / Text
#define HTTP_RES_TYPE_HTML        1
#define HTTP_RES_TYPE_CSS         2
#define HTTP_RES_TYPE_JS          3
#define HTTP_RES_TYPE_TEXT        4
#define HTTP_RES_TYPE_CGI         5
// Object notation format
#define HTTP_RES_TYPE_XML         6
#define HTTP_RES_TYPE_JSON        7
// Image
#define HTTP_RES_TYPE_GIF         8
#define HTTP_RES_TYPE_JPEG        9
#define HTTP_RES_TYPE_PNG         10
#define HTTP_RES_TYPE_FLASH       11
#define HTTP_RES_TYPE_ICO         12
// Font
#define HTTP_RES_TYPE_TTF         20
#define HTTP_RES_TYPE_OTF         21
#define HTTP_RES_TYPE_WOFF        22
#define HTTP_RES_TYPE_EOT         23
#define HTTP_RES_TYPE_SVG         24

// Web standards model / Text
#define HTTP_RES_STR_HTML         "text/html"
#define HTTP_RES_STR_CSS          "text/css"
#define HTTP_RES_STR_JS           "application/javascript"
#define HTTP_RES_STR_TEXT         "text/plain"
#define HTTP_RES_STR_CGI          "text/html"
// Object notation format
#define HTTP_RES_STR_XML          "text/xml"
#define HTTP_RES_STR_JSON         "application/json"
// Image
#define HTTP_RES_STR_GIF          "image/gif"
#define HTTP_RES_STR_JPEG         "image/jpeg"
#define HTTP_RES_STR_PNG          "image/png"
#define HTTP_RES_STR_FLASH        "application/x-shockwave-flash"
#define HTTP_RES_STR_ICO          "image/x-icon"
// Font
#define HTTP_RES_STR_TTF          "application/x-font-truetype"
#define HTTP_RES_STR_OTF          "application/x-font-opentype"
#define HTTP_RES_STR_WOFF         "application/font-woff"
#define HTTP_RES_STR_EOT          "application/vnd.ms-fontobject"
#define HTTP_RES_STR_SVG          "image/svg+xml"

// Example: Add more MIME type item
//  - #define HTTP_RES_TYPE_xxx   Sequence number
//  - #define HTTP_RES_STR_xxx    Content-Type string for HTTP response
//  - Add the item to the mime_table[] in httpParser_rest.c with file (resource) extension

/**
 @brief 	Structure of HTTP REQUEST 
 */

//#define MAX_URI_SIZE	1461
#define MAX_URI_SIZE	512

struct st_http_method
{
	uint8_t     method;
	const char* method_str;
};

struct st_http_status
{
	uint16_t    code;
	const char* code_str;
};

struct st_http_mime
{
	uint8_t     type;
	const char* filext1; // file extenstion (e.g., .html / .json / .xml ...)
	const char* filext2;
	const char* type_str;
};

#ifdef _OLD_
typedef struct _st_http_request
{
	uint8_t	METHOD;						/**< request method(METHOD_GET...). */
	uint8_t	TYPE;						/**< request type(PTYPE_HTML...).   */
	uint8_t	URI[MAX_URI_SIZE];			/**< request file name.             */
} st_http_request;

#else
typedef struct _st_http_request
{
	uint8_t  METHOD;						/**< request method(METHOD_GET...). */
	uint8_t  TYPE;						/**< request type(PTYPE_HTML...).   */
	uint8_t  URI[MAX_URI_SIZE];			/**< request file name.             */
	uint8_t  BODY_LEN;
	uint8_t* BODY;
} st_http_request;
#endif

extern const struct st_http_method method_table[];
extern const struct st_http_status code_table[];

void unescape_http_url(char * url);									/* convert escape character to ascii */
void parse_http_request(st_http_request *, uint8_t *);				/* parse request from peer */
void find_http_uri_type(uint8_t *, uint8_t *);						/* find MIME type of a file */
void make_http_response_header(char *, char, uint32_t, uint16_t);	/* make response header */
uint8_t * get_http_param_value(char* uri, char* param_name);		/* get the user-specific parameter value */
uint8_t get_http_uri_name(uint8_t * uri, uint8_t * uri_buf);		/* get the requested URI name */
#ifdef _OLD_
uint8_t * get_http_uri_name(uint8_t * uri);
#endif

// Utility functions
uint16_t ATOI(uint8_t * str, uint8_t base);
void mid(char* src, char* s1, char* s2, char* sub);
void inet_addr_(uint8_t * addr, uint8_t * ip);

#endif	/* end of __HTTPPARSER_H__ */

// 구현 계획

// GET/HEAD에 대한 성공 응답: 200
  // HEAD에 대한 성공 응답: 200 header only
  // GET에 대한 성공 응답: 200 + entity
  // GET/HEAD에 대한 실패 응답: Resource가 존재하지 않을 때: 404 header only
  
// PUT(update)에 대한 성공 응답: 204	
	// PUT에 대한 성공 응답: Resource Update 성공 후 응답이 entity를 포함하지 않을 때: 204 header only	
	// PUT에 대한 실패 응답: Resource가 없을 때: 404 header only
	// PUT에 대한 실패 응답: Resource가 존재하나, 허용하지 않는 method일 때: 405 header only
	
// POST(create)에 대한 성공 응답: 200 or 201 or 204 (201만 사용)
  // POST에 대한 성공 응답: 생성된 Resource가 URI를 가질 때: 201 header only
  // POST에 대한 성공 응답: 생성된 Resource가 URI가 없으며 응답이 entity를 포함하지 않을 때: 204 header only
  // POST에 대한 성공 응답: 생성된 Resource가 URI가 없으며 응답이 entity를 포함할 때: 200 + entity
  // POST에 대한 실패 응답: 이미 존재하는 resource일 때: 409 header only
  
// DELETE에 대한 성공 응답: 200 or 204
	// DELETE에 대한 성공 응답: 응답이 entity를 포함하지 않을 때: 204 header only
  // DELETE에 대한 성공 응답: 응답이 entity를 포함할 때: 200 + entity
  // DELETE에 대한 실패 응답: Resource가 존재하나, 허용하지 않는 method일 때: 405 header only
  
// * 존재하는 Resource이고, GET / HEAD / POST / DELETE method 지만 허용하지 않는 경우 일 때: 405 header only


/* Old code: HTTP responses */
/*
#define		STATUS_OK			200
#define		STATUS_CREATED		201
#define		STATUS_ACCEPTED		202
#define		STATUS_NO_CONTENT	204
#define		STATUS_MV_PERM		301
#define		STATUS_MV_TEMP		302
#define		STATUS_NOT_MODIF	304
#define		STATUS_BAD_REQ		400
#define		STATUS_UNAUTH		401
#define		STATUS_FORBIDDEN	403
#define		STATUS_NOT_FOUND	404
#define		STATUS_INT_SERR		500
#define		STATUS_NOT_IMPL		501
#define		STATUS_BAD_GATEWAY	502
#define		STATUS_SERV_UNAVAIL	503
*/
