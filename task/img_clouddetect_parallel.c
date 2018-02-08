#include "img_config.h"
#include <stdio.h>
#include <fsu_pthread.h>
#include <unistd.h>
//#include <time.h>
#include <sys/time.h>
#include "task.h"

#define SLOTCLOCK 100000//10000
#define STDTIMECLOCK 1000000
static int calculate_reflect_1=0;
static int calculate_reflect_2=0;
static int calculate_reflect_3=0;
static int calculate_reflect_4=0;

/*
struct graph_yp{
     short cursys; // 任务所处状态
     short cores; // 核的使用情况？
     int time; // 
     double result; // 
 };
  
 struct graph_pj{
     short cursys;
     short cores;
     int time;
     double result; 
 };
struct graph_ys{
    short cursys;
    short cores;
    int time;
    double result;
};
struct graphproc{
    int which_graph_task;  // 表示图像任务的种类
    union{
         struct graph_yp gra_yp; // 云判 0
         struct graph_pj gra_pj; // 拼接 1
         struct graph_ys gra_ys; // 压缩 2
      }cur_gra_task;
 };
*/

/*int cloud_lock;*/
int flag_complete_lock;
int flag_complete_num = 0;

/*儒略日*/
int julian_day = 123; 
/*太阳高度角*/
double sun_elevation = 50.62211;
/*六个通道的太阳辐照度*/
double esun[6] = {1821.99,1557.66,982.06,451.00,358.42,226.69};
//计算cos(90 - sun_elevation)
double cos_value ;
/*日地距离*/
double d ;
unsigned int cloud_detect_ref[20*1024];//80k的ref存储区
//timeval start_t,end_t;
struct task_result img_task_enable;
typedef struct timevalue                                                    
{      
    unsigned long sec; //second
    unsigned long snum; //intermission
    unsigned long usec; // second/100000
} timevalue;

timevalue old_time;
timevalue new_time;
timevalue diff_time;
unsigned int* cloud_detect_mat_base_addr=0x0;
unsigned int* cloud_detect_ref_base_addr=cloud_detect_ref;

unsigned short mat(int x,int y, int z)
{
	//return /*htons*/(*((volatile unsigned short *)(CLOUD_DETECT_MAT_BASEADDR) + x * nxsize * nysize + nxsize * y + z));
	return /*htons*/(*((unsigned short *)(cloud_detect_mat_base_addr) + x * nxsize * nysize + nxsize * y + z));
}

double* ref(int x,int y,int z)
{
	//return ((volatile double *)CLOUD_DETECT_REF_BASEADDR + x *nxsize * nysize + nxsize * y + z);
	return ((volatile double *)cloud_detect_ref_base_addr + x *nxsize * nysize + nxsize * y + z);
}
void getnow(timevalue *nowtime) //获取当前时间                                                         
{
     nowtime->sec = 0;
     nowtime->snum = 0;
     nowtime->usec = 0;
}



void prepare_data_for_cloud_detect()
{
	printf("cloud detect start...\n");
	unsigned int *BASEADDR  = (unsigned int *)get_current_pic_src_addr();
	unsigned int cloud_data_length;
	unsigned int i;
	
	nxsize = *(unsigned short *)(BASEADDR);
	nysize = *(((unsigned short *)(BASEADDR)) + 0x1);
	//已经做好大小端转换
	//nxsize = htons(nxsize);
	//nysize = htons(nysize);
	
	printf("nxsize is %x, nysize is %x, address: 0x%x, 0x%x\n", nxsize, nysize, &nxsize, &nysize);

	cloud_data_length = nxsize*nysize*12;       // 6个通道的数据，每个数据均为short型

	//1-14
    //SDRAM先不与FLASH校对
//	ReadFlash(CLOUD_DETECT_MAT_FLASHADDR + 4,cloud_data_length/4,CLOUD_DETECT_MAT_BASEADDR);

	/*
	for (i = 0; i < (cloud_data_length/4 ); ++i) {
		*((volatile unsigned int *)CLOUD_DETECT_MAT_BASEADDR + i) =  
		*((volatile unsigned int *)CLOUD_DETECT_MAT_FLASHADDR + 1 + i);
	}
	*/
	

	//校对SDRAM和flash的数据是否一致
    //SDRAM先不与FLASH校对

/*
	for (i = 0; i < cloud_data_length; ++i) {
		if (*((volatile unsigned char *)CLOUD_DETECT_MAT_BASEADDR + i) != 
		*((volatile unsigned char *)CLOUD_DETECT_MAT_FLASHADDR + 4 + i)) 
			printf("data not match!!!, i is %d, %d , %d\n", i, *((volatile unsigned char *)CLOUD_DETECT_MAT_BASEADDR + i),
			 *((volatile unsigned char *)CLOUD_DETECT_MAT_FLASHADDR + 4 + i));
	}
*/
    cloud_detect_mat_base_addr= (unsigned int *)get_current_pic_src_addr()+1;//前四个字节为图片长宽
	printf("prepare_data_for_cloud_detect done!\n");
}

