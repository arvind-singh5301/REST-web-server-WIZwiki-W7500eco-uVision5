/**
  ******************************************************************************
  * @file    W7500x Application: RESTful Web Server & I/O Control by REST API
  * @author  Eric Jung, Team Wiki
  * @version v0.8.2
  * @date    Fab-2016
  * @brief   Main program body
  ******************************************************************************
  * @attention
  * @par Revision history
  *    <2015/03/02> first release v0.8.2 Develop
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, WIZnet SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2016 WIZnet Co., Ltd.</center></h2>
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "W7500x_crg.h"
#include "W7500x_gpio.h"
#include "W7500x_uart.h"
#include "W7500x_wztoe.h"
#include "W7500x_miim.h"
#include "common.h"
#include "W7500x_board.h"

#include "configData.h"
#include "timerHandler.h"
#include "uartHandler.h"
#include "gpioHandler.h"

#include "httpServer_rest.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
//#define _MAIN_DEBUG_	// debugging message enable
//#define _USE_DNS_
//#define _USE_DHCP_

/* Private function prototypes -----------------------------------------------*/
static void W7500x_Init(void);
static void W7500x_WZTOE_Init(void);


// Debug messages
void display_Dev_Info_header(void);
void display_Dev_Info_main(void);
void display_Dev_Info_http(void);

void delay(__IO uint32_t milliseconds); // Notice: used ioLibray
void TimingDelay_Decrement(void);

#ifdef _USE_DHCP_
	#include "dhcp.h"
	#include "dhcp_cb.h"
	int8_t process_dhcp(void);
	void display_Dev_Info_dhcp(void);
	uint8_t flag_process_dhcp_success = OFF;
#endif

#ifdef _USE_DNS_
	#include "dns.h"
	int8_t process_dns(void);
	void display_Dev_Info_dns(void);
	uint8_t flag_process_dns_success = OFF;
#endif

/* Private variables ---------------------------------------------------------*/
static __IO uint32_t TimingDelay;

/* Public variables ---------------------------------------------------------*/
// Shared buffer declaration
uint8_t g_send_buf[DATA_BUF_SIZE];
uint8_t g_recv_buf[DATA_BUF_SIZE];

uint8_t flag_application_running = OFF;

// Temp MAC address
uint8_t mac[] = {0x00, 0x08, 0xDC, 0xaa, 0xbb, 0xcc};

// H/W Sockets for HTTP Server
#define MAX_HTTPSOCK		3
#define HTTP_SERVER_PORT	80
uint8_t sock_list[] = {3, 4, 5};

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
	DevConfig *dev_config = get_DevConfig_pointer();
	uint8_t i;
	
	////////////////////////////////////////////////////////////////////////////////////////////////////
	// W7500x Hardware Initialize
	////////////////////////////////////////////////////////////////////////////////////////////////////
	
	/* W7500x MCU Initialization */
	W7500x_Init(); // MCU init, includes UART2 initialize code for print out debugging messages
	
	/* W7500x WZTOE (Hardwired TCP/IP stack) Initialization */
	W7500x_WZTOE_Init();
	
	/* W7500x Board Initialization */
	W7500x_Board_Init();
	
	////////////////////////////////////////////////////////////////////////////////////////////////////
	// W7500x Application: Initialize
	////////////////////////////////////////////////////////////////////////////////////////////////////
	
	/* Default Configuration settings */
	set_DevConfig_to_factory_value();
	
	/* Set the MAC address to WIZCHIP */
	set_mac(mac);
	Mac_Conf();
	
	//set_dhcp_mode();
	set_static_mode();
	
	/* GPIO Initialization*/
	IO_Configuration();
	
	// Debug UART: Device information print out
	display_Dev_Info_header();
	display_Dev_Info_main();
	
	////////////////////////////////////////////////////////////////////////////////////////////////////
	// W7500x Application: DHCP client
	////////////////////////////////////////////////////////////////////////////////////////////////////
	
	/* Network Configuration - DHCP client */
	// Initialize Network Information: DHCP or Static IP allocation
#ifdef _USE_DHCP_
	if(dev_config->options.dhcp_use)
	{
		if(process_dhcp() == DHCP_IP_LEASED) // DHCP success
		{
			flag_process_dhcp_success = ON;
		}
		else // DHCP failed
		{
			Net_Conf(); // Set default static IP settings
		}
	}
	else
#endif
	{
		Net_Conf(); // Set default static IP settings
	}
	
	// Debug UART: Network information print out (includes DHCP IP allocation result)
	display_Net_Info();
	display_Dev_Info_http();
	
#ifdef _USE_DHCP_
	display_Dev_Info_dhcp();
#endif
	
