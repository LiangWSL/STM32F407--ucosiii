#ifndef _CAN_DATABASE_H
#define _CAN_DATABASE_H
	#include "stm32f4xx.h"
	#include "led.h"
	extern int BALL_X_SHIFTED;
	extern int BALL_Y_SHIFTED;
	
	#ifndef NULL
	#define NULL ((void *)0)
	#endif
	
	#define fp64 double
	#define fp32  float
	
union fi64_to_u8     //fp64�����ݣ���int64_t������ת��u8�����ݽ���can���߷���
	{
		fp64	f64_data;
		int64_t i64_data;
		uint64_t u64_data;
		uint16_t u16_data[4];
		int16_t i16_data[4];
		uint8_t u8_data[8];
	};
	
	union fi32_to_u8     //fp32�����ݣ���int32_t������ת��u8�����ݽ���can���߷���
	{
		fp32	f32_data;
		int32_t i32_data;
		uint32_t u32_data;
		int16_t i16_data[2];
		uint8_t u8_data[4];
	};
	
	union u16_to_u8
	{
		uint16_t u16_data;
		uint8_t u8_data[2];
	};

	#define READ_ONLY  0	//���ض�������д
	#define WRITE_ONLY 1	//����д�������
	#define Control    0  //��������
	#define Message    1	//��Ϣ����	
	#define MotorBaseID 0x200

