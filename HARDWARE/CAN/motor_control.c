#include "can.h"
#include "usart.h"
#include "delay.h"
#include "motor_control.h"
 
extern UART_HandleTypeDef huart1;//not use

CAN_TxHeaderTypeDef TxHeader;
CAN_RxHeaderTypeDef RxHeader;
//uint8_t TxData[8] = {0};
uint32_t TxMailbox; 
static uint32_t std_id = 0x140;  
uint32_t Angle;
uint32_t Speed;
uint8_t Communication_mode;
uint8_t Control_Mode=0;

void Error_Handler(void)
{
	printf("CAN_Error_Handler sending failed\r\n");
}

uint8_t Checksumcrc(uint8_t *aData, uint8_t StartIndex, uint8_t DataLength);

//buf:数据缓存区;	 
//返回值:0,无数据被收到;
//		 其他,接收的数据长度;
static u8 CAN1_Receive_Msg(u8 *std, u8 *buf)
{
 	u32 i;
	u8	RxData[8];

	if(HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) != 1)
	{
		return 0xF1;
	}

	if(HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK)
	{
		return 0xF2;
	}
    RxHeader.StdId = *std;
    for(i=0;i<RxHeader.DLC;i++)
    buf[i]=RxData[i];
	return RxHeader.DLC;
}



void Read_PID(MotorId Motor_ID,motor_pid* pid)
{
  uint8_t TxData[8] = {0};
	uint8_t RxData[8] = {0};
  uint8_t Get_Std_id;
	uint8_t i=0;
	motor_pid getpid;
	
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.IDE = CAN_ID_STD;            
  TxHeader.StdId = std_id + Motor_ID;
  TxHeader.TransmitGlobalTime = DISABLE;
  TxHeader.DLC = 8;
    
  TxData[0] = 0x30;
  TxData[1] = 0;
  TxData[2] = 0;
  TxData[3] = 0;
  TxData[4] = 0; 
  TxData[5] = 0;
  TxData[6] = 0;
  TxData[7] = 0;
       
 if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
 {
	 /* Transmission request Error */
	 Error_Handler();
 }
 
	if(CAN1_Receive_Msg(&Get_Std_id,RxData) < 9)//读取接收到信息
	{	
		printf("CANid:0x%2x, Receive data :",Get_Std_id);		
		for(i=0;i<8;i++)
		{									    
			printf("%02x ",RxData[i]);
		}
		printf("\r\n");
	}
	getpid.anglePidKp = RxData[2];
	getpid.anglePidKi = RxData[3];
	getpid.speedPidKp = RxData[4];
	getpid.speedPidKi = RxData[5];
	getpid.iqPidKp = RxData[6];
	getpid.iqPidKi = RxData[7];
	*pid = getpid;
}
//****************************************************************
//数据写入RAM，断电后参数失效
//****************************************************************
void Write_PID_to_RAM(MotorId Motor_ID,motor_pid pid)
{
	uint8_t TxData[8] = {0};
	uint8_t RxData[8] = {0};
	uint8_t Get_Std_id;
	uint8_t i=0;
	
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.IDE = CAN_ID_STD;            
  TxHeader.StdId = std_id + Motor_ID;
  TxHeader.TransmitGlobalTime = DISABLE;
  TxHeader.DLC = 8;
    
  TxData[0] = 0x30;
  TxData[1] = 0;
  TxData[2] = pid.anglePidKp;
  TxData[3] = pid.anglePidKi;
  TxData[4] = pid.speedPidKp; 
  TxData[5] = pid.speedPidKi;
  TxData[6] = pid.iqPidKp;
  TxData[7] = pid.iqPidKi;
       
 if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
 {
	 /* Transmission request Error */
	 Error_Handler();
 }
 
	if(CAN1_Receive_Msg(&Get_Std_id,RxData) < 9)
	{
		printf("CANid:0x%2x, Receive data :",Get_Std_id);		
		for(i=0;i<8;i++)
		{
      printf("%02x ",RxData[i]);									    
			if(RxData[i] != TxData[i])
			{
				printf("send fail! \r\n");
				break;
			}
		}
		if(i==8)
		printf("send success! \r\n");
				
	}	
}
//****************************************************************
//数据写入ROM，断电后参数仍然有效
//**************************************************************** 
void Write_PID_to_ROM(MotorId Motor_ID,motor_pid pid)
{
  uint8_t TxData[8] = {0};
	uint8_t RxData[8] = {0};
	uint8_t Get_Std_id;
	uint8_t i=0;
	
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.IDE = CAN_ID_STD;            
  TxHeader.StdId = std_id + Motor_ID;
  TxHeader.TransmitGlobalTime = DISABLE;
  TxHeader.DLC = 8;
    
  TxData[0] = 0x31;
  TxData[1] = 0;
  TxData[2] = pid.anglePidKp;
  TxData[3] = pid.anglePidKi;
  TxData[4] = pid.speedPidKp; 
  TxData[5] = pid.speedPidKi;
  TxData[6] = pid.iqPidKp;
  TxData[7] = pid.iqPidKi;
       
 if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
 {
	 /* Transmission request Error */
	 Error_Handler();
 }
 
	if(CAN1_Receive_Msg(&Get_Std_id,RxData) < 9)
	{
		printf("CANid:0x%2x, Receive data :",Get_Std_id);		
		for(i=0;i<8;i++)
		{
      printf("%02x ",RxData[i]);									    
			if(RxData[i] != TxData[i])
			{
				printf("send fail! \r\n");
				break;
			}
		}
		if(i==8)
		printf("send success! \r\n");
		}		
}

