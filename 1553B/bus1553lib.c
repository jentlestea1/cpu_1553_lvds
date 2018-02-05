#include <stdio.h>
#include "bus1553type.h"
#include "bus1553lib.h"
#include "bus1553base.h"

//#include"pcb.h"
//#include "time.h"

unsigned int Cur_Msg_Id = 0;

#define MAX_SUBADDR_NUM 20
#define MAX_BUFFER_SIZE 3072

int Configed_Num = 0;
unsigned int Configed_Size = 0;
unsigned int Cur_Buf_Ptr = 0;
bool Rt_Started = false;
bool Config_Error = false;

//---------------from BC_Receive & BC_WriteMsg-----------------
unsigned int msgblk_ptr; 
unsigned int stack_ptr;
//-----------------------------------------------
CONFIGWORD Config_Table[MAX_SUBADDR_NUM];


/**********************************************************
 * 当1553控制器为BC模式下，下列函数有用
 * 
 *
 *
 *
 *init,start,write/read
 *********************************************************/
 /* BC初始化 */
void BC_Init()
{
	// ACE内部寄存器映射
    // 地址   寄存器描述
    // 0x0    中断掩码寄存器
    // 0x1    1号配置寄存器
    // 0x2    2号配置寄存器
    // 0x3    开始/重置寄存器(写时)
    // 0x3    BC/RT命令栈指针寄存器(读时)
    // 0x4    BC控制字/RT子地址控制字寄存器
    // 0x5    时间标签寄存器
    // 0x6    中断状态寄存器(只写)
    // 0x7    3号配置寄存器
    // 0x8    4号配置寄存器
    // 0x9    5号配置寄存器
    // 0xa    数据栈地址寄存器

//---------------------------------------------
	BT_SetReg(0x00,0x00);
//===============================================
    //寄存器复位START_RESET_REG
	BT_SetReg(0x03,0x01);
    //在config3中设置为增强模式
	BT_SetReg(0x07,0x8000);
    //设置中断屏蔽寄存器,BC_MSG_EOM
//    BT_SetReg(0x00,0x10);
    //CONFIG1
    // DOUBLED/SINGLE RETRY
    // RETRY ENABLED
    // INTERMESSAGE GAP TIMER ENABLED
    // FRAME STOP-ON-ERROR
    BT_SetReg(0x01,0x0838);
    //CONFIG2
    // LEVEL/PULSE* INTERRUPT REQUEST(水平/脉冲中断请求)
    // INTERRUPT STATUS AUTO CLEAR(中断状态自动清零)
    // 256-WORD BOUNDARY DISABLE(256字边界禁用)
    // ENHANCED INTERRUPTS(增强中断？？)
    BT_SetReg(0x02,0x8418);
    //CONFIG4
    // 2ND RETRY ALT/SAME BUS
    // EXTERNAL BIT WORD ENABLE(外部位字使能)
    BT_SetReg(0x08,0x1080);
    //CONFIG5
    // EXPANDED CROSSING ENABLED
    BT_SetReg(0x09,0x0800);
    //TIMETAG_REG
    
    BT_SetReg(0x05,0x00);
    
    //RAM CLEAR
    BT_Memset(0x0, 0 ,0x1000);//must check
    //SET STACK POINT
    BT_MemWrite(0x0100,0x0);
    BT_MemWrite(0x0104,0x0); 
}

