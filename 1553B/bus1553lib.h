
#ifndef _bus1553lib_
#define _bus1553lib_

#include "bus1553type.h"
/*��ģ�����32λ��ֻ�е�16λ��Ч*/

/*bc*/
//BC��ʼ��
void BC_Init();
void BC_Start();
bool BC_FrmInProc();
void BC_MsgStart(BCMSGBUF* pMsg);
void BC_SendData(unsigned int wRTAddr, unsigned int wSubAddr, unsigned short* wDataBuf, unsigned int wDataLen, BUSCHANNEL wBusChan);
void BC_ReceiveData(unsigned int wRTAddr, unsigned int wSubAddr, unsigned int* wDataBuf, unsigned int wDataLen, BUSCHANNEL wBusChan);
unsigned int BC_GenCtrlWord(MESSAGETYPE wMsgType, unsigned int wCtrlWord);

#endif