//****************************************************************
//读取当前电机的加速度参数
//**************************************************************** 
void Read_Accel(MotorId Motor_ID,int32_t* Accel)
{
  uint8_t TxData[8] = {0};
	uint8_t RxData[8] = {0};
  uint8_t Get_Std_id;
	uint8_t i=0;
	
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.IDE = CAN_ID_STD;            
  TxHeader.StdId = std_id + Motor_ID;
  TxHeader.TransmitGlobalTime = DISABLE;
  TxHeader.DLC = 8;
    
  TxData[0] = 0x33;
  TxData[1] = 0;
  TxData[2] = 0;
  TxData[3] = 0;
  TxData[4] = 0; 
  TxData[5] = 0;
  TxData[6] = 0;
  TxData[7] = 0;
       
 if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
 {
	 /* Transmission request Error */
	 Error_Handler();
 }
 
	if(CAN1_Receive_Msg(&Get_Std_id,RxData) < 9)//读取接收到信息
	{	
		printf("CANid:0x%2x, Receive data :",Get_Std_id);		
		for(i=0;i<8;i++)
		{									    
			printf("%02x ",RxData[i]);
		}
		printf("\r\n");
	}
//加速度占4字节，4-7data，低位在前
	*Accel = (RxData[7]<<(4*3))|(RxData[6]<<(4*2))|(RxData[5]<<(4*1))|(RxData[4]<<(4*0));
}

//****************************************************************
//写入电机的加速度参数，断电后失效
//**************************************************************** 
void Write_Accel_to_RAM(MotorId Motor_ID,int32_t Accel)
{
  uint8_t TxData[8] = {0};
	uint8_t RxData[8] = {0};
	uint8_t Get_Std_id;
	uint8_t i=0;
	
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.IDE = CAN_ID_STD;            
  TxHeader.StdId = std_id + Motor_ID;
  TxHeader.TransmitGlobalTime = DISABLE;
  TxHeader.DLC = 8;
    
  TxData[0] = 0x34;
  TxData[1] = 0;
  TxData[2] = 0;
  TxData[3] = 0;
  TxData[4] = (uint8_t)((Accel&0x000F)>>0);//分别获取Accel每个字节的数据
  TxData[5] = (uint8_t)((Accel&0x00F0)>>4);
  TxData[6] = (uint8_t)((Accel&0x0F00)>>8);
  TxData[7] = (uint8_t)((Accel&0xF000)>>12);
       
 if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
 {
	 /* Transmission request Error */
	 Error_Handler();
 }
 
	if(CAN1_Receive_Msg(&Get_Std_id,RxData) < 9)
	{
		printf("CANid:0x%2x, Receive data :",Get_Std_id);		
		for(i=0;i<8;i++)
		{				
      printf("%02x ",RxData[i]);					    
			if(RxData[i] != TxData[i])
			{
				printf("send fail! \r\n");
				break;
			}
		}
		if(i==8)
		printf("send success! \r\n");
	}	  
}

//****************************************************************
//读取电机的加速度参数
//**************************************************************** 
void Read_Encoder(MotorId Motor_ID,motor_Encoder* Encoder)
{
  uint8_t TxData[8] = {0};
	uint8_t RxData[8] = {0};
  uint8_t Get_Std_id;
	uint8_t i=0;
	motor_Encoder getEncoder;
	
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.IDE = CAN_ID_STD;            
  TxHeader.StdId = std_id + Motor_ID;
  TxHeader.TransmitGlobalTime = DISABLE;
  TxHeader.DLC = 8;
    
  TxData[0] = 0x90;
  TxData[1] = 0;
  TxData[2] = 0;
  TxData[3] = 0;
  TxData[4] = 0; 
  TxData[5] = 0;
  TxData[6] = 0;
  TxData[7] = 0;
       
 if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
 {
	 /* Transmission request Error */
	 Error_Handler();
 }
 
	if(CAN1_Receive_Msg(&Get_Std_id,RxData) < 9)//读取接收到信息
	{	
		printf("CANid:0x%2x, Receive data :",Get_Std_id);		
		for(i=0;i<8;i++)
		{									    
			printf("%02x ",RxData[i]);
		}
		printf("\r\n");
	}
	getEncoder.encoder = (RxData[3]>>4)|RxData[2];
	getEncoder.encoderRaw =(RxData[5]>>4)|RxData[4];
	getEncoder.encoderOffset = (RxData[7]>>4)|RxData[6];

	*Encoder = getEncoder;
}