/* 写BC消息 */
void BC_WriteMsg(BCMSGBUF* pMsg)
{
		int i;
//    unsigned int stack_ptr;
//    unsigned int msgblk_ptr;
    unsigned int word_count; 
    unsigned int* pCmdWd1 = (unsigned int*)&pMsg->Command1;
    unsigned int* pCmdWd2 = (unsigned int*)&pMsg->Command2;

    pMsg->MsgID = Cur_Msg_Id;
    stack_ptr  = pMsg->MsgID * 4;//point to 4*32bits (0, 0, pMsg->GapTime, msgblk_ptr)
    msgblk_ptr = 0x0108 + Cur_Msg_Id * 38;//point to 38*32bits (pMsg->Ctrl, Word *pCmdWd1, 32*32bits)
    if (pMsg->Command1.WCOUNT > 0)
        word_count = pMsg->Command1.WCOUNT;
    else
        word_count = 32;

    /* 生成控制字 */
    if(pMsg->Channel == CHANNEL_A)
    	pMsg->CtrlWord = BC_GenCtrlWord(pMsg->MsgType, 0x0180);
    else
    	pMsg->CtrlWord = BC_GenCtrlWord(pMsg->MsgType, 0x0100);

    /* 写RAM消息块 */
    BT_MemWrite(msgblk_ptr, pMsg->CtrlWord);  /* 控制字 */
    BT_MemWrite(msgblk_ptr + 1, *pCmdWd1);     /* 指令字 */

        //printf("bc_write_ctrlword: %08x   ", pMsg->CtrlWord);     
        //printf("    cmdword: %08x", BT_MemRead(msgblk_ptr + 1));      

    switch(pMsg->MsgType){
	case BC_RT :
		for(i = 0 ; i < word_count ; i++)
		{
			//printf("bc_rt %i ,\n",pMsg->Data[i] & 0xFFFF);
			BT_MemWrite(msgblk_ptr + i + 2, pMsg->Data[i] & 0xFFFF);
		}
		break;
         case RT_BC :
            break;
         case RT_RT :
            BT_MemWrite(msgblk_ptr + 2, *pCmdWd2);
            break;
         case BCST :
            for(i = 0 ; i < word_count ; i++)
               BT_MemWrite(msgblk_ptr + i + 2, pMsg->Data[i] & 0xFFFF);
            break;
         case RT_RTS :
            BT_MemWrite(msgblk_ptr + 2, *pCmdWd2);
            break;
         case MCND :
            break;
         case TX_MCWD :
            break;
         case RX_MCWD :
            BT_MemWrite(msgblk_ptr + 2, pMsg->MCData);
            break;
         case BCST_MCND :
            break;
         case BCST_MCWD :
            BT_MemWrite(msgblk_ptr + 2, pMsg->MCData);
            break;
         default:
         	break;
    	}
    
      BT_MemWrite(stack_ptr, 0);
      BT_MemWrite(stack_ptr + 1, 0);
      BT_MemWrite(stack_ptr + 2, pMsg->GapTime);
      BT_MemWrite(stack_ptr + 3, msgblk_ptr);
      /* 当前消息号增1 */
	//printf(" msg_id: %i  ", Cur_Msg_Id);		
			
      Cur_Msg_Id = Cur_Msg_Id + 1;
}

