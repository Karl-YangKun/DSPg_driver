#ifndef D10L_UTILS_H
#define D10L_UTILS_H

#include "DSPg_interface.h"
#include "dx_params.h"
#include "dx_params_flavor.h"

#define MODEL_OR_ASRP_HEADER_LENGTH 10
#define WRITE_DELAY     10   // Due to no ready and common event_ready options // sometimes no ready is available
// defines for configure_clocks function:
#define SWITCH_TO_TDM 		1
#define NO_SWITCH_TO_TDM 	0
#define SWITCH_TO_MCLK 		1
#define NO_SWITCH_TO_MCLK 	0
#define NO_WANTED_PLL	 	0
#define NO_WANTED_AHB_CLK 	0
#define NO_WANTED_APB_CLK 	0
#define NO_USE_PLL_POST_DIV	0
#define USE_PLL_POST_DIV	1
#define DEFAULT_DMIC_FREQ	1
#define DEFAULT_AMIC_FREQ	1

//clock config struct
typedef struct
{
    uint16 input_clk;
    uint32 wanted_tl3_clk;
    uint32 wanted_ahb_clk;
    uint32 wanted_apb_clk;
    uint32 wanted_pll;
    uint32 digital_mic_freq;
    uint32 analog_mic_freq;
    unsigned switch_to_tdm_clk:1;
    unsigned switch_to_mclk:1;
    unsigned use_pll_post_div:1;
    unsigned unused_bit:13;
} dspg_clock_config_t;

bool D10L_Sync(void);
bool D10L_CheckError(void);
void D10L_PrebootRequest(void);
void D10L_ReadID(void);
void D10L_ReadVersion(void);
bool D10L_VerifyCheckSum(uint8 *expect);
void D10L_RunChip(void);
trigger_word_t D10L_VoiceTrigger(uint32 interrupt_events);
void D10L_AfterPowerUp(void);
void D10L_ModelLoading(const source_t file, wake_engine wwe);
bool D10L_UsecaseRcuNrEnter(const source_t file,wake_engine wwe);
bool D10L_UsecaseBargeInEnter(const source_t file,wake_engine wwe);
bool D10L_UsecaseLpEnter(wake_engine wwe);
void D10L_UsecaseNrExit(bool keep_asrp_and_model_loaded);
void D10L_UsecaseLpExit(void);
void D10L_UsecaseBrageInExit(void);
bool D10L_LoadFile(const source_t fw,uint8 skip_size);

#endif // D10L_UTILS_H
