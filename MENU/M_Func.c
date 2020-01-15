/* Includes ------------------------------------------------------------------*/
#include "M_List.h"
#include "M_Func.h"
#include "MyTypeDef.h"
#include "lcd.h"

/* Private typedef -----------------------------------------------------------*/
//typedef void(* func)(void);

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
u8 KeyBoard_State = 0;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*
 * ������: Show_Menu_List
 * ��  ��: �˵���ʾ
 * ��  ��: ��
 * ��  ��: ��
 * ��  ��: �ⲿ����
 */
void Show_Menu_List(void)
{
	int i=0,j=0;
	struct M_Type *M_TypePtr;
	char *str;

	Show_Keyboard();
	
	POINT_COLOR=BLACK;
	for (i=0; i<2; i++)//������ʾ��ÿ��4��
		for(j=0;j<M_NUM/2;j++)
		{
			M_TypePtr = CurrentMenu + (i*(M_NUM/2) + j);
			str = M_TypePtr->Name;
			LCD_ShowString(i*lcddev.width/2,6+j*36,lcddev.width/2,24,24,(u8*)str);
		}
}

void Show_Keyboard(void)
{
	u8 i;
	
	LCD_Clear(WHITE);
	POINT_COLOR=BLACK;
	if (KeyBoard_State == 0)
	{
		LCD_DrawLine(0, lcddev.height-1-48, lcddev.width-1, lcddev.height-1-48);				//����
		LCD_DrawLine(lcddev.width/2, lcddev.height-1-48, lcddev.width/2, lcddev.height-1);		//����
		LCD_ShowString(lcddev.width/4-24, lcddev.height-1-36, 200, 24, 24,"Back");
		LCD_ShowString(3*lcddev.width/4-48, lcddev.height-1-36, 200, 24, 24,"Keyboard");
	}
	else
	{
		for (i = 1; i <= 3; ++i)
		{
			LCD_DrawLine(0, lcddev.height-1-i*48, lcddev.width-1, lcddev.height-1-i*48);					//����
			LCD_DrawLine(i*lcddev.width/4, lcddev.height-1-4*48, i*lcddev.width/4, lcddev.height-1);		//����
		}
		LCD_DrawLine(0, lcddev.height-1-4*48, lcddev.width-1, lcddev.height-1-4*48);						//����
		
		LCD_ShowString(lcddev.width/8-24, lcddev.height-1-48+12, 200, 24, 24,"Sign");
		LCD_ShowString(3*lcddev.width/8-6, lcddev.height-1-48+12, 200, 24, 24,"0");
		LCD_ShowString(5*lcddev.width/8-6, lcddev.height-1-48+12, 200, 24, 24,".");
		LCD_ShowString(7*lcddev.width/8-24, lcddev.height-1-48+12, 200, 24, 24,"Hide");
		LCD_ShowString(lcddev.width/8-6, lcddev.height-1-2*48+12, 200, 24, 24,"1");
		LCD_ShowString(3*lcddev.width/8-6, lcddev.height-1-2*48+12, 200, 24, 24,"2");
		LCD_ShowString(5*lcddev.width/8-6, lcddev.height-1-2*48+12, 200, 24, 24,"3");
		LCD_ShowString(7*lcddev.width/8-24, lcddev.height-1-2*48+12, 200, 24, 24,"Back");
		LCD_ShowString(lcddev.width/8-6, lcddev.height-1-3*48+12, 200, 24, 24,"4");
		LCD_ShowString(3*lcddev.width/8-6, lcddev.height-1-3*48+12, 200, 24, 24,"5");
		LCD_ShowString(5*lcddev.width/8-6, lcddev.height-1-3*48+12, 200, 24, 24,"6");
		LCD_ShowString(7*lcddev.width/8-36, lcddev.height-1-3*48+12, 200, 24, 24,"Delete");
		LCD_ShowString(lcddev.width/8-6, lcddev.height-1-4*48+12, 200, 24, 24,"7");
		LCD_ShowString(3*lcddev.width/8-6, lcddev.height-1-4*48+12, 200, 24, 24,"8");
		LCD_ShowString(5*lcddev.width/8-6, lcddev.height-1-4*48+12, 200, 24, 24,"9");
		LCD_ShowString(7*lcddev.width/8-12, lcddev.height-1-4*48+12, 200, 24, 24,"OK");
	}
}

/*
 * ������: KeyNumOperate
 * ��  ��: ������Ӧ
 * ��  ��: ��
 * ��  ��: ��
 * ��  ��: �ⲿ����
 */
void KeyNumOperate(u8 KeyValue)
{
	switch (KeyValue)
	{           
		case key1:
			GoKeyNum(1);
			break;
		case key2:
			GoKeyNum(2);
			break;
		case key3:
			GoKeyNum(3);
			break;
		case key4:
			GoKeyNum(4);
			break;
		case keyback:
			if(CurrentMenu != MainMenu)
				CurrentMenu = CurrentMenu->Mnode.prev;
			break;
		case key5:
			GoKeyNum(5);
			break;
		case key6:
			GoKeyNum(6);
			break;
		case key7:
			GoKeyNum(7);
			break;
		case key8:
			GoKeyNum(8);
			break;
		case key9:
			GoKeyNum(9);
			break;
		case keyboardtab:
			KeyBoard_State = !KeyBoard_State;
			Show_Keyboard();
		default:
			break;
	}
	Show_Menu_List();
}

/*
 * ������: GoKeyNum
 * ��  ��: ������Ӧ������
 * ��  ��: -KeyNum ����ļ�ֵ
 * ��  ��: ��
 * ��  ��: �ڲ�����
 */
void GoKeyNum(int KeyNum)
{
	struct M_Type *MenuTemp;
	
	MenuTemp = CurrentMenu + KeyNum - 1;
	
	if(MenuTemp->M_Attrib == M_SMenu)//������Ӳ˵��������ǲ˵�����ʾ���Ӳ˵��б�
		CurrentMenu = MenuTemp->Mnode.next;
	else if(MenuTemp->M_Attrib == M_UFunc)//�������û���������ִ�к���
		MenuTemp->MenuFunc(); 
	else if(MenuTemp->M_Attrib == M_Empty) {;}//�����ǿգ���ִ��һ������
}

/********************************    END OF FILE    ***************************/