/* 读取BC发送后反馈的消息 */
void BC_ReadMsg(BCMSGBUF* pMsg)
{
	unsigned int stack_ptr;
//	unsigned int msgblk_ptr;
	unsigned int word_count;
	int i ;
	
    stack_ptr = pMsg->MsgID * 4;
    msgblk_ptr = 0x0108 + pMsg->MsgID * 38;
    if (pMsg->Command1.WCOUNT > 0) 
    	word_count = pMsg->Command1.WCOUNT;
    else
    	word_count = 32;
    /* 读消息块 */
    switch(pMsg->MsgType){
    	case BC_RT :
        	pMsg->Loopback = BT_MemRead(msgblk_ptr + word_count + 2);
            pMsg->Status1 = BT_MemRead(msgblk_ptr + word_count + 3);
            break;
        case RT_BC :
            pMsg->Loopback = BT_MemRead(msgblk_ptr + 2);
            pMsg->Status1 = BT_MemRead(msgblk_ptr + 3);
            for(i = 0 ; i < word_count ; i++)
            	pMsg->Data[i] = BT_MemRead(msgblk_ptr + i + 4) & 0xFFFF;
            break;
        case RT_RT :
            pMsg->Loopback = BT_MemRead(msgblk_ptr + 3);
            pMsg->Status1 = BT_MemRead(msgblk_ptr + 4);
            for(i = 0 ; i < word_count ; i++)
                pMsg->Data[i] = BT_MemRead(msgblk_ptr + i + 5) & 0xFFFF;
            pMsg->Status2 = BT_MemRead(msgblk_ptr + word_count + 5);
            break;
        case BCST :
            pMsg->Loopback = BT_MemRead(msgblk_ptr + word_count + 2);
            break;
        case RT_RTS :
            pMsg->Loopback = BT_MemRead(msgblk_ptr + 3);
            pMsg->Status1 = BT_MemRead(msgblk_ptr + 4);
            for(i = 0 ; i < word_count ; i++)
                pMsg->Data[i] = BT_MemRead(msgblk_ptr + i + 5) & 0xFFFF;
            break;
        case MCND :
            pMsg->Loopback = BT_MemRead(msgblk_ptr + 2);
            pMsg->Status1 = BT_MemRead(msgblk_ptr + 3);
            break;
        case TX_MCWD :
            pMsg->Loopback = BT_MemRead(msgblk_ptr + 2);
            pMsg->Status1 = BT_MemRead(msgblk_ptr + 3);
            pMsg->MCData = BT_MemRead(msgblk_ptr + 4);
            break;
        case RX_MCWD :
            pMsg->Loopback = BT_MemRead(msgblk_ptr + 3);
            pMsg->Status1 = BT_MemRead(msgblk_ptr + 4);
            break;
        case BCST_MCND :
            pMsg->Loopback = BT_MemRead(msgblk_ptr + 2);
            break;
        case BCST_MCWD :
            pMsg->Loopback = BT_MemRead(msgblk_ptr + 3);
            break;
        default:
        	break;
    	}
    /* 读消息描述符 */
    pMsg->BlkStatus = BT_MemRead(stack_ptr);
    pMsg->TimeTag = BT_MemRead(stack_ptr + 1);
}

/* 启动BC */
void BC_Start()
{
	unsigned int msg_count;
	msg_count = 0xffff - Cur_Msg_Id;
	BT_MemWrite(0x0101, msg_count);
	BT_MemWrite(0x0100, 0x0);
	// BC/MT START
	// BC STOP-ON-FRAME
	BT_SetReg(0x03, 0x0022);
}
////////////////////////////////////////////////////////////////////
/* 获取BC发送状态 */
bool BC_FrmInProc()
{
/*
	unsigned int value;
	bool   fip;
	//printf("Enter frmproc:");
	value = BT_GetReg(0x01);
	//printf("Config#1# %08x\n",value);
	if((0x02 & value) > 0)
		fip = true;
	else
		fip = false;
	return fip;
*/
	// 7-17 9:36:33
	if((0x02 & BT_GetReg(0x01)) > 0){
        printf("BC_FrmInProc return true\n");
        return true;
    }
	else 
        return false;

}

/* 集成化BC消息发送和接收数据 */
void BC_MsgStart(BCMSGBUF* pMsg)
{
    /* 消息块清零 */
    BT_Memset(0x0108, 0, 38);
    Cur_Msg_Id = 0;
    BC_WriteMsg(pMsg);
    BC_Start();
    
    while(BC_FrmInProc());
    BC_ReadMsg(pMsg);
}

