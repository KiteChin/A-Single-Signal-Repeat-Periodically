#include "./tim/Tim1_DMA.h"


//临时数据和存储
VALUE_ADC Value_ADC;
u16 Value_temp[1000] = {0};
u16 Wave_Size = 0;
u16 Wave_Star = 0;
u16 Wave_End = 0;
u8 Wave_Flag = 0;
u8 Wave_Break_Flag = 0;
u16 Wave_Num = 0;

u16 Wave_Stable = 0;

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
	
	DMA_ITConfig(DMA2_Stream5,DMA_IT_TC|DMA_IT_HT,ENABLE);
	
	
	
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

__INLINE static u16 Get_Fre(uint16_t Value_Offset)
{
	uint16_t Stab_Val = 0;
	uint8_t Stab_Num = 0;

	for(u8 i = 0; i < 20; i++)
	{
		int16_t temp = Value_temp[i*25 + Value_Offset] - Value_temp[(20-i)*25 + Value_Offset - 1];
		if(temp > -0x100 && temp < 0x100)
		{
			Stab_Val = Value_temp[(20-i)*25 + Value_Offset];
			Stab_Num++;
		}else 
		{
		}
	}

	if(Stab_Num != 20)
	{
		return Stab_Val;
	}else return 0;
}	


	

void DMA2_Stream5_IRQHandler(void)
{
	uint16_t Wave_Num = 0;
	uint16_t Value_Offset = 0;

	if(DMA_GetITStatus(DMA2_Stream5,DMA_IT_HTIF5))
	{
		Value_Offset = 0;
		uint16_t Value_Stable = Get_Fre(Value_Offset);
		static uint8_t HT_Star_Flag = 0;
		if(HT_Star_Flag == 1 && Wave_Star != 0)
		{
			Wave_Star = 0;
			HT_Star_Flag = 0;
		}
		if(Value_Stable != 0)
		{
			uint16_t Start_Num = 0;
			uint16_t End_Num = 0;
			for(u16 i = 0; i < 500; i++)
			{
				if(Value_temp[i] > Value_Stable - 30 && Value_temp[i] < Value_Stable + 30)
				{
					if(Wave_Star != 0)
					{
						End_Num++;
						if(End_Num == 5)
						{
							Wave_End = i - Start_Num + 1;
							Wave_Stable = Value_Stable;
						}				
					}
					
				}
				else
				{
					Wave_Num++;
					if(Wave_Star == 0)
					{
						Start_Num++;
						if(Start_Num == 5)
						{
							Wave_Star = i - Start_Num + 1;
							HT_Star_Flag = 1;
						}
					}			
				}
			}
		}
		
	}
	else if (DMA_GetITStatus(DMA2_Stream5,DMA_IT_TCIF5))
	{
		Value_Offset = 500;
		uint16_t Value_Stable = Get_Fre(Value_Offset);
		static uint8_t TC_Star_Flag = 0;
		if(TC_Star_Flag == 1 && Wave_Star != 0) 
		{
			Wave_Star = 0;
			TC_Star_Flag = 0;
		}
		if(Value_Stable != 0)
		{
			uint16_t Start_Num = 0;
			uint16_t End_Num = 0;
			for(u16 i = 500; i < 1000; i++)
			{
				if(Value_temp[i] > Value_Stable - 30 && Value_temp[i] < Value_Stable + 30)
				{
					if(Wave_Star != 0)
					{
						End_Num++;
						if(End_Num == 5)
						{
							Wave_End = i - Start_Num + 1;
							Wave_Stable = Value_Stable;
						}				
					}
					
				}
				else
				{
					Wave_Num++;
					if(Wave_Star == 0)
					{
						Start_Num++;
						if(Start_Num == 5)
						{
							Wave_Star = i - Start_Num + 1;
							TC_Star_Flag = 1;
						}
					}			
				}
			}

		}		
	}
	if(Wave_Star && Wave_End)
	{
		if(Wave_Star < Wave_End) 
		{
			Wave_Size = Wave_End - Wave_Star;
		}else
		{
			Wave_Size = Wave_End + 1000 - Wave_Star;
			Wave_Break_Flag = 1;
		}
		Wave_Flag = 1;
		
		
	}	
	
	DMA_ClearFlag(DMA2_Stream5,DMA_IT_TCIF5|DMA_IT_HTIF5);
}
