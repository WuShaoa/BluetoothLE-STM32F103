#include "timer.h"

//////////////////////////////////////////////////////////////////////////////////
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F103开发板
//定时器 驱动代码
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/5/4
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved
//////////////////////////////////////////////////////////////////////////////////

extern vu16 USART3_RX_STA;
extern long int time_counter;
//配置TIM7预装载周期值
void TIM7_SetARR(u16 period)
{
    TIM_SetCounter(TIM7, 0); //计数器清空
    TIM7->ARR &= 0x00;     //先清预装载周期值为0
    TIM7->ARR |= period;   //更新预装载周期值

}

//定时器7中断服务程序
void TIM7_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET) //是更新中断
    {
        TIM_ClearITPendingBit(TIM7, TIM_IT_Update  ); //清除TIM7更新中断标志

        USART3_RX_STA |= 1 << 15; //标记接收完成
        TIM_Cmd(TIM7, DISABLE); //关闭TIM7
    }
}

//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
//通用定时器中断初始化
//APB1分频系数=AHB/APB时钟=2
//通用定时器时钟CK_INT=APB1时钟*分频系数=72M
//这里始终选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
void TIM7_Int_Init(u16 arr, u16 psc)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);//TIM7时钟使能

    //定时器TIM7初始化
    TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
    TIM_TimeBaseStructure.TIM_Prescaler = psc; //设置用来作为TIMx时钟频率除数的预分频值
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
    TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位

    TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE ); //使能指定的TIM7中断,允许更新中断

    TIM_Cmd(TIM7, ENABLE); //开启定时器7

    NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0 ; //抢占优先级0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  //子优先级2
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;   //IRQ通道使能
    NVIC_Init(&NVIC_InitStructure); //根据指定的参数初始化VIC寄存器

}

//通用定时器3初始化
//时钟选择为APB1的2倍
//arr:自动重装值
//psc:时钟预分频
//这里使用的是定时器3! 
void TIM3_Int_Init(u16 arr,u16 psc) 
{ 
 TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure; 
 NVIC_InitTypeDef NVIC_InitStructure; 
 RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟定时器3使能
          
//TIM3 初始化
 TIM_TimeBaseStructure.TIM_Period = arr;        //设置自动重装载寄存器周期值   
 TIM_TimeBaseStructure.TIM_Prescaler =psc;    //除数预分频值
 TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //时钟分割
 TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;    //TIM 向上计数
TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);                    //初始化TIM3   
 TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE );                          //允许更新中断  
 
 //中断优先级NVIC
 NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;                //TIM3 中断
 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;    //占先优先级0 
 NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;                //从优先级3
 NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;            //IRQ 通道使能
 NVIC_Init(&NVIC_InitStructure);                                                  //初始化 NVIC 
 
 TIM_Cmd(TIM3, ENABLE);                                                          //使能TIM3 
          
} 
//定时器3 中断服务
void TIM3_IRQHandler(void)      //TIM3 中断
{ 
 if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //检查 TIM3 更新中断发生
  { 
  TIM_ClearITPendingBit(TIM3, TIM_IT_Update    ); //清除更新终端标志
  time_counter++;
  } 
}

