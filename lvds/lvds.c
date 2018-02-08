#include "lvds.h"
/*********************************
//function      :
//name of register  :SDRAM_DBS
//cpu address   :0x2005c040  addr[18:2]=0x17010
/********************************/
//sdram data bus source
#define SDRAM_DBS *(volatile unsigned int *)(0x2005c040)
//default:write sdram data from CPU,read sdram data to cpu
#define SDRAM_DATA_DEFAULT      0x00000000  
//WR_SDRAM_DATA_FROM_CPU:write sdram data from FIFO(FPGA)
#define WR_SDRAM_DATA_FROM_FIFO  0x00000001
//RD_SDRAM_DATA_TO_FIFO:read sdram data to FIFO
#define RD_SDRAM_DATA_TO_FIFO   0x00000002
/*********************************
//function      :FIFO BLOCK FLOG
//name of register  :FIFO_BF
//cpu address   :0x2005c104 addr[18:2]=0x17011
*********************************/
//
#define FIFO_BF *(volatile unsigned int *)(0x2005c044)  //output fifo is also write-fifo
//channel = 1,2,3,4,5
//1,2,3,4:IN_DATA  5:outDATA
#define IS_256words_infifo_flag(channel) ((FIFO_BF & (0x00000001 << (channel-1)*4)) >> (channel-1)*4)
#define IS_768words_infifo_flag(channel) ((FIFO_BF & (0x00000001 << (channel+2)*4)) >> (channel+2)*4)

#define IS_512words_outfifo_flag ((FIFO_BF & (0x00000001 << 17))>>17)
/*********************************
//function      :output-FIFO
//name of register  :CLVDS_OUTR
//cpu address   :0x2005c100 addr[18:2]=0x17040
*********************************/
//Control output-lvds register
#define CLVDS_OUTR *(volatile unsigned int *)(0x2005c104)  //output fifo is also write-fifo
#define SET_lvds_out_en 0x00000001
/*********************************
//function      :cpu request read input-FIFO
//name of register  :CLVDS_INR
//cpu address   :0x2005c100 addr[18:2]=0x17040
*********************************/
//Control output-lvds register
#define CLVDS_INR *(volatile unsigned int *)(0x2005c100)  //output fifo is also write-fifo
/*
--	read FIFO user words
*/
#define INPUT_FIFO1_uswrd 0x2005e000
#define INPUT_FIFO2_uswrd 0x2005e004
#define INPUT_FIFO3_uswrd 0x2005e008
#define INPUT_FIFO4_uswrd 0x2005e00c
#define OUTPUT_FIFO_uswrd 0x2005e010
//#define sdram_size 258*k*k      //64M
#define sdram_addr_base_move 0x00000000  //sdram的控制器 60000000
#define sdram_addr_IN_DATA2 0x02000000  //sdram的控制器 60000000
#define sdram_addr_IN_DATA3 0x04000000  //sdram的控制器 60000000
#define sdram_addr_IN_DATA4 0x06000000  //sdram的控制器 60000000
#define sdram_addr_OUT_DATA 0x08000000  //sdram的控制器 60000000
#define sdram_addr_base   (0x60000000 + sdram_addr_base_move) //将sdram地址嫁接到IO的区，但仍然用sdram的控制器 60000000
////////////////////////
#define 	k		1024			//定义1K=1024B
#define sdram_size 0x400     // 
//------------memery configuration register-----------
#define MEM_CFG1 *(volatile unsigned int*)(0x80000000) 
#define MEM_CFG2 *(volatile unsigned int*)(0x80000004)  //存储器配置寄存器2
#define MEM_CFG3 *(volatile unsigned int*)(0x80000008)  //存储器配置寄存器3

static void mem_init (){
    MEM_CFG1 = 0x140802ff;
    MEM_CFG2 = 0xe2c05060;
    //  MEM_CFG3 = 0x000ff000;  
}

