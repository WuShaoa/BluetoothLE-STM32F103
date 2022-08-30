#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "ble_app.h"
#include "usart3.h"

//for debugging utilities
//TODO: clock() cant use!!
// #define DEBUG_VERSION
#define TIMER_3_VERSION

#if defined (DEBUG_VERSION)
#include <time.h>
#endif

#if defined (TIMER_3_VERSION)
#include "timer.h"
long int time_counter = 0;
#endif

#include "stm32f10x.h"
#include "FSR.h"
#include "adc.h"
#include "ble_app.h"

// 角速度传感器

#include "usart2.h"
#include "imu901.h"

//下面4项内容需要根据实际型号和量程修正

//最小量程 根据具体型号对应手册获取,单位是g
#define PRESS_MIN 500
//最大量程 根据具体型号对应手册获取,单位是g
#define PRESS_MAX 20000

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

/* unuse MAX_BLE_SEND
#define MAX_BLE_SEND 100
*/
u8 state = 0;
u16 val = 0;
u16 value_AD = 0;
u16 value_AD_0 = 0;
u16 value_AD_1 = 0;

int VOLTAGE_AO = 0;
int VOLTAGE_AO_0 = 0;
int VOLTAGE_AO_1 = 0;

long PRESS_AO = 0;
long PRESS_AO_0 = 0;
long PRESS_AO_1 = 0;

long map(long x, long in_min, long in_max, long out_min, long out_max);

