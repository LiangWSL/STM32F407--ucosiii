#include "lcd.h"
#include "touch.h"
#include "M_Func.h"
#include "delay.h"
#include "MyTypeDef.h"
#include "key.h"
#include "usart.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//������������֧��ADS7843/7846/UH7843/7846/XPT2046/TSC2046/OTT2001A�ȣ� ����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/7
//�汾��V1.2
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									   
//********************************************************************************
//�޸�˵��
//V1.1 20140721
//����MDK��-O2�Ż�ʱ,�����������޷���ȡ��bug.��TP_Write_Byte�������һ����ʱ,�������.
//V1.2 20141130 
//���ݴ���������FT5206��֧��
//////////////////////////////////////////////////////////////////////////////////

_m_tp_dev tp_dev=
{
	TP_Init,
	TP_Scan,
	TP_Adjust,
	0,
	0, 
	0,
	0,
	0,
	0,	  	 		
	0,
	0,	  	 		
};					
//Ĭ��Ϊtouchtype=0������.
u8 CMD_RDX=0XD0;
u8 CMD_RDY=0X90;

u8 use_touch_adj_data = 1;
 	 			    					   
//SPIд����
//������ICд��1byte����    
//num:Ҫд�������
void TP_Write_Byte(u8 num)    
{  
	u8 count=0;   
	for(count=0;count<8;count++)  
	{ 	  
		if(num&0x80)TDIN=1;  
		else TDIN=0;   
		num<<=1;    
		TCLK=0; 
		delay_us(1);
		TCLK=1;		//��������Ч	        
	}		 			    
} 		 
//SPI������ 
//�Ӵ�����IC��ȡadcֵ
//CMD:ָ��
//����ֵ:����������	   
u16 TP_Read_AD(u8 CMD)	  
{ 	 
	u8 count=0; 	  
	u16 Num=0; 
	TCLK=0;		//������ʱ�� 	 
	TDIN=0; 	//����������
	TCS=0; 		//ѡ�д�����IC
	TP_Write_Byte(CMD);//����������
	delay_us(6);//ADS7846��ת��ʱ���Ϊ6us
	TCLK=0; 	     	    
	delay_us(1);    	   
	TCLK=1;		//��1��ʱ�ӣ����BUSY
	delay_us(1);    
	TCLK=0; 	     	    
	for(count=0;count<16;count++)//����16λ����,ֻ�и�12λ��Ч 
	{ 				  
		Num<<=1; 	 
		TCLK=0;	//�½�����Ч  	    	   
		delay_us(1);    
 		TCLK=1;
 		if(DOUT)Num++; 		 
	}  	
	Num>>=4;   	//ֻ�и�12λ��Ч.
	TCS=1;		//�ͷ�Ƭѡ	 
	return(Num);   
}
//��ȡһ������ֵ(x����y)
//������ȡREAD_TIMES������,����Щ������������,
//Ȼ��ȥ����ͺ����LOST_VAL����,ȡƽ��ֵ 
//xy:ָ�CMD_RDX/CMD_RDY��
//����ֵ:����������
#define READ_TIMES 5 	//��ȡ����
#define LOST_VAL 1	  	//����ֵ
u16 TP_Read_XOY(u8 xy)
{
	u16 i, j;
	u16 buf[READ_TIMES];
	u16 sum=0;
	u16 temp;
	for(i=0;i<READ_TIMES;i++)buf[i]=TP_Read_AD(xy);		 		    
	for(i=0;i<READ_TIMES-1; i++)//����
	{
		for(j=i+1;j<READ_TIMES;j++)
		{
			if(buf[i]>buf[j])//��������
			{
				temp=buf[i];
				buf[i]=buf[j];
				buf[j]=temp;
			}
		}
	}	  
	sum=0;
	for(i=LOST_VAL;i<READ_TIMES-LOST_VAL;i++)sum+=buf[i];
	temp=sum/(READ_TIMES-2*LOST_VAL);
	return temp;   
} 
//��ȡx,y����
//��Сֵ��������100.
//x,y:��ȡ��������ֵ
//����ֵ:0,ʧ��;1,�ɹ���
u8 TP_Read_XY(u16 *x,u16 *y)
{
	u16 xtemp,ytemp;			 	 		  
	xtemp=TP_Read_XOY(CMD_RDX);
	ytemp=TP_Read_XOY(CMD_RDY);	  												   
	//if(xtemp<100||ytemp<100)return 0;//����ʧ��
	*x=xtemp;
	*y=ytemp;
	return 1;//�����ɹ�
}
//����2�ζ�ȡ������IC,�������ε�ƫ��ܳ���
//ERR_RANGE,��������,����Ϊ������ȷ,�����������.	   
//�ú����ܴ�����׼ȷ��
//x,y:��ȡ��������ֵ
//����ֵ:0,ʧ��;1,�ɹ���
#define ERR_RANGE 50 //��Χ 
u8 TP_Read_XY2(u16 *x,u16 *y) 
{
	u16 x1,y1;
 	u16 x2,y2;
 	u8 flag;    
    flag=TP_Read_XY(&x1,&y1);   
    if(flag==0)return(0);
    flag=TP_Read_XY(&x2,&y2);	   
    if(flag==0)return(0);   
    if(((x2<=x1&&x1<x2+ERR_RANGE)||(x1<=x2&&x2<x1+ERR_RANGE))//ǰ�����β�����+-50��
    &&((y2<=y1&&y1<y2+ERR_RANGE)||(y1<=y2&&y2<y1+ERR_RANGE)))
    {
        *x=(x1+x2)/2;
        *y=(y1+y2)/2;
        return 1;
    }else return 0;	  
}  
//////////////////////////////////////////////////////////////////////////////////		  
//��LCD�����йصĺ���  
//��һ��������
//����У׼�õ�
//x,y:����
//color:��ɫ
void TP_Drow_Touch_Point(u16 x,u16 y,u16 color)
{
	POINT_COLOR=color;
	LCD_DrawLine(x-12,y,x+13,y);//����
	LCD_DrawLine(x,y-12,x,y+13);//����
	LCD_DrawPoint(x+1,y+1);
	LCD_DrawPoint(x-1,y+1);
	LCD_DrawPoint(x+1,y-1);
	LCD_DrawPoint(x-1,y-1);
	LCD_Draw_Circle(x,y,6);//������Ȧ
}	  
//��һ�����(2*2�ĵ�)		   
//x,y:����
//color:��ɫ
void TP_Draw_Big_Point(u16 x,u16 y,u16 color)
{	    
	POINT_COLOR=color;
	LCD_DrawPoint(x,y);//���ĵ� 
	LCD_DrawPoint(x+1,y);
	LCD_DrawPoint(x,y+1);
	LCD_DrawPoint(x+1,y+1);	 	  	
}						  
//////////////////////////////////////////////////////////////////////////////////		  
//��������ɨ��
//tp:0,��Ļ����;1,��������(У׼�����ⳡ����)
//����ֵ:��ǰ����״̬.
//0,�����޴���;1,�����д���
u8 TP_Scan(u8 tp)
{			   
	if(PEN==0)//�а�������
	{
		if(tp)TP_Read_XY2(&tp_dev.x[0],&tp_dev.y[0]);//��ȡ��������
		else if(TP_Read_XY2(&tp_dev.x[0],&tp_dev.y[0]))//��ȡ��Ļ����
		{
	 		tp_dev.x[0]=tp_dev.xfac*tp_dev.x[0]+tp_dev.xoff;//�����ת��Ϊ��Ļ����
			tp_dev.y[0]=tp_dev.yfac*tp_dev.y[0]+tp_dev.yoff;  
	 	} 
		if((tp_dev.sta&TP_PRES_DOWN)==0)//֮ǰû�б�����
		{		 
			tp_dev.sta=TP_PRES_DOWN|TP_CATH_PRES;//��������  
			tp_dev.x[4]=tp_dev.x[0];//��¼��һ�ΰ���ʱ������
			tp_dev.y[4]=tp_dev.y[0];  	   			 
		}			   
	}else
	{
		if(tp_dev.sta&TP_PRES_DOWN)//֮ǰ�Ǳ����µ�
		{
			tp_dev.sta&=~(1<<7);//��ǰ����ɿ�	
		}else//֮ǰ��û�б�����
		{
			tp_dev.x[4]=0;
			tp_dev.y[4]=0;
			tp_dev.x[0]=0xffff;
			tp_dev.y[0]=0xffff;
		}	    
	}
	return tp_dev.sta&TP_PRES_DOWN;//���ص�ǰ�Ĵ���״̬
}	  
//////////////////////////////////////////////////////////////////////////	 
////������EEPROM����ĵ�ַ�����ַ,ռ��13���ֽ�(RANGE:SAVE_ADDR_BASE~SAVE_ADDR_BASE+12)
//#define SAVE_ADDR_BASE 40
////����У׼����										    
//void TP_Save_Adjdata(void)
//{
//	s32 temp;			 
//	//����У�����!		   							  
//	temp=tp_dev.xfac*100000000;//����xУ������      
//    AT24CXX_WriteLenByte(SAVE_ADDR_BASE,temp,4);   
//	temp=tp_dev.yfac*100000000;//����yУ������    
//    AT24CXX_WriteLenByte(SAVE_ADDR_BASE+4,temp,4);
//	//����xƫ����
//    AT24CXX_WriteLenByte(SAVE_ADDR_BASE+8,tp_dev.xoff,2);		    
//	//����yƫ����
//	AT24CXX_WriteLenByte(SAVE_ADDR_BASE+10,tp_dev.yoff,2);	
//	//���津������
//	AT24CXX_WriteOneByte(SAVE_ADDR_BASE+12,tp_dev.touchtype);	
//	temp=0X0A;//���У׼����
//	AT24CXX_WriteOneByte(SAVE_ADDR_BASE+13,temp); 
//}
//�õ�������EEPROM�����У׼ֵ
//����ֵ��1���ɹ���ȡ����
//        0����ȡʧ�ܣ�Ҫ����У׼
//u8 TP_Get_Adjdata(void)
//{					  
//	s32 tempfac;
//	tempfac=AT24CXX_ReadOneByte(SAVE_ADDR_BASE+13);//��ȡ�����,���Ƿ�У׼���� 		 
//	if(tempfac==0X0A)//�������Ѿ�У׼����			   
//	{    												 
//		tempfac=AT24CXX_ReadLenByte(SAVE_ADDR_BASE,4);		   
//		tp_dev.xfac=(float)tempfac/100000000;//�õ�xУ׼����
//		tempfac=AT24CXX_ReadLenByte(SAVE_ADDR_BASE+4,4);			          
//		tp_dev.yfac=(float)tempfac/100000000;//�õ�yУ׼����
//	    //�õ�xƫ����
//		tp_dev.xoff=AT24CXX_ReadLenByte(SAVE_ADDR_BASE+8,2);			   	  
// 	    //�õ�yƫ����
//		tp_dev.yoff=AT24CXX_ReadLenByte(SAVE_ADDR_BASE+10,2);				 	  
// 		tp_dev.touchtype=AT24CXX_ReadOneByte(SAVE_ADDR_BASE+12);//��ȡ�������ͱ��
//		if(tp_dev.touchtype)//X,Y��������Ļ�෴
//		{
//			CMD_RDX=0X90;
//			CMD_RDY=0XD0;	 
//		}else				   //X,Y��������Ļ��ͬ
//		{
//			CMD_RDX=0XD0;
//			CMD_RDY=0X90;	 
//		}		 
//		return 1;	 
//	}
//	return 0;
//}	 
//��ʾ�ַ���
u8* const TP_REMIND_MSG_TBL="Please use the stylus click the cross on the screen.The cross will always move until the screen adjustment is completed.";
 					  
