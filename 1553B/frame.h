#ifndef _FRAME_H_
#define _FRAME_H_

#define REQUEST_TYPE 0x1
#define RESULT_TYPE 0x2
#define YP_SUBTYPE 0x5
#define PJ_SUBTYPE 0x6
#define YS_SUBTYPE 0x7
extern short RT2BC_buf[33];
extern short BC2RT_buf[33];

typedef struct command_t{
		unsigned char dst;
		unsigned char src;
		unsigned char type;
		unsigned char subtype;
		unsigned char size;
}command_t;
typedef struct command_packet{
		struct command_t head;
		char buf[58];
}command_packet;
typedef struct task_resquest{
		unsigned char task_id;
		unsigned char cores;
		unsigned char ready;
}task_request;
typedef struct task_response{
		unsigned char task_id;
		unsigned char number;
		unsigned char tran_protocol;
}task_response;

typedef struct RC_frame{
		int sync:8;
		int version:3;
		int frame_data_len:8;
		int frame_type:4;
		int frame_dest:4;
		int fragment:2;
		int count:13;
		int crc_check_sum:16;
		int onoffinst:2;
		int delay_time:8;
		int reserve:12;
		unsigned char buf[20];
}RC_frame;
typedef struct graph_yp{
		short curstate;
		short cores;
		int time;
		double result;
}graph_yp;
typedef struct graph_pj{
		short curstate;
		short cores;
		int time;
		double result;
}graph_pj;
typedef struct graph_ys{    //图像压缩结果
		short curstate;
		short cores;
		int time;
		double result;
}graph_ys;
typedef struct task_result{          //结果帧格式
		int which_gra_task;
		union{
			struct graph_yp gra_yp;	
			struct graph_pj gra_pj;	
			struct graph_ys gra_ys;	
		}cur_gra_task;
}task_result;
//typedef task_result{
//		int corestates;
//		float syspayload;     //序列化的时候必须紧靠
//		graphproc gra_proc;
//}task_result;

//序列化

void copy2info_buf(void*buf,void*rc_frame);
void copy2command_packet(void* packet,void* src);
void copy2task_response(void* task,void* rc_frame);
void serial_command_packet(unsigned char* dst,void* packet,void* task,int type,int sub_type,unsigned cores,unsigned char ready);   //序列化请求帧
void serial_status_packet(unsigned char* dst);
void set_corestate(int corestate_t);
void set_syspayload(float* syspayload_t,int len);
void set_proc_gra_result(int task_type,short curstate,short cores,int time,double result);
#endif
