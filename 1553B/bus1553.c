#include <stdio.h>
#include "bus1553lib.h"
#include "bus1553.h"
#include "bus1553type.h"
#include "bus1553base.h"
//#include "IO.h"
//#include "time.h"

//#include "pcb.h"


#define MCFG1	0x80000000
#define MCFG2	0x80000004
#define MCFG3	0x80000008

#define MCFG1_INT() *(volatile unsigned int *)(MCFG1) = 0x140802ff;
#define MCFG2_INT() *(volatile unsigned int *)(MCFG2) = 0xe2c05060;
#define IO_EN_SET() *(volatile unsigned int *)(MCFG1) = (*(volatile unsigned int *)(MCFG1) | 0x00080000) // 开启IO


void send_1553(img_type *IMG_SEND){
	int cnt = IMG_SEND->img_size * 30 / 32 + 1;
	//*((volatile unsigned int *) (0x80000000)) = 0x140802ff;

	int i,j;

	volatile unsigned short SEOF[34];
	SEOF[0] = 0x1101;
	for(i = 1;i < 32;++i)SEOF[i] = 0xffff;

	BC_Init();
	
	unsigned int wRTAddr = 1;
	unsigned int wSubAddr = 1;
	unsigned short* wDataBuf = (unsigned short*)IMG_SEND->img_base;
	unsigned int wDataLen = 32 * 1;
	BUSCHANNEL wBusChan = CHANNEL_A;
	printf("start! %x\n",cnt);
	while(1) {
		
		for (i = 0; i < 10000; ++i);
		//printf("%d ",cnt);
		//for (i = 0; i < 200000; ++i);

		if(cnt % 10 == 0){
			printf("\r%4d",cnt);
		}
		else{
			for (i = 0; i < 150000; ++i);
		}

		if(cnt-- <= 0)break;

		BC_SendData(wRTAddr, wSubAddr, wDataBuf, wDataLen, wBusChan);
		wDataBuf = wDataBuf + wDataLen;

	}//while(1)
	if(IMG_SEND->img_last)BC_SendData(wRTAddr, wSubAddr, wDataBuf, IMG_SEND->img_last, wBusChan);

	wDataBuf = SEOF;
	BC_SendData(wRTAddr, wSubAddr, wDataBuf, wDataLen, wBusChan);

	printf("\ndone!\n");
}


int error;

void clear(unsigned int *tar,int wDataLen){
	int i,j;
	i = 0;
	for (j = 0; j < wDataLen; j++)
	{
		tar[i++] = 0x0;
	}
}