//****************************************************************
//写入编码器的零位偏移
//****************************************************************
void Write_Expect_Encoder_to_ROM(MotorId Motor_ID,motor_Encoder Encoder)
{
  uint8_t TxData[8] = {0};
	uint8_t RxData[8] = {0};
	uint8_t Get_Std_id;
	uint8_t i=0;
	
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.IDE = CAN_ID_STD;            
  TxHeader.StdId = std_id + Motor_ID;
  TxHeader.TransmitGlobalTime = DISABLE;
  TxHeader.DLC = 8;
    
  TxData[0] = 0x91;
  TxData[1] = 0;
  TxData[2] = 0;
  TxData[3] = 0;
  TxData[4] = 0;
  TxData[5] = 0;
  TxData[6] = (uint8_t)((Encoder.encoderOffset&0x0F)>>0);
  TxData[7] = (uint8_t)((Encoder.encoderOffset&0xF0)>>4);
       
 if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
 {
	 /* Transmission request Error */
	 Error_Handler();
 }
 
	if(CAN1_Receive_Msg(&Get_Std_id,RxData) < 9)
	{
		printf("CANid:0x%2x, Receive data :",Get_Std_id);		
		for(i=0;i<8;i++)
		{				
      printf("%02x ",RxData[i]);					    
			if(RxData[i] != TxData[i])
			{
				printf("send fail! \r\n");
				break;
			}
		}
		if(i==8)
		printf("send success! \r\n");
	}	  
}
//****************************************************************
//写入当前编码器位置作为初始位置写入ROM（电机零点标定），重新上电生效
//****************************************************************
void Write_Current_Encoder_to_ROM(MotorId Motor_ID,motor_Encoder *Encoder)
{
  uint8_t TxData[8] = {0};
	uint8_t RxData[8] = {0};
	uint8_t Get_Std_id;
	uint8_t i=0;
	motor_Encoder getEncoder;

  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.IDE = CAN_ID_STD;            
  TxHeader.StdId = std_id + Motor_ID;
  TxHeader.TransmitGlobalTime = DISABLE;
  TxHeader.DLC = 8;
    
  TxData[0] = 0x19;
  TxData[1] = 0;
  TxData[2] = 0;
  TxData[3] = 0;
  TxData[4] = 0;
  TxData[5] = 0;
  TxData[6] = 0;
  TxData[7] = 0;
       
 if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
 {
	 /* Transmission request Error */
	 Error_Handler();
 }
 
	if(CAN1_Receive_Msg(&Get_Std_id,RxData) < 9)//读取接收到信息
	{	
		printf("CANid:0x%2x, Receive data :",Get_Std_id);		
		for(i=0;i<8;i++)
		{									    
			printf("%02x ",RxData[i]);
		}
		printf("\r\n");
	}
	getEncoder.encoderOffset = (RxData[7]>>4)|RxData[6];
	*Encoder = getEncoder;  
}
//****************************************************************
//读取当前电机多圈绝对角度值，单位0.01°/LSB，正值顺时针累计角度，负值逆时针累计角度
//****************************************************************
void Read_MotorAngle(MotorId Motor_ID,int64_t* angle)
{
  uint8_t TxData[8] = {0};
	uint8_t RxData[8] = {0};
  uint8_t Get_Std_id;
	uint8_t i=0;
	
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.IDE = CAN_ID_STD;            
  TxHeader.StdId = std_id + Motor_ID;
  TxHeader.TransmitGlobalTime = DISABLE;
  TxHeader.DLC = 8;
    
  TxData[0] = 0x92;
  TxData[1] = 0;
  TxData[2] = 0;
  TxData[3] = 0;
  TxData[4] = 0; 
  TxData[5] = 0;
  TxData[6] = 0;
  TxData[7] = 0;
       
 if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
 {
	 /* Transmission request Error */
	 Error_Handler();
 }
 
	if(CAN1_Receive_Msg(&Get_Std_id,RxData) < 9)//读取接收到信息
	{	
		printf("CANid:0x%2x, Receive data :",Get_Std_id);		
		for(i=0;i<8;i++)
		{									    
			printf("%02x ",RxData[i]);
		}
		printf("\r\n");
	}
//角度占6字节，2-7data，低位在前
	*angle = (RxData[4]<<(4*3))|(RxData[3]<<(4*2))|(RxData[2]<<(4*1))|(RxData[1]<<(4*0)) \
            |(RxData[7]<<(4*6))|(RxData[6]<<(4*5))|(RxData[5]<<(4*4));
}
//****************************************************************
//读取当前电机单圈角度值，单位0.01°/LSB，编码器零点为起点，顺时针增加
//****************************************************************
void Read_CircleAngle(MotorId Motor_ID,uint16_t* angle)
{
  uint8_t TxData[8] = {0};
	uint8_t RxData[8] = {0};
  uint8_t Get_Std_id;
	uint8_t i=0;
	
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.IDE = CAN_ID_STD;            
  TxHeader.StdId = std_id + Motor_ID;
  TxHeader.TransmitGlobalTime = DISABLE;
  TxHeader.DLC = 8;
    
  TxData[0] = 0x94;
  TxData[1] = 0;
  TxData[2] = 0;
  TxData[3] = 0;
  TxData[4] = 0; 
  TxData[5] = 0;
  TxData[6] = 0;
  TxData[7] = 0;
       
 if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
 {
	 /* Transmission request Error */
	 Error_Handler();
 }
 
	if(CAN1_Receive_Msg(&Get_Std_id,RxData) < 9)//读取接收到信息
	{	
		printf("CANid:0x%2x, Receive data :",Get_Std_id);		
		for(i=0;i<8;i++)
		{									    
			printf("%02x ",RxData[i]);
		}
		printf("\r\n");
	}
	*angle = (RxData[7]<<(4*1))|(RxData[6]<<(4*0));
}

