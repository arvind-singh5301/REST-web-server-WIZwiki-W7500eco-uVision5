#include "W7500x.h"
#include "W7500x_gpio.h"

#include "common.h"
#include "ConfigData.h"
#include "W7500x_board.h"
#include "gpioHandler.h"

#ifdef _GPIO_DEBUG_
	#include <stdio.h>
#endif

const uint16_t USER_IO_PIN[USER_IOn] =     {USER_IO_A_PIN, USER_IO_B_PIN, USER_IO_C_PIN, USER_IO_D_PIN};
GPIO_TypeDef*  USER_IO_PORT[USER_IOn] =    {USER_IO_A_PORT, USER_IO_B_PORT, USER_IO_C_PORT, USER_IO_D_PORT};
ADC_CH  USER_IO_ADC_CH[USER_IOn] =         {USER_IO_A_ADC_CH, USER_IO_B_ADC_CH, USER_IO_C_ADC_CH, USER_IO_D_ADC_CH};

uint8_t        USER_IO_SEL[USER_IOn] =     {USER_IO_A, USER_IO_B, USER_IO_C, USER_IO_D};
const char*    USER_IO_STR[USER_IOn] =     {"a\0", "b\0", "c\0", "d\0"};
const char*    USER_IO_PIN_STR[USER_IOn] = {"p30\0", "p29\0", "p28\0", "p27\0",}; 

/**
  * @brief  xxx Function
  */
void IO_Configuration(void)
{
	/* GPIOs Initialization */
	// Expansion GPIOs (4-pins, A to D)
	// Available to the ANALOG input pins or DIGITAL input/output pins
	if(get_user_io_enabled(USER_IO_A)) init_user_io(USER_IO_A);
	if(get_user_io_enabled(USER_IO_B)) init_user_io(USER_IO_B);
	if(get_user_io_enabled(USER_IO_C)) init_user_io(USER_IO_C);
	if(get_user_io_enabled(USER_IO_D)) init_user_io(USER_IO_D);
}

void init_user_io(uint8_t io_sel)
{
	struct __user_io_info *user_io_info = (struct __user_io_info *)&(get_DevConfig_pointer()->user_io_info);
	uint8_t idx = 0;
	GPIOMode_TypeDef gpio_mode;

	if((user_io_info->user_io_enable & io_sel) == io_sel)
	{
		idx = get_user_io_bitorder(io_sel);
		
		if((user_io_info->user_io_type & io_sel) == io_sel) // IO_ANALOG_IN == 1 
		{
			// Analog Input
			ADC_Init();
			GPIO_Configuration(USER_IO_PORT[idx], USER_IO_PIN[idx], GPIO_Mode_IN, USER_IO_AIN_PAD_AF);
		}
		else
		{
			// Digital Input / Output
			if((user_io_info->user_io_direction & io_sel) == io_sel) // IO_OUTPUT == 1
			{
				gpio_mode = GPIO_Mode_OUT;
			}
			else
			{
				gpio_mode = GPIO_Mode_IN;
			}
			
			GPIO_Configuration(USER_IO_PORT[idx], USER_IO_PIN[idx], gpio_mode, USER_IO_DEFAULT_PAD_AF);
			if(gpio_mode == GPIO_Mode_OUT) GPIO_ResetBits(USER_IO_PORT[idx], USER_IO_PIN[idx]); // Pin init (set to low)
		}
	}
}

uint8_t set_user_io_enable(uint8_t io_sel, uint8_t enable)
{
	struct __user_io_info *user_io_info = (struct __user_io_info *)&(get_DevConfig_pointer()->user_io_info);
	uint8_t ret = 1;
	
	
	if(enable == IO_ENABLE)
	{
		// IO enables
		user_io_info->user_io_enable |= io_sel;
	}
	else if(enable == IO_DISABLE)
	{
		// IO disables
		user_io_info->user_io_enable &= ~(io_sel);
	}
	else
		ret = 0;
	
	return ret;
}


uint8_t set_user_io_type(uint8_t io_sel, uint8_t type)
{
	struct __user_io_info *user_io_info = (struct __user_io_info *)&(get_DevConfig_pointer()->user_io_info);
	uint8_t ret = 1;
	
	
	if(type == IO_ANALOG_IN)
	{
		set_user_io_direction(io_sel, IO_INPUT);
		
		user_io_info->user_io_type |= io_sel;
		init_user_io(io_sel); // IO reinitialize
	}
	else if(type == IO_DIGITAL)
	{
		user_io_info->user_io_type &= ~(io_sel);
		init_user_io(io_sel); // IO reinitialize
	}
	else
		ret = 0;
	
	init_user_io(io_sel);
	
	return ret;
}