//数据流向，sdram到outfifo
void append_data2sdram(char*buf,int data_size){   //buf为挂载的写数据流地址,data_size为要写入的字节大小
	SDRAM_DBS = 0x00000000; //数据总线由cpu接到sdram
	unsigned int tmp=0;
    char* p_tmp=(char*)&tmp;
	int i=0;
    int count=data_size%4==0?(data_size/4):(data_size/4+1);
    unsigned int addr = 0x8000000;
    //每次写四个字节
    for(;i<count;i++){
        int j=0;
        for(;j<4;j++){
            *(p_tmp+j)=*(buf+i*4+j);
        }
	 *(volatile unsigned int *)(sdram_addr_base+addr) = tmp;
     addr+=4;//每次写四个字节
    }
}
int lvds_send_data_back(char*buf,unsigned int data_size){
    append_data2sdram(buf,data_size);
    read_data_sdram2outfifo(data_size);
    return 1;
}
void get_data_from_sdram(char*buf,int data_size){
	SDRAM_DBS = 0x00000000; //数据总线由cpu接到sdram
	unsigned int tmp=0;
    char* p_tmp=(char*)&tmp;
	int i=0;
    int channel=1;
    int count=data_size%4==0?(data_size/4):(data_size/4+1);
    unsigned int addr = 0x02000000*(channel-1);//channel默认为1
    for(;i<count;i++){
	    tmp = *(volatile unsigned int *)(sdram_addr_base+ addr);
        int j=0;
            for(;j<4;j++){
                *(buf+i*4+j)=*(p_tmp+j);
            }
        addr+=4;//每次写四个字节
    }
}
int lvds_get_data(char*buf,unsigned int data_size){
    write_data2sdram(data_size ,1);
    get_data_from_sdram(buf,data_size);
    return 1;
}
unsigned int read_data_sdram2outfifo(unsigned int data_size){
    unsigned int count=0;
    unsigned int data;
    unsigned int addr;
    unsigned int flag_break ;
    unsigned int num_data_size;
    int i; 
    flag_break = data_size/4/256;
    num_data_size = data_size%(256*4);
    CLVDS_OUTR = 0x00000000 | SET_lvds_out_en; 
    addr = 0x8000000;
    SDRAM_DBS = (0x00000000 | RD_SDRAM_DATA_TO_FIFO)/**/;
    while(1){
        if(IS_512words_outfifo_flag && flag_break > count){
          for(i=0;i<sdram_size/4;i++){    
            data = *(volatile unsigned int *)(sdram_addr_base+ addr);
            addr = addr + 4; 
          } 
          count++; 
        }
        else if(IS_512words_outfifo_flag && (flag_break == count)){
          for(i=0;i<num_data_size/4;i++){    
            data = *(volatile unsigned int *)(sdram_addr_base+ addr);
            addr = addr + 4; 
          } 
          break;
        }
          
    }
}
//数据流向，infifo到sdram，channel : 1 ,2,3,4
unsigned int write_data2sdram(unsigned int data_size ,unsigned int channel ){
  int count = 0;
  int i;
  unsigned int fifo_usewd;
  unsigned int addr ;
  unsigned int flag_break;
  flag_break = data_size%1024==0?data_size/1024:(data_size/1024+1);
  CLVDS_INR  = 0x0000000f; 
  //CLVDS_OUTR = 0x00000000 | SET_lvds_out_en; 
  //sdram data configure
  SDRAM_DBS = (0x00000000 | WR_SDRAM_DATA_FROM_FIFO)/**/; 

  addr = 0x02000000*(channel-1);
/******************开始，方式1******************//**/
while(1){
    //256为1个单位
    if(IS_256words_infifo_flag(1)){//读一个IO时序
      for(i=0;i<sdram_size/4;i++){  //write sdram
          *(volatile unsigned int *)(sdram_addr_base+addr) = 1;
          addr = addr + 4;
       }
       count++;
    }
   // printf("count : %d\n",count);
    if(count >= flag_break) break; 
} 
return 1;

/********************结束，方式1*****************/ 
/*
  //printf("fifo usewd:%d \n-------------------\n",*(volatile unsigned int *)(0x2005e000));
  fifo_usewd = *(volatile unsigned int *)(0x2005e000);  
  //printf("fifo usewd:%d \n ok",*(volatile unsigned int *)(0x2005e000));
  for(i=0;i<fifo_usewd;i++){  //write sdram
      *(volatile unsigned int *)(sdram_addr_base+addr) = i;
      addr = addr +4;
      //printf("num of fifo data:%d \n ",*(volatile unsigned int *)(0x2005e000));
  }

  return *(volatile unsigned int *)(0x2005e000);
*/

}
/*
int main(){
  int count = 0;
	int i;
  unsigned int addr = 0;
  unsigned int infifo1_addr = 0x00000000;
  unsigned int infifo2_addr = 0x02000000;
  unsigned int infifo3_addr = 0x04000000;
  unsigned int infifo4_addr = 0x06000000;
  unsigned int write_sdram_flag = 0;
  mem_init();
  //lvds enable
	CLVDS_INR  = 0x0000000f; 
  CLVDS_OUTR = 0x00000000 | SET_lvds_out_en; 

  //sdram data configure
  SDRAM_DBS = 0x00000000 | WR_SDRAM_DATA_FROM_FIFO; 
  printf("\nSDRAM test......\n");	
  while(1){
  	     if(IS_256words_infifo_flag(1)) { addr = infifo1_addr;write_sdram_flag = 1;}
    else if(IS_256words_infifo_flag(2)) { addr = infifo2_addr;write_sdram_flag = 1;}
    else if(IS_256words_infifo_flag(3)) { addr = infifo3_addr;write_sdram_flag = 1;}
    else if(IS_256words_infifo_flag(4)) { addr = infifo4_addr;write_sdram_flag = 1;}
    else {addr = 0x00000000; write_sdram_flag = 0; }
    //printf("%x\t %x\t %x\t %x\n",IS_256words_infifo_flag(1),IS_256words_infifo_flag(2),IS_256words_infifo_flag(3),IS_256words_infifo_flag(4));
    if(write_sdram_flag == 1)
      for(i=0;i<sdram_size/4;i++){	//write sdram
          *(volatile unsigned int *)(sdram_addr_base+addr) = i;
          addr = addr + 4;
          count = count +1;
       } 
         if(addr < 0x02000000) infifo1_addr = addr;
    else if(addr < 0x04000000) infifo2_addr = addr;
    else if(addr < 0x06000000) infifo3_addr = addr;
    else if(0x06000000 <= addr < 0x08000000) infifo4_addr = addr;

    //if((*(volatile unsigned int *)(0x2005c054) & 0x00000001)==1) 
       // printf("ok,%d,%d",*(volatile unsigned int *)(0x2005c048),*(volatile unsigned int *)(0x2005c04c));
  }


}*/