void flag_complete_augment()
{

//	turn_off_irq(THIS_CPU());
//	SPINLOCK_SPIN(flag_complete_lock);                
	//flag_complete_num++;
//	SPINLOCK_CLEAR(flag_complete_lock);               
//	turn_on_irq(THIS_CPU());   
}

// 计算反射率
void reflection_1()
{
	int i,j,t;
	double r;

	for (i = 0; i < 1; ++i) 
	{
	for (j = 0; j < nysize; ++j)
	    for (t = 0; t < nxsize; ++t) {	
	        r = (PI * d * d * ((double)mat(i,j,t) / 40)) / (esun[i] * cos_value);
				*ref(i,j,t) = r;
	    }
	}

    calculate_reflect_1=1;
	//flag_complete_augment();
}

void reflection_2()
{
	int i,j,t;
    double r;
    	
	for (i = 1; i < 2; ++i) 
	{
        for (j = 0; j < nysize; ++j)
            for (t = 0; t < nxsize; ++t) 
			{
                r = (PI * d * d * ((double)mat(i,j,t) / 40)) / (esun[i] * cos_value);
				*ref(i,j,t) = r;
            }
    }

    calculate_reflect_2=1;
	//flag_complete_augment();
}

void reflection_3()
{
	int i,j,t;
	double r;
    	
	for (i = 2; i < 4; ++i) 
	{
       for (j = 0; j < nysize; ++j)
           for (t = 0; t < nxsize; ++t) {
               r = (PI * d * d * ((double)mat(i,j,t) / 80)) / (esun[i] * cos_value); 
               *ref(i,j,t) = r;
           }
    }

    calculate_reflect_3=1;
	//flag_complete_augment();
}

void reflection_4()
{
	int i,j,t;
    double r;
    	
	for (i = 4; i < 6; ++i) 
	{
       for (j = 0; j < nysize; ++j)
           for (t = 0; t < nxsize; ++t) {
               r = (PI * d * d * ((double)mat(i,j,t) / 80)) / (esun[i] * cos_value); 
               *ref(i,j,t) = r;
           }
    }

    calculate_reflect_4=1;
	//flag_complete_augment();
}

//实际云判
void cloud_detect(int *cloud)
{
    //cloud detect start
    double ndsi;
    double dsi;
	int j,t;

    for (j = 0; j < nysize; ++j)
   	{
        for (t = 0; t < nxsize; ++t) 
        {
                    
            ndsi = (*ref(0,j,t) -*ref(5,j,t)) / (*ref(0,j,t) + *ref(5,j,t));
            if (*ref(4,j,t) > 0.1 && *ref(1,j,t) / *ref(2,j,t) < 0.7 && ndsi < -0.01) 
            {
				(*cloud)++;
                continue;
            }

            if (*ref(1,j,t) > 0.3 && *ref(1,j,t) / *ref(2,j,t) > 0.7 ) 
            {
                dsi = (*ref(3,j,t) - *ref(5,j,t)) / (*ref(3,j,t) + *ref(5,j,t));
                if (dsi >= -0.01) 
                {
                     if (ndsi > 0.4) continue;
					 (*cloud)++;
                }
            }
        }
     }
}

/*
 * 测试用parallel_4
 */
void task_cloud_detect_parallel_4()
{   pthread_t th;
    int i=1;
	d = 1 + 0.0167 * sin(2 * PI * (julian_day - 93.5) / 360);
    cos_value = cos(K * (90 - sun_elevation));

	pthread_create(&th,NULL,(pthread_func_t)reflection_1,&i);
	pthread_create(&th,NULL,(pthread_func_t)reflection_2,&i);
	pthread_create(&th,NULL,(pthread_func_t)reflection_3,&i);
	pthread_create(&th,NULL,(pthread_func_t)reflection_4,&i);
    
    //while(flag_complete_num<4)
	//	sleep(1);
    //while(!calculate_reflect_1||!calculate_reflect_2||!calculate_reflect_3||!calculate_reflect_4)
	//	sleep(1);
	
}

/*
 * 云判双核运行
 */