#ifdef _USE_DNS_
	/* DNS client */
	if(dev_config->options.dns_use) 
	{
		if(process_dns()) flag_process_dns_success = ON; // DNS success
	}
	
	// Debug UART: DNS results print out
	display_Dev_Info_dns();
#endif
	
	////////////////////////////////////////////////////////////////////////////////////////////////////
	// W7500x Application: Main Routine
	////////////////////////////////////////////////////////////////////////////////////////////////////
	
	flag_application_running = ON;
	LED_On(LED1);
	
	httpServer_init(g_send_buf, g_recv_buf, MAX_HTTPSOCK, sock_list);
	
	while(1) // main loop
	{
		for(i = 0; i < MAX_HTTPSOCK; i++) httpServer_run(HTTP_SERVER_PORT);
		
#ifdef _USE_DHCP_
		if(dev_config->options.dhcp_use) DHCP_run(); // DHCP client handler for IP renewal
#endif
		
		if(flag_check_main_routine)
		{
			// Device working indicator
			// LEDs blink rapidly (100ms)
			LED_Toggle(LED1);
			LED_Toggle(LED2);
			flag_check_main_routine = 0;
		}
	} // End of application main loop
} // End of main


/*****************************************************************************
 * Private functions
 ****************************************************************************/
static void W7500x_Init(void)
{
	////////////////////////////////////////////////////
	// W7500x MCU Initialize
	////////////////////////////////////////////////////
	
	/* External Clock */
	CRG_PLL_InputFrequencySelect(CRG_OCLK);
	
	/* Set system clock setting: 48MHz */
	*(volatile uint32_t *)(0x41001014) = 0x0060100; //clock setting 48MHz
	
	/* Set Systme init */
	SystemInit();
	
	/* DualTimer Initialization */
	Timer_Configuration();
	
	/* Simple UART init for Debugging */
	UART2_Configuration();
	
	/* SysTick_Config */
	SysTick_Config((GetSystemClock()/1000));
	
#ifdef _MAIN_DEBUG_
	printf("\r\n GetSystemClock : %d (Hz) \r\n", GetSystemClock());  
#endif
}

static void W7500x_WZTOE_Init(void)
{
	////////////////////////////////////////////////////
	// W7500x WZTOE (Hardwired TCP/IP core) Initialize
	////////////////////////////////////////////////////
	
	/* Set Network Configuration: HW Socket Tx/Rx buffer size */
	uint8_t tx_size[8] = { 2, 2, 2, 2, 2, 2, 2, 2 };
	uint8_t rx_size[8] = { 2, 2, 2, 2, 2, 2, 2, 2 };
	
	/* Structure for TCP timeout control: RTR, RCR */
	wiz_NetTimeout * net_timeout;
	
#ifdef _MAIN_DEBUG_
	uint8_t i;
#endif
	
	/* Set WZ_100US Register */
	setTIC100US((GetSystemClock()/10000));
#ifdef _MAIN_DEBUG_
	printf(" GetSystemClock: %X, getTIC100US: %X, (%X) \r\n", GetSystemClock(), getTIC100US(), *(uint32_t *)WZTOE_TIC100US); // for debugging
#endif
	/* Set TCP Timeout: retry count / timeout val */
	// Retry count default: [8], Timeout val default: [2000]
	net_timeout->retry_cnt = 8;
	net_timeout->time_100us = 2500;
	wizchip_settimeout(net_timeout);
	
#ifdef _MAIN_DEBUG_
	wizchip_gettimeout(net_timeout); // TCP timeout settings
	printf(" Network Timeout Settings - RCR: %d, RTR: %dms\r\n", net_timeout->retry_cnt, net_timeout->time_100us);
#endif
	
	/* Set Network Configuration */
	wizchip_init(tx_size, rx_size);
	
#ifdef _MAIN_DEBUG_
	printf(" WZTOE H/W Socket Buffer Settings (kB)\r\n");
	printf(" [Tx] ");
	for(i = 0; i < _WIZCHIP_SOCK_NUM_; i++) printf("%d ", getSn_TXBUF_SIZE(i));
	printf("\r\n");
	printf(" [Rx] ");
	for(i = 0; i < _WIZCHIP_SOCK_NUM_; i++) printf("%d ", getSn_RXBUF_SIZE(i));
	printf("\r\n");
#endif
}

void display_Dev_Info_header(void)
{
	DevConfig *dev_config = get_DevConfig_pointer();

	printf("\r\n");
	printf("%s\r\n", STR_BAR);
#ifndef __W7500P__
	printf(" [W7500]");
#else
	printf(" [W7500P]");
#endif
	printf(" Application Examples\r\n");
	printf(" RESTful Web Server & I/O Control by REST API\r\n");
	printf(" Firmware version: %d.%d.%d %s\r\n", dev_config->fw_ver[0], dev_config->fw_ver[1], dev_config->fw_ver[2], STR_VERSION_STATUS);
	printf("%s\r\n", STR_BAR);
}

