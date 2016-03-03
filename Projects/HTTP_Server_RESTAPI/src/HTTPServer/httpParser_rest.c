/**
 @file		httpd.c
 @brief 		functions associated http processing
 */

#include <stdio.h>
#include <string.h>
#include "socket.h"
#include "httpParser_rest.h"

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/
uint8_t BUFPUB[512];

const struct st_http_method method_table[] = 
{
	{ HTTP_REQ_METHOD_GET,        HTTP_REQ_STR_GET         },
	{ HTTP_REQ_METHOD_HEAD,       HTTP_REQ_STR_HEAD        },
	{ HTTP_REQ_METHOD_POST,       HTTP_REQ_STR_POST        },
	{ HTTP_REQ_METHOD_PUT,        HTTP_REQ_STR_PUT         },
	{ HTTP_REQ_METHOD_DELETE,     HTTP_REQ_STR_DELETE      },
	
	{ NULL, NULL } // Last item should be set to NULL
};

const struct st_http_status code_table[] = 
{
	{ HTTP_RES_CODE_OK,           HTTP_RES_STR_OK          },
	{ HTTP_RES_CODE_CREATED,      HTTP_RES_STR_CREATED     },
	{ HTTP_RES_CODE_NO_CONTENT,   HTTP_RES_STR_NO_CONTENT  },
	{ HTTP_RES_CODE_BAD_REQUEST,  HTTP_RES_STR_BAD_REQUEST },
	{ HTTP_RES_CODE_FORBIDDEN,    HTTP_RES_STR_FORBIDDEN   },
	{ HTTP_RES_CODE_NOT_FOUND,    HTTP_RES_STR_NOT_FOUND   },
	{ HTTP_RES_CODE_NOT_ALLOWED,  HTTP_RES_STR_NOT_ALLOWED },
	{ HTTP_RES_CODE_CONFLICT,     HTTP_RES_STR_CONFLICT    },
	{ HTTP_RES_CODE_NOT_IMPLE,    HTTP_RES_STR_NOT_IMPLE   },
	
	{ NULL, NULL } // Last item should be set to NULL
};

const struct st_http_mime mime_table[] = 
{
	{ HTTP_RES_TYPE_HTML,  ".htm",  ".html", 	HTTP_RES_STR_HTML  },
	{ HTTP_RES_TYPE_CSS,   ".css",  ".CSS", 	HTTP_RES_STR_CSS   },
	{ HTTP_RES_TYPE_JS,    ".js",   ".JS",  	HTTP_RES_STR_JS    },
	{ HTTP_RES_TYPE_TEXT,  ".txt",  ".text", 	HTTP_RES_STR_TEXT  },
	{ HTTP_RES_TYPE_CGI,   ".cgi",  ".CGI", 	HTTP_RES_STR_CGI   },
	{ HTTP_RES_TYPE_XML,   ".xml",  ".XML", 	HTTP_RES_STR_XML   },
	{ HTTP_RES_TYPE_JSON,  ".json", ".JSON", 	HTTP_RES_STR_JSON  },
	{ HTTP_RES_TYPE_GIF,   ".gif",  ".GIF", 	HTTP_RES_STR_GIF   },
	{ HTTP_RES_TYPE_JPEG,  ".jpg",  ".jpeg", 	HTTP_RES_STR_JPEG  },
	{ HTTP_RES_TYPE_PNG,   ".png",  ".PNG", 	HTTP_RES_STR_PNG   },
	{ HTTP_RES_TYPE_FLASH, ".swf",  ".SWF", 	HTTP_RES_STR_FLASH },
	{ HTTP_RES_TYPE_ICO,   ".ico",  ".ICO", 	HTTP_RES_STR_ICO   },
	{ HTTP_RES_TYPE_TTF,   ".ttf",  ".TTF", 	HTTP_RES_STR_TTF   },
	{ HTTP_RES_TYPE_OTF,   ".otf",  ".OTF", 	HTTP_RES_STR_OTF   },
	{ HTTP_RES_TYPE_WOFF,  ".woff", ".WOFF", 	HTTP_RES_STR_WOFF  },
	{ HTTP_RES_TYPE_EOT,   ".eot",  ".EOT", 	HTTP_RES_STR_EOT   },
	{ HTTP_RES_TYPE_SVG,   ".svg",  ".SVG", 	HTTP_RES_STR_SVG   },
	
	{ NULL, NULL, NULL, NULL } // Last item should be set to NULL
};

/*****************************************************************************
 * Private functions
 ****************************************************************************/
static void replacetochar(uint8_t * str, uint8_t oldchar, uint8_t newchar); 	/* Replace old character with new character in the string */
static uint8_t C2D(uint8_t c); 												/* Convert a character to HEX */

/**
 @brief	convert escape characters(%XX) to ASCII character
 */ 
void unescape_http_url(
	char * url	/**< pointer to be converted ( escape characters )*/
	)
{
	int x, y;

	for (x = 0, y = 0; url[y]; ++x, ++y) {
		if ((url[x] = url[y]) == '%') {
			url[x] = C2D(url[y+1])*0x10+C2D(url[y+2]);
			y+=2;
		}
	}
	url[x] = '\0';
}


