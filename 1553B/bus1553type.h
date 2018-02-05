
#ifndef _bus1553type_
#define _bus1553type_

#define BUS_1553_INST_ISR_ROUTE    0x03030303
#define BUS_1553_SET_REG_VALUE   0x04040404
#define BUS_1553_GET_REG_VALUE   0x05050505

#define bool int
#define true 1
#define false 0
/*
#define BC_PORT 0x20024000
#define BC_MEM  0x20020000
#define BC_INT_NUM 1   // External Interrupt 1
*/

#define ACE_PORT 0x20020000
#define ACE_MEM  0x20030000

#define DEV_1553_A 0x50505050
#define DEV_1553_B 0x0A0A0A0A

#define BC_MAX_MSG_NUM 64

/*
* -----------------------------------------------------------|
* |BC_RT      | BC to RT Msg                                 |
* |----------------------------------------------------------|
* |RT_BC      | RT to BC Msg                                 |
* |----------------------------------------------------------|
* |RT_RT      | RT to single RT                              |
* |----------------------------------------------------------|
* |BCST       | Broadcast to every RT                        |
* |----------------------------------------------------------|
* |RT_RTS     | RT to every RTs                              |
* |----------------------------------------------------------|
* |MCND       | Mode code with no data                       |
* |----------------------------------------------------------|
* |TX_MCWD    | Mode code need RT transmit data to BC        |
* |----------------------------------------------------------|
* |RX_MCWD    | Mode code need RT receive data from BC       |
* |----------------------------------------------------------|
* |BCST_MCND  | Broadcast mode code with no data to every RT |
* |----------------------------------------------------------|
* |BCST_MCWD  | Broadcast mode code with data to every RT    |
* |----------------------------------------------------------|  
*/
typedef enum {BC_RT, RT_BC, RT_RT, BCST, RT_RTS, MCND, 
 TX_MCWD, RX_MCWD, BCST_MCND, BCST_MCWD} MESSAGETYPE;

typedef enum {CHANNEL_A, CHANNEL_B} BUSCHANNEL;

/*typedef enum {RX, TX} SUBADDRTYPE;*/
#define RX 0
#define TX 1

#pragma pack(4)
typedef struct CommandWord
{
    unsigned int Reserve : 16;
    unsigned int RTADDR  : 5;
    unsigned int TR      : 1;     
    unsigned int SADDR   : 5;  
    unsigned int WCOUNT  : 5;
} COMMANDWORD,*PCOMMANDWORD;


typedef struct BcMsgBuf
{
    unsigned int      MsgID;
    MESSAGETYPE MsgType;     // int
    BUSCHANNEL  Channel;     // int
    unsigned int      CtrlWord;
    COMMANDWORD Command1;    
    COMMANDWORD Command2;    
    unsigned int      Status1;     //ReadMsg
    unsigned int      Status2;     //ReadMsg
    unsigned short      Data[32];    
    unsigned int      Loopback;    //ReadMsg  
    unsigned int      MCData;      //ReadMsg
    unsigned int      BlkStatus;   //ReadMsg
    unsigned int      TimeTag;     //ReadMsg
    unsigned int      GapTime;    
}BCMSGBUF,*PBCMSGBUF;
    
typedef struct ConfigWord
{
    unsigned int Reserve : 16;
    unsigned int RX1     : 1;  
    unsigned int TX1     : 1;  
    unsigned int SA1     : 5;  
    unsigned int RX2     : 1;  
    unsigned int TX2     : 1;  
    unsigned int SA2     : 5;  
    unsigned int EOM     : 1;  
    unsigned int CIRC    : 1; 
} CONFIGWORD,*PCONFIGWORD;

#endif
