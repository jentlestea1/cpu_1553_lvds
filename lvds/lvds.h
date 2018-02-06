#ifndef _lvds_
#define _lvds_

unsigned int write_data2sdram(unsigned int data_size ,unsigned int channel );
void lvds_get_data(char*buf,int data_size);
void lvds_send_data_back(char*buf,int data_size);


#endif