//****************************************************************
//读取当前电机的温度，电压和错误状态位
//****************************************************************
void Read_MotorState1(MotorId Motor_ID,motor_state* state)
{
  uint8_t TxData[8] = {0};
	uint8_t RxData[8] = {0};
  uint8_t Get_Std_id;
	uint8_t i=0;
	motor_state getstate;
	
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.IDE = CAN_ID_STD;            
  TxHeader.StdId = std_id + Motor_ID;
  TxHeader.TransmitGlobalTime = DISABLE;
  TxHeader.DLC = 8;
    
  TxData[0] = 0x9A;
  TxData[1] = 0;
  TxData[2] = 0;
  TxData[3] = 0;
  TxData[4] = 0; 
  TxData[5] = 0;
  TxData[6] = 0;
  TxData[7] = 0;
       
 if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
 {
	 /* Transmission request Error */
	 Error_Handler();
 }
 
	if(CAN1_Receive_Msg(&Get_Std_id,RxData) < 9)//读取接收到信息
	{	
		printf("CANid:0x%2x, Receive data :",Get_Std_id);		
		for(i=0;i<8;i++)
		{									    
			printf("%02x ",RxData[i]);
		}
		printf("\r\n");
	}
  getstate.temperature = RxData[1];
  getstate.voltage = (RxData[4]>>4)|RxData[3];
  getstate.errorState = RxData[7];

	*state = getstate;
}

//****************************************************************
//读取当前电机的温度，转矩电流Iq，转速，编码器位置
//****************************************************************
void Read_MotorState2(MotorId Motor_ID,motor_state* state)
{
  uint8_t TxData[8] = {0};
	uint8_t RxData[8] = {0};
  uint8_t Get_Std_id;
	uint8_t i=0;
	motor_state getstate;
	
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.IDE = CAN_ID_STD;            
  TxHeader.StdId = std_id + Motor_ID;
  TxHeader.TransmitGlobalTime = DISABLE;
  TxHeader.DLC = 8;
    
  TxData[0] = 0x9C;
  TxData[1] = 0;
  TxData[2] = 0;
  TxData[3] = 0;
  TxData[4] = 0; 
  TxData[5] = 0;
  TxData[6] = 0;
  TxData[7] = 0;
       
 if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
 {
	 /* Transmission request Error */
	 Error_Handler();
 }
 
	if(CAN1_Receive_Msg(&Get_Std_id,RxData) < 9)//读取接收到信息
	{	
		printf("CANid:0x%2x, Receive data :",Get_Std_id);		
		for(i=0;i<8;i++)
		{									    
			printf("%02x ",RxData[i]);
		}
		printf("\r\n");
	}
  getstate.temperature = RxData[1];
  getstate.iq = (RxData[3]>>4)|RxData[2];
  getstate.speed = (RxData[5]>>4)|RxData[4];
  getstate.encoder = (RxData[7]>>4)|RxData[6];
	*state = getstate;
}

