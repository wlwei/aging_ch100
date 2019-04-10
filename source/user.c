//#include "stdint.h"
#include "stm32f0xx.h"
//#include "stm32f0xx_conf.h"
#include "user_init.h"

#include "adc.h"
#include "metronome.h"
#include "eeprom_emulation.h"

#include "i2c_slave.h"

#define TC_CAL1            ((uint16_t *)0x1FFFF7B8)
#define VREFINT_CAL            ((uint16_t *)0x1FFFF7BA)

/* ----------------------- Defines ------------------------------------------*/
//const uint8_t REG_INPUT_START = 0;
//const uint8_t REG_INPUT_NREGS = 8;
const uint8_t REG_HOLDING_START = 0;
const uint8_t REG_HOLDING_NREGS = 32;
float T,H;
/* ----------------------- Static variables ---------------------------------*/
//static uint16_t   usRegInputStart = REG_INPUT_START;
//static uint16_t   usRegInputBuf[REG_INPUT_NREGS];
static uint16_t   usRegHoldingStart = REG_HOLDING_START;
static uint16_t   usRegHoldingBuf[REG_HOLDING_NREGS];
/* ----------------------- Start implementation -----------------------------*/
int main( void )
{
	
	static uint16_t tmp1,tmp2;
	uint8_t i;
	//static int16_t sens_coff[8];
	//static int16_t temp_coff[8];
	//static int16_t humi_coff[4];
	//static uint16_t gain,rs,rl;
	static float f1,v0;
	
	//clock settings and interrupt management
	RCC_cfg();
	NVIC_cfg();
	//For ADC
	adc_GPIO_cfg();
	ADC_cfg();
	DMA_cfg();
	//For Modbus
	//usart1_GPIO_cfg();
	//mb_USART_cfg();
	//mb_TIM16_cfg();
	
	i2c1_GPIO_cfg();
	i2c1_cfg();
	
	//For led toggle
	mtn_TIM17_cfg();
	//led_GPIO_cfg();
	//hdc_TIM14_cfg();

	ADC_StartOfConversion(ADC1);
	
	rd_coff(&usRegHoldingBuf[15]);
	usRegHoldingBuf[4] = 0x1888;
	usRegHoldingBuf[18] = 0x3F14;
	usRegHoldingBuf[19] = 0x7AE1;

  while(1)
  {
		//mb_Service();		//modbus service

		(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4))?I2C_Cmd(I2C1,ENABLE):I2C_Cmd(I2C1,DISABLE);
		
		if(usRegHoldingBuf[30]==0x9999)
		{
			wr_coff(&usRegHoldingBuf[15]);
		}
		//using timer operate the step every 3second
		if(time2tell()==1)
		{

			v0 = adcDataProcess(usRegHoldingBuf); //RS update adc data
			//0.58 big-endian
			
		}
  }
}

void i2cRegUpdate(uint8_t *p,uint16_t regAddr,uint16_t regNum,uint8_t mode)
{
	uint16_t i;
	uint16_t iRegIndex;
	if((regAddr >= REG_HOLDING_START) && ((regAddr+regNum) <= (REG_HOLDING_START+REG_HOLDING_NREGS)))
	{
		iRegIndex = (uint16_t)(regAddr-usRegHoldingStart);
		
		switch(mode)
		{
			case 1:
			{
				for(i=0;i<regNum;i++)
				{
					*p++ = (uint8_t)(usRegHoldingBuf[iRegIndex+i] >> 8 );  //High byte first
					*p++ = (uint8_t)(usRegHoldingBuf[iRegIndex+i] & 0xFF); //low byte first
				}
			}
			break;
			case 0:
			{
				for(i=0;i<regNum;i++)
				{
						*(usRegHoldingBuf+iRegIndex+i)= ((*p++))<<8; 	//High byte
						*(usRegHoldingBuf+iRegIndex+i)|= *(p++); 			//Low byte
				}
				break;

			}	
		}
	}
}

void SystemInit()
{
	
}
