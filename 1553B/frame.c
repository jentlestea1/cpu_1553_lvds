#include "frame.h"

int corestates=0x00001111;//载核状态全满
float syspayload[5]={4.0,4.0,4.0,4.0,4.0};//载核为常数4
graph_yp proc_gra_yp={
	2,4,4,4.0
};
graph_pj proc_gra_pj={
	2,4,5,5.0
};
graph_ys proc_gra_ys={
	2,4,6,6.0
};

void set_corestate(int corestate_t){
	corestates=corestate_t;
}
void set_syspayload(float* syspayload_t,int len){
	if(len<0||len>5){
		printf("不能设置syspayload\n");
		return;
	}
	memcpy(syspayload,syspayload_t,len);
}
void set_proc_gra_result(int task_type,short curstate,short cores,int time,double result){
	switch(task_type){
        case 1 : 
		{
		    proc_gra_yp.curstate=curstate;
		    proc_gra_yp.cores=cores;
		    proc_gra_yp.time=time;
		    proc_gra_yp.result=result;
		    break;
		}
        case 2 :
		{
		    proc_gra_pj.curstate=curstate;
		    proc_gra_pj.cores=cores;
		    proc_gra_pj.time=time;
		    proc_gra_pj.result=result;
		    break;
		}
        case 3 :
		{
	        proc_gra_ys.curstate=curstate;
		    proc_gra_ys.cores=cores;
		    proc_gra_ys.time=time;
		    proc_gra_ys.result=result;
		    break;
		}	
		default:
			break;
	}
}
void copy2info_buf(void* buf,void* rc_frame){      //去掉帧头和帧尾，分别为unsigned char cores,int size[4]
		unsigned char* p=(unsigned char*)buf;
		RC_frame* p_r_f=(RC_frame*)rc_frame;
		unsigned char tmp[30]={0};
		int len=4*sizeof(int)+sizeof(unsigned char);
		memcpy((void*)tmp,(void*)((unsigned char*)p_r_f->buf+1),len+2);   //RCframe 1个字节偏移
		//int i=0;
		/*for(;i<len+2;i++){
			printf("in 0x%x",*((char*)tmp+i));
		}
		*/
		if(tmp[0]==16&&tmp[len+1]==17){
			int i;
			*p=*(tmp+1);
		
			for(i=1;i<4*sizeof(unsigned int)+1;i++){
				*(p+i)=*(tmp+i+1);
			}
				//memcpy((void*)p,tmp+1,len);
		}
		else if(tmp[0]==18&&tmp[len+1]==19){
		        int i;
			*p=*(tmp+1);
		
			for(i=1;i<4*sizeof(unsigned int)+1;i++){
				*(p+i)=*(tmp+i+1);
			}
		}
		else if(tmp[0]==20&&tmp[len+1]==21){
			int i;
			*p=*(tmp+1);
		
			for(i=1;i<4*sizeof(unsigned int)+1;i++){
				*(p+i)=*(tmp+i+1);
			}
		}
		else{
			printf("收到task_info的错误帧\n");
		}
}
void copy2task_response(void* task,void* rc_frame){   //从RC_frame buffer拷贝
		task_response* p=(task_response*)task;
		RC_frame* p_r_f=(RC_frame*)rc_frame;
		unsigned char tmp[10]={0};
		int len=sizeof(task_response);
		memcpy((void*)tmp,(void*)((char*)p_r_f->buf+1),len+2);        //RCframe 加一个字节偏移
		int i=0;
		//for(;i<5;i++){
		//	printf("in 0x%x",*((char*)tmp+i));
		//}
		if(tmp[0]==16&&tmp[len+1]==17){
			if(tmp[1]==1){
				p->task_id=(unsigned char)tmp[1];
				p->number=(unsigned char)tmp[2];
				p->tran_protocol=(unsigned char)tmp[3];
				//memcpy((void*)p,tmp+1,len);
			}
			else{
				printf("收到帧16出错,task_id:%d\n",tmp[1]);
			}
		}
		else if(tmp[0]==18&&tmp[len+1]==19){
			if(tmp[1]==3){
				p->task_id=(unsigned char)tmp[1];
				p->number=(unsigned char)tmp[2];
				p->tran_protocol=(unsigned char)tmp[3];
				//memcpy((void*)p,tmp+1,len);
			}
				else{
				printf("收到帧18出错,task_id:%d\n",tmp[1]);
			}
		}
		else if(tmp[0]==20&&tmp[len+1]==21){
			if(tmp[1]==2){
				p->task_id=(unsigned char)tmp[1];
				p->number=(unsigned char)tmp[2];
				p->tran_protocol=(unsigned char)tmp[3];
				//memcpy((void*)p,tmp+1,len);
			}
				else{
				printf("收到帧20出错,task_id:%d\n",tmp[1]);
			}
		}
}
void copy2command_packet(void* packet,void* src){           //拷贝接收到的遥控帧
		int len=sizeof(command_packet);  
		//不做任何收发判断
		if(*((unsigned char*)src)==0)return;
		memcpy(packet,(unsigned char*)src+1,len);
}

