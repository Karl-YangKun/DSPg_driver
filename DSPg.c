#include "DSPg.h"
#include "DSPg_interface.h"

void DSPg_Delay(uint16 ms);
void DSPg_Write(const uint8 *data,uint32 data_size);
void DSPg_WriteReg(uint32 reg,uint32 data,ins_t mode);
void DSPg_Read(uint8 *data,uint16 data_size);
void DSPg_ReadReg(uint32 reg,uint32 *data,ins_t mode);
void DSPg_Log(uint8 n_args, const char *format,  ...);
void DSPg_SetIO(dspg_io_t io, bool high);
void DSPg_ProcessConfigTable(const config_table_t *tbl,uint16 tbl_size);
comm_inf_t DSPg_GetCommType(void);


handler_t dspg_handler;
interface_t inf;

/*********************The below functions are APIs for application************************/
//Initialize the dspg system
bool DSPg_Init(interface_t interfaces)
{
    dspg_handler=DSPg_GetHander();
    inf = interfaces;

    return dspg_handler.init(inf.fw);
}

//Enter usecase or exit 
bool DSPg_SetMode(usecase_t mode)
{
    if(mode != dspg_idle)
        return dspg_handler.load_uc(mode,inf.use_case[mode]);
    else
        return dspg_handler.exit_uc();
}




/*********************The below functions are prepared for the dspg procedures************************/
//tranfer character to hex
static uint16 atoh__(const char *String)
{
	uint16 Value = 0, Digit;
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
    inf.Debug(format,n_args,arg);
    va_end(arg);
}

void DSPg_Delay(uint16 ms)
{
    inf.Delay(ms);
}

void DSPg_Write(const uint8 *data,uint32 data_size)
{
    inf.Write(data,data_size);
}

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
        len = sprintf(str, "%03xw%04x", reg, (data)&0xffff);
        break;
    case r16d32:
        len = sprintf(str, "%03xW%08lx", reg, (data)&0xffffffffUL);
        break;
    case r32d32:
        len = sprintf(str, "%03xW%08lx", 0x5,   (reg)&0xffffffffUL);
        str[len] = 0;
        inf.Write((uint8 *)str, strlen(str));
        DELAY(5);
        len = sprintf(str, "%03xW%08lx", 0x7,   (data)&0xffffffffUL);
        break;
    
    default:
        break;
    }
    
    str[len] = 0;

    inf.Write((uint8 *)str, strlen(str));
    DELAY(5);
}

void DSPg_Read(uint8 *data,uint16 data_size)
{
    inf.Read(data,data_size);
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
        len = sprintf(str, "%03xr", reg&0xffff);
        str[len] = 0;
        inf.Write((uint8 *)str, strlen(str));
        DELAY(5);
        inf.Read((uint8 *)str, 5);
        str[6]='\0'; 
        break;
    case r16d32:
        break;
    case r32d32:
        break;
    
    default:
        break;
    }

    if(inf.comm == SPI)
        start = 1;

    *data = atoh__(&str[start]);
}

void DSPg_SetIO(dspg_io_t io, bool high)
{
    inf.Set_IO(io,high);
}

comm_inf_t DSPg_GetCommType(void)
{
    return inf.comm;
}

//wirite the config table to dspg chip
void DSPg_ProcessConfigTable(const config_table_t *tbl,uint16 tbl_size)
{
    config_table_t temp;
    for(uint16 i=0; i<tbl_size; i++)
    {
        temp = tbl[i];

        DSPg_WriteReg(temp.reg,temp.data,temp.form);
        DELAY(temp.delay);
    }
}
