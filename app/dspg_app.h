#ifndef DSPG_APP_H
#define DSPG_APP_H

#include "DSPg_driver/DSPg_config.h"

/************sink_dspg_config***************/
extern const uint8 fw_array[];
extern const uint8 model_array[];
extern const config_table_t fw_config[];
extern const config_table_t model_config[];

extern uint32 DspgApp_getSize(uint8 numb);

/************dspg app api***************/
#ifndef ADK6
extern bool DspgApp_Init(Task init_task)
#else
extern void DspgApp_Init(void);
#endif
extern void DspgApp_EnterVcMode(void);
extern void DspgApp_ExitVcMode(void);

#endif // DSPG_APP_H