/**
 @brief	make response header such as html, gif, jpeg,etc.
 */ 
void make_http_response_header(
	char * buf,            /**< pointer to response header to be made */
	char type,             /**< response type */
	uint32_t len,          /**< size of response content */
	uint16_t http_status   /**< http status */
	)
{
	const char * status_code;
	const char * content_type;
	uint8_t i;
	uint16_t str_len;

	// Find the HTTP status code
	for(i = 0; code_table[i].code != NULL; i++)
	{
		if(http_status == code_table[i].code)
		{
			status_code = code_table[i].code_str;
			break;
		}
	}
	
	// Find the HTTP Content-Type
	for(i = 0; mime_table[i].type != NULL; i++)
	{
		if(type == mime_table[i].type)
		{
			content_type = mime_table[i].type_str;
			break;
		}
	}
	
	if(status_code == NULL) status_code = HTTP_RES_STR_NOT_IMPLE;
	if(content_type == NULL) content_type = HTTP_RES_STR_JSON;
	
	// Generate HTTP response header string
	str_len = sprintf(buf, "%s %s\r\n", HTTP_VERSION_STR, status_code);
	str_len += sprintf(buf+str_len, "%s %s\r\n", HTTP_RES_HEADER_TYPE, content_type);
	str_len += sprintf(buf+str_len, "%s %d\r\n", HTTP_RES_HEADER_LEN, len);
	str_len += sprintf(buf+str_len, "%s %s\r\n", HTTP_RES_HEADER_CONN, "close"); // or "keep-alive"
	strcat(buf, "\r\n");
}


/**
 @brief	find MIME type of a file
 */ 
void find_http_uri_type(
	uint8_t * type, 	/**< type to be returned */
	uint8_t * buff		/**< file name */
	) 
{
	/* Decide content-type according to file extension */
	uint8_t i;
	char * buf;
	buf = (char *)buff;
	
	for(i = 0; mime_table[i].type != NULL; i++)
	{
		if(strstr(buf, mime_table[i].filext1) || strstr(buf, mime_table[i].filext2))
		{
			*type = mime_table[i].type;
			return;
		}
	}
	
	// Cannot find the matched type
	*type = HTTP_RES_TYPE_ERR;
}


/**
 @brief	Parse http request from a peer
 */ 
void parse_http_request(
	st_http_request * request,  /**< request to be returned */
	uint8_t * buf               /**< pointer to be parsed */
	)
{
#ifndef _OLD_
	// Todo: PUT update
	/*
	uint8_t tmp_buf[10]={0x00, };
	char * body_ptr = NULL;
	uint16_t body_len = 0;
	*/
#endif
	
	char * nexttok;
	nexttok = strtok((char*)buf," ");
	
	if(!nexttok)
	{
		request->METHOD = HTTP_REQ_METHOD_ERR;      // Error
		return;
	}
	
	// All registered HTTP methods are all upper-case
	// e.g., GET / POST / PUT / DELETE ...
	if(!strcmp(nexttok, HTTP_REQ_STR_GET))          // GET method
	{
		request->METHOD = HTTP_REQ_METHOD_GET;
		nexttok = strtok(NULL," ");
	}
	else if (!strcmp(nexttok, HTTP_REQ_STR_HEAD))   // HEAD method
	{
		request->METHOD = HTTP_REQ_METHOD_HEAD;
		nexttok = strtok(NULL," ");
	}
	else if (!strcmp(nexttok, HTTP_REQ_STR_POST))   // POST method
	{
		request->METHOD = HTTP_REQ_METHOD_POST;
		nexttok = strtok(NULL,"\0");
	}
	else if (!strcmp(nexttok, HTTP_REQ_STR_PUT))    // PUT method
	{
		request->METHOD = HTTP_REQ_METHOD_PUT;
		nexttok = strtok(NULL,"\0");
	}
	else if (!strcmp(nexttok, HTTP_REQ_STR_DELETE)) // DELETE method
	{
		request->METHOD = HTTP_REQ_METHOD_DELETE;
		nexttok = strtok(NULL,"\0");
	}
	else
	{
		request->METHOD = HTTP_REQ_METHOD_ERR;      // Error
	}

	if(!nexttok)
	{
		request->METHOD = HTTP_REQ_METHOD_ERR;      // Error
		return;
	}
	
	strcpy((char *)request->URI, nexttok);

#ifndef _OLD_
	
	// Todo: PUT update
	/*
	if((request->METHOD == HTTP_REQ_METHOD_GET) || (request->METHOD == HTTP_REQ_METHOD_HEAD) || (request->METHOD == HTTP_REQ_METHOD_ERR)) return;
	
	// Parse the additional part: Content length / Content body
	mid((char *)request->URI, "Content-Length: ", "\r\n", (char *)tmp_buf);
	body_len = ATOI(tmp_buf, 10);
	body_ptr = strstr((char *)request->URI, "\r\n\r\n");
	body_ptr+=4;
	
	request->BODY_LEN = body_len;
	request->BODY = (uint8_t *)body_ptr;
	
	printf("  >> bodylen = %d\r\n", request->BODY_LEN);
	printf("  >> body\r\n%s\r\n", request->BODY);
	*/
#endif
}


