#ifndef DSPG_INTERFACE_H_
#define DSPG_INTERFACE_H_

#include "DSPg_config.h"
#include <stdio.h>


/*************************interface to dspg system******************************/
//the varible will be gotten by dspg systen, and using it init chip, enter and exit usecase 
typedef struct
{
    bool (*init)(const source_t fw);

    bool (*load_uc)(usecase_t ucase,source_t model);

    bool (*exit_uc)(void);
} handler_t;

extern handler_t DSPg_GetHander(void);


/*******The below functions are prepared for the dspg procedures***************/ 
extern void DSPg_Delay(uint16 ms);
extern void DSPg_Write(const uint8 *data,uint32 data_size);
extern void DSPg_WriteReg(uint32 reg,uint32 data,ins_t mode);
extern void DSPg_Read(uint8 *data,uint16 data_size);
extern void DSPg_ReadReg(uint32 reg,uint32 *data,ins_t mode);
extern void DSPg_Log(const char *format, ...);
extern void DSPg_ProcessConfigTable(const config_table_t *tbl,uint16 tbl_size);
extern void DSPg_SetIO(dspg_io_t io, bool high);
extern comm_inf_t DSPg_GetCommType(void);


/***********************shorten the name****************************/ 
#define WRITE(data,size) DSPg_Write((uint8 *)(data),(size))
#define READ(data,size) DSPg_Read((uint8 *)(data),(size))
#define WRITE_REG(reg,data,form) DSPg_WriteReg((reg),(data),(form))
#define READ_REG(reg,data,form) DSPg_ReadReg((uint32)(reg),(uint32 *)(data),(form))
#define DELAY(ms) DSPg_Delay(ms)

#ifdef ENABLE_DSPG_DEBUG
#define DBM_DEBUG(...) DSPg_Log(__VA_ARGS__)
#define DBM_DATA_DEBUG(data, data_size) 
#else
#define DBM_DEBUG(...)
#define DBM_DATA_DEBUG(data, data_size)
#endif


#endif /* DSPG_INTERFACE_H_ */
