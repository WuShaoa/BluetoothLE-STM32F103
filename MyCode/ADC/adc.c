 #include "adc.h"
 #include "delay.h"

	   
		   
//初始化ADC
//这里我们仅以规则通道为例
//我们默认将开启通道0~3		

/*
	ADC通道分为规则通道和注入通道
	STM32F103有3个ADC，每个ADC有18个通道，可测量16个外部信号和2个内部信号源
	ADC可以独立使用，也可以双重使用，提高采样率
	
*/
void  Adc_Init(void)
{ 	
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;
//------------------------(1)开启PA口时钟和AD1时钟，设置PA1为模拟输入------------------------------
//------------------------(2)复位ADC1，设置ADC1分频因子------------------------------
	/*	
	使能GPIOA和ADC时钟。
	要使用端口复用，要使能端口的时钟。
	复用外设ADC，外设时钟也要使能。
	*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_ADC1	, ENABLE );	  //使能ADC1通道时钟


	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M

	/*
		ADC1的通道1在PA1上。
	  设置PA1为模拟输入，使用GPIO_Init函数。
	*/
	//PA1 作为模拟通道输入引脚                         
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//模拟输入引脚
	GPIO_Init(GPIOA, &GPIO_InitStructure);	

	ADC_DeInit(ADC1);  //复位ADC1,将外设 ADC1 的全部寄存器重设为缺省值
//------------------------(3)初始化ADC1参数，设置ADC1的工作模式以及规则序列的相关信息。-----------------------------
/*
设置单次转换模式、触发方式选择、数据对齐方式、ADC1规则序列的相关信息

*/
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC工作模式:ADC1和ADC2工作在独立模式
	/*是否开启扫描模式，因为是单次转换，所以选择不开启*/
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//模数转换工作在单通道模式
	/*用来设置是否开启连续转换模式，因为是单次转换模式，所以不开启*/
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//模数转换工作在单次转换模式
	/*用来设置启动规则转换组转换的外部事件*/
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC数据右对齐
	/*用来设置规则序列的长度*/
	ADC_InitStructure.ADC_NbrOfChannel = 1;	//顺序进行规则转换的ADC通道的数目
	/*第一个参数是指定ADC号*/
	ADC_Init(ADC1, &ADC_InitStructure);	//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器   

  //------------------------(4)使能ADC并校准-----------------------------
	/*
	使能AD转换器，执行复位校准和AD校准，这两步是必须的，不校准将导致结果很不准确。
	
	*/
	ADC_Cmd(ADC1, ENABLE);	//使能指定的ADC1
	
	ADC_ResetCalibration(ADC1);	//使能复位校准  
	 /*每次进行校准之后要等待校准结束*/
	while(ADC_GetResetCalibrationStatus(ADC1));	//等待复位校准结束
	
	ADC_StartCalibration(ADC1);	 //开启AD校准
 
	while(ADC_GetCalibrationStatus(ADC1));	 //等待校准结束
 
//	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能

}
  //------------------------(5)读取ADC值-----------------------------
/*
  设置规则序列1里面的通道，采样顺序，以及通道的采样周期，然后启动ADC转换。在转换结束之后，读取ADC转换结果。
  读取通道1的ADC值，Get_Adc(1)

*/
//获得ADC值
//ch:通道值 0~3
u16 Get_Adc(u8 ch)   
{
  	//设置指定ADC的规则组通道，一个序列，采样时间
	/*设置规则序列通道以及采样周期，规则序列的第1个转换，采样周期为239.5*/
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道,采样时间为239.5周期	  			    
  /*软件开启ADC转换的方法*/
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能	
	 /*在AD转换中，要根据数据状态寄存器的标志位来获取AD转换的各个状态信息。以下是判断ADC1的转换是否结束*/
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束

	return ADC_GetConversionValue(ADC1);	//返回最近一次ADC1规则组的转换结果
}
/*
用于多次获取ADC值，取平均，用来提高准确率
*/

u16 Get_Adc_Average(u8 ch,u8 times)
{
	u32 temp_val=0;
	u8 t;
	for(t=0;t<times;t++)
	{
		temp_val+=Get_Adc(ch);
		delay_ms(5);
	}
	return temp_val/times;
} 	 



























