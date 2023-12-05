#include "DSPg.h"
#include "DSPg_interface.h"

void DSPg_Delay(uint16 ms);
void DSPg_Write(const uint8 *data,uint32 data_size);
void DSPg_WriteReg(uint32 reg,uint32 data,ins_t mode);
void DSPg_Read(uint8 *data,uint16 data_size);
void DSPg_ReadReg(uint32 reg,uint32 *data,ins_t mode);
void DSPg_Log(uint8 n_args, const char *format,  ...);
void DSPg_SetIO(dspg_io_t io, bool high);
comm_inf_t DSPg_GetCommType(void);


handler_t dspg_handler;
interface_t *inf;
usecase_t current_mode = dspg_idle;

/*********************The below functions are APIs for application************************/
//Initialize the dspg system
bool DSPg_Init(interface_t *interfaces)
{
    //Get some handles of dspg system from chip procedure
    dspg_handler=DSPg_GetHander();
    //Store this interfaces for further application use
    inf = interfaces;
    //Initialize dspg chip to idle mode
    return dspg_handler.init(inf->fw);
}

//Enter usecase or exit 
bool DSPg_SetMode(usecase_t mode)
{
    bool ret = FALSE;
    if(mode == current_mode) return TRUE;

    //exit mode
    switch(current_mode)
    {
        case dspg_low_power:
        case dspg_voice_call:
        case dspg_voice_rcu:
            ret = dspg_handler.exit_uc(current_mode);
            break;

        case dspg_hibernate:
            ret = dspg_handler.exit_hibernate();
            break;

        default:
            break;
    }

    //enter mode
    switch (mode)
    {
        case dspg_low_power:
        case dspg_voice_call:
        case dspg_voice_rcu:
            ret = dspg_handler.load_uc(mode,inf->model,inf->asrp);
            break;

        case dspg_hibernate:
            ret = dspg_handler.enter_hibernate();
            break;

        default:
            break;
    }


    if(ret)
        current_mode = mode;
    return ret;
}

//This function will be invoked, when application detect a io interrupt
trigger_word_t DSPg_InterruptHandler(void)
{
    if(dspg_handler.int_handler !=NULL)
        return dspg_handler.int_handler();
    else
        return no_trigger;
}


/*********************The below functions are prepared for the dspg procedures************************/
//tranfer character to hex
static uint32 atoh__(const char *String)
{
    uint32 Value = 0, Digit;
	char c;

	while ((c = *String++) != '\0')
	{
		if (c >= '0' && c <= '9')
			Digit = (uint16) (c - '0');
		else if (c >= 'a' && c <= 'f')
			Digit = (uint16) (c - 'a') + 10;
		else if (c >= 'A' && c <= 'F')
			Digit = (uint16) (c - 'A') + 10;
		else
			break;

		Value = (Value << 4) + Digit;
	}
	return Value;
}

void DSPg_Log(uint8 n_args, const char *format, ...)
{
    va_list arg;
    va_start(arg, format);
    inf->Debug(format,n_args,arg);
    va_end(arg);
}

void DSPg_Delay(uint16 ms)
{
    inf->Delay(ms);
}

void DSPg_Write(const uint8 *data,uint32 data_size)
{
    inf->Write(data,data_size);
}

void DSPg_Read(uint8 *data,uint16 data_size)
{
    inf->Read(data,data_size);
}

void DSPg_SetIO(dspg_io_t io, bool high)
{
    inf->Set_IO(io,high);
}

comm_inf_t DSPg_GetCommType(void)
{
    return inf->comm;
}


#ifdef ENABLE_DSPG_REG_DEBUG
#define DBM_REG_DEBUG(...) DSPg_Log(VA_NARGS(__VA_ARGS__),__VA_ARGS__)
#endif
/**
 *  \brief Tranfer write instruction to string base on mode.
 *  \param reg The register will be wriiten to
 *  \param data The data will be written.
 *  \param mode indicate how many words of reg and data in using.
 * */
void DSPg_WriteReg(uint32 reg,uint32 data,ins_t mode)
{
    char str[16];
    uint16 len = 0;

    switch (mode)
    {
    case r16d16:
        len = sprintf(str, "%03xw%04x", (uint16)reg, (uint16)(data)&0xffff);
        break;
    case r16d32:
        len = sprintf(str, "%03xW%08lx", (uint16)reg, (data)&0xffffffffUL);
        break;
    case r32d32:
        len = sprintf(str, "%03xW%08lx", 0x5,   (reg)&0xffffffffUL);
        str[len] = 0;
        inf->Write((uint8 *)str, strlen(str));
        DELAY(5);
        len = sprintf(str, "%03xW%08lx", 0x7,   (data)&0xffffffffUL);
        break;
    
    default:
        break;
    }
    
    str[len] = 0;

    inf->Write((uint8 *)str, strlen(str));
    DELAY(5);

    DBM_REG_DEBUG("dspg 0x%x write 0x%x",reg,data);
}

/**
 *  \brief Tranfer read instruction to string base on mode.
 *  \param reg The register will be read from.
 *  \param data The pointer to save the data what we get.
 *  \param mode indicate how many words of reg and data in using.
 * */
void DSPg_ReadReg(uint32 reg,uint32 *data,ins_t mode) 
{
    char str[10];
    uint16 len;
    uint8 start=0;

    memset(str,0,10);
    switch (mode)
    {
    case r16d16:
        len = sprintf(str, "%03xr", (uint16)reg&0xffff);
        str[len] = 0;
        inf->Write((uint8 *)str, strlen(str));
        DELAY(5);
        DBM_REG_DEBUG("dspg read 0x%x",reg);
        inf->Read((uint8 *)str, 5);
        str[6]='\0'; 
        break;
    case r16d32:
        len = sprintf(str, "%03xR", (uint16)reg&0xffff);
        str[len] = 0;
        inf->Write((uint8 *)str, strlen(str));
        DELAY(5);
        DBM_REG_DEBUG("dspg read 0x%x",reg);
        inf->Read((uint8 *)str, 9);
        str[9]='\0';
        break;
    case r32d32:
        len = sprintf(str, "%03xW%08lx", 0x5,   (reg)&0xffffffffUL);
        str[len] = 0;
        inf->Write((uint8 *)str, strlen(str));
        DELAY(5);
        DBM_REG_DEBUG("dspg read 0x%x ",reg);

        len = sprintf(str, "%03xR", 0x07);
        str[len] = 0;
        inf->Write((uint8 *)str, strlen(str));
        DELAY(5);
        inf->Read((uint8 *)str, 9);
        str[9]='\0';
        break;
    
    default:
        break;
    }

    if(inf->comm == SPI)
        start = 1;

    *data = atoh__(&str[start]);

    DBM_REG_DEBUG("dspg read return %d(0x%x)",*data,*data);
}