void task_cloud_detect_parallel_2()
{   pthread_t th;
    int i=1;
	d = 1 + 0.0167 * sin(2 * PI * (julian_day - 93.5) / 360);
    cos_value = cos(K * (90 - sun_elevation));

	pthread_create(&th,NULL,(pthread_func_t)reflection_1,&i);
	pthread_create(&th,NULL,(pthread_func_t)reflection_2,&i);
	pthread_create(&th,NULL,(pthread_func_t)reflection_3,&i);
	pthread_create(&th,NULL,(pthread_func_t)reflection_4,&i);
    
    while(flag_complete_num<4)
		sleep(1);
	
}
void task_cloud_detect_parallel_1()
{   pthread_t th;
    int i=1;
	d = 1 + 0.0167 * sin(2 * PI * (julian_day - 93.5) / 360);
    cos_value = cos(K * (90 - sun_elevation));

	pthread_create(&th,NULL,(pthread_func_t)reflection_1,&i);
	pthread_create(&th,NULL,(pthread_func_t)reflection_2,&i);
	pthread_create(&th,NULL,(pthread_func_t)reflection_3,&i);
	pthread_create(&th,NULL,(pthread_func_t)reflection_4,&i);
    
    //while(flag_complete_num<4)
    while(!calculate_reflect_1||!calculate_reflect_2||!calculate_reflect_3||!calculate_reflect_4)
		sleep(1);
	
}
/*
void task_cloud_detect_parallel_2()
{	
	d = 1 + 0.0167 * sin(2 * PI * (julian_day - 93.5) / 360);
	cos_value = cos(K * (90 - sun_elevation));

	kernel_create_thread(reflection_1, prio_5, 0, "");
	kernel_create_thread(reflection_2, prio_5, 1, "");
	kernel_create_thread(reflection_3, prio_5, 0, "");
	kernel_create_thread(reflection_4, prio_5, 1, "");

	while(flag_complete_num<4)
		sleep(1);
	
}
*/
/*
 * 云判单核运行
 */
/*
void task_cloud_detect_parallel_1()
{
	d = 1 + 0.0167 * sin(2 * PI * (julian_day - 93.5) / 360);
    cos_value = cos(K * (90 - sun_elevation));

	kernel_create_thread(reflection_1, prio_5, 0, "");
	kernel_create_thread(reflection_2, prio_5, 0, "");
	kernel_create_thread(reflection_3, prio_5, 0, "");
	kernel_create_thread(reflection_4, prio_5, 0, "");

	while(flag_complete_num<4)
		sleep(1);
}
*/
void task_cloud_detect_parallel(int core_num)
{
	flag_complete_lock = 0;
	flag_complete_num = 0;
	//printfLock("core_num = %d\n",core_num);

	// 2015-1-8
	img_task_enable.which_gra_task = 0x1; // 云判任务开启标志
	img_task_enable.cur_gra_task.gra_yp.curstate = 0x1; // 状态1 表示任务图像传输中
	img_task_enable.cur_gra_task.gra_yp.cores = core_num; // 任务运行核数目
	img_task_enable.cur_gra_task.gra_yp.time = 0; // 任务时间？
	img_task_enable.cur_gra_task.gra_yp.result = 0; // 任务运行结果

	prepare_data_for_cloud_detect(); 
	//int core_num = 4;                 // 运行内核数
	int cloud = 0;                    // 含云量

	//ReadFlash(CLOUD_DETECT_MAT_FLASHADDR + 4,16,CLOUD_DETECT_MAT_BASEADDR);
	//unsigned int buf[1002];
	//ReadFlash((volatile unsigned int *)0x0,1000,buf);

	

	getnow(&old_time);
	
	if(core_num == 1)
		task_cloud_detect_parallel_1();
	else if (core_num == 2)
		task_cloud_detect_parallel_2();
	else if (core_num == 4)
		task_cloud_detect_parallel_4();
	else{
		printf("Error core number. \n");
		return;
	}
	cloud_detect(&cloud);
	getnow(&new_time);

	printf("Old Running time is %d %d. \n",old_time.sec,old_time.usec);
	printf("New Running time is %d %d. \n",new_time.sec,new_time.usec);
	printf("Diff time is %d.\n",(new_time.sec - old_time.sec) * STDTIMECLOCK + ( new_time.snum - old_time.snum) + SLOTCLOCK + (new_time.usec - old_time.usec));
	printf("pix num is %d\n", cloud);
    if(nxsize*nysize!=0)
	printf("cloud ratio: %lf\n", (double)cloud / (nxsize * nysize));

	// 2015-1-8
	img_task_enable.which_gra_task = 0x1; // 云判任务开启标志
	img_task_enable.cur_gra_task.gra_yp.curstate = 0x2; // 状态2 表示任务完成 返回结果
	img_task_enable.cur_gra_task.gra_yp.cores = core_num; // 任务运行核数目
	img_task_enable.cur_gra_task.gra_yp.time = (new_time.sec - old_time.sec) * STDTIMECLOCK + ( new_time.snum - old_time.snum) + SLOTCLOCK + (new_time.usec - old_time.usec); // 任务时间？
	img_task_enable.cur_gra_task.gra_yp.result = (double)cloud / (nxsize * nysize); // 任务运行结果

}
//int main(){

 //   task_cloud_detect_parallel(4);
 //   return 0;
//}