/**
 @brief	get next parameter value in the request
 */
uint8_t * get_http_param_value(char* uri, char* param_name)
{

	uint8_t * name;
	uint8_t * ret = BUFPUB;
	uint8_t * pos2;
	uint16_t len = 0, content_len = 0;
	uint8_t tmp_buf[10]={0x00, };

	if(!uri || !param_name) return 0;

	/***************/
	mid(uri, "Content-Length: ", "\r\n", (char *)tmp_buf);
	content_len = ATOI(tmp_buf, 10);
	//printf("content len=%d\r\n",content_len);
	uri = strstr(uri, "\r\n\r\n");
	uri+=4;
	//printf("uri=%s\r\n",uri);
	uri[content_len] = 0;
	/***************/
	
	name = (uint8_t *)strstr(uri, param_name);
	if(name != NULL)
	{
		name += strlen(param_name) + 1;
		pos2 = (uint8_t*)strstr((char*)name, "&");
		if(!pos2)
		{
			pos2 = name + strlen((char*)name);
		}
		len = pos2 - name;

		if(len)
		{
			ret[len] = 0;
			strncpy((char*)ret,(char*)name, len);
			unescape_http_url((char *)ret);
			replacetochar(ret, '+' ,' ');
			//ret[len] = 0;
			//ret[strlen((int8*)ret)] = 0;
			//printf("len=%d\r\n",len);
		}
		else
		{
			ret[0] = 0;
		}
	}
	else
	{
		return 0;
	}
#ifdef _HTTPPARSER_DEBUG_
	printf("  %s=%s\r\n", param_name, ret);
#endif
	return ret;
}


uint8_t get_http_uri_name(uint8_t * uri, uint8_t * uri_buf)
{
	uint8_t * uri_ptr;
	if(!uri) return 0;

	strcpy((char *)uri_buf, (char *)uri);

	uri_ptr = (uint8_t *)strtok((char *)uri_buf, " ?");

	if(strcmp((char *)uri_ptr,"/")) uri_ptr++;
	strcpy((char *)uri_buf, (char *)uri_ptr);

#ifdef _HTTPPARSER_DEBUG_
	printf("  uri_name = %s\r\n", uri_buf);
#endif

	return 1;
}


void inet_addr_(uint8_t * addr, uint8_t *ip)
{
	uint8_t i;
	uint8_t taddr[30];
	uint8_t * nexttok;
	uint8_t num;

	strcpy((char *)taddr, (char *)addr);

	nexttok = taddr;
	for(i = 0; i < 4 ; i++)
	{
		nexttok = (uint8_t *)strtok((char *)nexttok, ".");
		if(nexttok[0] == '0' && nexttok[1] == 'x') num = ATOI(nexttok+2,0x10);
		else num = ATOI(nexttok,10);
		ip[i] = num;
		nexttok = NULL;
	}
}


/**
@brief	CONVERT STRING INTO INTEGER
@return	a integer number
*/
uint16_t ATOI(
	uint8_t * str,	/**< is a pointer to convert */
	uint8_t base	/**< is a base value (must be in the range 2 - 16) */
	)
{
        unsigned int num = 0;
// debug_2013_11_25
//        while (*str !=0)
        while ((*str !=0) && (*str != 0x20)) // not include the space(0x020)
                num = num * base + C2D(*str++);
	return num;
}

/**
 * @brief Check strings and then execute callback function by each string.
 * @param src The information of URI
 * @param s1 The start string to be researched
 * @param s2 The end string to be researched
 * @param sub The string between s1 and s2
 * @return The length value atfer working
 */
void mid(char* src, char* s1, char* s2, char* sub)
{
	char* sub1;
	char* sub2;
	uint16_t n;

	sub1=strstr((char*)src,(char*)s1);
	sub1+=strlen((char*)s1);
	sub2=strstr((char*)sub1,(char*)s2);

	n=sub2-sub1;
	strncpy((char*)sub,(char*)sub1,n);
	sub[n]='\0';
}

////////////////////////////////////////////////////////////////////
// Static functions
////////////////////////////////////////////////////////////////////

/**
@brief	replace the specified character in a string with new character
*/
static void replacetochar(
		uint8_t * str, 		/**< pointer to be replaced */
		uint8_t oldchar, 	/**< old character */
		uint8_t newchar	/**< new character */
	)
{
	int x;
	for (x = 0; str[x]; x++)
		if (str[x] == oldchar) str[x] = newchar;
}

/**
@brief	CONVERT CHAR INTO HEX
@return	HEX

This function converts HEX(0-F) to a character
*/
static uint8_t C2D(
		uint8_t c	/**< is a character('0'-'F') to convert to HEX */
	)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return 10 + c -'a';
	if (c >= 'A' && c <= 'F')
		return 10 + c -'A';

	return (char)c;
}