void serial_command_packet(unsigned char* dst,void* packet,void* task,int type,int sub_type,unsigned cores,unsigned char ready){     //序列化要发的遥测帧
		command_packet* p=(command_packet*)packet;
		int len=sizeof(command_packet);
		int anchor=0;
		unsigned char* p_buf=(unsigned char*)(p->buf);
		p->head.dst=2;
		p->head.src=1;
 		p->head.subtype=1;
		if(type==REQUEST_TYPE){
			p->head.type=10;		
		}
		else p->head.type=1;
		unsigned char* p_cores=(unsigned char*)&corestates;
        	unsigned char* p_sysload=(unsigned char*)syspayload;
        	task_result task_result_t;
        	memset((void*)&task_result_t,0,sizeof(task_result));
        	switch(sub_type){
        	case YP_SUBTYPE:
        	{
        		 task_result_t.which_gra_task=1;
        		 task_result_t.cur_gra_task.gra_yp=proc_gra_yp;
        	}
        	case PJ_SUBTYPE:
        	{
				 task_result_t.which_gra_task=2;
        		 task_result_t.cur_gra_task.gra_pj=proc_gra_pj;
        	}
        	case YS_SUBTYPE:
        	{
				 task_result_t.which_gra_task=3;
        		 task_result_t.cur_gra_task.gra_ys=proc_gra_ys;
        	}
        	default:
        		break;
        	}
        	int i=0;
        	for(;i<sizeof(int);i++){
        		*(p_buf+anchor)=*(p_cores+i);
        		anchor++;
        	}
        	i=0;
        	for(;i<5*sizeof(float);i++){
        		*(p_buf+anchor)=*(p_sysload+i);
        		anchor++;
        	}
		if(type==REQUEST_TYPE){
			task_request* p_t_req=(task_request*)task;
		 
		    p->head.size=sizeof(task_request)+sizeof(int)+5*sizeof(float);
		    if(sub_type==YP_SUBTYPE){
		    		p_t_req->task_id=1;
		    }
		    else if(sub_type==PJ_SUBTYPE){
					p_t_req->task_id=2;
		    }
		    else if(sub_type==YS_SUBTYPE){
					p_t_req->task_id=3;
		    }
		    p_t_req->ready=ready;
		    p_t_req->cores=cores;
		    *(p_buf+anchor)=p_t_req->task_id;
		    anchor++;
		    *(p_buf+anchor)=p_t_req->cores;
		    anchor++;
		    *(p_buf+anchor)=p_t_req->ready;
		    anchor++;
	        } 
		else if(type==RESULT_TYPE){
		    task_result* p_t_res=(task_result*)task;
		   
		    p->head.size=sizeof(task_result)+sizeof(int)+5*sizeof(float);  
	            unsigned char* p_tmp;
		    memcpy((unsigned char*)p_t_res,&task_result_t,sizeof(task_result));
			/*for(;i<sizeof(int);i++){
				*(p_buf+j)=*(p_tmp+i);
				j++;
			}
            p_tmp=(unsigned char*)(&p_t_res->syspayload);
			for(i=0;i<sizeof(float);i++){
				*(p_buf+j)=*(p_tmp+i);
				j++;
			}*/
		
		//接下来直接按原来格式写入
		   p_tmp=(unsigned char*)(&task_result_t);
		   memcpy(p_buf+anchor,p_tmp,sizeof(task_result));
		   anchor+=sizeof(task_result);

	        }
	        *dst=len;
	        memcpy((void*)(dst+1),(void*)packet,len);

}   //序列化请求帧
void serial_status_packet(unsigned char* dst){      //纯状态帧，地测软件只会更新对应的状态位
		command_packet packet;
	    command_packet* p=(command_packet*)&packet;
		int len=sizeof(command_packet);
		int anchor=0;
		unsigned char* p_buf=(unsigned char*)(p->buf);
		p->head.dst=2;
		p->head.src=1;
		p->head.type=10;
		p->head.subtype=1;
		p->head.size=sizeof(int)+sizeof(short);
		unsigned char* p_cores=(unsigned char*)&corestates;
        unsigned char* p_sysload=(unsigned char*)&syspayload;
        int i=0;
        for(;i<sizeof(int);i++){
        		*(p_buf+anchor)=*(p_cores+i);
        		anchor++;
        }
        i=0;
        for(;i<sizeof(float);i++){
        		*(p_buf+anchor)=*(p_sysload+i);
        		anchor++;
        }
        *dst=len;
	    memcpy((void*)(dst+1),(void*)&packet,len);
}
