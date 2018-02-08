#include "do_frame.h"
#include "../lvds/lvds.h"
#define RECV_BUF_MAX_SIZE 0x10000 //一张图片最多只能有64k
#define SEND_BUF_MAX_SIZE 0x20000 //回传一张图片最多只能有128k
#define RECV_PIC_MAX_NUM 0x4
static int g_task=PROC_IDE;
short RT2BC_buf[33]={0};
short BC2RT_buf[33]={0};
static unsigned char recv_buf[RECV_PIC_MAX_NUM][RECV_BUF_MAX_SIZE]; //用来接收MFC端传过来的图片
static unsigned char send_buf[SEND_BUF_MAX_SIZE]; //用来接收MFC端传过来的图片
static unsigned int ret_pic_size=0;

struct current_task{
    //每个任务需要用到的几个变量
    task proc_task;
    unsigned int pic_num;
    unsigned int picture_size[RECV_PIC_MAX_NUM];
    unsigned char* pic_src_addr;
    unsigned char* pic_dst_addr;
};
struct current_task proc_current_task;
//设置task的图片源地址，目的地址
//
void set_result_pic_size(unsigned int size){
    ret_pic_size=size;
}
void clear_recv_buf(){
    int i=0;
    for(;i<RECV_PIC_MAX_NUM;i++){
        memset(recv_buf[i],0,RECV_BUF_MAX_SIZE);
    }
}
void clear_send_buf(){
    memset(send_buf,0,SEND_BUF_MAX_SIZE);
}

void set_current_task(int task_type,int pic_num_t,unsigned int* pic_size_t){
    clear_send_buf();
    proc_current_task.pic_src_addr=(unsigned char*)recv_buf[0];
    proc_current_task.pic_dst_addr=(unsigned char*)send_buf;
    proc_current_task.proc_task=(task)task_type;
    proc_current_task.pic_num=pic_num_t;
    int i=0;
    for(;i<pic_num_t;i++){
        proc_current_task.picture_size[i]=*(pic_size_t+i);
    }
}
task get_current_task_type(){
    return proc_current_task.proc_task;
}
unsigned int get_current_pic_num(){
    return proc_current_task.pic_num;
}

unsigned int get_current_pic_size(int pic_id){
    if(pic_id>4||pic_id<0||pic_id>=proc_current_task.pic_num)return 0;
    return proc_current_task.picture_size[pic_id];
}

char* get_current_pic_src_addr(){
    return proc_current_task.pic_src_addr;
}

char* get_current_pic_dst_addr(){
    return proc_current_task.pic_dst_addr;
}