//��ʾУ׼���(��������)
void TP_Adj_Info_Show(u16 x0,u16 y0,u16 x1,u16 y1,u16 x2,u16 y2,u16 x3,u16 y3,u16 fac)
{	  
	POINT_COLOR=RED;
	LCD_ShowString(40,160,lcddev.width,lcddev.height,16,"x1:");
 	LCD_ShowString(40+80,160,lcddev.width,lcddev.height,16,"y1:");
 	LCD_ShowString(40,180,lcddev.width,lcddev.height,16,"x2:");
 	LCD_ShowString(40+80,180,lcddev.width,lcddev.height,16,"y2:");
	LCD_ShowString(40,200,lcddev.width,lcddev.height,16,"x3:");
 	LCD_ShowString(40+80,200,lcddev.width,lcddev.height,16,"y3:");
	LCD_ShowString(40,220,lcddev.width,lcddev.height,16,"x4:");
 	LCD_ShowString(40+80,220,lcddev.width,lcddev.height,16,"y4:");  
 	LCD_ShowString(40,240,lcddev.width,lcddev.height,16,"fac is:");     
	LCD_ShowNum(40+24,160,x0,4,16);		//��ʾ��ֵ
	LCD_ShowNum(40+24+80,160,y0,4,16);	//��ʾ��ֵ
	LCD_ShowNum(40+24,180,x1,4,16);		//��ʾ��ֵ
	LCD_ShowNum(40+24+80,180,y1,4,16);	//��ʾ��ֵ
	LCD_ShowNum(40+24,200,x2,4,16);		//��ʾ��ֵ
	LCD_ShowNum(40+24+80,200,y2,4,16);	//��ʾ��ֵ
	LCD_ShowNum(40+24,220,x3,4,16);		//��ʾ��ֵ
	LCD_ShowNum(40+24+80,220,y3,4,16);	//��ʾ��ֵ
 	LCD_ShowNum(40+56,240,fac,3,16); 	//��ʾ��ֵ,����ֵ������95~105��Χ֮��.

}