bool verify(unsigned int *tar){
	unsigned int temp = 0;
	int i;

	for(i=0;i<30;i++){
		temp += tar[i];
	}
	temp++;
	temp = temp & 0xffff;
	if(temp != tar[30]){
		return false;
	}
	
	temp = 0;;
	for(i=0;i<32;i++){
		temp ^= tar[i];
	}
	if(temp != 0){
		return false;
	}
	return true;
}
/*
img_type receive_1553(unsigned int *SDRAM_BASE){
	//*((volatile unsigned int *) (0x80000000)) = 0x140802ff;
	img_type res;

	res.img_size = 0; //双字总数 32位表示 最大表示 2^34 字节
	res.img_last = 0; //尾帧字数
	res.img_base = SDRAM_BASE;

	if(!IS_SDRAM(res.img_base))return res;

	BC_Init();
	unsigned int recv_buf[256+3];
	volatile unsigned int send_buf[3][6] = 
		{{0x1101ffff, 0xffffffff, 0x3, 0x4},
		{0x11010000, 0xffffffff, 0x3, 0x4},
		{0x1101ffff, 0xffffffff, 0x3, 0x4}};
	int choosen = 2;
	
	int i = 0, j = 0;

	unsigned int wRTAddr = 1;
	unsigned int wSubAddr = 1;
	volatile unsigned int* wDataBuf = recv_buf;
	unsigned int wDataLen = 32 * 1;
	BUSCHANNEL wBusChan = CHANNEL_A;

	int flag = 0; //首帧标志
	int tar_cnt = 0; //双字计数值 

//	int tmp;// 中间值

	error= 0; // 重传计数

	int count = 0;
	
printf("start!\n"); 	

//printf("%x\n",send_buf[choosen][0]);
	while(1) {
		if(flag == 0){
			BC_ReceiveData(wRTAddr, wSubAddr, recv_buf, wDataLen, wBusChan);
			
			if(recv_buf[0] != 0x1101)
			{
				printf("\nNo Input!\n");
				continue;
			}
			flag = 1;
			
			res.img_size = recv_buf[1];
			res.img_last = recv_buf[2];
			count = 0;
			
			BC_SendData(1, 1, send_buf[choosen], 4, wBusChan);
			
			clear(res.img_base,res.img_size*32);
			choosen = 0;
			
			printf("count = 0x%x(%d)\nlast =  %x\n",res.img_size ,res.img_size ,res.img_last);
			
		}
		else{
			BC_ReceiveData(wRTAddr, wSubAddr, recv_buf, wDataLen, wBusChan);
			//int jj;
			//for(jj = 0;jj<wDataLen ; ++jj)
			//	printf("%x ",recv_buf[jj]);
			//printf("\n");
			
//-----------------------------------校验---------------------------------------

			if(!verify(recv_buf)){
				choosen = 1;
				printf("error happen!\n");
				error++;
				
				BC_SendData(1, 1, send_buf[choosen], 4, wBusChan);
				if(choosen == 1)choosen = 0;
				
				for(i=0;i>1000;++i);//yanshitianchong
				continue;
			}
//-----------------------------------------------------------------
/*
			if(count == fram_num)break;

			if(count == fram_num - 1){	
				for (j = 0; j < last && j < wDataLen-3; j+=2) {
					tmp = recv_buf[j];
					tmp = tmp<< 16 | recv_buf[j+1];
					SDRAM_BASE[tar_cnt++] = tmp;
				}
				for (i = 0; i < wDataLen; ++i)recv_buf[i] = 0;
			}
			else{
				for (j = 0; j < wDataLen-3; j+=2) {
					tmp = recv_buf[j];
					tmp = tmp << 16;
					tmp = tmp | recv_buf[j+1];
					SDRAM_BASE[tar_cnt++] = tmp;
				}
				for (i = 0; i < wDataLen; ++i)recv_buf[i] = 0;
			}*/
			
			//if(count == fram_num)break;

