#ifndef IMG_CLOUD_DETECT_TASK
#define IMG_CLOUD_DETECT_TASK

//#include "bus1553.h"
//#include "Remote.h"
//大端小端转换
#define htons(A) (((A & 0x00ff) << 8) | ((A & 0xff00) >> 8))
#define htonl(A) (((A & 0x000000ff) << 24) | ((A & 0x0000ff00) << 8) | ((A & 0x00ff0000) >> 8) | ((A & 0xff000000) >> 24))
#define K (0.017453292519943295769236907684886l)

#define PI (3.1415926)


//extern volatile unsigned int *BASEADDR; 
unsigned int *BASEADDR; 
unsigned int nxsize ;
unsigned int nysize ;

#define GET_POINT(A) ((volatile unsigned int *)(A))

#define CLOUD_DATA_BASEADDR 0x60000000
//#define CLOUD_DETECT_MAT_WIDTH 101
//#define CLOUD_DETECT_MAT_HEIGHT 256
#define CLOUD_DETECT_MAT_BASEADDR 0x60000004 
#define CLOUD_DETECT_MAT_FLASHADDR 0x00400000 
#define CLOUD_DETECT_REF_BASEADDR 0x60200000 

/*
#define MAT(X,Y,Z)  htons(*((volatile unsigned short *)\
					(CLOUD_DETECT_MAT_BASEADDR) + X *  CLOUD_DETECT_MAT_WIDTH * CLOUD_DETECT_MAT_HEIGHT \
					+ CLOUD_DETECT_MAT_WIDTH * Y + Z))
*/
// 反射率
/*
#define REF(X,Y,Z) (*((volatile double *)CLOUD_DETECT_REF_BASEADDR + X *  CLOUD_DETECT_MAT_WIDTH * CLOUD_DETECT_MAT_HEIGHT \
					+ CLOUD_DETECT_MAT_WIDTH * Y + Z))
*/

					
//#define MOSAIC_MAT_WIDTH 66
//#define MOSAIC_MAT_HEIGHT 401
#define MOSAIC_MAT1_BASEADDR  0x60400000  //拼接图像1在sdram地址 
#define MOSAIC_MAT2_BASEADDR 0x60800000   //拼接图像2在sdram地址 
#define MOSAIC_MAT3_BASEADDR 0x60c00000   //拼接图像3在sdram地址 
#define MOSAIC_MAT4_BASEADDR 0x61000000   //拼接图像4在sdram地址 
#define MOSAIC_MAT1_FLASHADDR 0x00500000  //拼接图像1在flash地址 
#define MOSAIC_MAT2_FLASHADDR 0x00600000  //拼接图像2在flash地址 
#define MOSAIC_MAT3_FLASHADDR 0x00700000  //拼接图像3在falsh地址 
#define MOSAIC_MAT4_FLASHADDR 0x00800000  //拼接图像4在flash地址 
#define MOSAIC_MAT_1_2_BASEADDR 0x61400000 //拼接图像1，2在sdram地址
#define MOSAIC_MAT_3_4_BASEADDR 0x61800000 //拼接图像3，4在sdram地址 
#define MOSAIC_RESULT_BASEADDR 0x61c00000  // 拼接完成后在sdram地址

//#define MATLEFT(Y,X) (*((volatile unsigned char *)(MOSAIC_MAT1_BASEADDR) + MOSAIC_MAT_WIDTH * Y + X  ))
//#define MATRIGHT(Y,X) (*((volatile unsigned char *)(MOSAIC_MAT2_BASEADDR) + MOSAIC_MAT_WIDTH * Y + X ))
//#define MATLEFT_2(Y,X) (*((volatile unsigned char *)(MOSAIC_MAT3_BASEADDR) + MOSAIC_MAT_WIDTH * Y + X  ))
//#define MATRIGHT_2(Y,X) (*((volatile unsigned char *)(MOSAIC_MAT4_BASEADDR) + MOSAIC_MAT_WIDTH * Y + X ))

//#define COMPRESS_DATA_LENGTH (24968 - 8)
//#define COMPRESS_IMG_WIDTH 76
//#define COMPRESS_IMG_HEIGHT 101

#define COMPRESS_IMG_BASEADDR 0x62800000 //8-28
#define COMPRESS_IMG_FLASHADDR 0x00900000 
#define COMPRESS_IMG_RESULT 0x60400000 
#define COMPRESS_IMG_RESULT_TMP_2 0x60c00000
#define COMPRESS_IMG_RESULT_TMP_3 0x61400000
#define COMPRESS_IMG_RESULT_TMP_4 0x61c00000


//double task_cloud_detect(img_type *IMG_CLOUD);
void task_cloud_detect_parallel(int core_num);
void task_mosaic_parallel(int core_num);
void task_compress_parallel(int corenum);

#endif