void TP_Adj_Parameters_Show(void)//��ʾУ׼����
{
	POINT_COLOR=BLACK;
	
	LCD_printf(30,40,200,24,24,"xfac:%.10lf",tp_dev.xfac);
	LCD_printf(30,70,200,24,24,"yfac:%.10lf",tp_dev.yfac);
	LCD_printf(30,100,200,24,24,"xoff:%d",tp_dev.xoff);
	LCD_printf(30,130,200,24,24,"yoff:%d",tp_dev.yoff);
	LCD_printf(30,160,200,24,24,"touchtype:%d",tp_dev.touchtype);
}

void TP_Adj_Parameters_Set(void)//����У׼����
{
//	tp_dev.xfac = -0.085261873;
//	tp_dev.yfac = -0.127167627;
//	tp_dev.xoff = 335;
//	tp_dev.yoff = 497;    //��
	tp_dev.xfac = 0.0851581544;
	tp_dev.yfac = 0.1263278723;
	tp_dev.xoff = -18;
	tp_dev.yoff = -17;      //��
	tp_dev.touchtype = 0;
	
	if(tp_dev.touchtype)//X,Y��������Ļ�෴
	{
		CMD_RDX=0X90;
		CMD_RDY=0XD0;	 
	}else				   //X,Y��������Ļ��ͬ
	{
		CMD_RDX=0XD0;
		CMD_RDY=0X90;	 
	}
}
		 
