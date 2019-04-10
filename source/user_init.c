#include "stm32f0xx.h"
#include "user_init.h"

static NVIC_InitTypeDef NVIC_InitStructure;

/*void print2usart(uint16_t value)
{
	uint8_t i=0;
	uint8_t a[5];
		
	for(i=0;i<5;i++)
	{
		a[i] = value % 10;
		value /= 10;
	}
	for(i=0;i<5;i++)
	{
		USART_SendData(USART1,a[4-i]+48);
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	}
	USART_SendData(USART1,'\n');
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}
*/
void RCC_cfg()
{
	/*******Using HSI = 8MHz***********/
	/*RCC_PLLConfig(RCC_PLLSource_HSI_Div2,RCC_PLLMul_6);//HSI=8MHz,HSI/2*6
	RCC_HSEConfig(RCC_HSE_OFF);*/
	
	/****Using HSE***When select HSE, freq is much slower than expect??
	RCC_HSICmd(DISABLE);
	RCC_HSEConfig(RCC_HSE_ON);
	RCC_WaitForHSEStartUp();
	RCC_PREDIV1Config(RCC_PREDIV1_Div2);//12MHz/2=6MHz
	RCC_PLLConfig(RCC_PLLSource_PREDIV1,RCC_PLLMul_4);//4*6=24MHz*/
	RCC_HSEConfig(RCC_HSE_OFF);
	
	RCC_PLLCmd(DISABLE);
	RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI); //sysclk=8MHz
	RCC_HCLKConfig(RCC_SYSCLK_Div1); 	//Hclk=8MHz
	RCC_PCLKConfig(RCC_HCLK_Div2); 		//Pclk=4MHz*/
	
	/*RCC_HSEConfig(RCC_HSE_OFF);
	RCC_PLLConfig(RCC_PLLSource_HSI_Div2,RCC_PLLMul_3);//HSI=8MHz,HSI/2*3
	RCC_PLLCmd(ENABLE);
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK); //sysclk=12MHz
	RCC_HCLKConfig(RCC_SYSCLK_Div2); 	//Hclk=6MHz
	RCC_PCLKConfig(RCC_HCLK_Div1); 		//Pclk=6MHz*/
	
	//RCC_ADCCLKConfig(RCC_ADCCLK_PCLK_Div2); //ADC_clk=6MHz
	
	/*****stm030f4 pehri**************************************
	 ***APB1: TIM3,TIM14,WWDG ********************************
	 ***APB2: TIM1,TIM16,TIM17, USART1,SPI1,ADC  *************
	 *****stm030C8 pehri**************************************
	 ***APB1: TIM3,TIM6,TIM14, USART2,I2C1,I2C2,SPI2,WWDG ****
	 ***APB2: TIM1,TIM16,TIM17, USART1,SPI1,ADC **************
	 *******By lwang @home,2018/03/14*************************/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14 | RCC_APB1Periph_I2C1,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14 | RCC_APB1Periph_I2C1,DISABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14 | RCC_APB1Periph_I2C1,ENABLE);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_ADC1 | \
	                       RCC_APB2Periph_TIM16 | RCC_APB2Periph_TIM17 | \
												 RCC_APB2Periph_SYSCFG,ENABLE);

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_DMA1,ENABLE);
	
}
void NVIC_cfg()
{
	NVIC_InitStructure.NVIC_IRQChannel = I2C1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	/*************For infrared receive************/
	NVIC_InitStructure.NVIC_IRQChannel = TIM14_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 4;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure); 
	/*************For modbus***********************/
	NVIC_InitStructure.NVIC_IRQChannel = TIM16_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	/*************For Led***********************/
	NVIC_InitStructure.NVIC_IRQChannel = TIM17_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 5;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	/*NVIC_InitStructure.NVIC_IRQChannel = EXTI4_15_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);*/
}

