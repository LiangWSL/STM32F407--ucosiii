#include "string.h"
#include "exfuns.h"
#include "fattester.h"	
#include "malloc.h"
#include "usart.h"
#include "stdio.h"
#include "stdarg.h"
#include "stdlib.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//FATFS ��չ����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/15
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	
#define FILE_MAX_TYPE_NUM		7	//���FILE_MAX_TYPE_NUM������
#define FILE_MAX_SUBT_NUM		4	//���FILE_MAX_SUBT_NUM��С��

 //�ļ������б�
u8*const FILE_TYPE_TBL[FILE_MAX_TYPE_NUM][FILE_MAX_SUBT_NUM]=
{
{"BIN"},			//BIN�ļ�
{"LRC"},			//LRC�ļ�
{"NES"},			//NES�ļ�
{"TXT","C","H"},	//�ı��ļ�
{"WAV","MP3","APE","FLAC"},//֧�ֵ������ļ�
{"BMP","JPG","JPEG","GIF"},//ͼƬ�ļ�
{"AVI"},			//��Ƶ�ļ�
};
///////////////////////////////�����ļ���,ʹ��malloc��ʱ��////////////////////////////////////////////
FATFS *fs[_VOLUMES];//�߼����̹�����.	 
FIL *file;	  		//�ļ�1
FIL *ftemp;	  		//�ļ�2.
UINT br,bw;			//��д����
FILINFO fileinfo;	//�ļ���Ϣ
DIR dir;  			//Ŀ¼

u8 *fatbuf;			//SD�����ݻ�����
///////////////////////////////////////////////////////////////////////////////////////
//Ϊexfuns�����ڴ�
//����ֵ:0,�ɹ�
//1,ʧ��
u8 exfuns_init(void)
{
	u8 i;
	for(i=0;i<_VOLUMES;i++)
	{
		fs[i]=(FATFS*)mymalloc(SRAMIN,sizeof(FATFS));	//Ϊ����i�����������ڴ�	
		if(!fs[i])break;
	}
	file=(FIL*)mymalloc(SRAMIN,sizeof(FIL));		//Ϊfile�����ڴ�
	ftemp=(FIL*)mymalloc(SRAMIN,sizeof(FIL));		//Ϊftemp�����ڴ�
	fatbuf=(u8*)mymalloc(SRAMIN,512);				//Ϊfatbuf�����ڴ�
	if(i==_VOLUMES&&file&&ftemp&&fatbuf)return 0;  //������һ��ʧ��,��ʧ��.
	else return 1;	
}

//��Сд��ĸתΪ��д��ĸ,���������,�򱣳ֲ���.
u8 char_upper(u8 c)
{
	if(c<'A')return c;//����,���ֲ���.
	if(c>='a')return c-0x20;//��Ϊ��д.
	else return c;//��д,���ֲ���
}	      
//�����ļ�������
//fname:�ļ���
//����ֵ:0XFF,��ʾ�޷�ʶ����ļ����ͱ��.
//		 ����,����λ��ʾ��������,����λ��ʾ����С��.
u8 f_typetell(u8 *fname)
{
	u8 tbuf[5];
	u8 *attr='\0';//��׺��
	u8 i=0,j;
	while(i<250)
	{
		i++;
		if(*fname=='\0')break;//ƫ�Ƶ��������.
		fname++;
	}
	if(i==250)return 0XFF;//������ַ���.
 	for(i=0;i<5;i++)//�õ���׺��
	{
		fname--;
		if(*fname=='.')
		{
			fname++;
			attr=fname;
			break;
		}
  	}
	strcpy((char *)tbuf,(const char*)attr);//copy
 	for(i=0;i<4;i++)tbuf[i]=char_upper(tbuf[i]);//ȫ����Ϊ��д 
	for(i=0;i<FILE_MAX_TYPE_NUM;i++)	//����Ա�
	{
		for(j=0;j<FILE_MAX_SUBT_NUM;j++)//����Ա�
		{
			if(*FILE_TYPE_TBL[i][j]==0)break;//�����Ѿ�û�пɶԱȵĳ�Ա��.
			if(strcmp((const char *)FILE_TYPE_TBL[i][j],(const char *)tbuf)==0)//�ҵ���
			{
				return (i<<4)|j;
			}
		}
	}
	return 0XFF;//û�ҵ�		 			   
}	 

