#include "./tim/Tim1_DMA.h"


//临时数据和存储
VALUE_ADC Value_ADC;
u16 Value_temp[1000] = {0};
u16 Value_size = 200;
u8 Wave_Flag = 0;
u16 Wave_Star = 0;
u16 Wave_End = 0;
static void Tim1_DMA_NVIC(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  /* 配置子优先级：1 */
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  /* 使能中断通道 */
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);	
}

void Tim1_DMA_Config(uint32_t *value, uint32_t buffersize)
{
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 , ENABLE);	
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(ADC1->DR) ;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)value;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = buffersize;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_InitStructure.DMA_Channel = DMA_Channel_6;  
	DMA_Init(DMA2_Stream5, &DMA_InitStructure);
	DMA_Cmd(DMA2_Stream5,ENABLE);
	
	Tim1_DMA_NVIC();
	
	DMA_ITConfig(DMA2_Stream5,DMA_IT_TC,ENABLE);
	
	
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);
  TIM_TimeBaseStructure.TIM_Period = 168 - 1;       
	// 高级控制定时器时钟源TIMxCLK = HCLK=168MHz 
	// 设定定时器频率为=TIMxCLK/(TIM_Prescaler+1)=10000Hz
  TIM_TimeBaseStructure.TIM_Prescaler = 1 - 1;	
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;

  TIM_TimeBaseStructure.TIM_RepetitionCounter=0;
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseStructure);
	TIM_Cmd(TIM1,ENABLE);
	
	
	TIM_DMACmd(TIM1,TIM_DMA_Update,ENABLE);
	
}

void DMA2_Stream5_IRQHandler(void)
{
	DMA_Cmd(DMA2_Stream5,DISABLE);
	if(DMA_GetITStatus(DMA2_Stream5,DMA_IT_TCIF5))
	{
		uint16_t i = 0;
		uint16_t num = 0;
		uint8_t flag = 0;
		uint8_t useful_num = 0;
		for(i = 0; i < 1000-1; i++)
		{
			if(Value_temp[i] > 0x600 && Value_temp[i]< 0x700)
//			if(Value_temp[i] - Value_temp[i+1] < 0x10)
			{
				num++;
				useful_num = 0;
			}
			else
			{
				useful_num++;
				if(useful_num == 5 && flag == 0)
				{
					Wave_Star = i - useful_num + 1;
					flag = 1;
				}
//				if(flag ==0)
//				{
//					Wave_Star = i;
//					flag = 1;
//				}
				
			}

		}
		if(num < 1000*19/20) 
			Wave_Flag = 1;
	}
	DMA_ClearFlag(DMA2_Stream5,DMA_IT_TCIF5);
	if(Wave_Flag != 1)
		DMA_Cmd(DMA2_Stream5,ENABLE);
}