/* 发送大块数据 */
void BC_SendData(unsigned int wRTAddr, unsigned int wSubAddr, unsigned short* wDataBuf, unsigned int wDataLen, BUSCHANNEL wBusChan)
{
    BCMSGBUF msg;
    int msg_num;
    int i;
    int data_ptr = 0;
    
    Cur_Msg_Id = 0;  /* 消息数置0 */
    /* 写消息内容 */
    msg.Command1.RTADDR = wRTAddr;
    msg.Command1.TR = 0;
    msg.Command1.SADDR = wSubAddr;
    msg.Command1.WCOUNT = 0;
    // 控制字设置成RETRY ENABLED，CHANNEL A/B
    if (wBusChan == CHANNEL_A)
        msg.CtrlWord = BC_GenCtrlWord(BC_RT, 0x0180);
    else
        msg.CtrlWord = BC_GenCtrlWord(BC_RT, 0x0100);
            
        //printf("ctrlword: %08x", msg.CtrlWord);       

    msg.MsgType = BC_RT;
    //msg.channel = wBusChan;
    msg.Channel = wBusChan;

    for(i = 0 ; i < wDataLen ; i++)
    {
        msg.Data[data_ptr] = wDataBuf[i];
        data_ptr++;
        if(data_ptr == 32)
        {
            data_ptr = 0;
            BC_WriteMsg(&msg);
            msg_num++; 
            continue;
        }
        if(i == (wDataLen - 1))
        {
            msg.Command1.WCOUNT = data_ptr;       
            BC_WriteMsg(&msg);
            msg_num++;
        }  

    }
    /*generate msg and write Command1, CtrlWord, MsgType, Channel, Data*/
    /* 启动总线发送消息 */
    //printf("Msg Num is %d\n", Cur_Msg_Id);
    /********************
    for(i = 0 ; i < 38 * 4 ; i++)
    {
        //printf("%#x ",BT_MemRead(0x108 + i));
    }
    //Printf("\N");
    ********************/

//ENTER_CRITICAL(THIS_CPU());

    BC_Start();
    /* 等待总线发送完消息 */
//EXIT_CRITICAL(THIS_CPU());

	for(i = 0;i<170000;++i);
    while(BC_FrmInProc())printf("\r...");
}



