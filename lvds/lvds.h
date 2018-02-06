#ifndef _lvds_
#define _lvds_

unsigned int write_data2sdram(unsigned int data_size ,unsigned int channel );
unsigned int read_data_sdram2outfifo(unsigned int data_size);
int lvds_get_data(char*buf,unsigned int data_size);
int lvds_send_data_back(char*buf,unsigned int data_size);


#endif
