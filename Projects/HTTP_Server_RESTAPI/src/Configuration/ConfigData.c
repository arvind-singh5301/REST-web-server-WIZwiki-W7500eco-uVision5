/*
 * ConfigData.c 
 */

#include <stdio.h>
#include <string.h>
#include "common.h"
#include "W7500x_wztoe.h"
#include "W7500x_board.h"
#include "ConfigData.h"
#include "uartHandler.h"

static DevConfig dev_config;

DevConfig* get_DevConfig_pointer(void)
{
	return &dev_config;
}

void set_DevConfig_to_factory_value(void)
{
	dev_config.packet_size = sizeof(DevConfig);
	
	memset(dev_config.module_name, 0x00, 25);
	memcpy(dev_config.module_name, DEVICE_ID_DEFAULT, sizeof(DEVICE_ID_DEFAULT));
	
	dev_config.fw_ver[0] = MAJOR_VER;
	dev_config.fw_ver[1] = MINOR_VER;
	dev_config.fw_ver[2] = MAINTENANCE_VER;
	
	dev_config.network_info_common.local_ip[0] = 192;
	dev_config.network_info_common.local_ip[1] = 168;
	dev_config.network_info_common.local_ip[2] = 11;
	dev_config.network_info_common.local_ip[3] = 5;
	dev_config.network_info_common.gateway[0] = 192;
	dev_config.network_info_common.gateway[1] = 168;
	dev_config.network_info_common.gateway[2] = 11;
	dev_config.network_info_common.gateway[3] = 1;
	dev_config.network_info_common.subnet[0] = 255;
	dev_config.network_info_common.subnet[1] = 255;
	dev_config.network_info_common.subnet[2] = 255;
	dev_config.network_info_common.subnet[3] = 0;
	dev_config.network_info_common.local_port = 5000;
	
	dev_config.network_info.remote_ip[0] = 192;
	dev_config.network_info.remote_ip[1] = 168;
	dev_config.network_info.remote_ip[2] = 11;
	dev_config.network_info.remote_ip[3] = 113;
	dev_config.network_info.remote_port = 5000;
	
	dev_config.options.dhcp_use = 0;
	dev_config.options.dns_use = 0;
	dev_config.options.dns_server_ip[0] = 8;	// Default DNS server IP: Google Public DNS (8.8.8.8)
	dev_config.options.dns_server_ip[1] = 8;
	dev_config.options.dns_server_ip[2] = 8;
	dev_config.options.dns_server_ip[3] = 8;
	memset(dev_config.options.dns_domain_name, 0x00, 50);
	
	dev_config.user_io_info.user_io_enable = USER_IO_A | USER_IO_B | USER_IO_C | USER_IO_D; // [Enabled] / Disabled
	//dev_config.user_io_info.user_io_enable = USER_IO_A | USER_IO_B; // Enabled / [Disabled]
	//dev_config.user_io_info.user_io_type = ~(USER_IO_A | USER_IO_B | USER_IO_C | USER_IO_D); // [Digital] / Analog
	dev_config.user_io_info.user_io_type = USER_IO_A | USER_IO_B; // A, B = Analog / C, D = Digital
	dev_config.user_io_info.user_io_direction = ~(USER_IO_A | USER_IO_B | USER_IO_C | USER_IO_D); // [Input] / Output
}

void get_DevConfig_value(void *dest, const void *src, uint16_t size)
{
	memcpy(dest, src, size);
}

void set_DevConfig_value(void *dest, const void *value, const uint16_t size)
{
	memcpy(dest, value, size);
}

void set_DevConfig(wiz_NetInfo *net)
{
	set_DevConfig_value(dev_config.network_info_common.mac, net->mac, sizeof(dev_config.network_info_common.mac));
	set_DevConfig_value(dev_config.network_info_common.local_ip, net->ip, sizeof(dev_config.network_info_common.local_ip));
	set_DevConfig_value(dev_config.network_info_common.gateway, net->gw, sizeof(dev_config.network_info_common.gateway));
	set_DevConfig_value(dev_config.network_info_common.subnet, net->sn, sizeof(dev_config.network_info_common.subnet));
	set_DevConfig_value(dev_config.options.dns_server_ip, net->dns, sizeof(dev_config.options.dns_server_ip));
	if(net->dhcp == NETINFO_STATIC)
		dev_config.options.dhcp_use = 0;
	else
		dev_config.options.dhcp_use = 1;
}

