#include "ble_app.h"
#include "ble_uart.h"
#include "ble_cfg.h"
#include "usart3.h"
#include "usart.h"
#include "string.h"
#include "led.h"
#include "delay.h"
#include "key.h"


//////////////////////////////////////////////////////////////////////////////////
//������ֻ��ѧϰʹ�ã�δ���������ɣ��������������κ���;
//ALIENTEK STM32F103������
//ATK-BLE01ģ�鹦������
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2020/6/5
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) �������������ӿƼ����޹�˾ 2020-2030
//All rights reserved
//********************************************************************************
//��

//BLEģ��IO��ʼ��
void Ble_IoInit(void)
{
		printf("ble_io_init\r\n");
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); /*ʹ��PA�˿�ʱ��*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); /*ʹ�ܸ��ù���ʱ��*/

    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); /*��ֹJTAG,�Ӷ�PA15��������ͨIOʹ��,����PA15��������ͨIO!!!*/

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;          /*BLE_WKUP*/
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    /*�������*/
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   /*IO���ٶ�Ϊ50MHz*/
    GPIO_Init(GPIOA, &GPIO_InitStructure);              /*������� ��IO���ٶ�Ϊ50MHz*/

    BLE_WKUP = 1;
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;            /*BLE_STA*/
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;        /*��������*/
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    /*IO���ٶ�Ϊ50MHz*/
    GPIO_Init(GPIOA, &GPIO_InitStructure);               /*�����趨������ʼ��GPIOA.4*/
    
    usart3_init(115200);/*��������*/

}

u8 sendbuf[50];

//BLEģ���������
void Ble_SetInit(void)
{

    LED0 = 0; /*LED0��*/

    /*****��������ģʽ*****/
    while (ble_send_cmd("+++a", "a+ok", 5))
    {
        if (!ble_send_cmd("+++a", "ERROR", 10)) break; /**�п���ģ����������ģʽ,��ʱ����+++a,�᷵��error*/

        LED1 = ~LED1;
        delay_ms(100);
    }

    /*****AT����*****/
    if (!ble_send_cmd("AT", "OK", 5))
    {
        printf("AT OK\r\n");
    }

    /*****������������*****/
    sprintf((char *)sendbuf, "AT+NAME=%s", BLE_NAME);

    if (!ble_send_cmd(sendbuf, "OK", 10))
    {
        printf("%s  OK\r\n", sendbuf);
    }else
    {
        printf("%s  ERROR\r\n", sendbuf);
    }

    /*****������ӭ������*****/
    sprintf((char *)sendbuf, "AT+HELLO=%s", BLE_HELLO);

    if (!ble_send_cmd(sendbuf, "OK", 10))
    {
        printf("%s  OK\r\n", sendbuf);
    }else
    {
        printf("%s  ERROR\r\n", sendbuf);
    }

    /*****���书������*****/
    sprintf((char *)sendbuf, "AT+TPL=%d", BLE_TP_P2dBm);

    if (!ble_send_cmd(sendbuf, "OK", 10))
    {
        printf("%s  OK\r\n", sendbuf);
    }
    else{
        printf("%s  ERROR\r\n", sendbuf);
    }

    /*****��������*****/
    sprintf((char *)sendbuf, "AT+UART=%d,%d,%d,%d", BLE_UARTBPS, BLE_UARTDATE, BLE_UARTPR, BLE_UARTSTOP);

    if (!ble_send_cmd(sendbuf, "OK", 10))
    {
        printf("%s  OK\r\n", sendbuf);
    }else
    {
        printf("%s  ERROR\r\n", sendbuf);
    }

    /*****�㲥�ٶ�����*****/
    sprintf((char *)sendbuf, "AT+ADPTIM=%d", BLE_ADPTIM );

    if (!ble_send_cmd(sendbuf, "OK", 10))
    {
        printf("%s  OK\r\n", sendbuf);
    }else
    {
        printf("%s  ERROR\r\n", sendbuf);
    }

   /*****��·ƥ����������*****/
#if BLE_LINKPASSEN

    if (!ble_send_cmd("AT+LINKPASSEN=ON", "OK", 10)) /*ʹ��*/
    {
        printf("AT+LINKPASSEN=ON  OK\r\n");
    }else
    {
        printf("AT+LINKPASSEN=ON  ERROR\r\n");
    }

#else
    
    if (!ble_send_cmd("AT+LINKPASSEN=OFF", "OK", 10)) /*�ر�*/
    {
        printf("AT+LINKPASSEN=OFF  OK\r\n");
    }else
    {
        printf("AT+LINKPASSEN=OFF  ERROR\r\n");
    }
    
#endif    
    
    /*****����LED����*****/
#if BLE_LEDEN

    if (!ble_send_cmd("AT+LEDEN=ON", "OK", 10)) /*ʹ��*/
    {
        printf("AT+LEDEN=ON  OK\r\n");
    }else
    {
        printf("AT+LEDEN=ON  ERROR\r\n");
    }

#else

    if (!ble_send_cmd("AT+LEDEN=OFF", "OK", 50)) /*�ر�*/
    {
        printf("AT+LEDEN=OFF  OK\r\n");
    }else
    {
        printf("AT+LEDEN=OFF  ERROR\r\n");
    }

#endif

    /*****���豸����˯������*****/
#if BLE_SLAVESLEEPEN

    if (!ble_send_cmd("AT+SLAVESLEEPEN=ON", "OK", 10)) /*ʹ��*/
    {
        printf("AT+SLAVESLEEPEN=ON  OK\r\n");
    }else
    {
        printf("AT+SLAVESLEEPEN=ON  ERROR\r\n");
    }

#else

    if (!ble_send_cmd("AT+SLAVESLEEPEN=OFF", "OK", 10)) /*�ر�*/
    {
        printf("AT+SLAVESLEEPEN=OFF  OK\r\n");
    }else
    {
        printf("AT+SLAVESLEEPEN=OFF  ERROR\r\n");
    }

#endif

    /*****��������������*****/
#if BLE_MAXPUT

    if (!ble_send_cmd("AT+MAXPUT=ON", "OK", 10)) /*ʹ��*/
    {
        printf("AT+MAXPUT=ON  OK\r\n");
    }else
    {
        printf("AT+MAXPUT=ON  ERROR\r\n");
    }

#else

    if (!ble_send_cmd("AT+MAXPUT=OFF", "OK", 10)) /*�ر�*/
    {
        printf("AT+MAXPUT=OFF  OK\r\n");
    }else
    {
        printf("AT+MAXPUT=OFF  ERROR\r\n");
    }

#endif

    /*****����ģʽ����(ģʽ���ú�,ģ��������λ����͸��ģʽ)*****/
    sprintf((char *)sendbuf, "AT+MODE=%c", BLE_MODE_S );

    if (!ble_send_cmd(sendbuf, "OK", 10))
    {
        printf("%s  OK\r\n", sendbuf);
    }else
    {
        printf("%s  ERROR\r\n", sendbuf);
    }

    LED0 = 1;/*LED0��*/
}