void display_Dev_Info_main(void)
{
	DevConfig *dev_config = get_DevConfig_pointer();
	
	printf(" - Device name: %s\r\n", dev_config->module_name);
	printf(" - Network settings: \r\n");
	printf("\t- IP assign: %s\r\n", (dev_config->options.dhcp_use == 1)?"DHCP":"Static");
	printf("%s\r\n", STR_BAR);
}

void display_Dev_Info_http(void)
{
	printf(" # HTTP Server Port: %d\r\n", HTTP_SERVER_PORT);
}

#ifdef _USE_DHCP

int8_t process_dhcp(void)
{
	uint8_t ret = 0;
	uint8_t dhcp_retry = 0;

#ifdef _MAIN_DEBUG_
	printf(" - DHCP Client running\r\n");
#endif
	DHCP_init(SOCK_DHCP, g_send_buf);
	reg_dhcp_cbfunc(w7500x_dhcp_assign, w7500x_dhcp_assign, w7500x_dhcp_conflict);
	
	while(1)
	{
		ret = DHCP_run();
		
		if(ret == DHCP_IP_LEASED)
		{
#ifdef _MAIN_DEBUG_
			printf(" - DHCP Success\r\n");
#endif
			break;
		}
		else if(ret == DHCP_FAILED)
		{
			dhcp_retry++;
#ifdef _MAIN_DEBUG_
			if(dhcp_retry <= 3) printf(" - DHCP Timeout occurred and retry [%d]\r\n", dhcp_retry);
#endif
		}

		if(dhcp_retry > 3)
		{
#ifdef _MAIN_DEBUG_
			printf(" - DHCP Failed\r\n\r\n");
#endif
			DHCP_stop();
			break;
		}
	}
	
	return ret;
}


void display_Dev_Info_dhcp(void)
{
	DevConfig *dev_config = get_DevConfig_pointer();
	
	if(dev_config->options.dhcp_use) 
	{
		if(flag_process_dhcp_success == ON) printf(" # DHCP IP Leased time : %u seconds\r\n", getDHCPLeasetime());
		else printf(" # DHCP Failed\r\n");
	}
}

#endif


#ifdef _USE_DNS_

int8_t process_dns(void)
{
	DevConfig *dev_config = get_DevConfig_pointer();
	
	int8_t ret = 0;
	uint8_t dns_retry = 0;
	uint8_t dns_server_ip[4];
	
#ifdef _MAIN_DEBUG_
	printf(" - DNS Client running\r\n");
#endif
	
	DNS_init(SOCK_DNS, g_send_buf);
	
	dns_server_ip[0] = dev_config->options.dns_server_ip[0];
	dns_server_ip[1] = dev_config->options.dns_server_ip[1];
	dns_server_ip[2] = dev_config->options.dns_server_ip[2];
	dns_server_ip[3] = dev_config->options.dns_server_ip[3];
	
	while(1) 
	{
		if((ret = DNS_run(dns_server_ip, (uint8_t *)dev_config->options.dns_domain_name, dev_config->network_info.remote_ip)) == 1)
		{
#ifdef _MAIN_DEBUG_
			printf(" - DNS Success\r\n");
#endif
			break;
		}
		else
		{
			dns_retry++;
#ifdef _MAIN_DEBUG_
			if(dns_retry <= 2) printf(" - DNS Timeout occurred and retry [%d]\r\n", dns_retry);
#endif
		}

		if(dns_retry > 2) {
#ifdef _MAIN_DEBUG_
			printf(" - DNS Failed\r\n\r\n");
#endif
			break;
		}
		if(dev_config->options.dhcp_use) DHCP_run();
	}
	
	return ret;
}

void display_Dev_Info_dns(void)
{
	DevConfig *dev_config = get_DevConfig_pointer();
	
	if(dev_config->options.dns_use) 
	{
		if(flag_process_dns_success == ON)
		{
			printf(" # DNS: %s => %d.%d.%d.%d : %d\r\n", dev_config->options.dns_domain_name, 
														dev_config->network_info.remote_ip[0],
														dev_config->network_info.remote_ip[1],
														dev_config->network_info.remote_ip[2],
														dev_config->network_info.remote_ip[3],
														dev_config->network_info.remote_port);
		}
		else printf(" # DNS Failed\r\n");
	}
	
	printf("\r\n");
}

#endif


/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in milliseconds.
  * @retval None
  */
void delay(__IO uint32_t milliseconds)
{
	TimingDelay = milliseconds;
	while(TimingDelay != 0);
}


/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
	if(TimingDelay != 0x00)
	{
		TimingDelay--;
	}
}

