#ifndef _DO_FRAME_H_
#define _DO_FRAME_H_
#include "frame.h"
#define  PROC_IDE 0x100
#define  PROC_TASK_YP 0x1
#define  PROC_TASK_PJ 0x2
#define  PROC_TASK_YS 0x3

//注意接收到的字符串第一个为字符串大小
void m_sleep();
void proc_main_task(int (*recv_func)(), int (*send_func)());
void proc_request_task(int task_type,unsigned char cores,unsigned char ready,int (*send_func)());
void proc_result_task(int task_type,unsigned char cores,int(*recv_func)(),int (*send_func)());
#endif
