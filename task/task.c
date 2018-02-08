#include "task.h"
int exec_task(){
    //获取核数，图片大小，图片地址，图片数量

    task task_id = get_current_task_type();
    unsigned int pic_num = get_current_pic_num();
    unsigned int pic_size = get_current_pic_size(0);
    char* pic_base_addr = get_current_pic_src_addr();
    char* pic_dst_addr = get_current_pic_dst_addr();
    printf("正在执行云判任务,任务编号:%d,图片数量:%d,图片大小:%d,图片基地址:0x%x,图片目标地址:0x%x\n",\
            task_id,pic_num,pic_size,pic_base_addr,pic_dst_addr);
    //注意返回前一定要set result,保证结果正确写入

    return 1;
}
