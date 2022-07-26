#ifndef SINK_DSPG_H
#define SINK_DSPG_H

#include "DSPg_driver/DSPg_config.h"

/************sink_dspg_config***************/
extern const uint8 fw_array[];
extern const uint8 model_array[];
extern const config_table_t fw_config[];
extern const config_table_t model_config[];

extern uint32 SinkDspgConfig_getSize(uint8 numb);

/************sink_dspg***************/
void sinkDspgInit(void);
void sinkDspgEnterVcMode(void);
void sinkDspgExitVcMode(void);

#endif // SINK_DSPG_H
