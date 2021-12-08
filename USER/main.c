#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "ble_app.h"
#include "usart3.h"

//for debugging utilities
#define DEBUG_VERSION

#if defined (DEBUG_VERSION)
#include <time.h>
#endif
/************************************************************************************
							本例程提供自以下店铺：
								Ilovemcu.taobao.com
							实验相关外围扩展模块均来自以上店铺
							作者：神秘藏宝室							
*************************************************************************************/
#include "stm32f10x.h"

#include "FSR.h"

#include "adc.h"
#include "ble_app.h"

// 角速度传感器

#include "usart2.h"

#include "imu901.h"

//下面4项内容需要根据实际型号和量程修正

//最小量程 根据具体型号对应手册获取,单位是g，这里以RP-18.3-ST型号为例，最小量程是20g
#define PRESS_MIN 20
//最大量程 根据具体型号对应手册获取,单位是g，这里以RP-18.3-ST型号为例，最大量程是6kg
#define PRESS_MAX 6000

//以下2个参数根据获取方法：
//理论上：
// 1.薄膜压力传感器不是精准的压力测试传感器，只适合粗略测量压力用，不能当压力计精确测量。
// 2. AO引脚输出的电压有效范围是0.1v到3.3v，而实际根据不同传感器范围会在这个范围内，并不一定是最大值3.3v，也可能低于3.3v，要实际万用表测量，
// 	例程只是给出理论值，想要精确请自行万用表测量然后修正以下2个AO引脚电压输出的最大和最小值
//调节方法：
//薄膜压力传感器的AO引脚输出的增益范围是通过板载AO_RES电位器调节实现的，
//想要稍微精准点，需要自己给定具体已知力，然后调节AO_RES电位器到串口输出重量正好是自己给定力就可以了
#define VOLTAGE_MIN 150
#define VOLTAGE_MAX 3300
#define MAX_BLE_SEND 100
u8 state = 0;
u16 val = 0;
u16 value_AD = 0;

long PRESS_AO = 0;
int VOLTAGE_AO = 0;

long map(long x, long in_min, long in_max, long out_min, long out_max);

//@int main(void)
//@{
//@	delay_init();				//延时函数初始化
//@	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
//@	uart_init(9600);	 //串口初始化波特率为9600
//@	/*通道初始化*/
//@	Adc_Init();
//@
//@	delay_ms(1000);
//@
//@	//TODO data passing
//@
//@  LED_Init();                                     //LED初始化
//@  KEY_Init();                                     //KEY初始化
//@	//-Ble_IoInit(); //BLE???u?'??
//@  //-Ble_SetInit();//BLE????????
//@	printf("Test start\r\n");
//@	while(1)
//@	{
//@		/*多次获取ADC1的值，取平均*/
//@		value_AD = Get_Adc_Average(1,10);	//10次平均值
//@		/*AO引脚输出的电压有效范围是0.1v到3.3v*/
//@		VOLTAGE_AO = map(value_AD, 0, 4095, 0, 3300);
//@		if(VOLTAGE_AO < VOLTAGE_MIN)
//@		{
//@			PRESS_AO = 0;
//@		}
//@		else if(VOLTAGE_AO > VOLTAGE_MAX)
//@		{
//@			PRESS_AO = PRESS_MAX;
//@		}
//@		else
//@		{
//@			/*将[VOLTAGE_MIN, VOLTAGE_MAX]范围内的VOLTAGE_AO映射到[PRESS_MIN, PRESS_MAX]内*/
//@			PRESS_AO = map(VOLTAGE_AO, VOLTAGE_MIN, VOLTAGE_MAX, PRESS_MIN, PRESS_MAX);
//@		}
//@		//-Ble_Test(PRESS_AO); //sending data by ble
//@		printf("AD值 = %d,电压 = %d mv,压力 = %ld g\r\n",value_AD,VOLTAGE_AO,PRESS_AO);
//@
//@		delay_ms(10);// round faster
//@	}
//@
//@}
//@