/***************************�͹��Ļ��Ѵ���***************************/

/*�������ݻ���*/
void Ble_UartWakeUp(void)
{
    u3_printf("12345");
    printf("\r\nUART Wake_Up:12345\r\n");
}
/*WKUP���Ż���*/
void Ble_WkupWakeUp(void)
{
    printf("\r\nWKUP Wake_Up\r\n");
    BLE_WKUP = 0;
    delay_ms(1);
    BLE_WKUP = 1;
    delay_ms(1);
}

/*******************************************************************/


/*BLEģ�����ݷ��ʹ���*/
// todo: global enum
void Ble_SendData(u16 data)
{
    static u8 num = 0;

	u3_printf("cnt:%d data:%ld \r\n", num, data);
	printf("S: ATK-BLE01 DATA:%ld cnt:%d\r\n", data,num);

    num++;
    if (num == 6) num = 0;

}

/*BLEģ��������ݴ���*/
void Ble_ReceData(void)
{
    u16 i = 0;
    u16 len = 0;

    //����������
    if (USART3_RX_STA & 0x8000)
    {
        len = USART3_RX_STA & 0X7FFF;
        USART3_RX_BUF[len] = 0; //���ӽ�����
        USART3_RX_STA = 0;

        printf("R:");

        for (i = 0; i < len; i++)
        {
            while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET); //ѭ������,ֱ���������
            USART_SendData(USART1, USART3_RX_BUF[i]);
        }

        if (strstr((const char *)USART3_RX_BUF, LED0_ON) && (len == (sizeof(LED0_ON) - 1)))        LED0 = 0; /*LED0����*/
        else if (strstr((const char *)USART3_RX_BUF, LED0_OFF) && (len == (sizeof(LED0_OFF) - 1))) LED0 = 1; /*LED0�ر�*/

        memset((char *)USART3_RX_BUF, 0x00, len); //���ڽ��ջ�������0
        printf("\r\n");
    }

}

/*BLE���̴���*/
void Ble_Process(u16 data)
{
    u8 key = 0;
    u8 t = 0;
    //u8 BLE_LINK = 0; //1:�������� 0:δ����
    static u8 flag1, flag2 = 0;

    printf("\r\nBLE PROC START!\r\n");
    //printf("\r\nKEY1:WKUP���Ż���\r\n");
    //printf("\r\nKEY0:UART���ڻ���\r\n\r\n");
    
    //while (BLE_ENABLED)
    //{

        //key = KEY_Scan(0);

        //if(key == WKUP_PRES) //��������    
        //{
            if (BLE_STA) /*����������*/
            {
                Ble_SendData(data);/*���ݷ��ʹ���*/
							 //break; // to break the ble loop
            }
						else{ printf("CHECK BLE STATE!"); }
        //}

        if(!BLE_WKUP) /*WKUP����*/
        {
            Ble_WkupWakeUp();
        }
				else{Ble_UartWakeUp();}
        //
        //if(key == KEY0_PRES) /*���ڻ���*/
        //{
            
        //}

        /*���ݽ��մ���*/
				//cmdtype command;
        //command=Ble_ReceData();
				//switch command
				//	case comm1: senddata(datatype type, u16 data);
        /*BLE���Ӵ���*/
        //if (BLE_STA)
        //{
        //    BLE_LINK = 1; //��������
        //
        //    if (!flag1)
        //    {
        //        flag1 = 1;
        //        flag2 = 0;
        //        printf("\r\nble connect!\r\n");
        //    }
        //}
        //else
        //{
        //    BLE_LINK = 0; //�����Ͽ�
        //
        //    if (!flag2)
        //    {
        //
        //        flag2 = 1;
        //        flag1 = 0;
        //        printf("\r\nble disconnect!\r\n");
        //    }
        //}

        t++;
        if (t == 20)
        {
            t = 0;
            LED1 = ~LED1;
        }
        delay_ms(10);
		printf("\r\nBLE PROC STOP!\r\n");
    //}

}

//�����Ժ���
void Ble_Test(u16 data)
{

    printf("\r\nATK-BLE01 TEST\r\n\r\n");

    Ble_Process(data);//���ݲ���

}
