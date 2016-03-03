#ifndef _COMMON_H
#define _COMMON_H

#include <stdint.h>

////////////////////////////////
// Product Version			  //
////////////////////////////////
/* Application Firmware Version */
#define MAJOR_VER			0
#define MINOR_VER			8
#define MAINTENANCE_VER		2

#define STR_VERSION_STATUS	"Develop" // or "Stable"
//#define STR_VERSION_STATUS	"Stable"


////////////////////////////////
// W7500x HW Socket Definition//
////////////////////////////////
#define SOCK_DHCP			6
#define SOCK_DNS			7


////////////////////////////////
// Ethernet					  //
////////////////////////////////
/* Buffer size */
#define DATA_BUF_SIZE		2048


////////////////////////////////
// Available board list		  //
////////////////////////////////
#define WIZwiki_W7500		0
#define WIZwiki_W7500ECO	1
#define WIZwiki_W7500P		2
#define UNKNOWN_DEVICE		0xff


////////////////////////////////
// Defines					  //
////////////////////////////////

// On/Off Status
typedef enum
{
	OFF		= 0,
	ON		= 1
} OnOff_State_Type;

#define OP_COMMAND	0
#define OP_DATA		1

#define RET_OK			0
#define RET_NOK			-1
#define RET_TIMEOUT		-2

//#define STR_UART		"UART"
//#define STR_ENABLED		"Enabled"
//#define STR_DISABLED	"Disabled"
#define STR_BAR			"============================================="


#endif //_COMMON_H
