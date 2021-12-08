 #include "adc.h"
 #include "delay.h"

	   
		   
//��ʼ��ADC
//�������ǽ��Թ���ͨ��Ϊ��
//����Ĭ�Ͻ�����ͨ��0~3		

/*
	ADCͨ����Ϊ����ͨ����ע��ͨ��
	STM32F103��3��ADC��ÿ��ADC��18��ͨ�����ɲ���16���ⲿ�źź�2���ڲ��ź�Դ
	ADC���Զ���ʹ�ã�Ҳ����˫��ʹ�ã���߲�����
	
*/
void  Adc_Init(void)
{ 	
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;
//------------------------(1)����PA��ʱ�Ӻ�AD1ʱ�ӣ�����PA1Ϊģ������------------------------------
//------------------------(2)��λADC1������ADC1��Ƶ����------------------------------
	/*	
	ʹ��GPIOA��ADCʱ�ӡ�
	Ҫʹ�ö˿ڸ��ã�Ҫʹ�ܶ˿ڵ�ʱ�ӡ�
	��������ADC������ʱ��ҲҪʹ�ܡ�
	*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_ADC1	, ENABLE );	  //ʹ��ADC1ͨ��ʱ��


	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //����ADC��Ƶ����6 72M/6=12,ADC���ʱ�䲻�ܳ���14M

	/*
		ADC1��ͨ��1��PA1�ϡ�
	  ����PA1Ϊģ�����룬ʹ��GPIO_Init������
	*/
	//PA1 ��Ϊģ��ͨ����������                         
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//ģ����������
	GPIO_Init(GPIOA, &GPIO_InitStructure);	

	ADC_DeInit(ADC1);  //��λADC1,������ ADC1 ��ȫ���Ĵ�������Ϊȱʡֵ
//------------------------(3)��ʼ��ADC1����������ADC1�Ĺ���ģʽ�Լ��������е������Ϣ��-----------------------------
/*
���õ���ת��ģʽ��������ʽѡ�����ݶ��뷽ʽ��ADC1�������е������Ϣ

*/
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC����ģʽ:ADC1��ADC2�����ڶ���ģʽ
	/*�Ƿ���ɨ��ģʽ����Ϊ�ǵ���ת��������ѡ�񲻿���*/
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//ģ��ת�������ڵ�ͨ��ģʽ
	/*���������Ƿ�������ת��ģʽ����Ϊ�ǵ���ת��ģʽ�����Բ�����*/
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//ģ��ת�������ڵ���ת��ģʽ
	/*����������������ת����ת�����ⲿ�¼�*/
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//ת��������������ⲿ��������
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC�����Ҷ���
	/*�������ù������еĳ���*/
	ADC_InitStructure.ADC_NbrOfChannel = 1;	//˳����й���ת����ADCͨ������Ŀ
	/*��һ��������ָ��ADC��*/
	ADC_Init(ADC1, &ADC_InitStructure);	//����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ���   

  //------------------------(4)ʹ��ADC��У׼-----------------------------
	/*
	ʹ��ADת������ִ�и�λУ׼��ADУ׼���������Ǳ���ģ���У׼�����½���ܲ�׼ȷ��
	
	*/
	ADC_Cmd(ADC1, ENABLE);	//ʹ��ָ����ADC1
	
	ADC_ResetCalibration(ADC1);	//ʹ�ܸ�λУ׼  
	 /*ÿ�ν���У׼֮��Ҫ�ȴ�У׼����*/
	while(ADC_GetResetCalibrationStatus(ADC1));	//�ȴ���λУ׼����
	
	ADC_StartCalibration(ADC1);	 //����ADУ׼
 
	while(ADC_GetCalibrationStatus(ADC1));	 //�ȴ�У׼����
 
//	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//ʹ��ָ����ADC1�����ת����������

}
  //------------------------(5)��ȡADCֵ-----------------------------
/*
  ���ù�������1�����ͨ��������˳���Լ�ͨ���Ĳ������ڣ�Ȼ������ADCת������ת������֮�󣬶�ȡADCת�������
  ��ȡͨ��1��ADCֵ��Get_Adc(1)

*/
//���ADCֵ
//ch:ͨ��ֵ 0~3
u16 Get_Adc(u8 ch)   
{
  	//����ָ��ADC�Ĺ�����ͨ����һ�����У�����ʱ��
	/*���ù�������ͨ���Լ��������ڣ��������еĵ�1��ת������������Ϊ239.5*/
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADCͨ��,����ʱ��Ϊ239.5����	  			    
  /*�������ADCת���ķ���*/
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//ʹ��ָ����ADC1�����ת����������	
	 /*��ADת���У�Ҫ��������״̬�Ĵ����ı�־λ����ȡADת���ĸ���״̬��Ϣ���������ж�ADC1��ת���Ƿ����*/
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//�ȴ�ת������

	return ADC_GetConversionValue(ADC1);	//�������һ��ADC1�������ת�����
}
/*
���ڶ�λ�ȡADCֵ��ȡƽ�����������׼ȷ��
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



























