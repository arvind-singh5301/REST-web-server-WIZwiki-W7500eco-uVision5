/*
 * ConfigData.h
 */

#ifndef __CONFIGDATA_H__
#define __CONFIGDATA_H__

#include <stdint.h>
#include "wizchip_conf.h"

struct __network_info_common {
	uint8_t mac[6];
	uint8_t local_ip[4];
	uint8_t gateway[4];
	uint8_t subnet[4];
	uint16_t local_port;
} __attribute__((packed));

struct __network_info {
	uint8_t remote_ip[4];
	uint16_t remote_port;
} __attribute__((packed));

struct __options {
	uint8_t dhcp_use;
	uint8_t dns_use;
	uint8_t dns_server_ip[4];
	char dns_domain_name[50];
} __attribute__((packed));

// ## Eric, Field added for expansion I/Os
struct __user_io_info {
	uint16_t user_io_enable;		// 0: Disable / 1: Enable
	uint16_t user_io_type;			// 0: Digital / 1: Analog
	uint16_t user_io_direction;		// 0: Input / 1: Output
} __attribute__((packed));

typedef struct __DevConfig {
	uint16_t packet_size;
	uint8_t module_name[25];
	uint8_t fw_ver[3];					// Major Version . Minor Version . Maintenance Version
	struct __network_info_common network_info_common;
	struct __network_info network_info;
	struct __options options;
	struct __user_io_info user_io_info;		// Enable / Type / Direction
} __attribute__((packed)) DevConfig;

DevConfig* get_DevConfig_pointer(void);
void set_DevConfig_to_factory_value(void);
void get_DevConfig_value(void *dest, const void *src, uint16_t size);
void set_DevConfig_value(void *dest, const void *value, const uint16_t size);
void set_DevConfig(wiz_NetInfo *net);
void get_DevConfig(wiz_NetInfo *net);

void display_Net_Info(void);
void Mac_Conf(void);
void Net_Conf(void);
void set_dhcp_mode(void);
void set_static_mode(void);
void set_mac(uint8_t *mac);

#endif /* S2E_PACKET_H_ */