/*
将[in_min, in_max]范围内的x等比映射到[out_min, out_max]内
*/
long map(long x, long in_min, long in_max, long out_min, long out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int main(void)
{
	int count = 0;
	uint32_t times = 0;
	uint8_t ch;

	delay_init();									//延时函数初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
													//@	uart_init(9600);	 //串口初始化波特率为9600
													//@	/*通道初始化*/
	Adc_Init();

	//sys_stm32_clock_init(RCC_PLL_MUL9);     /* 设置时钟, 72Mhz */
	/* 延时初始化 */
	uart_init(115200); /* 串口初始化为115200 */
	LED_Init();		   /* 初始化LED */
	usart2_init(115200);
	imu901_init(); /* IMU901模块初始 */

	KEY_Init();	//KEY初始化
	Ble_IoInit();  //BLE???u?'??
	Ble_SetInit(); //BLE????????

	while (1)
	{
		//imu901_uart_receive(&ch, 1);

		//上位机
		//fputc(ch, stdout);

		//加速度传感器模块
		if (imu901_uart_receive(&ch, 1)) /*!< 获取串口fifo一个字节 */
		{
			if (imu901_unpack(ch)) /*!< 解析出有效数据包 */
			{
				if (rxPacket.startByte2 == UP_BYTE2) /*!< 主动上传的数据包 */
				{
					atkpParsing(&rxPacket);
				}
			}
		}
		else
		{
			//fflush(stdout);
			delay_ms(1);
	
			times++;
	
			//if (times % 300  == 0) LED0_TOGGLE(); 	/* 闪烁LED,提示系统正在运行 */
	
			// ble和压力传感器
			/*多次获取ADC1的值，取平均*/
			value_AD = Get_Adc_Average(1, 10); //10次平均值
												/*AO引脚输出的电压有效范围是0.1v到3.3v*/
			VOLTAGE_AO = map(value_AD, 0, 4095, 0, 3300);
			if (VOLTAGE_AO < VOLTAGE_MIN)
			{
				PRESS_AO = 0;
			}
			else if (VOLTAGE_AO > VOLTAGE_MAX)
			{
				PRESS_AO = PRESS_MAX;
			}
			else
			{
				/*将[VOLTAGE_MIN, VOLTAGE_MAX]范围内的VOLTAGE_AO映射到[PRESS_MIN, PRESS_MAX]内*/
				PRESS_AO = map(VOLTAGE_AO, VOLTAGE_MIN, VOLTAGE_MAX, PRESS_MIN, PRESS_MAX);
			}
			//Ble_Test(PRESS_AO); //sending data by ble
	
			printf("AD值 = %d,电压 = %d mv,压力 = %ld g\r\n", value_AD, VOLTAGE_AO, PRESS_AO);
	
			//if (times % 1000 == 0) 					/*!< 1秒打印一次数据 */
			//{
			printf("\r\n");
			printf("姿态角[XYZ]:    %-6.1f     %-6.1f     %-6.1f   (°)\r\n", attitude.roll, attitude.pitch, attitude.yaw);
			printf("加速度[XYZ]:    %-6.3f     %-6.3f     %-6.3f   (g)\r\n", gyroAccData.faccG[0], gyroAccData.faccG[1], gyroAccData.faccG[2]);
			printf("角速度[XYZ]:    %-6.1f     %-6.1f     %-6.1f   (°/s)\r\n", gyroAccData.fgyroD[0], gyroAccData.fgyroD[1], gyroAccData.fgyroD[2]);
			//printf("磁场[XYZ]  :    %-6d     %-6d     %-6d   (uT)\r\n", magData.mag[0], magData.mag[1], magData.mag[2]);
			//printf("气压 	   :    %-6dPa   %-6dcm\r\n", baroData.pressure, baroData.altitude);
	
			//}
	
			//if (times % 1000 == 0) 					/*!< 1秒打印一次数据 */
			//{
	
			
			u8 key = KEY_Scan(0);
			if (key == WKUP_PRES) //????????
			{
				count = 0;
			}
	
			//}
			// 当WAKE_UP按下时发送MAX_BLE_SEND次数据
			if (count < MAX_BLE_SEND)
			{
				
				if (BLE_STA){
					//delay_ms(1);
					u3_printf("%d %d %ld ", value_AD, VOLTAGE_AO, PRESS_AO); //压力
					delay_ms(1);
					u3_printf("%-6.1f %-6.1f %-6.1f ", attitude.roll, attitude.pitch, attitude.yaw);
					delay_ms(1);
					u3_printf("%-6.3f %-6.3f %-6.3f ", gyroAccData.faccG[0], gyroAccData.faccG[1], gyroAccData.faccG[2]);
					delay_ms(1);
					u3_printf("%-6.1f %-6.1f %-6.1f ", gyroAccData.fgyroD[0], gyroAccData.fgyroD[1], gyroAccData.fgyroD[2]);
					//u3_printf("%-6d %-6d %-6d (uT) ", magData.mag[0], magData.mag[1], magData.mag[2]);
					//u3_printf("%-6dPa %-6dcm ", baroData.pressure, baroData.altitude);
					delay_ms(1);
					u3_printf("\r\n");
					delay_ms(1);
					count++;
				}
				else{ printf("CHECK BLE STATE! \r\n"); }
			}
			
				if(!BLE_WKUP){
					BLE_WKUP = 0;
					delay_ms(1);
					BLE_WKUP = 1;
					delay_ms(1);
				}
			//}
		}
	}
}
