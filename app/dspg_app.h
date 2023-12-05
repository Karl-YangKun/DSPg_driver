#ifndef DSPG_APP_H
#define DSPG_APP_H

#include "DSPg_driver/DSPg_config.h"

/************sink_dspg_config***************/
extern uint32 DspgApp_getFW(const uint8 **p);

extern uint32 DspgApp_getAsrp(const uint8 **p);

extern uint32 DspgApp_getModel(const uint8 **p);


/************dspg app api***************/
#ifndef ADK6
extern bool DspgApp_Init(Task init_task);
#include <audio_pcm_common.h>
extern const pcm_registry_per_user_t dspgApp_Registry;
#else
trigger_word_t DSPgApp_InterruptHandler(void);
extern void DspgApp_Init(void);
#endif
extern void DspgApp_EnterVcMode(void);
extern void DspgApp_ExitVcMode(void);
void DspgApp_EnterRcuMode(void);
void DspgApp_ExitRcuMode(void);
void DspgApp_EnterLpMode(void);
void DspgApp_ExitLpMode(void);
#endif // DSPG_APP_H