//****************************************************************
//读取当前电机的温度，ABC相电流
//****************************************************************
void Read_MotorState3(MotorId Motor_ID,motor_state* state)
{
  uint8_t TxData[8] = {0};
	uint8_t RxData[8] = {0};
  uint8_t Get_Std_id;
	uint8_t i=0;
	motor_state getstate;
	
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.IDE = CAN_ID_STD;            
  TxHeader.StdId = std_id + Motor_ID;
  TxHeader.TransmitGlobalTime = DISABLE;
  TxHeader.DLC = 8;
    
  TxData[0] = 0x9D;
  TxData[1] = 0;
  TxData[2] = 0;
  TxData[3] = 0;
  TxData[4] = 0; 
  TxData[5] = 0;
  TxData[6] = 0;
  TxData[7] = 0;
       
 if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
 {
	 /* Transmission request Error */
	 Error_Handler();
 }
 
	if(CAN1_Receive_Msg(&Get_Std_id,RxData) < 9)//读取接收到信息
	{	
		printf("CANid:0x%2x, Receive data :",Get_Std_id);		
		for(i=0;i<8;i++)
		{									    
			printf("%02x ",RxData[i]);
		}
		printf("\r\n");
	}
  getstate.temperature = RxData[1];
  getstate.phaseA_current = (RxData[3]>>4)|RxData[2];
  getstate.phaseB_current = (RxData[5]>>4)|RxData[4];
  getstate.phaseC_current = (RxData[7]>>4)|RxData[6];
	*state = getstate;
}

//****************************************************************
//清除电机的错误状态，电机收到后返回
//****************************************************************
void Clear_errorState(MotorId Motor_ID)
{
  uint8_t TxData[8] = {0};
	uint8_t RxData[8] = {0};
  uint8_t Get_Std_id;
	uint8_t i=0;
	
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.IDE = CAN_ID_STD;            
  TxHeader.StdId = std_id + Motor_ID;
  TxHeader.TransmitGlobalTime = DISABLE;
  TxHeader.DLC = 8;
    
  TxData[0] = 0x9B;
  TxData[1] = 0;
  TxData[2] = 0;
  TxData[3] = 0;
  TxData[4] = 0; 
  TxData[5] = 0;
  TxData[6] = 0;
  TxData[7] = 0;
       
 if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
 {
	 /* Transmission request Error */
	 Error_Handler();
 }
}

//****************************************************************
//关闭电机，清除控制指令
//****************************************************************
void Motor_Off(MotorId Motor_ID)
{
  uint8_t TxData[8] = {0};
	uint8_t RxData[8] = {0};
	uint8_t Get_Std_id;
	uint8_t i=0;
	
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.IDE = CAN_ID_STD;            
  TxHeader.StdId = std_id + Motor_ID;
  TxHeader.TransmitGlobalTime = DISABLE;
  TxHeader.DLC = 8;
    
  TxData[0] = 0x80;
  TxData[1] = 0;
  TxData[2] = 0;
  TxData[3] = 0;
  TxData[4] = 0; 
  TxData[5] = 0;
  TxData[6] = 0;
  TxData[7] = 0;
       
 if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
 {
	 /* Transmission request Error */
	 Error_Handler();
 }
 
	if(CAN1_Receive_Msg(&Get_Std_id,RxData) < 9)
	{
		printf("CANid:0x%2x, Receive data :",Get_Std_id);		
		for(i=0;i<8;i++)
		{
      printf("%02x ",RxData[i]);									    
			if(RxData[i] != TxData[i])
			{
				printf("send fail! \r\n");
				break;
			}
		}
		if(i==8)
		printf("send success! \r\n");
		}  
}

//****************************************************************
//电机暂停，不清除电机的运行状态及之前收到的控制指令
//****************************************************************
void Motor_Stop(MotorId Motor_ID)
{
  uint8_t TxData[8] = {0};
	uint8_t RxData[8] = {0};
	uint8_t Get_Std_id;
	uint8_t i=0;
	
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.IDE = CAN_ID_STD;            
  TxHeader.StdId = std_id + Motor_ID;
  TxHeader.TransmitGlobalTime = DISABLE;
  TxHeader.DLC = 8;
    
  TxData[0] = 0x81;
  TxData[1] = 0;
  TxData[2] = 0;
  TxData[3] = 0;
  TxData[4] = 0; 
  TxData[5] = 0;
  TxData[6] = 0;
  TxData[7] = 0;
       
 if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
 {
	 /* Transmission request Error */
	 Error_Handler();
 }
 
	if(CAN1_Receive_Msg(&Get_Std_id,RxData) < 9)
	{
		printf("CANid:0x%2x, Receive data :",Get_Std_id);		
		for(i=0;i<8;i++)
		{
      printf("%02x ",RxData[i]);									    
			if(RxData[i] != TxData[i])
			{
				printf("send fail! \r\n");
				break;
			}
		}
		if(i==8)
		printf("send success! \r\n");
		}  
}

