/******************************************************************************
/// @brief
/// @copyright Copyright (c) 2017 <dji-innovations, Corp. RM Dept.>
/// @license MIT License
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction,including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense,and/or sell
/// copies of the Software, and to permit persons to whom the Software is furnished
/// to do so,subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
/// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
*******************************************************************************/

#include "can.h"
#include "bsp_can.h"

#include "stdio.h"			//用里面的printf调试语句
#define DEBUG_MODE		0	//启用后，会在can1接收中断回调函数内发送相关信息！
moto_measure_t moto_chassis[4] = {0};//4 chassis moto

void get_total_angle(moto_measure_t *p);
void get_moto_offset(moto_measure_t *ptr, CAN_HandleTypeDef* hcan);

/*******************************************************************************************
  * @Func		my_can_filter_init
  * @Brief    CAN1和CAN2滤波器配置
  * @Param		CAN_HandleTypeDef* hcan
  * @Retval		None
  * @Date     2015/11/30
 *******************************************************************************************/
void my_can_filter_init_recv_all(CAN_HandleTypeDef* _hcan)
{
	//can1 &can2 use same filter config
	CAN_FilterTypeDef		CAN_FilterConfigStructure;
//	static CAN_TxHeaderTypeDef		Tx1Message;
//	static CAN_RxHeaderTypeDef 		Rx1Message;


	//CAN_FilterConfigStructure.FilterNumber = 0;
	CAN_FilterConfigStructure.FilterMode = CAN_FILTERMODE_IDMASK;
	CAN_FilterConfigStructure.FilterScale = CAN_FILTERSCALE_32BIT;
	CAN_FilterConfigStructure.FilterIdHigh = 0x0000;
	CAN_FilterConfigStructure.FilterIdLow = 0x0000;
	CAN_FilterConfigStructure.FilterMaskIdHigh = 0x0000;
	CAN_FilterConfigStructure.FilterMaskIdLow = 0x0000;
	CAN_FilterConfigStructure.FilterFIFOAssignment = CAN_FilterFIFO0;
	//CAN_FilterConfigStructure.BankNumber = 14;//can1(0-13)和can2(14-27)分别得到一半的filter
	CAN_FilterConfigStructure.FilterBank = 0;	//YQY新增，用于替换上面的语句
	CAN_FilterConfigStructure.FilterActivation = ENABLE;

	if(HAL_CAN_ConfigFilter(_hcan, &CAN_FilterConfigStructure) != HAL_OK)
	{
		//err_deadloop(); //show error!
	}


//	if(_hcan == &hcan1){
//		_hcan->pTxMsg = &Tx1Message;
//		_hcan->pRxMsg = &Rx1Message;
//	}


}


/*******************************************************************************************
  * @Func			void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef* _hcan)
  * @Brief    HAL库中标准的CAN接收完成回调函数，需要在此处理通过CAN总线接收到的数据 YQY重新修订
  * @Param		
  * @Retval		None 
  * @Date     2022/3/25
 *******************************************************************************************/
