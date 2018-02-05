#include "bus1553base.h"
/*
unsigned int
get_word (unsigned addr)
{
  return *((volatile unsigned int *) (addr));
}

void
set_word (unsigned value, unsigned addr)
{
  *((volatile unsigned int *) (addr)) = value;
}
*/
void BT_MemWrite(unsigned int wMemAddr,unsigned int wData)
{
    *((volatile unsigned int *) (wMemAddr * 4 + ACE_MEM)) = wData;
    //set_word(wData,wMemAddr * 4 + ACE_MEM);
}


unsigned int BT_MemRead(unsigned int wMemAddr)
{
    //return get_word((wMemAddr * 4 + ACE_MEM)) & 0xFFFF;
    return (*((volatile unsigned int *) (wMemAddr * 4 + ACE_MEM)) & 0xFFFF);
}

void BT_Memset(unsigned int wHead, unsigned int wData, unsigned int wLen)
{
    unsigned int i = 0;
    for (i = 0 ; i < wLen ; i++)
        //BT_MemWrite(wData,wHead + i); // ???
        BT_MemWrite(wHead + i, wData);
}


void BT_SetReg(unsigned int wRegID,unsigned int wData)
{
    *((volatile unsigned int *) (wRegID * 4 + ACE_PORT)) = wData;
   	//set_word(wData,wRegID * 4 + ACE_PORT);
}

unsigned int BT_GetReg(unsigned int wRegID)
{
  return (*((volatile unsigned int *) (wRegID * 4 + ACE_PORT)) & 0xFFFF);
	//return get_word(wRegID*4 + ACE_PORT) & 0xFFFF;
}

void BT_OrReg(unsigned int wRegID, unsigned int wValue)
{
	unsigned int value1 = BT_GetReg(wRegID);
    unsigned int value2 = value1 | wValue;
    BT_SetReg(wRegID,value2);
}

void BT_AndReg(unsigned int wRegID, unsigned int wValue)
{
		unsigned int value1 = BT_GetReg(wRegID);
  	unsigned int value2 = value1 & wValue;
  	BT_SetReg(wRegID,value2);
}