//������У׼����
//�õ��ĸ�У׼����
void TP_Adjust(void)
{								 
	u16 pos_temp[4][2];//���껺��ֵ
	u8  cnt=0;	
	u16 d1,d2;
	u32 tem1,tem2;
	double fac; 	
	u16 outtime=0;
 	cnt=0;				
	POINT_COLOR=BLUE;
	BACK_COLOR =WHITE;
	LCD_Clear(WHITE);//����   
	POINT_COLOR=RED;//��ɫ 
	LCD_Clear(WHITE);//���� 	   
	POINT_COLOR=BLACK;
	LCD_ShowString(40,40,160,100,16,(u8*)TP_REMIND_MSG_TBL);//��ʾ��ʾ��Ϣ
	TP_Drow_Touch_Point(20,20,RED);//����1 
	tp_dev.sta=0;//���������ź� 
	tp_dev.xfac=0;//xfac��������Ƿ�У׼��,����У׼֮ǰ�������!�������	 
	while(1)//�������10����û�а���,���Զ��˳�
	{
		tp_dev.scan(1);//ɨ����������
		if((tp_dev.sta&0xc0)==TP_CATH_PRES)//����������һ��(��ʱ�����ɿ���.)
		{	
			outtime=0;		
			tp_dev.sta&=~(1<<6);//��ǰ����Ѿ����������.
						   			   
			pos_temp[cnt][0]=tp_dev.x[0];
			pos_temp[cnt][1]=tp_dev.y[0];
			cnt++;	  
			switch(cnt)
			{			   
				case 1:						 
					TP_Drow_Touch_Point(20,20,WHITE);				//�����1 
					TP_Drow_Touch_Point(lcddev.width-20,20,RED);	//����2
					break;
				case 2:
 					TP_Drow_Touch_Point(lcddev.width-20,20,WHITE);	//�����2
					TP_Drow_Touch_Point(20,lcddev.height-20,RED);	//����3
					break;
				case 3:
 					TP_Drow_Touch_Point(20,lcddev.height-20,WHITE);			//�����3
 					TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,RED);	//����4
					break;
				case 4:	 //ȫ���ĸ����Ѿ��õ�
	    		    //�Ա����
					tem1=abs(pos_temp[0][0]-pos_temp[1][0]);//x1-x2
					tem2=abs(pos_temp[0][1]-pos_temp[1][1]);//y1-y2
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//�õ�1,2�ľ���
					
					tem1=abs(pos_temp[2][0]-pos_temp[3][0]);//x3-x4
					tem2=abs(pos_temp[2][1]-pos_temp[3][1]);//y3-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//�õ�3,4�ľ���
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05||d1==0||d2==0)//���ϸ�
					{
						cnt=0;
 				    	TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);	//�����4
   	 					TP_Drow_Touch_Point(20,20,RED);								//����1
 						TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//��ʾ����   
 						continue;
					}
					tem1=abs(pos_temp[0][0]-pos_temp[2][0]);//x1-x3
					tem2=abs(pos_temp[0][1]-pos_temp[2][1]);//y1-y3
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//�õ�1,3�ľ���
					
					tem1=abs(pos_temp[1][0]-pos_temp[3][0]);//x2-x4
					tem2=abs(pos_temp[1][1]-pos_temp[3][1]);//y2-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//�õ�2,4�ľ���
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05)//���ϸ�
					{
						cnt=0;
 				    	TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);	//�����4
   	 					TP_Drow_Touch_Point(20,20,RED);								//����1
 						TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//��ʾ����   
						continue;
					}//��ȷ��
								   
					//�Խ������
					tem1=abs(pos_temp[1][0]-pos_temp[2][0]);//x1-x3
					tem2=abs(pos_temp[1][1]-pos_temp[2][1]);//y1-y3
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//�õ�1,4�ľ���
	
					tem1=abs(pos_temp[0][0]-pos_temp[3][0]);//x2-x4
					tem2=abs(pos_temp[0][1]-pos_temp[3][1]);//y2-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//�õ�2,3�ľ���
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05)//���ϸ�
					{
						cnt=0;
 				    	TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);	//�����4
   	 					TP_Drow_Touch_Point(20,20,RED);								//����1
 						TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//��ʾ����   
						continue;
					}//��ȷ��
					//������
					tp_dev.xfac=(float)(lcddev.width-40)/(pos_temp[1][0]-pos_temp[0][0]);//�õ�xfac		 
					tp_dev.xoff=(lcddev.width-tp_dev.xfac*(pos_temp[1][0]+pos_temp[0][0]))/2;//�õ�xoff
						  
					tp_dev.yfac=(float)(lcddev.height-40)/(pos_temp[2][1]-pos_temp[0][1]);//�õ�yfac
					tp_dev.yoff=(lcddev.height-tp_dev.yfac*(pos_temp[2][1]+pos_temp[0][1]))/2;//�õ�yoff  
					if(abs(tp_dev.xfac)>2||abs(tp_dev.yfac)>2)//������Ԥ����෴��.
					{
						cnt=0;
 				    	TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);	//�����4
   	 					TP_Drow_Touch_Point(20,20,RED);								//����1
						LCD_ShowString(40,26,lcddev.width,lcddev.height,16,"TP Need readjust!");
						tp_dev.touchtype=!tp_dev.touchtype;//�޸Ĵ�������.
						if(tp_dev.touchtype)//X,Y��������Ļ�෴
						{
							CMD_RDX=0X90;
							CMD_RDY=0XD0;	 
						}else				   //X,Y��������Ļ��ͬ
						{
							CMD_RDX=0XD0;
							CMD_RDY=0X90;	 
						}			    
						continue;
					}		
					POINT_COLOR=BLUE;
					LCD_Clear(WHITE);//����
					LCD_ShowString(35,110,lcddev.width,lcddev.height,16,"Touch Screen Adjust OK!");//У�����
					delay_ms(1000);
