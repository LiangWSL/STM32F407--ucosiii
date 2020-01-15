#include "timer.h"
#include "led.h"

#include "all.h"
float pitch,roll,yaw; 		//ŷ����
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//��ʱ�� ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/4
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	 

u32 TIM12_OVERFLOW_CNT = 0;

//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz
//����ʹ�õ��Ƕ�ʱ��3!
void TIM12_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM12,ENABLE);  ///ʹ��TIMʱ��

	TIM_TimeBaseInitStructure.TIM_Period = arr; 	//�Զ���װ��ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler = psc;  //��ʱ����Ƶ
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1; 

	TIM_TimeBaseInit(TIM12,&TIM_TimeBaseInitStructure);//��ʼ��TIM
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM8_BRK_TIM12_IRQn; //��ʱ���ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x02; //��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; //�����ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_ClearITPendingBit(TIM12,TIM_IT_Update);  //����жϱ�־λ
	TIM_ITConfig(TIM12,TIM_IT_Update,ENABLE); //����ʱ�������ж�
	TIM_Cmd(TIM12,DISABLE);
}


//��ʱ���жϷ�����
//void TIM8_BRK_TIM12_IRQHandler(void)
//{
//	static int count=0;
//	if (TIM_GetITStatus(TIM12,TIM_IT_Update) == SET) //����ж�
//	{
//		++TIM12_OVERFLOW_CNT;
//		
//		count++;
//		
//		
//		if(count<200&&((count%2)==0))
//		{
////			if(BALL_X_SHIFTED == 1)
////				LCD_printf(0,6,300,24,24,"x:%d,y:%d",ball_x,ball_y);
//			/*if(json_handle_flag == 1)
//			{
//				json_t* object;
//				json_error_t* error;
//				
//				object=json_loads(Json_RxBuffer,0,error);
//				void* iter=json_object_iter_at(object,"x");
//				x=json_number_value(json_object_iter_value(iter));
//				iter=json_object_iter_at(object,"y");
//				x=json_number_value(json_object_iter_value(iter));
//				LCD_printf(0,6,300,24,24,"x:%fy:%f",x,y);
//				json_handle_flag=0;
//			}*/
//		}
//		else if(count<200&&((count%2)==1))
//		{
//			if(mpu_mpl_get_data(&pitch,&roll,&yaw)==0);
//		}
//		else
//		{
//			count=0;
//			LED2=!LED2;
////				LCD_printf(0,6,300,24,24,"pitch:%f",pitch);
//		}
//		
//		TIM_ClearITPendingBit(TIM12,TIM_IT_Update);  //����жϱ�־λ
//	}
//}

void TIM8_BRK_TIM12_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM12,TIM_IT_Update) == SET) //����ж�
	{
	#ifdef BSP_USING_USART_SCREEN
		DMA_SetCurrDataCounter(USART1_DMA_TX_STREAM, LCD_Table_Size);
		DMA_Cmd(USART1_DMA_TX_STREAM, ENABLE);
	#endif
		//++TIM12_OVERFLOW_CNT;
		TIM_ClearITPendingBit(TIM12,TIM_IT_Update);  //����жϱ�־λ
	}
}