/* 获取RT大块数据 */
void BC_ReceiveData(unsigned int wRTAddr, unsigned int wSubAddr, unsigned int* wDataBuf, unsigned int wDataLen, BUSCHANNEL wBusChan)
{
	int i,j,k;
	unsigned int loopnum1;
	unsigned int rema1;
	unsigned int loopnum2;
	unsigned int rema2;
    union{
      	COMMANDWORD command1;
      	unsigned int cmd_word;
      	}cmdword;
    unsigned int ctrl_word;

    
    unsigned int count = 0;
    unsigned int msg_num;

	cmdword.cmd_word = 0;
	
	loopnum1 = wDataLen / (32 * 64);   /* 单次最多发送64消息，获取32×64字数据 */
    rema1  = wDataLen % (32 * 64);
    /* 生成命令字 */
    cmdword.command1.RTADDR = wRTAddr;
    cmdword.command1.TR = 1;
    cmdword.command1.SADDR = wSubAddr;
    cmdword.command1.WCOUNT = 0;         /* 每个消息32个字 */
    /* 生成控制字 */
    if(wBusChan == CHANNEL_A)
    	ctrl_word = BC_GenCtrlWord(RT_BC, 0x0180);
    else
    	ctrl_word = BC_GenCtrlWord(RT_BC, 0x0100);

    for( i = 0 ; i < loopnum1 ; i++){
//        BT_Memset(0x0108, 0, 38 * BC_MAX_MSG_NUM);  /* 存储区清零 */
        for( j = 0 ; j < BC_MAX_MSG_NUM ; j++){
//            stack_ptr = j * 4;
//            msgblk_ptr = 0x0108 + j * 38;
//            BT_MemWrite(msgblk_ptr, ctrl_word);      /* 写控制字 */
//            BT_MemWrite(msgblk_ptr + 1, cmdword.cmd_word);   /* 写指令字 */
            /* 写消息描述符入堆栈 */
//            BT_MemWrite(stack_ptr, 0);
//            BT_MemWrite(stack_ptr + 1, 0);
//            BT_MemWrite(stack_ptr + 2, 20);
//            BT_MemWrite(stack_ptr + 3, msgblk_ptr);
        }
//        BT_MemWrite(0x0101, 0xFFFF - BC_MAX_MSG_NUM);  /* 设置消息数*/ 
//        BT_MemWrite(0x0100, 0x0000);       /* 位堆栈指针*/
//        BT_SetReg(0x03, 0x0022);           /* 启动BC，帧结束停止*/
 //       while (BC_FrmInProc());//printf("..!\n");               /* 判断BC处理帧是否结束 */
            
 /*       for( j = 0 ; j < loopnum1 ; j++){
            msgblk_ptr = 0x0108 + j * 38;
            for(k = 0 ; k < 32 ; k++) {
               wDataBuf[count] = (unsigned short)(BT_MemRead(msgblk_ptr + k + 4) & 0xFFFF);
//printf("%4x ",wDataBuf[count]);
               count = count + 1;
            }
//printf("\n");
        }*/
    }
    if(rema1 > 0) {                    /*还存在剩余数据*/
         loopnum2 = rema1 / 32;           /*剩余数据所需消息数*/
         rema2 = rema1 % 32;
         BT_Memset(0x0108, 0, 38 * (loopnum2 + 1));  /*存储区清零*/
         for(i = 0 ; i < loopnum2 ; i++){ 
            stack_ptr = i * 4;
            msgblk_ptr = 0x0108 + i * 38;
            BT_MemWrite(msgblk_ptr, ctrl_word);     /*写控制字*/
            BT_MemWrite(msgblk_ptr + 1, cmdword.cmd_word);  /*写指令字*/
            /*写消息描述符入堆栈*/
            BT_MemWrite(stack_ptr, 0);
            BT_MemWrite(stack_ptr + 1, 0);
            BT_MemWrite(stack_ptr + 2, 20);
            BT_MemWrite(stack_ptr + 3, msgblk_ptr); 
         }
         
         if(rema2 > 0){
            stack_ptr = loopnum2 * 4;
            msgblk_ptr = 0x0108 + loopnum2 * 38;
            cmdword.command1.WCOUNT = rema2;               /* rema2个数据 */
            BT_MemWrite(msgblk_ptr, ctrl_word);     /* 写控制字 */
            BT_MemWrite(msgblk_ptr + 1, cmdword.cmd_word);  /* 写指令字 */
            /*写消息描述符入堆栈*/
            BT_MemWrite(stack_ptr, 0);
            BT_MemWrite(stack_ptr + 1, 0);
            BT_MemWrite(stack_ptr + 2, 20);
            BT_MemWrite(stack_ptr + 3, msgblk_ptr);
            msg_num = loopnum2 + 1;
            }
         else
            msg_num = loopnum2;

//ENTER_CRITICAL(THIS_CPU());


         BT_MemWrite(0x0101, 0xFFFF - msg_num);  /*设置消息数*/ 
         BT_MemWrite(0x0100, 0x0000);       /*复位堆栈指针*/
         BT_SetReg(0x03, 0x0022);           /*启动BC，帧结束停止*/
            for(i = 0;i<200000;++i);

        while(BC_FrmInProc())printf("\r...");            /*判断BC处理帧是否结束*/

         for(i = 0 ; i < loopnum2 ; i++){
            msgblk_ptr = 0x0108 + i * 38;
            for(k = 0 ; k < 32 ; k++){ 
               wDataBuf[count] = (unsigned short)(BT_MemRead(msgblk_ptr + k + 4) & 0xFFFF);
               count++;
            }
         }
         if(rema2 > 0){
            msgblk_ptr = 0x0108 + loopnum2 * 38;
            for(k = 0 ; k < rema2 ; k++){
               wDataBuf[count] = (unsigned short)(BT_MemRead(msgblk_ptr + k + 4) & 0xFFFF);
               count++;
            }
//printf("\n");
         } 

//EXIT_CRITICAL(THIS_CPU());

    }

}
/* 生成控制字 */
unsigned int BC_GenCtrlWord(MESSAGETYPE wMsgType, unsigned int wCtrlWord)
{
    unsigned int value;
    unsigned int temp = 0;
    switch (wMsgType) {
         case BC_RT :
         case RT_BC :
            temp = 0x0000;
            break;
         case RT_RT : 
            temp = 0x0001;
            break;
         case BCST  : 
            temp = 0x0002;
            break;
         case RT_RTS :
            temp = 0x0003;
            break;
         case MCND :
         case TX_MCWD :
         case RX_MCWD :
            temp = 0x0004;
            break;
         case BCST_MCND :
         case BCST_MCWD :
            temp = 0x0006;
            break;
         default:
            ;
    }

    value = wCtrlWord + temp;
    return value;
}