uint8_t set_user_io_direction(uint8_t io_sel, uint8_t dir)
{
	struct __user_io_info *user_io_info = (struct __user_io_info *)&(get_DevConfig_pointer()->user_io_info);
	uint8_t ret = 1;
	
	
	if(dir == IO_OUTPUT)
	{
		user_io_info->user_io_direction |= io_sel;
		init_user_io(io_sel); // IO reinitialize
	}
	else if(dir == IO_INPUT)
	{
		user_io_info->user_io_direction &= ~(io_sel);
		init_user_io(io_sel); // IO reinitialize
	}
	else
		ret = 0;
	
	init_user_io(io_sel);
	
	return ret;
}


uint8_t get_user_io_enabled(uint8_t io_sel)
{
	struct __user_io_info *user_io_info = (struct __user_io_info *)&(get_DevConfig_pointer()->user_io_info);
	uint8_t ret;
	
	if((user_io_info->user_io_enable & io_sel) == io_sel)
	{
		ret = IO_ENABLE;
	}
	else
	{
		ret = IO_DISABLE;
	}
	
	return ret;
}

uint8_t get_user_io_type(uint8_t io_sel)
{
	struct __user_io_info *user_io_info = (struct __user_io_info *)&(get_DevConfig_pointer()->user_io_info);
	uint8_t ret;
	
	if((user_io_info->user_io_type & io_sel) == io_sel)
	{
		ret = IO_ANALOG_IN;
	}
	else
	{
		ret = IO_DIGITAL;
	}
	
	return ret;
}


uint8_t get_user_io_direction(uint8_t io_sel)
{
	struct __user_io_info *user_io_info = (struct __user_io_info *)&(get_DevConfig_pointer()->user_io_info);
	uint8_t ret;
	
	if((user_io_info->user_io_direction & io_sel) == io_sel)
	{
		ret = IO_OUTPUT;
	}
	else
	{
		ret = IO_INPUT;
	}
	
	return ret;
}


// Analog input: 	Returns ADC value (12-bit resolution)
// Digital in/out: 	Returns I/O status to match the bit ordering
uint8_t get_user_io_val(uint16_t io_sel, uint16_t * val)
{
	struct __user_io_info *user_io_info = (struct __user_io_info *)&(get_DevConfig_pointer()->user_io_info);
	
	uint8_t ret = 0; // I/O Read failed
	uint8_t idx = 0;
	uint8_t status = 0;
	
	*val = 0;
	
	if((user_io_info->user_io_enable & io_sel) ==io_sel) // Enable
	{
		idx = get_user_io_bitorder(io_sel);
		
		if((user_io_info->user_io_type & io_sel) == io_sel) // IO_ANALOG == 1
		{
			// Analog Input: value
			*val = read_ADC(USER_IO_ADC_CH[idx]);
			//printf("===============> Analog input: %d <============\r\n", *val);
		}
		else // IO_DIGITAL == 0
		{
			// Digital Input / Output
			if((user_io_info->user_io_direction & io_sel) == io_sel) // IO_OUTPUT == 1
			{
				// Digital Output: status
				status = (uint16_t)GPIO_ReadOutputDataBit(USER_IO_PORT[idx], USER_IO_PIN[idx]);
				//printf("===============> Digital Output: %d <==============\r\n", status);
			}
			else // IO_INPUT == 0
			{
				// Digital Input: status
				status = (uint16_t)GPIO_ReadInputDataBit(USER_IO_PORT[idx], USER_IO_PIN[idx]);
				//printf("===============> Digital input: %d <==============\r\n", status);
			}
			
			//*val |= (status << i);
			*val = status;
		}
		
		ret = 1; // I/O Read success
	}
	
	return ret;
}

uint8_t set_user_io_val(uint16_t io_sel, uint16_t * val)
{
	struct __user_io_info *user_io_info = (struct __user_io_info *)&(get_DevConfig_pointer()->user_io_info);
	uint8_t ret = 0; // I/O Read failed
	uint8_t idx = 0;
	
	if((user_io_info->user_io_enable & io_sel) == io_sel) // Enable
	{
		// Digital output only (type == 0 && direction == 1)
		if(((user_io_info->user_io_type & io_sel) == IO_DIGITAL) && ((user_io_info->user_io_direction & io_sel) == io_sel))
		{
			idx = get_user_io_bitorder(io_sel);
			
			if(*val == 0) GPIO_ResetBits(USER_IO_PORT[idx], USER_IO_PIN[idx]);
			else if(*val == 1) GPIO_SetBits(USER_IO_PORT[idx], USER_IO_PIN[idx]);
			ret = 1;
		}
	}
	
	return ret;
}

uint8_t get_user_io_bitorder(uint16_t io_sel)
{
	uint8_t i;
	uint8_t ret = 0;
	
	for(i = 0; i < USER_IOn; i++)
	{
		if((io_sel >> i) == 1)
		{
			ret = i;
			break;
		}
	}
		
	return ret;
}

// 12-bit ADC resolution
uint16_t read_ADC(ADC_CH ch)
{
	ADC_ChannelSelect(ch);				///< Select ADC channel to CH0
	ADC_Start();						///< Start ADC
	while(ADC_IsEOC());					///< Wait until End of Conversion
	
	return ((uint16_t)ADC_ReadData());	///< read ADC Data
}

