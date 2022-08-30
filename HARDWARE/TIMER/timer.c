#include "timer.h"

//////////////////////////////////////////////////////////////////////////////////
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F103������
//��ʱ�� ��������
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/4
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved
//////////////////////////////////////////////////////////////////////////////////

extern vu16 USART3_RX_STA;
extern long int time_counter;
//����TIM7Ԥװ������ֵ
void TIM7_SetARR(u16 period)
{
    TIM_SetCounter(TIM7, 0); //���������
    TIM7->ARR &= 0x00;     //����Ԥװ������ֵΪ0
    TIM7->ARR |= period;   //����Ԥװ������ֵ

}

//��ʱ��7�жϷ������
void TIM7_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET) //�Ǹ����ж�
    {
        TIM_ClearITPendingBit(TIM7, TIM_IT_Update  ); //���TIM7�����жϱ�־

        USART3_RX_STA |= 1 << 15; //��ǽ������
        TIM_Cmd(TIM7, DISABLE); //�ر�TIM7
    }
}

//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz
//ͨ�ö�ʱ���жϳ�ʼ��
//APB1��Ƶϵ��=AHB/APBʱ��=2
//ͨ�ö�ʱ��ʱ��CK_INT=APB1ʱ��*��Ƶϵ��=72M
//����ʼ��ѡ��ΪAPB1��2������APB1Ϊ36M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
void TIM7_Int_Init(u16 arr, u16 psc)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);//TIM7ʱ��ʹ��

    //��ʱ��TIM7��ʼ��
    TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
    TIM_TimeBaseStructure.TIM_Prescaler = psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
    TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

    TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE ); //ʹ��ָ����TIM7�ж�,��������ж�

    TIM_Cmd(TIM7, ENABLE); //������ʱ��7

    NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0 ; //��ռ���ȼ�0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  //�����ȼ�2
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;   //IRQͨ��ʹ��
    NVIC_Init(&NVIC_InitStructure); //����ָ���Ĳ�����ʼ��VIC�Ĵ���

}

//ͨ�ö�ʱ��3��ʼ��
//ʱ��ѡ��ΪAPB1��2��
//arr:�Զ���װֵ
//psc:ʱ��Ԥ��Ƶ
//����ʹ�õ��Ƕ�ʱ��3! 
void TIM3_Int_Init(u16 arr,u16 psc) 
{ 
 TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure; 
 NVIC_InitTypeDef NVIC_InitStructure; 
 RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //ʱ�Ӷ�ʱ��3ʹ��
          
//TIM3 ��ʼ��
 TIM_TimeBaseStructure.TIM_Period = arr;        //�����Զ���װ�ؼĴ�������ֵ   
 TIM_TimeBaseStructure.TIM_Prescaler =psc;    //����Ԥ��Ƶֵ
 TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //ʱ�ӷָ�
 TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;    //TIM ���ϼ���
TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);                    //��ʼ��TIM3   
 TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE );                          //��������ж�  
 
 //�ж����ȼ�NVIC
 NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;                //TIM3 �ж�
 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;    //ռ�����ȼ�0 
 NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;                //�����ȼ�3
 NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;            //IRQ ͨ��ʹ��
 NVIC_Init(&NVIC_InitStructure);                                                  //��ʼ�� NVIC 
 
 TIM_Cmd(TIM3, ENABLE);                                                          //ʹ��TIM3 
          
} 
//��ʱ��3 �жϷ���
void TIM3_IRQHandler(void)      //TIM3 �ж�
{ 
 if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //��� TIM3 �����жϷ���
  { 
  TIM_ClearITPendingBit(TIM3, TIM_IT_Update    ); //��������ն˱�־
  time_counter++;
  } 
}

