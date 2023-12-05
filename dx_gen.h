/**
 * dx_gen.h  --  general (system wide) defines and externals.
 *
 * Copyright (C) 2016 DSP Group
 * All rights reserved.
 *
 * Unauthorized copying of this file is strictly prohibited.
 */

#ifndef _DX_GEN_H
#define _DX_GEN_H

#include <stdint.h>

typedef enum  {
    DBM_D2 = 2,
    DBM_D4,
    DBM_D5,
    DBM_D7,
    DBM_D8,
    DBM_D10
} ChipNames;

#define MODE_OPT 1		// FW optimized
#define MODE_RECORD 2
#define MODE_DEBUG 3

#define FW_NO_LOG 1
#define FW_AUDIO_LOG 2
#define FW_DEBUG_LOG 3
#define FW_DEBUG_LOG_HOST_ONLY 6	// init host log before FW is ready
#define FW_G_MODE_LOG 7				// activate log according to g_mode_flag

#define TRANSPORT_NOT_YET 0
#define TRANSPORT_SPI 1
#define TRANSPORT_I2C 2
#define TRANSPORT_UART 3

#define MASTER_CLOCK_32768 	32768
#define MASTER_CLOCK_19200 	19200
#define MASTER_CLOCK_24576 	24576
#define MASTER_CLOCK_2600 	2600
#define MASTER_CLOCK_13000 	13000
#define MASTER_CLOCK_12288 	12288
#define MASTER_CLOCK_26000 	26000

// for GPIO

#define ON 	1
#define OFF 0

#define OUT     1
#define IN      0

#define PULL_UP     1
#define PULL_DOWN   0

#define i2s_clks_chip_slave   0
#define i2s_clks_chip_master  1

#define LOW  0
#define HIGH 1

typedef enum  {
	MIC_NONE,
	MIC_DIGITAL,
    MIC_ANALOG,
//	MIC_VESPER		// obsolete
}mic_types;

typedef enum  {
	TDM0 = 0,
	TDM1,
	TDM2,
	TDM3,
}tdm_number;

typedef enum  {
	TRIGGER,
    COMMAND
}trigger_type;

typedef enum  {
	WWE_NONE = 0,
	WWE_SENSORY,
	WWE_AMAZON,
	WWE_GOOGLE,
	WWE_SAMSUNG,
	WWE_SENSING_SED,
	WWE_T3T4,
	WWE_DUAL,
	WWE_RETUNE,
	WWE_CYBERON
}wake_engine;

typedef enum  {
	TDM_RX,
    TDM_TX
}tdm_direction;

typedef enum  {
	TDM_SLAVE,
    TDM_MASTER
}tdm_mode;

#define NO_NSBF					0
#define NSBF_SINGLE_INSTANCE	1
#define NSBF_DUAL_INSTANCE		2
#define DEBUG
/* #define DEBUG 1 */
#ifdef DEBUG
	#define DEBUG_PRINT printf
#else
	#define DEBUG_PRINT(format, args...) ((void)0)
#endif

#endif /* _DX_GEN_H */