//****************************************************************
//电机运行，恢复之前停止前的控制方式
//****************************************************************
void Motor_Run(MotorId Motor_ID)
{
  uint8_t TxData[8] = {0};
	uint8_t RxData[8] = {0};
	uint8_t Get_Std_id;
	uint8_t i=0;
	
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.IDE = CAN_ID_STD;            
  TxHeader.StdId = std_id + Motor_ID;
  TxHeader.TransmitGlobalTime = DISABLE;
  TxHeader.DLC = 8;
    
  TxData[0] = 0x88;
  TxData[1] = 0;
  TxData[2] = 0;
  TxData[3] = 0;
  TxData[4] = 0; 
  TxData[5] = 0;
  TxData[6] = 0;
  TxData[7] = 0;
       
 if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
 {
	 /* Transmission request Error */
	 Error_Handler();
 }
 
	if(CAN1_Receive_Msg(&Get_Std_id,RxData) < 9)
	{
		printf("CANid:0x%2x, Receive data :",Get_Std_id);		
		for(i=0;i<8;i++)
		{
      printf("%02x ",RxData[i]);									    
			if(RxData[i] != TxData[i])
			{
				printf("send fail! \r\n");
				break;
			}
		}
		if(i==8)
		printf("send success! \r\n");
		}  

}


/**
  * @brief  send a Torque control frame via CAN bus
  * @param  motor ID ,1-32
  * @param  iq with the value range of -2000~ 2000, corresponding to the actual torque current range of -32A ~32A
  * @retval null
  */
motor_state iqControl(MotorId Motor_ID, int32_t iqControl)
{
  uint8_t TxData[8] = {0};
  uint8_t RxData[8] = {0};
  uint8_t Get_Std_id;
	uint8_t i=0;
	motor_state getstate;

  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.IDE = CAN_ID_STD;            
  TxHeader.StdId = std_id + Motor_ID;
  TxHeader.TransmitGlobalTime = DISABLE;
  TxHeader.DLC = 8;
    
  TxData[0] = 0xA1;
  TxData[1] = 0;
  TxData[2] = 0;
  TxData[3] = 0;
  TxData[4] = (uint8_t)((iqControl&0x000F)>>0); 
  TxData[5] = (uint8_t)((iqControl&0x00FF)>>4);
  TxData[6] = (uint8_t)((iqControl&0x0FFF)>>8);
  TxData[7] = (uint8_t)((iqControl&0xFFFF)>>12);
        
 if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
 {
   /* Transmission request Error */
   Error_Handler();
 }

 	if(CAN1_Receive_Msg(&Get_Std_id,RxData) < 9)//读取接收到信息
	{	
		printf("CANid:0x%2x, Receive data :",Get_Std_id);		
		for(i=0;i<8;i++)
		{									    
			printf("%02x ",RxData[i]);
		}
		printf("\r\n");
	}
  getstate.temperature = RxData[1];
  getstate.iq = (RxData[3]>>4)|RxData[2];
  getstate.speed = (RxData[5]>>4)|RxData[4];
  getstate.encoder = (RxData[7]>>4)|RxData[6];
	return  getstate;

}

/**
  * @brief  send a speed control frame via CAN bus
  * @param  motor ID ,1-32
  * @param  int32_t corresponding to the actual speed of 0.01 DPS /LSB.
  * @retval null
  */
motor_state speedControl(MotorId Motor_ID, int32_t speedControl)
{
  uint8_t TxData[8] = {0};
  uint8_t RxData[8] = {0};
  uint8_t Get_Std_id;
	uint8_t i=0;
	motor_state getstate;

  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.IDE = CAN_ID_STD;            
  TxHeader.StdId = std_id + Motor_ID;
  TxHeader.TransmitGlobalTime = DISABLE;
  TxHeader.DLC = 8;
    
  TxData[0] = 0xA2;
  TxData[1] = 0;
  TxData[2] = 0;
  TxData[3] = 0;
  TxData[4] = (uint8_t)((speedControl&0x000F)>>0); 
  TxData[5] = (uint8_t)((speedControl&0x00FF)>>4);
  TxData[6] = (uint8_t)((speedControl&0x0FFF)>>8);
  TxData[7] = (uint8_t)((speedControl&0xFFFF)>>12);
        
 if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
 {
   /* Transmission request Error */
   Error_Handler();
 }

 	if(CAN1_Receive_Msg(&Get_Std_id,RxData) < 9)//读取接收到信息
	{	
		printf("CANid:0x%2x, Receive data :",Get_Std_id);		
		for(i=0;i<8;i++)
		{									    
			printf("%02x ",RxData[i]);
		}
		printf("\r\n");
	}
  getstate.temperature = RxData[1];
  getstate.iq = (RxData[3]>>4)|RxData[2];
  getstate.speed = (RxData[5]>>4)|RxData[4];
  getstate.encoder = (RxData[7]>>4)|RxData[6];
	return  getstate;
}

/**
  * @brief  send a angle control frame via CAN bus
  * @param  motor ID ,1-32
  * @param  the actual position is 0.01degree/LSB, 36000 represents 360脗掳
  * @retval null
  */
