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
							�������ṩ�����µ��̣�
								Ilovemcu.taobao.com
							ʵ�������Χ��չģ����������ϵ���
							���ߣ����زر���							
*************************************************************************************/
#include "stm32f10x.h"

#include "FSR.h"

#include "adc.h"
#include "ble_app.h"

// ���ٶȴ�����

#include "usart2.h"

#include "imu901.h"

//����4��������Ҫ����ʵ���ͺź���������

//��С���� ���ݾ����ͺŶ�Ӧ�ֲ��ȡ,��λ��g��������RP-18.3-ST�ͺ�Ϊ������С������20g
#define PRESS_MIN 20
//������� ���ݾ����ͺŶ�Ӧ�ֲ��ȡ,��λ��g��������RP-18.3-ST�ͺ�Ϊ�������������6kg
#define PRESS_MAX 6000

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
#define MAX_BLE_SEND 100
u8 state = 0;
u16 val = 0;
u16 value_AD = 0;

long PRESS_AO = 0;
int VOLTAGE_AO = 0;

long map(long x, long in_min, long in_max, long out_min, long out_max);

//@int main(void)
//@{
//@	delay_init();				//��ʱ������ʼ��
//@	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
//@	uart_init(9600);	 //���ڳ�ʼ��������Ϊ9600
//@	/*ͨ����ʼ��*/
//@	Adc_Init();
//@
//@	delay_ms(1000);
//@
//@	//TODO data passing
//@
//@  LED_Init();                                     //LED��ʼ��
//@  KEY_Init();                                     //KEY��ʼ��
//@	//-Ble_IoInit(); //BLE???u?'??
//@  //-Ble_SetInit();//BLE????????
//@	printf("Test start\r\n");
//@	while(1)
//@	{
//@		/*��λ�ȡADC1��ֵ��ȡƽ��*/
//@		value_AD = Get_Adc_Average(1,10);	//10��ƽ��ֵ
//@		/*AO��������ĵ�ѹ��Ч��Χ��0.1v��3.3v*/
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
//@			/*��[VOLTAGE_MIN, VOLTAGE_MAX]��Χ�ڵ�VOLTAGE_AOӳ�䵽[PRESS_MIN, PRESS_MAX]��*/
//@			PRESS_AO = map(VOLTAGE_AO, VOLTAGE_MIN, VOLTAGE_MAX, PRESS_MIN, PRESS_MAX);
//@		}
//@		//-Ble_Test(PRESS_AO); //sending data by ble
//@		printf("ADֵ = %d,��ѹ = %d mv,ѹ�� = %ld g\r\n",value_AD,VOLTAGE_AO,PRESS_AO);
//@
//@		delay_ms(10);// round faster
//@	}
//@
//@}
//@

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
	uint32_t times = 0;
	uint8_t ch;

	delay_init();									//��ʱ������ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
													//@	uart_init(9600);	 //���ڳ�ʼ��������Ϊ9600
													//@	/*ͨ����ʼ��*/
	Adc_Init();

	//sys_stm32_clock_init(RCC_PLL_MUL9);     /* ����ʱ��, 72Mhz */
	/* ��ʱ��ʼ�� */
	uart_init(115200); /* ���ڳ�ʼ��Ϊ115200 */
	LED_Init();		   /* ��ʼ��LED */
	usart2_init(115200);
	imu901_init(); /* IMU901ģ���ʼ */

	KEY_Init();	//KEY��ʼ��
	Ble_IoInit();  //BLE???u?'??
	Ble_SetInit(); //BLE????????

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
	
			times++;
	
			//if (times % 300  == 0) LED0_TOGGLE(); 	/* ��˸LED,��ʾϵͳ�������� */
	
			// ble��ѹ��������
			/*��λ�ȡADC1��ֵ��ȡƽ��*/
			value_AD = Get_Adc_Average(1, 10); //10��ƽ��ֵ
												/*AO��������ĵ�ѹ��Ч��Χ��0.1v��3.3v*/
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
				/*��[VOLTAGE_MIN, VOLTAGE_MAX]��Χ�ڵ�VOLTAGE_AOӳ�䵽[PRESS_MIN, PRESS_MAX]��*/
				PRESS_AO = map(VOLTAGE_AO, VOLTAGE_MIN, VOLTAGE_MAX, PRESS_MIN, PRESS_MAX);
			}
			//Ble_Test(PRESS_AO); //sending data by ble
	
			printf("ADֵ = %d,��ѹ = %d mv,ѹ�� = %ld g\r\n", value_AD, VOLTAGE_AO, PRESS_AO);
	
			//if (times % 1000 == 0) 					/*!< 1���ӡһ������ */
			//{
			printf("\r\n");
			printf("��̬��[XYZ]:    %-6.1f     %-6.1f     %-6.1f   (��)\r\n", attitude.roll, attitude.pitch, attitude.yaw);
			printf("���ٶ�[XYZ]:    %-6.3f     %-6.3f     %-6.3f   (g)\r\n", gyroAccData.faccG[0], gyroAccData.faccG[1], gyroAccData.faccG[2]);
			printf("���ٶ�[XYZ]:    %-6.1f     %-6.1f     %-6.1f   (��/s)\r\n", gyroAccData.fgyroD[0], gyroAccData.fgyroD[1], gyroAccData.fgyroD[2]);
			//printf("�ų�[XYZ]  :    %-6d     %-6d     %-6d   (uT)\r\n", magData.mag[0], magData.mag[1], magData.mag[2]);
			//printf("��ѹ 	   :    %-6dPa   %-6dcm\r\n", baroData.pressure, baroData.altitude);
	
			//}
	
			//if (times % 1000 == 0) 					/*!< 1���ӡһ������ */
			//{
	
			
			u8 key = KEY_Scan(0);
			if (key == WKUP_PRES) //????????
			{
				count = 0;
			}
	
			//}
			// ��WAKE_UP����ʱ����MAX_BLE_SEND������
			if (count < MAX_BLE_SEND)
			{
				
				if (BLE_STA){
					//delay_ms(1);
					u3_printf("%d %d %ld ", value_AD, VOLTAGE_AO, PRESS_AO); //ѹ��
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
