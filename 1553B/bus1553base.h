#ifndef _bus1553base_
#define _bus1553base_

#include "bus1553type.h"

//设置读指定地址的值
//extern unsigned get_word (unsigned addr);

//根据地址设置指定数据
//extern void set_word (unsigned value, unsigned addr);   

/* Ê¹ÓÃµÄ²ÎÊýÎª1553ÐŸÆ¬ÖÐµÄµØÖ·£¬º¯ÊýÄÚ×Ô¶¯×ªÎª697µØÖ· */


//ACEÐŸÆ¬ÄÚŽæ¶ÁÐŽ
void BT_MemWrite(unsigned int wMemAddr,unsigned int wData);
unsigned int BT_MemRead(unsigned int wMemAddr);

//ACEÐŸÆ¬ÄÚŽæ¿éÉèÖÃ
void BT_Memset(unsigned int wHead, unsigned int wData, unsigned int wLen);

//ÉèÖÃ¡¢¶ÁÈ¡ACEŒÄŽæÆ÷
void BT_SetReg(unsigned int wRegID,unsigned int wData);
unsigned int BT_GetReg(unsigned int wRegID);

//ŒÄŽæÆ÷°ŽÎ»»ò¡¢Óë
void BT_OrReg(unsigned int wRegID, unsigned int wValue);
void BT_AndReg(unsigned int wRegID, unsigned int wValue);

#endif