motor_state Multi_angleControl_1(MotorId Motor_ID, int32_t angleControl)
{
  uint8_t TxData[8] = {0};
  uint8_t RxData[8] = {0};
  uint8_t Get_Std_id;
	uint8_t i=0;
	motor_state getstate;

  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.IDE = CAN_ID_STD;            
  TxHeader.StdId = std_id + Motor_ID;
  TxHeader.TransmitGlobalTime = DISABLE;
  TxHeader.DLC = 8;
    
  TxData[0] = 0xA3;
  TxData[1] = 0;
  TxData[2] = 0;
  TxData[3] = 0;
  TxData[4] = (uint8_t)((angleControl&0x000F)>>0); 
  TxData[5] = (uint8_t)((angleControl&0x00FF)>>4);
  TxData[6] = (uint8_t)((angleControl&0x0FFF)>>8);
  TxData[7] = (uint8_t)((angleControl&0xFFFF)>>12);
        
 if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
 {
   /* Transmission request Error */
   Error_Handler();
 }

 	if(CAN1_Receive_Msg(&Get_Std_id,RxData) < 9)//读取接收到信息
	{	
		printf("CANid:0x%2x, Receive data :",Get_Std_id);		
		for(i=0;i<8;i++)
		{									    
			printf("%02x ",RxData[i]);
		}
		printf("\r\n");
	}
  getstate.temperature = RxData[1];
  getstate.iq = (RxData[3]>>4)|RxData[2];
  getstate.speed = (RxData[5]>>4)|RxData[4];
  getstate.encoder = (RxData[7]>>4)|RxData[6];
	return  getstate;

}


motor_state Multi_angleControl_2(MotorId Motor_ID, uint16_t maxSpeed, int32_t angleControl)
{

  uint8_t TxData[8] = {0};
  uint8_t RxData[8] = {0};
  uint8_t Get_Std_id;
	uint8_t i=0;
	motor_state getstate;

  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.IDE = CAN_ID_STD;            
  TxHeader.StdId = std_id + Motor_ID;
  TxHeader.TransmitGlobalTime = DISABLE;
  TxHeader.DLC = 8;
    
  TxData[0] = 0xA4;
  TxData[1] = 0;
  TxData[2] = (uint8_t)((maxSpeed&0x0F)>>0);
  TxData[3] = (uint8_t)((maxSpeed&0xFF)>>4);
  TxData[4] = (uint8_t)((angleControl&0x000F)>>0); 
  TxData[5] = (uint8_t)((angleControl&0x00FF)>>4);
  TxData[6] = (uint8_t)((angleControl&0x0FFF)>>8);
  TxData[7] = (uint8_t)((angleControl&0xFFFF)>>12);
        
 if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
 {
   /* Transmission request Error */
   Error_Handler();
 }

 	if(CAN1_Receive_Msg(&Get_Std_id,RxData) < 9)//读取接收到信息
	{	
		printf("CANid:0x%2x, Receive data :",Get_Std_id);		
		for(i=0;i<8;i++)
		{									    
			printf("%02x ",RxData[i]);
		}
		printf("\r\n");
	}
  getstate.temperature = RxData[1];
  getstate.iq = (RxData[3]>>4)|RxData[2];
  getstate.speed = (RxData[5]>>4)|RxData[4];
  getstate.encoder = (RxData[7]>>4)|RxData[6];
	return  getstate;

}

motor_state Single_loop_angleControl_1(MotorId Motor_ID, uint8_t spinDirection, uint16_t angleControl)
{
  uint8_t TxData[8] = {0};
  uint8_t RxData[8] = {0};
  uint8_t Get_Std_id;
	uint8_t i=0;
	motor_state getstate;

  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.IDE = CAN_ID_STD;            
  TxHeader.StdId = std_id + Motor_ID;
  TxHeader.TransmitGlobalTime = DISABLE;
  TxHeader.DLC = 8;
    
  TxData[0] = 0xA5;
  TxData[1] = spinDirection;
  TxData[2] = 0;
  TxData[3] = 0;
  TxData[4] = (uint8_t)((angleControl&0x000F)>>0); 
  TxData[5] = (uint8_t)((angleControl&0x00FF)>>4);
  TxData[6] = 0;
  TxData[7] = 0;
        
 if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
 {
   /* Transmission request Error */
   Error_Handler();
 }

 	if(CAN1_Receive_Msg(&Get_Std_id,RxData) < 9)//读取接收到信息
	{	
		printf("CANid:0x%2x, Receive data :",Get_Std_id);		
		for(i=0;i<8;i++)
		{									    
			printf("%02x ",RxData[i]);
		}
		printf("\r\n");
	}
  getstate.temperature = RxData[1];
  getstate.iq = (RxData[3]>>4)|RxData[2];
  getstate.speed = (RxData[5]>>4)|RxData[4];
  getstate.encoder = (RxData[7]>>4)|RxData[6];
	return  getstate;

}

