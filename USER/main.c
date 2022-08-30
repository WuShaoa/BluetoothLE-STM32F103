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

// ���ٶȴ�����

#include "usart2.h"
#include "imu901.h"

//����4��������Ҫ����ʵ���ͺź���������

//��С���� ���ݾ����ͺŶ�Ӧ�ֲ��ȡ,��λ��g
#define PRESS_MIN 500
//������� ���ݾ����ͺŶ�Ӧ�ֲ��ȡ,��λ��g
#define PRESS_MAX 20000

//����2���������ݻ�ȡ������
//�����ϣ�
// 1.��Ĥѹ�����������Ǿ�׼��ѹ�����Դ�������ֻ�ʺϴ��Բ���ѹ���ã����ܵ�ѹ���ƾ�ȷ������
// 2. AO��������ĵ�ѹ��Ч��Χ��0.1v��3.3v����ʵ�ʸ��ݲ�ͬ��������Χ���������Χ�ڣ�����һ�������ֵ3.3v��Ҳ���ܵ���3.3v��Ҫʵ�����ñ������
// 	����ֻ�Ǹ�������ֵ����Ҫ��ȷ���������ñ����Ȼ����������2��AO���ŵ�ѹ�����������Сֵ
//���ڷ�����
//��Ĥѹ����������AO������������淶Χ��ͨ������AO_RES��λ������ʵ�ֵģ�
//��Ҫ��΢��׼�㣬��Ҫ�Լ�����������֪����Ȼ�����AO_RES��λ����������������������Լ��������Ϳ�����
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
��[in_min, in_max]��Χ�ڵ�x�ȱ�ӳ�䵽[out_min, out_max]��
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

	delay_init();									//��ʱ������ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	
	/*ͨ����ʼ��*/
	Adc_Init();

	/* ��ʱ��ʼ�� */
	uart_init(115200); /* ���ڳ�ʼ��Ϊ115200 */
	LED_Init();		   /* ��ʼ��LED */
	usart2_init(115200);
	imu901_init(); /* IMU901ģ���ʼ */

	KEY_Init();	//KEY��ʼ��
	Ble_IoInit(); 
	Ble_SetInit();
	 
	#if defined (TIMER_3_VERSION)
	//TIM3��ʼ��
	TIM3_Int_Init(9,7199); //ÿ�����ж�1��
	#endif

	while (1)
	{
		//imu901_uart_receive(&ch, 1);

		//��λ��
		//fputc(ch, stdout);

		//���ٶȴ�����ģ��
		if (imu901_uart_receive(&ch, 1)) /*!< ��ȡ����fifoһ���ֽ� */
		{
			if (imu901_unpack(ch)) /*!< ��������Ч���ݰ� */
			{
				if (rxPacket.startByte2 == UP_BYTE2) /*!< �����ϴ������ݰ� */
				{
					atkpParsing(&rxPacket);
				}
			}
		}
		else
		{
			//fflush(stdout);
			delay_ms(1);
	
			// ble��ѹ��������
			/*��λ�ȡADC1��ֵ��ȡƽ��*/
			value_AD = Get_Adc_Average(1, 5); //5��ƽ��ֵ
			value_AD_0 = Get_Adc_Average(10, 5); //5��ƽ��ֵ
			value_AD_1 = Get_Adc_Average(11, 5); //5��ƽ��ֵ
												/*AO��������ĵ�ѹ��Ч��Χ��0.1v��3.3v*/
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
				/*��[VOLTAGE_MIN, VOLTAGE_MAX]��Χ�ڵ�VOLTAGE_AOӳ�䵽[PRESS_MIN, PRESS_MAX]��*/
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
				/*��[VOLTAGE_MIN, VOLTAGE_MAX]��Χ�ڵ�VOLTAGE_AOӳ�䵽[PRESS_MIN, PRESS_MAX]��*/
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
				/*��[VOLTAGE_MIN, VOLTAGE_MAX]��Χ�ڵ�VOLTAGE_AOӳ�䵽[PRESS_MIN, PRESS_MAX]��*/
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
			printf("ADֵ = %d,��ѹ = %d mv,ѹ�� = %ld g\r\n", value_AD, VOLTAGE_AO, PRESS_AO);
			printf("ADֵ = %d,��ѹ = %d mv,ѹ�� = %ld g\r\n", value_AD_0, VOLTAGE_AO_0, PRESS_AO_0);
			printf("ADֵ = %d,��ѹ = %d mv,ѹ�� = %ld g\r\n", value_AD_1, VOLTAGE_AO_1, PRESS_AO_1);
			printf("��̬��[XYZ]:    %-6.1f     %-6.1f     %-6.1f   (��)\r\n", attitude.roll, attitude.pitch, attitude.yaw);
			printf("���ٶ�[XYZ]:    %-6.3f     %-6.3f     %-6.3f   (g)\r\n", gyroAccData.faccG[0], gyroAccData.faccG[1], gyroAccData.faccG[2]);
			printf("���ٶ�[XYZ]:    %-6.1f     %-6.1f     %-6.1f   (��/s)\r\n", gyroAccData.fgyroD[0], gyroAccData.fgyroD[1], gyroAccData.fgyroD[2]);
			printf("�ų�[XYZ]  :    %-6d     %-6d     %-6d   (uT)\r\n", magData.mag[0], magData.mag[1], magData.mag[2]);
			printf("��ѹ 	   :    %-6dPa   %-6dcm\r\n", baroData.pressure, baroData.altitude);
			
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
			// ��WAKE_UP����ʱ����MAX_BLE_SEND������
			#if defined (MAX_BLE_SEND)
			if (count < MAX_BLE_SEND)
			#else
			if (flag_wkup_pressed)
			#endif
			{
				
				if (BLE_STA){
					//delay_ms(1);
					u3_printf("%d %d %ld ", value_AD, VOLTAGE_AO, PRESS_AO); //ѹ��
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
