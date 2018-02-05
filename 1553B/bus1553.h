
#ifndef _bus1553_
#define _bus1553_

//----------------------------------------------------------
#define DATA_BASE           0x60000000
#define BASE_SHORT         ((volatile unsigned int *)(DATA_BASE))

#define IS_SDRAM(A) ((A >= 0x60000000)&&(A < 0x80000000)) 

typedef struct img_content
{
	/* data */
	int img_size;
	int img_last;
	unsigned int *img_base;
}img_type;

//-----------------------------fun------------------------------


void send_1553(img_type *IMG_SEND); // 发送BASE为起始的内容 可以手动指定大小通过修改其中局部变量cnt
void clear(unsigned int * tar,int wDataLen);
img_type receive_1553(unsigned int *SDRAM_BASE); // 接收到BASE（0x60000000）中 （修改BASE位参数）

//--------------GPIO 中断（移动到GPIO中集体管理）-------------------------
void gpio_1553b_int_handle(unsigned int irq);
void gpio_1553b_int();
//================================================
void bus1553_test(); //测试
int  func_RT_BC(unsigned int wRTAddr,unsigned int wSubAddr); 	//BC端，RT-BC模式
void func_BC_RT(unsigned int wRTAddr,unsigned int wSubAddr);	//BC端，BC-RT模式
int send_data2RT(unsigned int wRTAddr,unsigned int wSubAddr,unsigned char* src,const int len);
int recv_data_from_RT(unsigned int wRTAddr,unsigned int wSubAddr,unsigned char* dst);

#endif