/*			if(count >= res.img_size - 1){
				count++;
				for (j = 0; j < (res.img_last+1)/2 && j < wDataLen/2-1; j++) {
					res.img_base[tar_cnt++] = (recv_buf[2*j]<< 16) | recv_buf[2*j+1];
				}
				BC_SendData(1, 1, send_buf[choosen], 4, wBusChan);
				for(i = 0;i<200000;++i);
				//printf("\n%d\n",count);
				break;
			}

			for (j = 0; j < wDataLen/2-1; j++) {
				res.img_base[tar_cnt++] = (recv_buf[2*j]<< 16) | recv_buf[2*j+1];
			}
			count++;
			for (i = 0; i < wDataLen; ++i)recv_buf[i] = 0;
			BC_SendData(1, 1, send_buf[choosen], 4, wBusChan);

//			printf("\r%d",count);
			for(i = 0;i<100000;++i);
		}

	}//while(1)
	printf("count = %d\nerror = %d\n",count,error);
	return res;
}
*/
/*
void bus1553_test(unsigned int wRTAddr,unsigned int wSubAddr){
	 BC_Init();
//printf("%x\n",BT_GetReg(0x06));
	unsigned short data2[256+3] = {1, 2, 3, 4};
	unsigned short data[30] = {0x1101, 1, 2, 3, 'o', 'w', 's', 'e', 'r'};
	unsigned short data4[30] = {'U', 'C', 'B', 'R', '0', 'W', 'S', 'W', 'R'};
	int i = 0, j = 0;

	//unsigned int wRTAddr = 1;
	//unsigned int wSubAddr = 1;
	unsigned int* wDataBuf = BASE_SHORT;
	unsigned int wDataLen = 32 * 1;
	BUSCHANNEL wBusChan = CHANNEL_A;

	int tar_cnt = 0;
	while(1) {
		printf("Ready!\n");

		printf("%x\n",BT_GetReg(0x06));
		BC_SendData(1, 1, data, 4, wBusChan);


		BC_ReceiveData(wRTAddr, wSubAddr, data2, wDataLen, wBusChan);
//printf("Go!\n");

		for (j = 0; j < wDataLen; ++j) {
			//printf("%x ",data2[j]);

		}
		//printf("count = %d\n",tar_cnt);

		printf("done !\n");

	}//while(1)
}
*/
int func_RT_BC(unsigned int wRTAddr,unsigned int wSubAddr){
	BC_Init();
	BCMSGBUF msg;
	int i;
	int data;
//发送数据
	msg.MsgID = 0;
    //msg.MsgType = BC_RT;
    msg.MsgType = RT_BC;
    msg.Channel = CHANNEL_A;
    // 控制字设置成RETRY ENABLED，CHANNEL A/B
    //msg.CtrlWord = BC_GenCtrlWord(BC_RT, 0x0180);//channel A
    msg.CtrlWord = BC_GenCtrlWord(RT_BC, 0x0180);//channel A
    /* 写消息内容 */
    msg.Command1.RTADDR = wRTAddr;
    msg.Command1.TR = 1;//1：RT发送数据模式
    msg.Command1.SADDR = wSubAddr;
    msg.Command1.WCOUNT = 32; 

    //msg.Data[0]='A'; msg.Data[1]='B'; msg.Data[2]='C'; //参考
	//for(i=1000000000;i==0;i--);
	BC_MsgStart(&msg);
	for(i=1000000000;i==0;i--);
	for(i=0 ;i<32*2 /*msg.Command1.WCOUNT*/;i++){
		printf("data_%d:0x%x\t",i,*((char*)&msg.Data+i));
	}	
	printf("\n");
	printf("stetus1:0x%x\t status2:%x\n",msg.Status1,msg.Status2);
	return ((msg.Data[1]<<16)+msg.Data[0]);
}
void func_BC_RT(unsigned int wRTAddr,unsigned int wSubAddr){
	BC_Init();
	BCMSGBUF msg;
	int i;
//发送数据
	msg.MsgID = 0;
    msg.MsgType = BC_RT;
    msg.Channel = CHANNEL_A;
    // 控制字设置成RETRY ENABLED，CHANNEL A/B
    msg.CtrlWord = BC_GenCtrlWord(BC_RT, 0x0180);//channel A
    /* 写消息内容 */
    msg.Command1.RTADDR = wRTAddr;
    msg.Command1.TR = 0;//1：RT发送数据模式
    msg.Command1.SADDR = wSubAddr;
    msg.Command1.WCOUNT = 3; 
    msg.Data[0]='B'; msg.Data[1]='B'; msg.Data[2]='C'; //参考
	BC_MsgStart(&msg);
	printf("\n");
	printf("stets1:%x\t status2:%x\n",msg.Status1,msg.Status2);
}
int  send_data2RT(unsigned int wRTAddr,unsigned int wSubAddr,unsigned char* src,const int len){    //len默认为32
	BC_Init();
	BCMSGBUF msg;
	int i;
//发送数据
	msg.MsgID = 0;
    msg.MsgType = BC_RT;
    msg.Channel = CHANNEL_A;
    // 控制字设置成RETRY ENABLED，CHANNEL A/B
    msg.CtrlWord = BC_GenCtrlWord(BC_RT, 0x0180);//channel A
    /* 写消息内容 */
    msg.Command1.RTADDR = wRTAddr;
    msg.Command1.TR = 0;//1：RT发送数据模式
    msg.Command1.SADDR = wSubAddr;
    msg.Command1.WCOUNT = 30; 
    unsigned char* p_data=(unsigned char*)msg.Data;
    int j=0;
    for(;j<len*sizeof(unsigned short);j++){
    	*(p_data+j)=*(src+j);
    }
    //msg.Data[0]='B'; msg.Data[1]='B'; msg.Data[2]='C'; //参考
	BC_MsgStart(&msg);
	return 1;
	//printf("\n");
	//printf("stets1:%x\t status2:%x\n",msg.Status1,msg.Status2);
}
int recv_data_from_RT(unsigned int wRTAddr,unsigned int wSubAddr,unsigned char* dst){            //读取32个short
	BC_Init();
	BCMSGBUF msg;
	int i;
	int data;
//发送数据
	msg.MsgID = 0;
    //msg.MsgType = BC_RT;
    msg.MsgType = RT_BC;
    msg.Channel = CHANNEL_A;
    // 控制字设置成RETRY ENABLED，CHANNEL A/B
    //msg.CtrlWord = BC_GenCtrlWord(BC_RT, 0x0180);//channel A
    msg.CtrlWord = BC_GenCtrlWord(RT_BC, 0x0180);//channel A
    /* 写消息内容 */
    msg.Command1.RTADDR = wRTAddr;
    msg.Command1.TR = 1;//1：RT发送数据模式
    msg.Command1.SADDR = wSubAddr;
    msg.Command1.WCOUNT = 32; 
    unsigned char* p_data=(unsigned char*)msg.Data;
    //msg.Data[0]='A'; msg.Data[1]='B'; msg.Data[2]='C'; //参考
	//for(i=1000000000;i==0;i--);
	BC_MsgStart(&msg);
	for(i=1000000000;i==0;i--);
	for(i=0 ;i<32*sizeof(unsigned short) /*msg.Command1.WCOUNT*/;i++){
		//printf("data_%d:0x%x\t",i,*((char*)&msg.Data+i));
		*(dst+i)=*(p_data+i);
	}
	if(*dst!=0)return 1;
	else {
		memset(dst,0,32*sizeof(unsigned short));
		return 0;
	}	
	//printf("\n");
	//printf("stetus1:0x%x\t status2:%x\n",msg.Status1,msg.Status2);
	//return ((msg.Data[1]<<16)+msg.Data[0]);
}
/*
int main(){
	//MCFG2 register set,SDRAM 参数设置
	MCFG1_INT();
	MCFG2_INT();
	IO_EN_SET();

	int i;
	unsigned int wRTAddr;
	unsigned int wSubAddr;
	wRTAddr = 1;
	wSubAddr = 1;
	printf("wRTAddr:%d 	wSubAddr: %d\n",wRTAddr,wSubAddr);
	//bus1553_test(wRTAddr,wSubAddr);
	func_BC_RT(wRTAddr,wSubAddr);
	return 0;

}*/
/*
typedef struct BcMsgBuf
{
    unsigned int      MsgID;
    MESSAGETYPE MsgType;     // int
    BUSCHANNEL  Channel;     // int
    unsigned int      CtrlWord;
    COMMANDWORD Command1;    
    COMMANDWORD Command2;    
    unsigned int      Status1;     //ReadMsg
    unsigned int      Status2;     //ReadMsg
    unsigned int      Data[32];    
    unsigned int      Loopback;    //ReadMsg  
    unsigned int      MCData;      //ReadMsg
    unsigned int      BlkStatus;   //ReadMsg
    unsigned int      TimeTag;     //ReadMsg
    unsigned int      GapTime;    
}BCMSGBUF,*PBCMSGBUF;
 typedef struct CommandWord
{
    unsigned int Reserve : 16;
    unsigned int RTADDR  : 5;
    unsigned int TR      : 1;     
    unsigned int SADDR   : 5;  
    unsigned int WCOUNT  : 5;
} COMMANDWORD,*PCOMMANDWORD;
 */  
