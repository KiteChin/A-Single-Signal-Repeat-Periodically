#ifndef __TIM1_DAM_H
#define	__TIM1_DAM_H

#include "stm32f4xx.h"

typedef struct 
{
	u16 Value_1[200];
	u16 Value_2[200];
	u16 value_3[200];
}VALUE_ADC;

extern VALUE_ADC Value_ADC;
extern u16 Value_temp[1000];
extern u16 Wave_Size;
extern u8 Wave_Flag;
extern u8 Wave_Break_Flag;
extern u16 Wave_Star;
extern u16 Wave_End;
void Tim1_DMA_Config(uint32_t *value, uint32_t buffersize);

#endif /* __TIM1_DAM_H */
