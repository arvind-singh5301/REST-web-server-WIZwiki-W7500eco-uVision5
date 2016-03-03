#include "W7500x.h"
#include "W7500x_miim.h"
#include "W7500x_gpio.h"

#include "common.h"
#include "W7500x_board.h"
#include "timerHandler.h"

static void PHY_Init(void);

GPIO_TypeDef* LED_PORT[LEDn] = {LED1_GPIO_PORT, LED2_GPIO_PORT};
const uint16_t LED_PIN[LEDn] = {LED1_PIN, LED2_PIN};
PAD_Type LED_PAD[LEDn] = {LED1_GPIO_PAD, LED2_GPIO_PAD};
PAD_AF_TypeDef LED_PAD_AF[LEDn] = {LED1_GPIO_PAD_AF, LED2_GPIO_PAD_AF};

/* W7500x Board Initialization */
void W7500x_Board_Init(void)
{
	/* PHY Initialization */
	PHY_Init();
	
	/* LEDs Initialization */
	LED_Init(LED1);
	LED_Init(LED2);
}

static void PHY_Init(void)
{
#ifdef __DEF_USED_IC101AG__ // For using W7500 + (IC+101AG Phy)
	*(volatile uint32_t *)(0x41003068) = 0x64; //TXD0 - set PAD strengh and pull-up
	*(volatile uint32_t *)(0x4100306C) = 0x64; //TXD1 - set PAD strengh and pull-up
	*(volatile uint32_t *)(0x41003070) = 0x64; //TXD2 - set PAD strengh and pull-up
	*(volatile uint32_t *)(0x41003074) = 0x64; //TXD3 - set PAD strengh and pull-up
	*(volatile uint32_t *)(0x41003050) = 0x64; //TXE  - set PAD strengh and pull-up
	
	//printf("\r\n[MCU: W7500]\r\n");
#endif

#ifdef __W7500P__ // W7500P
	*(volatile uint32_t *)(0x41003070) = 0x61;
	*(volatile uint32_t *)(0x41003054) = 0x61;
	
	//printf("\r\n[MCU: W7500P]\r\n");
#endif

#ifdef __DEF_USED_MDIO__ 
	/* mdio Init */
	mdio_init(GPIOB, MDC, MDIO);
#endif
}

/**
  * @brief  Configures LED GPIO.
  * @param  Led: Specifies the Led to be configured.
  *   This parameter can be one of following parameters:
  *     @arg WIZWIKI-W7500 board: LED_R / LED_G / LED_B
  *     @arg WIZWIKI-W7500ECO board: LED1 / LED2
  * @retval None
  */
void LED_Init(Led_TypeDef Led)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	if(Led >= LEDn) return;
	
	/* Configure the GPIO_LED pin */
	GPIO_InitStructure.GPIO_Pin = LED_PIN[Led];
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(LED_PORT[Led], &GPIO_InitStructure);
	PAD_AFConfig(LED_PAD[Led], LED_PIN[Led], LED_PAD_AF[Led]);
	
	/* LED off */
	GPIO_SetBits(LED_PORT[Led], LED_PIN[Led]);
}

/**
  * @brief  Turns selected LED On.
  * @param  Led: Specifies the Led to be set on.
  *   This parameter can be one of following parameters:
  *     @arg WIZWIKI-W7500 board: LED_R / LED_G / LED_B
  *     @arg WIZWIKI-W7500ECO board: LED1 / LED2
  * @retval None
  */
void LED_On(Led_TypeDef Led)
{
	if(Led >= LEDn) return;
	GPIO_ResetBits(LED_PORT[Led], LED_PIN[Led]);
}

/**
  * @brief  Turns selected LED Off.
  * @param  Led: Specifies the Led to be set off.
  *   This parameter can be one of following parameters:
  *     @arg WIZWIKI-W7500 board: LED_R / LED_G / LED_B
  *     @arg WIZWIKI-W7500ECO board: LED1 / LED2
  * @retval None
  */
void LED_Off(Led_TypeDef Led)
{
	if(Led >= LEDn) return;
	GPIO_SetBits(LED_PORT[Led], LED_PIN[Led]);
}

/**
  * @brief  Toggles the selected LED.
  * @param  Led: Specifies the Led to be toggled.
  *   This parameter can be one of following parameters:
  *     @arg WIZWIKI-W7500 board: LED_R / LED_G / LED_B
  *     @arg WIZWIKI-W7500ECO board: LED1 / LED2
  * @retval None
  */
void LED_Toggle(Led_TypeDef Led)
{
	if(Led >= LEDn) return;
	LED_PORT[Led]->DATAOUT ^= LED_PIN[Led];
}

/**
  * @brief  Get the selected LED status
  * @param  Led: Specifies the Led.
  *   This parameter can be one of following parameters:
  *     @arg WIZWIKI-W7500 board: LED_R / LED_G / LED_B
  *     @arg WIZWIKI-W7500ECO board: LED1 / LED2
  * @retval Status of LED (on / off)
  */
uint8_t get_LED_Status(Led_TypeDef Led)
{
	return GPIO_ReadOutputDataBit(LED_PORT[Led], LED_PIN[Led]);
}
