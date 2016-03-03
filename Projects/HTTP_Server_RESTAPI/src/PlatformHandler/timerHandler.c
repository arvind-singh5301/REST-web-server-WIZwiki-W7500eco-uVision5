#include "W7500x_dualtimer.h"

#include "common.h"
#include "W7500x_board.h"
#include "timerHandler.h"
#include "dhcp.h"

static volatile uint16_t msec_cnt = 0;
static volatile uint8_t  sec_cnt = 0;
static volatile uint8_t  min_cnt = 0;
static volatile uint32_t hour_cnt = 0;

// For main routine
extern uint8_t flag_application_running;
uint8_t flag_check_main_routine = 0;
volatile uint16_t main_routine_check_time_msec = 0;

void Timer_Configuration(void)
{
	DUALTIMER_InitTypDef Dualtimer_InitStructure;
	
	NVIC_EnableIRQ(DUALTIMER0_IRQn);
	
	/* Dualtimer 0_0 clock enable */
	DUALTIMER_ClockEnable(DUALTIMER0_0);

	/* Dualtimer 0_0 configuration */
	Dualtimer_InitStructure.TimerLoad = 0x0000BB80;
	Dualtimer_InitStructure.TimerControl_Mode = DUALTIMER_TimerControl_Periodic;
	Dualtimer_InitStructure.TimerControl_OneShot = DUALTIMER_TimerControl_Wrapping;
	Dualtimer_InitStructure.TimerControl_Pre = DUALTIMER_TimerControl_Pre_1;
	Dualtimer_InitStructure.TimerControl_Size = DUALTIMER_TimerControl_Size_32;

	DUALTIMER_Init(DUALTIMER0_0, &Dualtimer_InitStructure);

	/* Dualtimer 0_0 Interrupt enable */
	DUALTIMER_IntConfig(DUALTIMER0_0, ENABLE);

	/* Dualtimer 0_0 start */
	DUALTIMER_Start(DUALTIMER0_0);
}

void Timer_IRQ_Handler(void)
{
	if(DUALTIMER_GetIntStatus(DUALTIMER0_0))
	{
		DUALTIMER_IntClear(DUALTIMER0_0);
		
		msec_cnt++; // millisecond counter
		
		if(flag_application_running)
		{
			if(++main_routine_check_time_msec >= MAIN_ROUTINE_CHECK_CYCLE_MSEC)
			{
				flag_check_main_routine = 1;
				main_routine_check_time_msec = 0;
			}
		}
		
		/* Second Process */
		if(msec_cnt >= 1000 - 1) //second //if((msec_cnt % 1000) == 0) 
		{
			msec_cnt = 0;
			sec_cnt++;
			
			DHCP_time_handler();	// Time counter for DHCP timeout
		}
		
		/* Minute Process */
		if(sec_cnt >= 60) //if((sec_cnt % 60) == 0) 
		{
			sec_cnt = 0;
			min_cnt++;
		}
		
		/* Hour Process */
		if(min_cnt >= 60)
		{
			min_cnt = 0;
			hour_cnt++;
		}
	}

	if(DUALTIMER_GetIntStatus(DUALTIMER0_1))
	{
		DUALTIMER_IntClear(DUALTIMER0_1);
	}
}

uint32_t getDeviceUptime_hour(void)
{
	return hour_cnt;
}

uint8_t getDeviceUptime_min(void)
{
	return min_cnt;
}

uint8_t getDeviceUptime_sec(void)
{
	return sec_cnt;
}

uint16_t getDeviceUptime_msec(void)
{
	return msec_cnt;
}