//					TP_Save_Adjdata();  
 					LCD_Clear(WHITE);//����   
					return;//У�����				 
			}
		}
		delay_ms(10);
		outtime++;
		if(outtime>10000)
		{
//			TP_Get_Adjdata();
			break;
	 	} 
 	}
}	
//��������ʼ��  		    
//����ֵ:0,û�н���У׼
//       1,���й�У׼
u8 TP_Init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;	
	
//	if(lcddev.id==0X5510)		//���ݴ�����
//	{
//		if(GT9147_Init()==0)	//��GT9147
//		{ 
//			tp_dev.scan=GT9147_Scan;	//ɨ�躯��ָ��GT9147������ɨ��
//		}else
//		{
//			OTT2001A_Init();
//			tp_dev.scan=OTT2001A_Scan;	//ɨ�躯��ָ��OTT2001A������ɨ��
//		}
//		tp_dev.touchtype|=0X80;	//������ 
//		tp_dev.touchtype|=lcddev.dir&0X01;//������������ 
//		return 0;
//	}else if(lcddev.id==0X1963)
//	{
//		FT5206_Init();
//		tp_dev.scan=FT5206_Scan;		//ɨ�躯��ָ��GT9147������ɨ��		
//		tp_dev.touchtype|=0X80;			//������ 
//		tp_dev.touchtype|=lcddev.dir&0X01;//������������ 
//		return 0;
//	}else
//	{
		
	  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOF, ENABLE);//ʹ��GPIOB,C,Fʱ��

    //GPIOB1,2��ʼ������
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;//PB1/PB2 ����Ϊ��������
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//����ģʽ
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
    GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;//PB0����Ϊ�������
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//���ģʽ
	  GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;//PC13����Ϊ�������
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//���ģʽ
	  GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��	
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PF11�����������
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//���ģʽ
	  GPIO_Init(GPIOF, &GPIO_InitStructure);//��ʼ��			
		
   
		TP_Read_XY(&tp_dev.x[0],&tp_dev.y[0]);//��һ�ζ�ȡ��ʼ��	 
//		AT24CXX_Init();		//��ʼ��24CXX
//		if(TP_Get_Adjdata())return 0;//�Ѿ�У׼
//		else			   //δУ׼?
//		{ 										    
//			LCD_Clear(WHITE);//����
//			TP_Adjust();  	//��ĻУ׼ 
//			TP_Save_Adjdata();	 
//		}			
//		TP_Get_Adjdata();
		if (use_touch_adj_data)
		{
			TP_Adj_Parameters_Set();
		}
		else
		{
			LCD_Clear(WHITE);//����
			TP_Adjust();  	//��ĻУ׼
			TP_Adj_Parameters_Show();
			delay_ms(1000);
			delay_ms(1000);
			delay_ms(1000);
		}
//	}
	return 1; 									 
}

u8 TP_MainMenu_Judge(u16 x, u16 y)
{
	if (KeyBoard_State)
	{
		if (x < lcddev.width/2)
		{
			if (y < 36) return key1;
			else if (y < 36*2) return key2;
			else if (y < 36*3) return key3;
			else if (y < 36*4) return key4;
		}
		else if (x < lcddev.width)
		{
			if (y < 36) return key5;
			else if (y < 36*2) return key6;
			else if (y < 36*3) return key7;
			else if (y < 36*4) return key8;
			else
			{
				if (x > 3*lcddev.width/4)
				{
					if (y > lcddev.height-1-48)
						return keyboardtab;
					else if (y > lcddev.height-1-2*48)
						return keyback;
				}
			}
		}
	}
	else
	{
		if (x < lcddev.width/2)
		{
			if (y < 36) return key1;
			else if (y < 36*2) return key2;
			else if (y < 36*3) return key3;
			else if (y < 36*4) return key4;
			else if (y > lcddev.height-1-48) return keyback;
		}
		else if (x < lcddev.width)
		{
			if (y < 36) return key5;
			else if (y < 36*2) return key6;
			else if (y < 36*3) return key7;
			else if (y < 36*4) return key8;
			else if (y > lcddev.height-1-48) return keyboardtab;
		}
	}
	return keyempty1;
}

