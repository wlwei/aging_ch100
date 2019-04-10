#include "stm32f0xx.h"
#include "adc.h"
#include "math.h"
#define ADC1_DR_Address    0x40012440
#define TC_CAL1            ((uint16_t *)0x1FFFF7B8)
#define VREFINT_CAL        ((uint16_t *)0x1FFFF7BA)

union pfloat{
	float a;
	uint8_t b[4];
};

static const uint8_t buf_len = 192;
static uint16_t dma4adc[buf_len];
static uint8_t dma_complete_flag;

static ADC_InitTypeDef ADC_InitStructure;
static DMA_InitTypeDef DMA_InitStructure;
static GPIO_InitTypeDef GPIO_InitStructure;

static uint16_t adcValue,tempValue,vrefValue;
//For curve correction
static float sa=0.6,sb=-0.5;
static float ta=0,tb=1;
static float ha=0,hb=1;
static float vout,vdd=3.3;

union pfloat prcv[16];

static uint16_t rstd = 1500;
static uint16_t rl = 470;
	
/*********DMA Complete flag set ********/
void setDmaCompleteFlag(void)
{
	dma_complete_flag=1;
}

float fx(float t,float h,float s)
{
	return((ta*t+tb)*(ha*h+hb)*sa*pow(s,sb));
}

/**********adc data processing*************/
/*****channel1 with 16bits
******temperature with 12bits
******reference voltage1.2V with 16bits
--by lwang @home 2018/03/12 
******************************************/
float adcDataProcess(uint16_t *pADC)
{
	uint32_t tmp1=0,tmp2=0,tmp3=0;
	uint16_t i,tDREF,vDREF;
	float vREF,rs;
	float x0,x1,x2,y0,y1;
	float temp,humi;
	
	union pfloat temp_1,humi_1,xroot;

	if(dma_complete_flag == 1)
		{
			dma_complete_flag = 0;
			tDREF = *TC_CAL1;
			vDREF = *VREFINT_CAL;
			vREF = 1.2;
			for(i=0;i<buf_len/3;i++)
			{
				tmp1 += dma4adc[0+3*i]&0xFFF;
				tmp2 += dma4adc[1+3*i]&0xFFF;
				tmp3 += dma4adc[2+3*i]&0xFFF;
			}
			adcValue = tmp1/(buf_len/3);
			tempValue = tmp2/(buf_len/3);
			vrefValue = tmp3/(buf_len/3);
			 
			/*temp.a = (tDREF-tempValue)/4.3+30; //temp
			*pADC++ = (temp.b[3]<<8) | temp.b[2];
			*pADC++ = (temp.b[1]<<8) | temp.b[0];
			
			iref.a = vrefValue*3.3/4095; //vref
			*pADC++ = (iref.b[3]<<8) | iref.b[2];
			*pADC++ = (iref.b[1]<<8) | iref.b[0];
			
			
			adc1.a = adcValue*vREF/vDREF;//channel 1
	    *pADC++ = (adc1.b[3]<<8) | adc1.b[2];
	    *pADC   = (adc1.b[1]<<8) | adc1.b[0];
			*/
			//vout = adcValue*vREF/vDREF;//channel 1
		}
			
		temp_1.b[3] = (*(pADC+2)>>8) & 0xFF;
		temp_1.b[2] = *(pADC+2) & 0xFF;
		temp_1.b[1] = (*(pADC+3)>>8) & 0xFF;
		temp_1.b[0] = *(pADC+3) & 0xFF;
		
		humi_1.b[3] = (*(pADC+4)>>8) & 0xFF;
		humi_1.b[2] = *(pADC+4) & 0xFF;
		humi_1.b[1] = (*(pADC+5)>>8) & 0xFF;
		humi_1.b[0] = *(pADC+5) & 0xFF;
		
		temp = temp_1.a;
		humi = humi_1.a;
		if (*(pADC+15)==0x0F0F)
		{
			rstd = *(pADC+16);
			rl = *(pADC+17);
			for(i=0;i<6;i++)
			{
				prcv[i].b[3] = (*(pADC+18+2*i)>>8) & 0xFF;
				prcv[i].b[2] = *(pADC+18+2*i) & 0xFF;
				prcv[i].b[1] = (*(pADC+18+2*i+1)>>8) & 0xFF;
				prcv[i].b[0] = *(pADC+18+2*i+1) & 0xFF;
			}
			sa = prcv[0].a;
			sb = prcv[1].a;
		
			ta = prcv[2].a;
			tb = prcv[3].a;

			ha = prcv[4].a;
			hb = prcv[5].a;
		}
		
		rs = (4095.0-adcValue)/adcValue*rl;
		*(pADC+6)   = (uint16_t)rs;
		
		x0 = 0;x1 = 0.01;x2 = 100.0;
		do
		{
			x0 = (x1+x2)/2.0;
			y0 = fx(temp,humi,x0)-rs/rstd;
			y1 = fx(temp,humi,x1)-rs/rstd;
			//y0 = (ta*temp+tb)*(ha*humi+hb)*sa*pow(x0,sb)-rs/rstd;
			if (y0*y1>0)
			{
				x1 = x0;
			}
			else
			{
				x2 = x0;
			}
		}while( (x1-x2>0.001) || (x1-x2<-0.001) );
		
		//xroot.a = pow(1.5,2.0);
		xroot.a = x0;
		*pADC = (xroot.b[3]<<8) | xroot.b[2];
		*(pADC+1) = (xroot.b[1]<<8) | xroot.b[0];
	
	return adcValue;
}


void adc_GPIO_cfg(void)
{
	/*********** Settings for ADC *****************/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
}

/*************ADC conversion config********************/
/********Turn on CH1,CH16,CH17*************************/
/********Measure external,temp,vref********************/
void ADC_cfg()
{
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_Init(ADC1, &ADC_InitStructure);
	
	ADC_ClockModeConfig(ADC1,ADC_ClockMode_SynClkDiv4); //PCLK/4
	ADC_ChannelConfig(ADC1,ADC_Channel_0 | ADC_Channel_16 | ADC_Channel_17,ADC_SampleTime_239_5Cycles);
	ADC_TempSensorCmd(ENABLE);
	ADC_VrefintCmd(ENABLE);
	ADC_GetCalibrationFactor(ADC1);
	//ADC_ITConfig(ADC1,ADC_IT_EOC,ENABLE);
	
  ADC_DMARequestModeConfig(ADC1, ADC_DMAMode_Circular);
  ADC_DMACmd(ADC1, ENABLE);
	ADC_Cmd(ADC1,ENABLE);
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_ADRDY)); 
	
}
/***************DMA control,256 data to average**********/
void DMA_cfg()
{
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)dma4adc;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = buf_len;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	DMA_ITConfig(DMA1_Channel1,DMA_IT_TC,ENABLE);
	DMA_Cmd(DMA1_Channel1, ENABLE);
}

/********ADC interupt processing*****************/
void ADC1_IRQHandler(void)
{
	
}
/********DMA interupt processing*****************/
void DMA1_Channel1_IRQHandler(void)
{
	if(DMA_GetFlagStatus(DMA1_FLAG_TC1) != RESET)
	{
		DMA_ClearITPendingBit(DMA1_IT_TC1);
		setDmaCompleteFlag();
	}
}
