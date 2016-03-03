/*
*
@file		W7500x_board.h
@brief
*/

#ifndef __W7500X_BOARD_H__ 
#define __W7500X_BOARD_H__ 

#include <stdint.h>
#include "common.h"

////////////////////////////////
// Product Configurations	  //
////////////////////////////////
/* Board Name */
#define DEVICE_BOARD_NAME	WIZwiki_W7500ECO

#ifdef DEVICE_BOARD_NAME
	#if (DEVICE_BOARD_NAME == WIZwiki_W7500P)
		#define __W7500P__
		#define DEVICE_ID_DEFAULT	"W7500P_Device" // W7500P Devices
	#else 
		#define DEVICE_ID_DEFAULT	"W7500_Device" // WIZwiki_W7500 or WIZwiki_W7500ECO Board
	#endif
#else
	#define DEVICE_BOARD_NAME	UNKNOWN_DEVICE
	#define DEVICE_ID_DEFAULT	"UNKNOWN"
#endif

#define __DEF_USED_MDIO__ 
#ifndef __W7500P__
	#define __DEF_USED_IC101AG__ // PHY initialize for WIZwiki-W7500 Board
#endif

////////////////////////////////
// Pin definitions			  //
////////////////////////////////

// Expansion GPIOs (4-Pins, GPIO A / B / C / D)
//#define MAX_USER_IOn    16
#define USER_IOn       4
#define USER_IO_A      (uint16_t)(0x01 <<  0)     // USER's I/O A
#define USER_IO_B      (uint16_t)(0x01 <<  1)     // USER's I/O B
#define USER_IO_C      (uint16_t)(0x01 <<  2)     // USER's I/O C
#define USER_IO_D      (uint16_t)(0x01 <<  3)     // USER's I/O D

#define USER_IO_DEFAULT_PAD_AF		PAD_AF1 // [2nd] GPIOs
#define USER_IO_AIN_PAD_AF			PAD_AF3 // [4th] AINs

#define USER_IO_A_PIN				GPIO_Pin_15
#define USER_IO_A_PORT				GPIOC
#define USER_IO_A_ADC_CH			ADC_CH0 // if the pin didn't support AIN, 'USER_IO_x_ADC_CH define' have to set to 0xff.

#define USER_IO_B_PIN				GPIO_Pin_14
#define USER_IO_B_PORT				GPIOC
#define USER_IO_B_ADC_CH			ADC_CH1

#define USER_IO_C_PIN				GPIO_Pin_13
#define USER_IO_C_PORT				GPIOC
#define USER_IO_C_ADC_CH			ADC_CH2

#define USER_IO_D_PIN				GPIO_Pin_12
#define USER_IO_D_PORT				GPIOC
#define USER_IO_D_ADC_CH			ADC_CH3

// Status LEDs define
#if (DEVICE_BOARD_NAME == WIZwiki_W7500ECO)

	#define LED1_PIN			GPIO_Pin_1
	#define LED1_GPIO_PORT		GPIOA
	#define LED1_GPIO_PAD		PAD_PA
	#define LED1_GPIO_PAD_AF	PAD_AF1		// PAD Config - LED used 2nd Function

	#define LED2_PIN			GPIO_Pin_2
	#define LED2_GPIO_PORT		GPIOA
	#define LED2_GPIO_PAD		PAD_PA
	#define LED2_GPIO_PAD_AF	PAD_AF1

#else // WIZwiki-W7500 board

		// [RGB LED] R: PC_08, G: PC_09, B: PC_05
	#define LED1_PIN			GPIO_Pin_8 // RGB LED: RED
	#define LED1_GPIO_PORT		GPIOC
	#define LED1_GPIO_PAD		PAD_PC
	#define LED1_GPIO_PAD_AF	PAD_AF1

	#define LED2_PIN			GPIO_Pin_9 // RGB LED: GREEN
	#define LED2_GPIO_PORT		GPIOC
	#define LED2_GPIO_PAD		PAD_PC
	#define LED2_GPIO_PAD_AF	PAD_AF1
	
/*	
	#define LED_R_PIN			GPIO_Pin_8
	#define LED_R_GPIO_PORT		GPIOC
	#define LED_R_GPIO_PAD		PAD_PC
	#define LED_R_GPIO_PAD_AF	PAD_AF1

	#define LED_G_PIN			GPIO_Pin_9
	#define LED_G_GPIO_PORT		GPIOC
	#define LED_G_GPIO_PAD		PAD_PC
	#define LED_G_GPIO_PAD_AF	PAD_AF1

	#define LED_B_PIN			GPIO_Pin_5
	#define LED_B_GPIO_PORT		GPIOC
	#define LED_B_GPIO_PAD		PAD_PC
	#define LED_B_GPIO_PAD_AF	PAD_AF1

	// LED
	#define LEDn		3
	typedef enum
	{
	  LED_R = 0,
	  LED_G = 1,
	  LED_B = 2  
	} Led_TypeDef;
*/
#endif
	
	// LED
	#define LEDn		2
	typedef enum
	{
	  LED1 = 0,	// PHY link status
	  LED2 = 1	// TCP connection status
	} Led_TypeDef;
	
	void W7500x_Board_Init(void);
	
	void LED_Init(Led_TypeDef Led);
	void LED_On(Led_TypeDef Led);
	void LED_Off(Led_TypeDef Led);
	void LED_Toggle(Led_TypeDef Led);
	uint8_t get_LED_Status(Led_TypeDef Led);
	
#endif