u8 TP_Keyboard_Judge(u16 x, u16 y)
{
	if (KeyBoard_State)
	{
		if (x < lcddev.width/4)
		{
			if (y > lcddev.height-1-48) return keysign;
			else if (y > lcddev.height-1-48*2) return key1;
			else if (y > lcddev.height-1-48*3) return key4;
			else if (y > lcddev.height-1-48*4) return key7;
		}
		else if (x < lcddev.width/2)
		{
			if (y > lcddev.height-1-48) return key0;
			else if (y > lcddev.height-1-48*2) return key2;
			else if (y > lcddev.height-1-48*3) return key5;
			else if (y > lcddev.height-1-48*4) return key8;
		}
		else if (x < lcddev.width*3/4)
		{
			if (y > lcddev.height-1-48) return keypoint;
			else if (y > lcddev.height-1-48*2) return key3;
			else if (y > lcddev.height-1-48*3) return key6;
			else if (y > lcddev.height-1-48*4) return key9;
		}
		else if (x < lcddev.width)
		{
			if (y > lcddev.height-1-48) return keyboardtab;
			else if (y > lcddev.height-1-48*2) return keyback;
			else if (y > lcddev.height-1-48*3) return keydelete;
			else if (y > lcddev.height-1-48*4) return keyOK;
		}
	}
	else
	{
		if (x < lcddev.width/2)
		{
			if (y > lcddev.height-1-48) return keyback;
		}
		else if (x < lcddev.width)
		{
			if (y > lcddev.height-1-48) return keyboardtab;
		}
	}
	return keyempty1;
}

u8 TP_Row_Judge(u16 x, u16 y)
{	
	if (KeyBoard_State)
	{
		if (x < lcddev.width/4)
		{
			if (y > lcddev.height-1-48) return keysign;
			else if (y > lcddev.height-1-48*2) return key1;
			else if (y > lcddev.height-1-48*3) return key4;
			else if (y > lcddev.height-1-48*4) return key7;
		}
		else if (x < lcddev.width/2)
		{
			if (y > lcddev.height-1-48) return key0;
			else if (y > lcddev.height-1-48*2) return key2;
			else if (y > lcddev.height-1-48*3) return key5;
			else if (y > lcddev.height-1-48*4) return key8;
		}
		else if (x < lcddev.width*3/4)
		{
			if (y > lcddev.height-1-48) return keypoint;
			else if (y > lcddev.height-1-48*2) return key3;
			else if (y > lcddev.height-1-48*3) return key6;
			else if (y > lcddev.height-1-48*4) return key9;
		}
		else if (x < lcddev.width)
		{
			if (y > lcddev.height-1-48) return keyboardtab;
			else if (y > lcddev.height-1-48*2) return keyback;
			else if (y > lcddev.height-1-48*3) return keydelete;
			else if (y > lcddev.height-1-48*4) return keyOK;
		}
	}
	else
	{
		if (y < 36) return key1;
		else if (y < 36*2) return key2;
		else if (y < 36*3) return key3;
		else if (y < 36*4) return key4;
		else if (y < 36*5) return key5;
		else if (y < 36*6) return key6;
		else if (y < 36*7) return key7;
		else if (y < 36*8) return key8;
		else if (y < 36*9) return key9;
		else if (y > lcddev.height-1-48 && x < lcddev.width/2) return keyback;
		else if (y > lcddev.height-1-48 && x < lcddev.width) return keyboardtab;
	}
	return keyempty1;
}

