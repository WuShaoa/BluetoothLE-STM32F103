/**
  ****************************************************************************************************
  * @file    	usart2.h
  * @author		����ԭ���Ŷ�(ALIENTEK)
  * @version    V1.0
  * @date		2020-04-17
  * @brief   	����2��������
  * @license   	Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
  ****************************************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:����ԭ�� STM32������
  * ������Ƶ:www.yuanzige.com
  * ������̳:www.openedv.com
  * ��˾��ַ:www.alientek.com
  * �����ַ:openedv.taobao.com
  *
  * �޸�˵��
  * V1.0 2020-04-17
  * ��һ�η���
  *
  ****************************************************************************************************
  */

#ifndef _USART2_H_
#define _USART2_H_


#include "sys.h"




void usart2_init(uint32_t bound);

void usart2_sendData(uint8_t *data);
uint16_t usart2_getRxData(uint8_t *buf, uint16_t len);


#endif /* _USART2_H_ */

/*******************************END OF FILE************************************/


