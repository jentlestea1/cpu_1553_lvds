#include "init.h"
#include "./1553B/do_frame.h"
#include "./task/task.h"
#include <unistd.h>

#define MEM_CFG1 *(volatile unsigned int*)(0x80000000) 
#define MEM_CFG2 *(volatile unsigned int*)(0x80000004)  //存储器配置寄存器2
#define MEM_CFG3 *(volatile unsigned int*)(0x80000008)  //存储器配置寄存器3
//#define SDRAM_BASE *(volatile unsigned int*)(0x60000000)
#define SDRAM_BASE *(volatile unsigned int*)(0x60000000) 
/*
void lvds_init(){
	//LVDS输入通道使能
	CLVDS_INR  = 0x0000000f;
	//LVDS输出通道使能 
  	CLVDS_OUTR = 0x00000000 | SET_lvds_out_en; 
}*/

int main(){
 
	long i;
	unsigned int fifo_usewd=0;
	int img_size =5466;
	unsigned int sdram_addr_base = 0x60000000;
	unsigned int wRTAddr;
	unsigned int wSubAddr;
	unsigned int wSubAddr_bc2rt;
	wRTAddr = 2;
	wSubAddr = 1;
	wSubAddr_bc2rt = 2;

  	//MCFG2 register set,SDRAM 参数设置
  	MEM_CFG1 = 0x140802ff;
  	MEM_CFG2 = 0xe2c05060;
  	//memset(&SDRAM_BASE,0,0x2000);
  	bzero(&SDRAM_BASE,0x2000);
  	SDRAM_BASE=0x12345678;
  	//  MEM_CFG3 = 0x000ff000; 
	//printf("wRTAddr:%d 	wSubAddr: %d\n",wRTAddr,wSubAddr);
	proc_main_task(recv_data_from_RT,send_data2RT,exec_task);       //执行任务
	//func_BC_RT(wRTAddr,wSubAddr_bc2rt);
	/*while(1){
		  m_sleep();
	      img_size = func_RT_BC(2,1);//单位，Bytes

        }*/
	//printf("img size :%d Bytes\n",img_size);
	
	//读写先操作这个  SDRAM_DBS = 0x00000000;
	//从sdram读  data = *(volatile unsigned int *)(sdram_addr_base+ addr);
	//往sdram写  *(volatile unsigned int *)(sdram_addr_base+addr) = 1;

	fifo_usewd = write_data2sdram(img_size,1);//channel : 1 ,2,3,4	
	printf("fifo_usewd :%d\n",fifo_usewd);
	printf("________________________________________________\n");
	return 0;
};