void Input_IntValue(int *address,char *name)//��������
{
	u8 tp_last, key_value, is_key = 0;
	u8 pre_KeyBoard_State = KeyBoard_State;
    
	char str[20]={'\0'};
	int temp;
	int i,j;

	str[0]=' ';

	i=1;
	
	KeyBoard_State = 1;
	Show_Keyboard();
	LCD_ShowString(0,6,300,24,24,(u8*)name);
	LCD_printf(0,6+36,300,24,24,"old:%d",*address);
	LCD_printf(0,6+2*36,300,24,24,"new:%s",str);
	for(;;)
	{		
		#ifdef BSP_USING_TOUCH_SCREEN
		tp_last = tp_dev.sta&TP_PRES_DOWN;
		tp_dev.scan(0);
		if ((tp_dev.sta&TP_PRES_DOWN) && !tp_last)
		{
			key_value = TP_Keyboard_Judge(tp_dev.x[0], tp_dev.y[0]);
			if (key_value != keyempty1)
				is_key = 1;
		}
		#endif
		#ifdef BSP_USING_USART_KEY
		#ifdef BSP_USING_TOUCH_SCREEN
		else if (Is_USART_Key_Receive)
		#else
		if (Is_USART_Key_Receive)
		#endif
		{
			Is_USART_Key_Receive = 0;
			key_value = Key_RxBuffer[0];
			if (key_value != keyempty1)
				is_key = 1;
		}
		#endif
		if (is_key)
		{
			is_key = 0;
			switch(key_value)
			{
				case key1://1
					if(i<14)
						str[i++]='1';
					break;
				case key2://2
					if(i<14)
						str[i++]='2';
					break;
				case key3://3
					if(i<14)
						str[i++]='3';
					break;
				case key4://4
					if(i<14)
						str[i++]='4';
					break;
				case key5://5
					if(i<14)
						str[i++]='5';
					break;
				case key6://6
					if(i<14)
						str[i++]='6';
					break;
				case key7://7
					if(i<14)
						str[i++]='7';
					break;
				case key8://8
					if(i<14)
						str[i++]='8';
					break;
				case key9://9
					if(i<14)
						str[i++]='9';
					break;
				case key0://0
					if(i<14)
						str[i++]='0';
					break;
				case keysign://-
					if(str[0]==' ')
						str[0]='-';
					else
						str[0]=' ';
					break;
				case keydelete://del
					if(i>1)
					{
						str[--i]='\0';
					}
					break;
				case keyOK://ok
					temp=0;
					for(j=1;j<i;j++)//��������ֵ
					{
						temp=temp*10+str[j]-'0';
					}
					if(str[0]=='-')
							temp=-temp;
					*address=temp;
					KeyBoard_State = pre_KeyBoard_State;
					return;
				case keyback://cancel
					KeyBoard_State = pre_KeyBoard_State;
					return;
				case keyboardtab:
					KeyBoard_State = !KeyBoard_State;
					Show_Keyboard();
					LCD_ShowString(0,6,300,24,24,(u8*)name);
					LCD_printf(0,6+36,300,24,24,"old:%d",*address);
					LCD_printf(0,6+2*36,300,24,24,"new:");
					break;
			}
			LCD_Fill(4*12,6+2*36,lcddev.width-1,6+2*36+24,WHITE);
			LCD_printf(4*12,6+2*36,300,24,24,"%s",str);
		}
		else
			delay_ms(1);
	}
}

void Input_FloatValue(float * address,char *name)//��������
{
	u8 tp_last, key_value, is_key = 0;
	u8 pre_KeyBoard_State = KeyBoard_State;
	float point=0;
	char str[20]={'\0'};
	float temp;
	int i,j;

	str[0]=' ';
	
	i=1;//��1��ʼ��0λ������
	point=0;
	KeyBoard_State = 1;
	Show_Keyboard();
	LCD_ShowString(0,6,300,24,24,(u8*)name);
	LCD_printf(0,6+36,300,24,24,"old:%lf",*address);
	LCD_printf(0,6+2*36,300,24,24,"new:%s",str);
	
	for(;;)
	{		
		#ifdef BSP_USING_TOUCH_SCREEN
		tp_last = tp_dev.sta&TP_PRES_DOWN;
		tp_dev.scan(0);
		if ((tp_dev.sta&TP_PRES_DOWN) && !tp_last)
		{
			key_value = TP_MainMenu_Judge(tp_dev.x[0], tp_dev.y[0]);
			if (key_value != keyempty1)
				is_key = 1;
		}
		#endif
		#ifdef BSP_USING_USART_KEY
		#ifdef BSP_USING_TOUCH_SCREEN
		else if (Is_USART_Key_Receive)
		#else
		if (Is_USART_Key_Receive)
		#endif
		{
			Is_USART_Key_Receive = 0;
			key_value = Key_RxBuffer[0];
			if (key_value != keyempty1)
				is_key = 1;
		}
		#endif
		if (is_key)
		{
			is_key = 0;
			switch(key_value)
			{
				case key1://1
					if(i<14)
						str[i++]='1';
					break;
				case key2://2
					if(i<14)
						str[i++]='2';
					break;
				case key3://3
					if(i<14)
						str[i++]='3';
					break;
				case key4://4
					if(i<14)
						str[i++]='4';
					break;
				case key5://5
					if(i<14)
						str[i++]='5';
					break;
				case key6://6
					if(i<14)
						str[i++]='6';
					break;
				case key7://7
					if(i<14)
						str[i++]='7';
					break;
				case key8://8
					if(i<14)
						str[i++]='8';
					break;
				case key9://9
					if(i<14)
						str[i++]='9';
					break;
				case key0://0
					if(i<14)
						str[i++]='0';
					break;
				case keypoint://.
					if(point==0&&i<14)
					{
						str[i++]='.';
						point=1;
					}
					break;
				case keysign://-
					if(str[0] == ' ')
						str[0] = '-';
					else
						str[0] = ' ';
					break;
				case keydelete://del
					if(i>1)
					{
						if(str[i-1]=='.')
							point=0;
						str[--i]='\0';
					}
					break;
				case keyOK://ok
					temp=0;
					point=0;
					for(j=1;j<i;j++)// 
					{
						if(str[j]=='.')
						{
							point=10;
						}
						else if(point==0)
						{
							temp=temp*10+str[j]-'0';
						}
						else
						{
							temp+=(str[j]-'0')/(float)point;
							point=point*10;
						}
					}
					if(str[0]=='-')
						temp=-temp;
					*address=temp;
					KeyBoard_State = pre_KeyBoard_State;
					return;
				case keyback://cancel
					KeyBoard_State = pre_KeyBoard_State;
					return;
				case keyboardtab:
					KeyBoard_State = !KeyBoard_State;
					Show_Keyboard();
					LCD_ShowString(0,6,300,24,24,(u8*)name);
					LCD_printf(0,6+36,300,24,24,"old:%d",*address);
					LCD_printf(0,6+2*36,300,24,24,"new:");
					break;
			}
			LCD_Fill(4*12,6+2*36,lcddev.width-1,6+2*36+24,WHITE);
			LCD_printf(4*12,6+2*36,300,24,24,"%s",str);
		}
		else
			delay_ms(1);
	}
}