motor_state Single_loop_angleControl_2(MotorId Motor_ID, uint8_t spinDirection, uint16_t maxSpeed, uint16_t angleControl)
{

  uint8_t TxData[8] = {0};
  uint8_t RxData[8] = {0};
  uint8_t Get_Std_id;
	uint8_t i=0;
	motor_state getstate;

  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.IDE = CAN_ID_STD;            
  TxHeader.StdId = std_id + Motor_ID;
  TxHeader.TransmitGlobalTime = DISABLE;
  TxHeader.DLC = 8;
    
  TxData[0] = 0xA6;
  TxData[1] = spinDirection;
  TxData[2] = (uint8_t)((maxSpeed&0x000F)>>0); ;
  TxData[3] = (uint8_t)((maxSpeed&0x000F)>>0); ;
  TxData[4] = (uint8_t)((angleControl&0x000F)>>0); 
  TxData[5] = (uint8_t)((angleControl&0x00FF)>>4);
  TxData[6] = 0;
  TxData[7] = 0;
        
 if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
 {
   /* Transmission request Error */
   Error_Handler();
 }

 	if(CAN1_Receive_Msg(&Get_Std_id,RxData) < 9)//读取接收到信息
	{	
		printf("CANid:0x%2x, Receive data :",Get_Std_id);		
		for(i=0;i<8;i++)
		{									    
			printf("%02x ",RxData[i]);
		}
		printf("\r\n");
	}
  getstate.temperature = RxData[1];
  getstate.iq = (RxData[3]>>4)|RxData[2];
  getstate.speed = (RxData[5]>>4)|RxData[4];
  getstate.encoder = (RxData[7]>>4)|RxData[6];
	return  getstate;

}

typedef union
{
  uint8_t data[22];
  struct 
  {
    uint8_t ID[2];
    uint8_t data0[5];
    uint8_t data1[5];
    uint8_t data2[5];
    uint8_t data3[5];
  }bit; 
}motor_uart_control;

typedef struct
{
  uint16_t ID;
  uint64_t data0;
  uint64_t data1;
  uint64_t data2;
  uint64_t data3;
}motor__control_data;

//================================================================  
//buf定义2+20个字符：前2位为ID，如01，后面为data，5位为一个data，共4组
//如：01 01234 55555 66666 44444 ; 
//================================================================  
void Motor_open_fanction_uart(uint8_t *buf,uint8_t size)
{
  uint8_t *data,flag;
  uint8_t j=0;
  uint8_t i=0;
  motor_uart_control uart_input;
  motor__control_data input;
  motor_state outputstate;
  memcpy(uart_input.data,buf,size);
  printf("motor_open%s\r\n",uart_input.data);

  input.data0 = (uart_input.bit.data0[4]-'0')*1 +\
                (uart_input.bit.data0[3]-'0')*10 +\
                (uart_input.bit.data0[2]-'0')*100+\
                (uart_input.bit.data0[1]-'0')*1000 +\
                (uart_input.bit.data0[0]-'0')*10000;

  input.data1 = (uart_input.bit.data1[4]-'0')*1 +\
                (uart_input.bit.data1[3]-'0')*10 +\
                (uart_input.bit.data1[2]-'0')*100 +\
                (uart_input.bit.data1[1]-'0')*1000 +\
                (uart_input.bit.data1[0]-'0')*10000;
//================================================================  
//实际没有用到
  input.data2 = (uart_input.bit.data2[4]-'0')*1 +\
                (uart_input.bit.data2[3]-'0')*10 +\
                (uart_input.bit.data2[2]-'0')*100 +\
                (uart_input.bit.data2[1]-'0')*1000 +\
                (uart_input.bit.data2[0]-'0')*10000;

  input.data3=  (uart_input.bit.data3[4]-'0')*1 +\
                (uart_input.bit.data3[3]-'0')*10 +\
                (uart_input.bit.data3[2]-'0')*100 +\
                (uart_input.bit.data3[1]-'0')*1000 +\
                (uart_input.bit.data3[0]-'0')*10000;
//================================================================  
  input.ID =    (uart_input.bit.ID[1]-'0')*1 +\
                (uart_input.bit.ID[0]-'0')*10;
                
  printf("\r\nID:%d,data0:%lld,data1:%lld,data2:%lld,data3:%lld\r\n",input.ID,input.data0,input.data1,input.data2,input.data3);
  switch (input.ID)
  {
  case 1:
  {
    outputstate = speedControl(input.data0, input.data1);
    printf("outputstate.temperature:%d, outputstate.iq:%d, outputstate.speed:%d, outputstate.encoder:%d\r\n",\
            outputstate.temperature,outputstate.iq,outputstate.speed,outputstate.encoder);
  }
    break;
  
  default:
    break;
  }                
}