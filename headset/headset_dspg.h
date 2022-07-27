#ifndef HEADSET_DSPG_H
#define HEADSET_DSPG_H

#include "DSPg_driver/DSPg_config.h"

/************sink_dspg_config***************/
extern const uint8 fw_array[];
extern const uint8 model_array[];
extern const config_table_t fw_config[];
extern const config_table_t model_config[];

extern uint32 HeadsetDspg_getSize(uint8 numb);

/************sink_dspg***************/
bool HeadsetDspg_Init(Task init_task);
void HeadsetDspg_EnterVcMode(void);
void HeadsetDspg_ExitVcMode(void);

#endif // HEADSET_DSPG_H