void get_DevConfig(wiz_NetInfo *net)
{
	get_DevConfig_value(net->mac, dev_config.network_info_common.mac, sizeof(net->mac));
	get_DevConfig_value(net->ip, dev_config.network_info_common.local_ip, sizeof(net->ip));
	get_DevConfig_value(net->gw, dev_config.network_info_common.gateway, sizeof(net->gw));
	get_DevConfig_value(net->sn, dev_config.network_info_common.subnet, sizeof(net->sn));
	get_DevConfig_value(net->dns, dev_config.options.dns_server_ip, sizeof(net->dns));
	if(dev_config.options.dhcp_use)
		net->dhcp = NETINFO_DHCP;
	else
		net->dhcp = NETINFO_STATIC;
}

void display_Net_Info(void)
{
	DevConfig *value = get_DevConfig_pointer();
	wiz_NetInfo gWIZNETINFO;

	ctlnetwork(CN_GET_NETINFO, (void*) &gWIZNETINFO);
	printf(" # MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n", gWIZNETINFO.mac[0], gWIZNETINFO.mac[1], gWIZNETINFO.mac[2], gWIZNETINFO.mac[3], gWIZNETINFO.mac[4], gWIZNETINFO.mac[5]);
	//printf(" # IP: %d.%d.%d.%d / Port: %d\r\n", gWIZNETINFO.ip[0], gWIZNETINFO.ip[1], gWIZNETINFO.ip[2], gWIZNETINFO.ip[3], value->network_info_common.local_port);
	printf(" # IP: %d.%d.%d.%d\r\n", gWIZNETINFO.ip[0], gWIZNETINFO.ip[1], gWIZNETINFO.ip[2], gWIZNETINFO.ip[3]);
	printf(" # GW: %d.%d.%d.%d\r\n", gWIZNETINFO.gw[0], gWIZNETINFO.gw[1], gWIZNETINFO.gw[2], gWIZNETINFO.gw[3]);
	printf(" # SN: %d.%d.%d.%d\r\n", gWIZNETINFO.sn[0], gWIZNETINFO.sn[1], gWIZNETINFO.sn[2], gWIZNETINFO.sn[3]);
	printf(" # DNS: %d.%d.%d.%d\r\n", gWIZNETINFO.dns[0], gWIZNETINFO.dns[1], gWIZNETINFO.dns[2], gWIZNETINFO.dns[3]);
	printf("\r\n");
}

void Mac_Conf(void)
{
	DevConfig *value = get_DevConfig_pointer();
	setSHAR(value->network_info_common.mac);
}

void Net_Conf(void)
{
	DevConfig *value = get_DevConfig_pointer();
	wiz_NetInfo gWIZNETINFO;

	/* wizchip netconf */
	get_DevConfig_value(gWIZNETINFO.mac, value->network_info_common.mac, sizeof(gWIZNETINFO.mac[0]) * 6);
	get_DevConfig_value(gWIZNETINFO.ip, value->network_info_common.local_ip, sizeof(gWIZNETINFO.ip[0]) * 4);
	get_DevConfig_value(gWIZNETINFO.gw, value->network_info_common.gateway, sizeof(gWIZNETINFO.gw[0]) * 4);
	get_DevConfig_value(gWIZNETINFO.sn, value->network_info_common.subnet, sizeof(gWIZNETINFO.sn[0]) * 4);
	get_DevConfig_value(gWIZNETINFO.dns, value->options.dns_server_ip, sizeof(gWIZNETINFO.dns));
	if(value->options.dhcp_use)
		gWIZNETINFO.dhcp = NETINFO_DHCP;
	else
		gWIZNETINFO.dhcp = NETINFO_STATIC;

	ctlnetwork(CN_SET_NETINFO, (void*) &gWIZNETINFO);
}

void set_dhcp_mode(void)
{
	DevConfig *value = get_DevConfig_pointer();

	value->options.dhcp_use = 1;
}

void set_static_mode(void)
{
	DevConfig *value = get_DevConfig_pointer();

	value->options.dhcp_use = 0;
}

void set_mac(uint8_t *mac)
{
	DevConfig *value = get_DevConfig_pointer();

	memcpy(value->network_info_common.mac, mac, sizeof(value->network_info_common.mac));
}

