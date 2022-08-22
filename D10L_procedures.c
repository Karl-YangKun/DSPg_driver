#include "DSPg_interface.h"


static bool D10L_Init(const source_t fw);
static bool D10L_EnterUseCase(usecase_t ucase,source_t model);
static bool D10L_ExitUseCase(void);
handler_t DSPg_GetHander(void);

const config_table_t model_pre_config[]=
{
    0,0x018,0x000a,5,
    0,0x018,0x000a,5,
    1,0x01d,0x0600,5,
    1,0x010,0x0d027000UL,100,
    00,0x00f,0x40c4,5
};

const config_table_t recording_config[]=
{
    0,0x128,0x011,5,
    0,0x129,0x0001,5,
    0,0x005,0x0000,5,
    0,0x006,0x0000,5,
    0,0x007,0x0000,5,
    0,0x030,0x800e,5, 
    0,0x030,0x0001,5
};

const config_table_t exit_usecase_config[]=
{
    0,0x031,0x0001,10,
    0,0x031,0x0001,10,
    0,0x037,0x0000,5,
    0,0x036,0x0000,5,
    0,0x031,0x0000,5,
    0,0x037,0x0000,5,
    0,0x034,0x0000,5,
    0,0x023,0x0000,5,
    0,0x025,0x0000,5,
    0,0x024,0x0000,5,
    0,0x304,0x0000,5
};

/*************************interface for dspg system******************************/
//the varible will be gotten by dspg systen, and using it init chip, enter and exit usecase 
handler_t  handler=
                {
                    .init = D10L_Init,
                    .load_uc = D10L_EnterUseCase,
                    .exit_uc  = D10L_ExitUseCase,
                };

handler_t DSPg_GetHander(void)
{
    return handler;
}

/*************************chip procedures******************************/
//when using uart to communication, synchronize the baud rate
static bool D10L_SyncBaudRate(void)
{
    char send_buf[16];
    DBM_DEBUG("--D10L_SyncBaudRate, sync baud rate");
    memset(send_buf,0,sizeof(send_buf));
    WRITE(send_buf, 16);
    uint8 data[4];
    char ok[4]={0x4f,0x4b,0x0a,0x0d};
    //read 5 times
    for(uint8 i=0; i<5; i++)
    {
        DELAY(10);
        READ(data,4);
        if (strncmp((char*)data,ok, 4) == 0)
            return TRUE;
    }
    return FALSE;
}

//read register to check if it have fw error
static bool D10L_CheckError(void)
{
    uint16 data;
    WRITE_REG(0xd,0,r16d16);
    READ_REG(0x5,&data,r16d16);
    DBM_DEBUG("--Read error status %d",data);

    return TRUE;
}

//send boot commands before loading fw
static void D10L_PreBootPowerUp(void)
{
    char send_buf_2[] = {0};
    DBM_DEBUG("--D10L_PreBootPowerUp, send boot commands");
    WRITE(send_buf_2, sizeof(send_buf_2));
    DELAY(20);
    char send_buf[] = {0x5A, /*HEADER_BYTE*/
                        0x02, /*OP_CODE*/
                        0x04, 0x00, 0x00, 0x00, /*NUM_OF_WORDS*/
                        0x02, 0x02, 0x00, 0x00,/*ADDR*/
                        0xCD, 0xAB, /*MAGIC_NUM*/
                        0x03, 0x00, /*VALID_MASK*/
                        0x01, 0x00,/*EN_EVENTS #value of register 0x12 - only for power up event*/
                        0x8E, 0x00 /*GPIO_CFG*/};
    WRITE(send_buf, sizeof(send_buf));

    send_buf[0] = 0x5A;
    send_buf[1] = 0x0F;
    WRITE(send_buf, 2);
}