/*���������Ӧ��ID���б�*/
/*
IDλ��	[	(GS:GPS Slave  MS:Motor Slave)							]
NO.7	[			0--GS			|		1--Others				]
NO.6	[	0--MOSI		|	1--MISO	|	0--MS MOSI	|	1--Others	]
NO.5~0	[	ID numbers				|	ID numbers  &  Others		]
	
	Others��
		B11-000-xxx ����&&ISOЭ����ͨ��ID
		B11-001-xxx ����&&cameraͨ��ID--MISO
		B11-011-xxx ����&&cameraͨ��ID--MOSI
		B11-010-xxx ������relayͨ��ID
		B11-100-xxx �����벿��Э����ͨ��ID   ��������������
*/
	typedef enum
	{
		W_MaxSpeed_ID = 0x1C,
		W_StartMoving_ID = 0x1D,
		W_DelayTime_ID = 0x1E,
			
		ControlMotorID = 0x200,
		ControlMotorID2 = 0x1FF,
		MOTOR1 = MotorBaseID + 1,
		MOTOR2 = MotorBaseID + 2,
		MOTOR3 = MotorBaseID + 3,
		MOTOR4 = MotorBaseID + 4,
//B0x-xxx-xxx ����&&GPSЭ����ͨ��ID*/
    //MOSI   B00-xxx-xxx
        M_G_CMD_ID      = 0x00,        //������GPS�巢�����ID
        //д��������
        
            //B00-001-xxx  дGPS����
        W_GPS_X_ID      = 0x08,
        W_GPS_Y_ID      = 0x09,
        W_GPS_ANG_ID    = 0x0A,
            //B00-010-xxx  дGYRO����
        W_GYRO_C1_ID    = 0x10,
        W_GYRO_C2_ID    = 0x11,
        W_ENC_COEFF_ID  = 0x30,
    //MISO   B01-xxx-xxx
        G_M_CMD_ID      = 0x40,        //GPS�������ط������ID
        //����������
            //B01-001-xxx  ��GPS����
        S_GPS_X_ID      = 0x48,
        S_GPS_Y_ID      = 0x49,
        S_GPS_ANG_ID    = 0x4A,
            //B01-010-xxx  ��GYRO����
        S_GYRO_C1_ID    = 0x50,
        S_GYRO_C2_ID    = 0x51,
        S_GYRO_RAD_ID   = 0x52,
        S_GYRO_FLOAT_ID =0x53,
            //B01-011-xxx  ��ENC1����
        S_ENC1_C1_ID    = 0x58,
        S_ENC1_C2_ID    = 0x59,
        S_ENC1_RAD_ID   = 0x5A,
        S_ENC1_ANG_ID   = 0x5B,
        S_ENC1_DIS_ID   = 0x5C,
            //B01-100-xxx  ��ENC2����
        S_ENC2_C1_ID    = 0x60,
        S_ENC2_C2_ID    = 0x61,
        S_ENC2_RAD_ID   = 0x62,
        S_ENC2_ANG_ID   = 0x63,
        S_ENC2_DIS_ID   = 0x64,
        S_GPS_ERROR_ID  = 0x65,	

	}ID_NUMDEF;	
	
	/*���غ�GPSЭ����֮��������ʽ*/
	typedef enum
	{
		NO_COMMAND			= 0x00,
		//M_G_CMD_ID�µ�����
		GPS_READ			= 0x01,		//��ȡGPS��Ϣ����
		CHECK_FLOAT			= 0x02,		//֪ͨGPSЭ����У׼��������Ư����
		GYRO_INIT			= 0x03,		//֪ͨGPSЭ���ر궨����������
		ENC_L_INIT			= 0x04,		//֪ͨGPSЭ���ر궨��������תϵ������
		ENC_R_INIT			= 0x05,		//֪ͨGPSЭ���ر궨������ת�뾶����
		//G_M_CMD_ID�µ�����
		CF_NORMAL			= 0x06,
		CF_CHANGED			= 0x07,
		CF_ERROR			= 0x08
	}COM_TYPE;
	
	typedef enum
	{
		FIND_ONCE = 0x01,
		FIND_CONTINIOUS = 0x02,
		FIND_NO_SEND = 0x03,
		FIND_STOP = 0x00
	}CAMERA_CMD;
	
	typedef struct
	{
		uint8_t  Data_type;
		ID_NUMDEF  Data_ID;
		uint8_t* Data_ptr;
		uint8_t  Data_length;
		
		//��can���߽ӵ����ݺ󣬵��õĺ���
		void (*MenuFunc)(void);			//��ں���		
		uint8_t  Channel;
		uint8_t  Fifo_num;			//�ڽ��շ�����ID���õ�fifo��
	}Can_Data;

	
	extern uint8_t Can_Data_Num;
	extern Can_Data Can_Database[];
	extern uint16_t HASH_TABLE[1000];
	extern union fi64_to_u8 CtrlCurrent;
	extern union fi64_to_u8 Motor2Stm[];
	extern union fi32_to_u8 Angle;
	extern union fi64_to_u8 MISO_GYRO_RAD;
	extern uint8_t MR2_StartFlag;
	extern uint8_t RestartFlag;
	extern uint8_t MR2_RiseFlag;
	extern uint8_t Cylinder_Flag;
	extern int ball_x,ball_y;
	  extern uint8_t MOSI_CMD;
    extern uint8_t MISO_CMD;
		extern union u16_to_u8 Angle_up;
		extern union u16_to_u8 Angle_down;
	
	  extern union fi64_to_u8 MOSI_GPS_X;
    extern union fi64_to_u8 MOSI_GPS_Y;
    extern union fi64_to_u8 MOSI_GPS_ANG;
    extern union fi64_to_u8 MOSI_GYRO_C1;
    extern union fi64_to_u8 MOSI_GYRO_C2;
    extern union fi64_to_u8 MOSI_ENC_COEFF;
	
	  extern union fi64_to_u8 MISO_GPS_X;
    extern union fi64_to_u8 MISO_GPS_Y;
    extern union fi64_to_u8 MISO_GPS_ANG;
    extern union fi64_to_u8 MISO_GYRO_C1;
    extern union fi64_to_u8 MISO_GYRO_C2;
    extern union fi64_to_u8 MISO_GYRO_RAD;
    extern union fi64_to_u8 MISO_ENC1_C1;
    extern union fi64_to_u8 MISO_ENC1_C2;
    extern union fi64_to_u8 MISO_ENC1_RAD;
    extern union fi64_to_u8 MISO_ENC1_ANG;
    extern union fi64_to_u8 MISO_ENC1_DIS;
    extern union fi64_to_u8 MISO_ENC2_C1;
    extern union fi64_to_u8 MISO_ENC2_C2;
    extern union fi64_to_u8 MISO_ENC2_RAD;
    extern union fi64_to_u8 MISO_ENC2_ANG;
    extern union fi64_to_u8 MISO_ENC2_DIS;
    extern union fi64_to_u8 MISO_GYRO_FLOAT;
	extern float Ang_up;
	extern float Ang_down;
	
	void Hash_Table_init(void);
	void Write_Database(ID_NUMDEF ID_NUM);
typedef union 
{
    u8 d[2];
    s16 v;
}u8tos16_t;

typedef struct
{
		u8tos16_t	x_or_angle;
		u8tos16_t	y_or_dis;
		u8tos16_t	z;
	  u8tos16_t	turn;  
		int time_flag;
}camera_t;

extern union fi64_to_u8 camera_data1;
extern uint8_t MISO_FAN_GRAB_STATUS;
extern uint8_t M0SI_FAN_GRAB;

#endif