static double temp;
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
	int flag_wkup_pressed = 0;
	
	#if defined (DEBUG_VERSION)
		clock_t clk_time = 0.0;
		int flag_first = 1;
	#endif
	

	
	uint8_t ch;

	delay_init();									//延时函数初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	
	/*通道初始化*/
	Adc_Init();

	/* 延时初始化 */
	uart_init(115200); /* 串口初始化为115200 */
	LED_Init();		   /* 初始化LED */
	usart2_init(115200);
	imu901_init(); /* IMU901模块初始 */

	KEY_Init();	//KEY初始化
	Ble_IoInit(); 
	Ble_SetInit();
	 
	#if defined (TIMER_3_VERSION)
	//TIM3初始化
	TIM3_Int_Init(9,7199); //每毫秒中断1次
	#endif

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
	
			// ble和压力传感器
			/*多次获取ADC1的值，取平均*/
			value_AD = Get_Adc_Average(1, 5); //5次平均值
			value_AD_0 = Get_Adc_Average(10, 5); //5次平均值
			value_AD_1 = Get_Adc_Average(11, 5); //5次平均值
												/*AO引脚输出的电压有效范围是0.1v到3.3v*/
			VOLTAGE_AO = map(value_AD, 0, 4095, 0, 3300);
			VOLTAGE_AO_0 = map(value_AD_0, 0, 4095, 0, 3300);
			VOLTAGE_AO_1 = map(value_AD_1, 0, 4095, 0, 3300);
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
			
			if (VOLTAGE_AO_0 < VOLTAGE_MIN)
			{
				PRESS_AO_0 = 0;
			}
			else if (VOLTAGE_AO_0 > VOLTAGE_MAX)
			{
				PRESS_AO_0 = PRESS_MAX;
			}
			else
			{
				/*将[VOLTAGE_MIN, VOLTAGE_MAX]范围内的VOLTAGE_AO映射到[PRESS_MIN, PRESS_MAX]内*/
				PRESS_AO_0 = map(VOLTAGE_AO_0, VOLTAGE_MIN, VOLTAGE_MAX, PRESS_MIN, PRESS_MAX);
			}
			
			if (VOLTAGE_AO_1 < VOLTAGE_MIN)
			{
				PRESS_AO_1 = 0;
			}
			else if (VOLTAGE_AO_1 > VOLTAGE_MAX)
			{
				PRESS_AO_1 = PRESS_MAX;
			}
			else
			{
				/*将[VOLTAGE_MIN, VOLTAGE_MAX]范围内的VOLTAGE_AO映射到[PRESS_MIN, PRESS_MAX]内*/
				PRESS_AO_1 = map(VOLTAGE_AO_1, VOLTAGE_MIN, VOLTAGE_MAX, PRESS_MIN, PRESS_MAX);
			}
			
			//Ble_Test(PRESS_AO); //sending data by ble
			#if defined (TIMER_3_VERSION)

			printf("clock time: %ld ms \r\n", time_counter);
			
			//time_t now;
			//struct tm *ts;
			//char buf[80];
			
			//now = time(NULL);
			//ts = localtime(& now);
			//strftime (buf, sizeof(buf), "% a% Y-% m-% d% H:% M:% S% Z", ts);
			//printf ("%s \r\n", buf);
			#endif
			printf("AD值 = %d,电压 = %d mv,压力 = %ld g\r\n", value_AD, VOLTAGE_AO, PRESS_AO);
			printf("AD值 = %d,电压 = %d mv,压力 = %ld g\r\n", value_AD_0, VOLTAGE_AO_0, PRESS_AO_0);
			printf("AD值 = %d,电压 = %d mv,压力 = %ld g\r\n", value_AD_1, VOLTAGE_AO_1, PRESS_AO_1);
			printf("姿态角[XYZ]:    %-6.1f     %-6.1f     %-6.1f   (°)\r\n", attitude.roll, attitude.pitch, attitude.yaw);
			printf("加速度[XYZ]:    %-6.3f     %-6.3f     %-6.3f   (g)\r\n", gyroAccData.faccG[0], gyroAccData.faccG[1], gyroAccData.faccG[2]);
			printf("角速度[XYZ]:    %-6.1f     %-6.1f     %-6.1f   (°/s)\r\n", gyroAccData.fgyroD[0], gyroAccData.fgyroD[1], gyroAccData.fgyroD[2]);
			printf("磁场[XYZ]  :    %-6d     %-6d     %-6d   (uT)\r\n", magData.mag[0], magData.mag[1], magData.mag[2]);
			printf("气压 	   :    %-6dPa   %-6dcm\r\n", baroData.pressure, baroData.altitude);
			
			u8 key = KEY_Scan(0);
			if (key == WKUP_PRES) //????????
			{
				#if defined (DEBUG_VERSION)
					int flag_first = 1;
				#endif
				count = 0;
				flag_wkup_pressed = 1;
				if(!BLE_WKUP){
					BLE_WKUP = 0;
					delay_ms(1);
					BLE_WKUP = 1;
					delay_ms(1);
				}
				
			}
	
			//}
			// 当WAKE_UP按下时发送MAX_BLE_SEND次数据
			#if defined (MAX_BLE_SEND)
			if (count < MAX_BLE_SEND)
			#else
			if (flag_wkup_pressed)
			#endif
			{
				
				if (BLE_STA){
					//delay_ms(1);
					u3_printf("%d %d %ld ", value_AD, VOLTAGE_AO, PRESS_AO); //压力
					//delay_ms(1);
					u3_printf("%-6.1f %-6.1f %-6.1f ", attitude.roll, attitude.pitch, attitude.yaw);
					//delay_ms(1);
					u3_printf("%-6.3f %-6.3f %-6.3f ", gyroAccData.faccG[0], gyroAccData.faccG[1], gyroAccData.faccG[2]);
					//delay_ms(1);
					u3_printf("%-6.1f %-6.1f %-6.1f ", gyroAccData.fgyroD[0], gyroAccData.fgyroD[1], gyroAccData.fgyroD[2]);
					u3_printf("%-6d %-6d %-6d (uT) ", magData.mag[0], magData.mag[1], magData.mag[2]);
					u3_printf("%-6dPa %-6dcm ", baroData.pressure, baroData.altitude);
					delay_ms(1);
					u3_printf("\r\n");
					delay_ms(1);
					count++;
				}
				else{ printf("CHECK BLE STATE! \r\n"); }
			}

			
			#if defined (DEBUG_VERSION)
				else if (count == MAX_BLE_SEND)
					{ 
						if (flag_first) {
							
							printf("\r\n Total time used in BLE transfer is %f per sec per round \r\n", \
								(1.0 * (clock() - clk_time) / CLOCKS_PER_SEC) / count); 
							printf("\r\n MAX_BLE_SEND is %d \r\n", count);
							
							temp = (1.0 * (clock() - clk_time) / CLOCKS_PER_SEC) / count;
							
							clk_time = clock(); //re-counting
							flag_first = 0;
						}
					}
				if(!flag_first)
				{
					printf("\r\n Total time used in BLE transfer is %f per sec per round \r\n", \
						temp); 
					printf("\r\n count is %d \r\n", count);
					printf("clock() is %f, clk_time is %f \r\n", 1.0 * clock(), 1.0 * clk_time); 
				}
			#endif
			
		}
	}
}
