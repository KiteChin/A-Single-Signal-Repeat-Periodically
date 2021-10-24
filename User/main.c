/**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2015-xx-xx
  * @brief   ADC例程
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火  STM32 F407 开发板
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */
  
#include "stm32f4xx.h"
#include "./usart/bsp_debug_usart.h"
#include "./adc/bsp_adc.h"
#include "./tim/Tim1_DMA.h"
#include "./dac/bsp_dac.h"

int main(void)
{	
	Value_size = 200;
  Debug_USART_Config();
	Rheostat_Init();	
	Tim1_DMA_Config((uint32_t *)Value_temp, 1000);
	DAC_DMA_Config((uint32_t *)&Value_ADC.Value_1, 200);
	while (1)
	{  
		if(Wave_Flag == 1)
		{
			uint16_t i = 0;
			for(i = 0; i < Value_size; i++)
			{
				Value_ADC.Value_1[i] = Value_temp[i+Wave_Star];
			}
			DMA_Cmd(DMA2_Stream5,ENABLE);
			Wave_Flag = 0;
		}
	}
}



/*********************************************END OF FILE**********************/