//�õ�����ʣ������
//drv:���̱��("0:"/"1:")
//total:������	 ����λKB��
//free:ʣ������	 ����λKB��
//����ֵ:0,����.����,�������
u8 exf_getfree(u8 *drv,u32 *total,u32 *free)
{
	FATFS *fs1;
	u8 res;
    u32 fre_clust=0, fre_sect=0, tot_sect=0;
    //�õ�������Ϣ�����д�����
    res =(u32)f_getfree((const TCHAR*)drv, (DWORD*)&fre_clust, &fs1);
    if(res==0)
	{											   
	    tot_sect=(fs1->n_fatent-2)*fs1->csize;	//�õ���������
	    fre_sect=fre_clust*fs1->csize;			//�õ�����������	   
#if _MAX_SS!=512				  				//������С����512�ֽ�,��ת��Ϊ512�ֽ�
		tot_sect*=fs1->ssize/512;
		fre_sect*=fs1->ssize/512;
#endif	  
		*total=tot_sect>>1;	//��λΪKB
		*free=fre_sect>>1;	//��λΪKB 
 	}
	return res;
}	

static const char mon_name[12][3] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static void get_date(char date[], u16 *year, u8 *month, u8 *day)
{
	u8 i;
	
	*year = (date[7]-'0')*1000 + (date[8]-'0')*100 + (date[9]-'0')*10 + (date[10]-'0');
	for (i = 0; i < 12; ++i)
		if (strncmp(date, mon_name[i], 3) == 0)
		{
			*month = i + 1;
			break;
		}
	*day = (date[4]-'0')*10 + (date[5]-'0');
}
static void get_time(char time[], u8 *hour, u8 *minute, u8 *second)
{
	*hour = (time[0]-'0')*10 + (time[1]-'0');
	*minute = (time[3]-'0')*10 + (time[4]-'0');
	*second = (time[6]-'0')*10 + (time[7]-'0');
}

void Get_Program_Date(u16 *year, u8 *month, u8 *day)
{
	get_date(__DATE__, year, month, day);
}

void Get_Program_Time(u8 *hour, u8 *minute, u8 *second)
{
	get_time(__TIME__, hour, minute, second);
}

FRESULT f_timestamp(const TCHAR* path)
{
	u16 f_year;
	u8 f_month, f_day, f_hour, f_minute, f_second;
	
	Get_Program_Date(&f_year, &f_month, &f_day);
	Get_Program_Time(&f_hour, &f_minute, &f_second);
	
	return f_set_timestamp(path, f_year, f_month, f_day, f_hour, f_minute, f_second);
}

FRESULT f_set_timestamp(const TCHAR* path, u16 f_year, u8 f_month, u8 f_day, u8 f_hour, u8 f_minute, u8 f_second)    /* Pointer to the file name */
{
	FILINFO fno;
	
	fno.fdate = (WORD)(((f_year - 1980) * 512U) | f_month * 32U | f_day);
	fno.ftime = (WORD)(f_hour * 2048U | f_minute * 32U | f_second / 2U);

	return f_utime(path, &fno);
}

FRESULT f_mknewdir(TCHAR* path)
{
	FRESULT res;
    FILINFO fno;
    u32 i = 0;
    char buf[255], ch[32];
	
    strcpy(buf, path);
	while (i < 0xFFFFFFFF)
	{
		sprintf(ch, " %d", i);
		strcpy(path, buf);
		strcat(path, ch);
		res = f_stat(path, &fno);
		if (res != FR_OK)
			break;
		++i;
	}
	if (i == 0xFFFFFFFF)
	{
		sprintf(ch, " %d", 0);
		strcpy(path, buf);
		strcat(path, ch);
	}
    return f_mkdir((const char *)path);
}

FRESULT f_record_init(TCHAR* path)
{
	FRESULT f_res;
	
	f_res = f_mknewdir(path);
	if (f_res == FR_OK)
	{
		f_res = f_timestamp(path);
		f_res = f_opendir(&dir, path);
		f_res = f_closedir(&dir);
		f_res = f_chdir(path);
	}
	
	return f_res;
}

FRESULT f_record(const TCHAR* path, FIL* fp, const TCHAR* fmt,	...)
{
	FRESULT f_res;
	va_list ap;
	char string[128];	
	
	f_res = f_open(fp, path, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);    //FA_OPEN_ALWAYS��������ļ������ڣ��ʹ���
	if (f_res == FR_OK)
	{
		va_start(ap,fmt);
		vsprintf(string,fmt,ap);
		va_end(ap);
		
		f_res = f_lseek(fp, fp->fsize);   // ��������д�����Ǵ�ͷд
		f_printf(fp, string);
		f_res = f_close(fp);                   //ֻ�����˹رղ������ļ��Ż�����
		f_res = f_timestamp(path);
	}
	
	return f_res;
}