void clear_RT2BC_buf(){
	memset(RT2BC_buf,0,32*sizeof(unsigned short));
}
void clear_BC2RT_buf(){
	memset(BC2RT_buf,0,32*sizeof(unsigned short));
}
void m_sleep(){
	int i;
	for(i=5000000;i>0;i--){
		i=i;
    }
}
void proc_request_task(int task_type,unsigned char cores,unsigned char ready,int (*send_func)()){
		//三种request
	    if(task_type==PROC_TASK_YP||task_type==PROC_TASK_PJ||task_type==PROC_TASK_YS){
	    	printf("prapare to send request frame,task id:%d...\n",task_type);
	    	m_sleep();
	    }
	    else{
	    	printf("can not exec current task\n");
	    	return;
	    }
		command_packet packet;
		task_request task;
		clear_BC2RT_buf();
		memset(&packet,0,sizeof(command_packet));
		memset(&task,0,sizeof(task_request));
		switch(task_type){                    //1为云判2为拼接3为压缩
			case PROC_TASK_YP:
				{
					serial_command_packet((unsigned char*)BC2RT_buf,(void*)&packet,(void*)&task,REQUEST_TYPE,YP_SUBTYPE,cores,ready);
					break;
			    }//case 1
			case PROC_TASK_PJ:
				{
					serial_command_packet((unsigned char*)BC2RT_buf,(void*)&packet,(void*)&task,REQUEST_TYPE,PJ_SUBTYPE,cores,ready);
					break;
			    }//case 1
			case PROC_TASK_YS:
				{
					serial_command_packet((unsigned char*)BC2RT_buf,(void*)&packet,(void*)&task,REQUEST_TYPE,YS_SUBTYPE,cores,ready);
					break;
			    }//case 1
				break;
		}
		int is_send_right=(*send_func)(1,2,(unsigned char*)BC2RT_buf,32);
		if(is_send_right==1){
			printf("发送任务请求帧成功,任务号:%d\n",task_type);
			//printf("send request success,task type:%d\n",task_type);
		}
		else{
			printf("send request fail,task type:%d\n",task_type);
		}
		//序列化完成后之后准备发送

}
void proc_interact_task(int task_type,unsigned char cores,int(*recv_func)(),int (*send_func)()){                  //接收遥控信号并返回结果
		if(task_type!=PROC_TASK_YP&&task_type!=PROC_TASK_PJ&&task_type!=PROC_TASK_YS){
	    	printf("can not exec current task\n");
	    	return;
	    }
	    command_packet recv_packet1;
	    command_packet recv_packet2;
	    command_packet result_packet;
	    task_response task_response_t;
	    task_result task_result_t;
	
	    command_packet packet;
		task_request task;
        unsigned int picture_num=0;
		memset(&packet,0,sizeof(command_packet));
		memset(&task,0,sizeof(task_request));
	    unsigned int pic_size[5];//

	    memset(&task_response_t,0,sizeof(task_response));
	    memset(&task_result_t,0,sizeof(task_result));

	    memset(&recv_packet1,0,sizeof(command_packet));
	    memset(&recv_packet2,0,sizeof(command_packet));
	    memset(&result_packet,0,sizeof(command_packet));
	    int is_recv_right=0;
	    int is_send_right=0;
        //先接收遥控包
        while(is_recv_right==0){
        	m_sleep();//可以限制循环的次数
        	//收之前清空
        	clear_RT2BC_buf();
        	is_recv_right=recv_func(2,1,(unsigned char*)RT2BC_buf);
        	if(is_recv_right==1){
			  printf("recv response packet\n");
        		  copy2command_packet((void*)&recv_packet1,(void*)RT2BC_buf);
        		  if(recv_packet1.head.dst!=1||recv_packet1.head.src!=2||recv_packet1.head.type!=11){  //11为response消息
        		        printf("recv error frame,dst:%d,src:%d,type:%d\n",recv_packet2.head.dst,recv_packet2.head.src,recv_packet2.head.type);
        		        is_recv_right=0;
        		        continue;
          		  }
			  else{
				printf("recv response packet successfully,task_id:%d\n",task_type);
			  }
        	}
        }
        //读取packet内的内容,此时的packet的内容为回复request的帧
        copy2task_response((void*)&task_response_t,(void*)&(recv_packet1.buf[0]));
        //判定response内容，准备第二次接收遥控帧
	    if(task_response_t.task_id!=task_type){
		    printf("response packet task_id error,current task_id:%d,packet task_id:%d",task_type,task_response_t.task_id);	
	    }
        else{
        picture_num=task_response_t.number;
		proc_request_task(task_type,cores,1,send_func);//请求info遥控数据包
        	//clear_BC2RT_buf();
        	//serial_command_packet((unsigned char*)BC2RT_buf,(void*)&packet,(void*)&task,REQUEST_TYPE,task_type);
        	//is_send_right=(*send_func)(1,1,(unsigned char*)BC2RT_buf,32);
			//if(is_send_right==1){
			//	printf("成功发回状态帧,任务类型：%d\n",task_type);
			//}
			//else{
			//	printf("发送状态帧失败,任务类型：%d\n",task_type);
			//}
        	//先接收遥控帧，该遥控帧描述了核数和每个图片的大小
 		 is_recv_right=0;
        	 while(is_recv_right==0){
        			m_sleep();//可以限制循环的次数
        			//收之前清空
        			clear_RT2BC_buf();
        			is_recv_right=recv_func(2,1,(unsigned char*)RT2BC_buf);
        			if(is_recv_right==1){
					 printf("recv info packet\n");
        		  		copy2command_packet((void*)&recv_packet2,(void*)RT2BC_buf);
        		  		if(recv_packet2.head.dst!=1||recv_packet2.head.src!=2||recv_packet2.head.type!=1){   //1为formation消息
        		        		printf("recv error frame,dst:%d,src:%d,type:%d\n",recv_packet2.head.dst,recv_packet2.head.src,recv_packet2.head.type);
        		        		is_recv_right=0;
        		        		continue;
          		  		}
          		  		else{	
						printf("recv info packet successfully,task_id:%d\n",task_type);
          		  			unsigned char buf[20];
          		  			copy2info_buf((void*)buf,(void*)recv_packet2.buf);
          		  			int i=0;
          		  			if(cores!=buf[0]){
          		  				printf("cores error,task type:%d\n",task_type);
          		  			}
          		  			for(i=0;i<4*sizeof(int);i++){
          		  				*((char*)pic_size+i)=*(buf+1+i);                  //注意大小端问题
          		  			}
						printf("picture%d: size:%d,picture%d: size:%d,picture%d: size:%d,picture%d: size:%d\n",1,pic_size[0],2,pic_size[1],3,pic_size[2],4,pic_size[3]);
          		  			continue;
          		  		}
        		    }
        	}
        	printf("open lvds,task id:%d,picture num:%d\n",task_response_t.task_id,task_response_t.number);
        	//开启lvds传输图片任务代码段,传入cores和pic_size
            int j=0;
            for(;j<picture_num;j++){
        	    int is_lvds_recv_right=lvds_get_data(recv_buf[j],pic_size[j]);
        	    if(is_lvds_recv_right==1){
        			//回传分析的结果
                    printf("接收到了正确的图片数据,已经将图片写入缓存,图片编号：%d\n",j+1);
        	    }
	    	    else{
        		    printf("使用lvds接受数据错误,图片编号:%d\n",task_response_t.task_id,j+1);
                    return;
        	    }   
            }
            //准备执行任务，先设置任务属性
            set_current_task(task_type,picture_num,pic_size);
            //执行任务，设置结果，回传结果和最终图像
            ////////////////////////////////////////////
            //exec task
            ////////////////////////////////////////////
        }
}
int do_exec_task(int (*exec_task_func)(),int cores){
        exec_task_func(cores);   
}
void proc_result_task(int task_type,unsigned char cores,int (*send_func)()){
			clear_BC2RT_buf();
	        command_packet packet;
		    task_request task;
		    memset(&packet,0,sizeof(command_packet));
		    memset(&task,0,sizeof(task_request));
			serial_command_packet((unsigned char*)BC2RT_buf,(void*)&packet,(void*)&task,RESULT_TYPE,task_type,cores,0);
			int is_send_right=(*send_func)(1,2,(unsigned char*)BC2RT_buf,32);
			if(is_send_right==1){
					printf("send result success,task type:%d\n",task_type);
			}
			else{
					printf("send result fail,task type:%d\n",task_type);
			}
            //清空收图片缓存区

        	int is_lvds_send_right=lvds_send_data_back(send_buf,ret_pic_size);
        	if(is_lvds_send_right==1){
        			//回传分析的结果
                printf("已传回结果图片\n");
        	}
	    	else
        		printf("传输处理后的图片时出现错误\n");
            clear_recv_buf();
}
void proc_main_task(int (*recv_func)(), int (*send_func)(),int (*exec_task_func)()){
      //依次执行三种任务
	  int cycle=1;
      int pic_size[4]={100,0,0,0};
	  int task_type=PROC_TASK_YP;
	  while(cycle){
			printf("prepare to send %d request\n",cycle);
	  		proc_request_task(task_type,4,1,send_func);
            set_current_task(1,1,pic_size);
	  		//proc_interact_task(task_type,4,recv_func,send_func);
            if(do_exec_task(exec_task_func,4)==1){   //核数为4
	  		    //proc_result_task(task_type,4,send_func);
            }
	  	    cycle--;
			task_type++;
			m_sleep();
	  }
	  return;
}