void Input_DoubleValue(double * address,char *name)//��������
{
	u8 tp_last, key_value, is_key = 0;
	u8 pre_KeyBoard_State = KeyBoard_State;
	double point=0;
	char str[20]={'\0'};
	double temp;
	int i,j;

	str[0]=' ';
	
	i=1;//��1��ʼ��0λ������
	point=0;
	KeyBoard_State = 1;
	Show_Keyboard();
	LCD_ShowString(0,6,300,24,24,(u8*)name);
	LCD_printf(0,6+36,300,24,24,"old:%lf",*address);
	LCD_printf(0,6+2*36,300,24,24,"new:%s",str);
	
	for(;;)
	{		
		#ifdef BSP_USING_TOUCH_SCREEN
		tp_last = tp_dev.sta&TP_PRES_DOWN;
		tp_dev.scan(0);
		if ((tp_dev.sta&TP_PRES_DOWN) && !tp_last)
		{
			key_value = TP_MainMenu_Judge(tp_dev.x[0], tp_dev.y[0]);
			if (key_value != keyempty1)
				is_key = 1;
		}
		#endif
		#ifdef BSP_USING_USART_KEY
		#ifdef BSP_USING_TOUCH_SCREEN
		else if (Is_USART_Key_Receive)
		#else
		if (Is_USART_Key_Receive)
		#endif
		{
			Is_USART_Key_Receive = 0;
			key_value = Key_RxBuffer[0];
			if (key_value != keyempty1)
				is_key = 1;
		}
		#endif
		if (is_key)
		{
			is_key = 0;
			switch(key_value)
			{
				case key1://1
					if(i<14)
						str[i++]='1';
					break;
				case key2://2
					if(i<14)
						str[i++]='2';
					break;
				case key3://3
					if(i<14)
						str[i++]='3';
					break;
				case key4://4
					if(i<14)
						str[i++]='4';
					break;
				case key5://5
					if(i<14)
						str[i++]='5';
					break;
				case key6://6
					if(i<14)
						str[i++]='6';
					break;
				case key7://7
					if(i<14)
						str[i++]='7';
					break;
				case key8://8
					if(i<14)
						str[i++]='8';
					break;
				case key9://9
					if(i<14)
						str[i++]='9';
					break;
				case key0://0
					if(i<14)
						str[i++]='0';
					break;
				case keypoint://.
					if(point==0&&i<14)
					{
						str[i++]='.';
						point=1;
					}
					break;
				case keysign://-
					if(str[0] == ' ')
						str[0] = '-';
					else
						str[0] = ' ';
					break;
				case keydelete://del
					if(i>1)
					{
						if(str[i-1]=='.')
							point=0;
						str[--i]='\0';
					}
					break;
				case keyOK://ok
					temp=0;
					point=0;
					for(j=1;j<i;j++)// 
					{
						if(str[j]=='.')
						{
							point=10;
						}
						else if(point==0)
						{
							temp=temp*10+str[j]-'0';
						}
						else
						{
							temp+=(str[j]-'0')/(double)point;
							point=point*10;
						}
					}
					if(str[0]=='-')
						temp=-temp;
					*address=temp;
					KeyBoard_State = pre_KeyBoard_State;
					return;
				case keyback://cancel
					KeyBoard_State = pre_KeyBoard_State;
					return;
				case keyboardtab:
					KeyBoard_State = !KeyBoard_State;
					Show_Keyboard();
					LCD_ShowString(0,6,300,24,24,(u8*)name);
					LCD_printf(0,6+36,300,24,24,"old:%d",*address);
					LCD_printf(0,6+2*36,300,24,24,"new:");
					break;
			}
			LCD_Fill(4*12,6+2*36,lcddev.width-1,6+2*36+24,WHITE);
			LCD_printf(4*12,6+2*36,300,24,24,"%s",str);
		}
		else
			delay_ms(1);
	}
}