uint32_t FlashTimer;
CAN_RxHeaderTypeDef   RxHeader;
uint8_t               RxData[8];

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *CanHandle)
{
	if(HAL_GetTick() - FlashTimer>300){										//用于测试，如果接收到数据，就让红色led闪烁
		HAL_GPIO_TogglePin(LED_R_GPIO_Port,LED_R_Pin);
		FlashTimer = HAL_GetTick();
		if(DEBUG_MODE)printf("angle(1 2): %d  %d ", moto_chassis[0].angle, moto_chassis[1].angle);
		if(DEBUG_MODE)printf("cnt(1 2): %d  %d\n ", moto_chassis[0].round_cnt, moto_chassis[1].round_cnt);
	}
  if (HAL_CAN_GetRxMessage(CanHandle, CAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK)	//读取FIFO0中的缓存数据
  {
    Error_Handler();
  }
	switch(RxHeader.StdId){																//这里处理ID 0-4的电机的返回报文，其它电机同理，往下面加就行
		case CAN_2006Moto1_ID:
		case CAN_2006Moto2_ID:
		case CAN_2006Moto3_ID:
		case CAN_2006Moto4_ID:
			{
				static u8 i;
				i = RxHeader.StdId - CAN_2006Moto1_ID;					//计算绝对偏移地址
				get_moto_measure(&moto_chassis[i], RxData);			//将获取到的报文信息传入子函数进行处理
			}
			break;
	}
}


///*******************************************************************************************
//  * @Func			void get_moto_measure(moto_measure_t *ptr, CAN_HandleTypeDef* hcan)
//  * @Brief    接收3508电机通过CAN发过来的信息		YQY重新修订
//  * @Param		
//  * @Retval		None
//  * @Date     2022/3/25
// *******************************************************************************************/
void get_moto_measure(moto_measure_t *ptr, uint8_t* YQY_Rx_Msg)
{

	ptr->last_angle = ptr->angle;																				//保存上一次的转子角度值
	ptr->angle = (uint16_t)(YQY_Rx_Msg[0]<<8 | YQY_Rx_Msg[1]) ;					//计算出这次的转子角度
	ptr->speed_rpm  = (int16_t)(YQY_Rx_Msg[2]<<8 | YQY_Rx_Msg[3]);			//计算这次的速度
	ptr->real_current = (YQY_Rx_Msg[4]<<8 | YQY_Rx_Msg[5])*5.f/16384.f;	//计算点击当前的电流

	ptr->hall = YQY_Rx_Msg[6];																					//数据手册中位[6]是空的，不知道有啥用
	
	if(ptr->angle - ptr->last_angle > 4096)															//计算电机旋转了几圈
		ptr->round_cnt --;
	else if (ptr->angle - ptr->last_angle < -4096)											//计算电机旋转了几圈
		ptr->round_cnt ++;
	ptr->total_angle = ptr->round_cnt * 8192 + ptr->angle - ptr->offset_angle;
}

/*this function should be called after system+can init */							//YQY挖坑：该函数未使用！
//void get_moto_offset(moto_measure_t *ptr, CAN_HandleTypeDef* hcan)
//{
//	ptr->angle = (uint16_t)(hcan->pRxMsg->Data[0]<<8 | hcan->pRxMsg->Data[1]) ;
//	ptr->offset_angle = ptr->angle;
//}

#define ABS(x)	( (x>0) ? (x) : (-x) )
/**
*@bref 电机上电角度=0， 之后用这个函数更新3510电机的相对开机后（为0）的相对角度。
	*/
void get_total_angle(moto_measure_t *p){															//YQY挖坑：该函数未使用！
	
	int res1, res2, delta;
	if(p->angle < p->last_angle){			//可能的情况
		res1 = p->angle + 8192 - p->last_angle;	//正转，delta=+
		res2 = p->angle - p->last_angle;				//反转	delta=-
	}else{	//angle > last
		res1 = p->angle - 8192 - p->last_angle ;//反转	delta -
		res2 = p->angle - p->last_angle;				//正转	delta +
	}
	//不管正反转，肯定是转的角度小的那个是真的
	if(ABS(res1)<ABS(res2))
		delta = res1;
	else
		delta = res2;

	p->total_angle += delta;
	p->last_angle = p->angle;
}
///*******************************************************************************************
//  * @Func			void get_moto_measure(moto_measure_t *ptr, CAN_HandleTypeDef* hcan)
//  * @Brief    控制2006电机的电流，进而控制其旋转
//  * @Param		CAN_HandleTypeDef* hcan 传入can1或者can2
//							iq1-iq4 传入电机ID 1-4的电流信息。目前未测量最大值
//  * @Retval		None
//  * @Date     2022/3/24
// *******************************************************************************************/
void YQY_set_moto_current(CAN_HandleTypeDef* hcan, int16_t iq1, int16_t iq2, int16_t iq3, int16_t iq4){
	uint8_t TxData[8] = {0};
	uint32_t TxMainBox = 0;
	CAN_TxHeaderTypeDef TxHeader;
	
	TxHeader.StdId = 0x200;
	TxHeader.IDE =  CAN_ID_STD;
	TxHeader.RTR = CAN_RTR_DATA;
	TxHeader.DLC = 0x08;
	
	TxData[0] = (iq1 >> 8);
	TxData[1] = iq1;
	TxData[2] = (iq2 >> 8);
	TxData[3] = iq2;
	TxData[4] = (iq3 >> 8);
	TxData[5] = iq3;
	TxData[6] = (iq4 >> 8);
	TxData[7] = iq4;
	
	if(HAL_CAN_AddTxMessage(hcan, &TxHeader, TxData, &TxMainBox) != HAL_OK)
	{
		Error_Handler();
	}
}