static bool D10L_LoadFw(const source_t fw)
{
    DBM_DEBUG("--D10L_LoadFw, load fw data to dspg chip");
    D10L_PreBootPowerUp();
    DELAY(10);  

    uint8 check[4];
    WRITE( fw.data, fw.data_size-4);
    memcpy(check,&fw.data[fw.data_size-4],4);

    char write_buf[] = {0x5A, 0x0E};
    uint8 c[7]; 

    DBM_DEBUG("--Read checksum");
    WRITE( write_buf, 2);

    READ(c, 7);

    uint8 start = 2;
    if(DSPg_GetCommType()==SPI)
    {
        start = 3;
    }

    if (strncmp((char*)&c[start],(char*)check, 4) == 0)
    {
        DBM_DEBUG("--Checksum is right");
        char buf_run[2] = {0x5A,0x0B};
	    DBM_DEBUG("--Send a command to exit loading. System will be run after that");
        WRITE(buf_run,2);
        DELAY(10);

        uint32 vers[3];
        DBM_DEBUG("--Read version");
        READ_REG(0x0,&vers[0],r16d16);
        READ_REG(0x5,&vers[1],r16d16);
        READ_REG(0x6,&vers[2],r16d16);
        DBM_DEBUG("--The version: %x.%x.%x", vers[0],vers[1],vers[2]);

        uint8 id[6]={0};
        char RegSet[5] = {0};

        uint8 len=sprintf(RegSet, "%03xr", 0x19);
        RegSet[len]='\0';
        DBM_DEBUG("--Read chip id");
        WRITE(RegSet,len);
        READ(id,5);
        DBM_DEBUG("--ID:%s",(char*)&id[1]);
        return TRUE;
    }
    else
    {
        DBM_DEBUG ("--Checksum error!!! got: %2x %2x %2x %2x expected: %2x %2x %2x %2x ",
                        c[start],c[start+1],c[start+2],c[start+3],check[0],check[1],check[2],check[3]);
        return FALSE;
    }
}

static void D10L_DefaultConfig(const source_t fw)
{
    DBM_DEBUG("--Set default config");
    DSPg_ProcessConfigTable(fw.config,fw.config_size);
}


static bool D10L_Init(const source_t fw)
{
    DBM_DEBUG("--D10L_Init, load fw data to dspg chip");

    DSPg_SetIO(reset_io,FALSE);
    DELAY(TIME_IN_RESET);
    DSPg_SetIO(reset_io,TRUE);
    DELAY(TIME_AFTER_RESET);

    if(DSPg_GetCommType() == UART && !D10L_SyncBaudRate())
    {
        DBM_DEBUG("Uart sync fail!");
        return FALSE;
    }

    if(!D10L_LoadFw(fw))
    {
        DBM_DEBUG("Loading fw fail!");
        return FALSE;
    } 

    D10L_DefaultConfig(fw);
    
    return TRUE;
}


static bool D10L_EnterUseCase(usecase_t ucase,source_t model)
{
    const uint8* ptr = model.data;
    char   header_buffer[10];
    uint32 num_of_words = 0;
    uint32 cur_ptr = 0;

    DBM_DEBUG("--D10L_EnterUseCase,enter enum:usecase_t:%d ",ucase);

    DSPg_ProcessConfigTable(model_pre_config,sizeof(model_pre_config)/sizeof(config_table_t));

    do
    {
        memcpy(header_buffer, ptr, 10);
        ptr += 10;
        cur_ptr += 10;
        if(!(header_buffer[0] == 0x5A && (header_buffer[1] == 0x01 || header_buffer[1] == 0x02)))
        {
            DBM_DEBUG("Load model file fail in %s, because of header error",__func__);
            return FALSE;
        }
        WRITE(header_buffer,10);
        DELAY(10);
        num_of_words =(uint8)header_buffer[5] * 0x1000000UL + (uint8)header_buffer[4] * 0x10000UL +(uint8)header_buffer[3] * 0x100UL + (uint8)header_buffer[2];
        WRITE(ptr,num_of_words*2);
        DELAY(10);
        ptr += num_of_words*2;
        cur_ptr += num_of_words*2;

        if(cur_ptr+2 == model.data_size)
            cur_ptr +=2;//we send 0x5a 0x0b
    } while (cur_ptr < model.data_size);
    
    char buf_run[2] = {0x5A,0x0B};
    DBM_DEBUG("--Send a command to exit loading. System will be run after that");
    WRITE(buf_run,2);
    DELAY(20);
    D10L_CheckError();

    if(ucase == dspg_voice_call)
    {
        DSPg_ProcessConfigTable(model.config,model.config_size);
#ifdef ENABLE_RECONDIG
        DSPg_ProcessConfigTable(recording_config,sizeof(recording_config)/sizeof(config_table_t));
#endif
    }

    return TRUE;
}

static bool D10L_ExitUseCase(void)
{
    uint16 size = sizeof(exit_usecase_config)/sizeof(config_table_t);
    DBM_DEBUG("--D10L_ExitUseCase,enter idle mode");
    DSPg_ProcessConfigTable(exit_usecase_config,size);
    return TRUE;
}
