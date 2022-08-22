#ifndef EARBUD_DSPG_H
#define EARBUD_DSPG_H

#include "DSPg_driver/DSPg_config.h"
#include "audio_pcm_common.h"

/************sink_dspg_config***************/
extern const uint8 fw_array[];
extern const uint8 model_array[];
extern const config_table_t fw_config[];
extern const config_table_t model_config[];
extern const pcm_registry_per_user_t earbudDspg_Registry;

extern uint32 EarbudDspg_getSize(uint8 numb);

/************sink_dspg***************/
bool EarbudDspg_Init(Task init_task);
void EarbudDspg_EnterVcMode(void);
void EarbudDspg_ExitVcMode(void);
const pcm_config_t *earbudDspg_GetPcmInterfaceSettings(void);

#endif // EARBUD_DSPG_H
