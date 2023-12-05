/**
 * dx_param_flavor.h  --  product parameters  dependant on init type / flavor.
 *
 * Copyright (C) 2016 DSP Group
 * All rights reserved.
 *
 * Unauthorized copying of this file is strictly prohibited.
 */

#ifndef _DX_PARAMS_FLAVOR_H
#define _DX_PARAMS_FLAVOR_H

#include "dx_params.h"
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
//			product parameters dependant on init type / flavor.
//
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

#ifdef INIT_TYPE_MELON_0	// Original Melon (Amazon + OKG)

// HW: triangle or hexagram or internal mic. QFN D10 board.

#define INIT_SEQUENCE		0

#define NNL_FW				0			// 0 - non NNL FW // 1 - NNL FW
#define FIRMWARE_IMAGE 		"melon/fw/melon_fw/Melon_D10_ver_4660_RC1_AmazonVT_290_OKG_210124_ASRP3_3973.bin"

//#define AMAZON_MODEL  		"melon/Amodels/Amazon/m_WS_50k.en-US.alexa.bin" //unpacked
#define AMAZON_MODEL  		"melon/Amodels/Amazon/p_WS_50k.en-US.alexa.bin"		//packed
#define GOOGLE_MODEL  		"melon/Amodels/Google/en_all.mmap"
#define SED_MODEL  			""
#define CYBERON_MODEL		""
#define CYBERON_MODEL_NNL	""
#define RETUNE_MODEL		""

#define WAKE_ENGINE			7	//{"0":"no VT", "1":"Sensory WWE", "2":"Amazon_WWE", "3":"Google WWE",
								// "4":"Samsung WWE", "5":"Sensing SED", "6":"T3T4", "7":"dual", etc.}
// Dual case:
#define WAKE_ENGINE1		9
#define WAKE_ENGINE1_MODEL	AMAZON_MODEL
#define WAKE_ENGINE2		3		// Must be set to valid number in case WAKE_ENGINE = DUAL
#define WAKE_ENGINE2_MODEL	GOOGLE_MODEL

#define MODEL1_LOCATION		MEM_TYPE_AHB
#define MODEL2_LOCATION		MEM_TYPE_AHB
#define ASRP_LOCATION		MEM_TYPE_DTCM

#define AUDIO_BUFFER_SIZE	AUDIO_BUFFER_HALF_SEC

#define	TDK_ENABLE			0		// 0 (disable) // 1 (enable)
#define VESPER_VAD			0
#define LP_HW_VAD			0
#define HW_VAD				0
#define ENVIRO_ADAPTATION	0
#define HW_VAD_ENABLE_PRE_ROLL_EXTENTION 	0
#define ENABLE_HW_VAD_BUFFER_RECORDING 		0
#define AMAZON_SW_VAD_ENABLE			0
#define AMAZON_KWD_METADATA_ENABLE		0

//----------------

//		MIC configuration

#define NO_OF_MICS			1	// in all use cases
#define MIC_TYPE			2	// 1 = digital // 2 = analog

#define EXTERNAL_MICS		0 		// 0  no external mics // 1  external mics
#define USE_HEXAGRAM_MICS	0
#define SOD_HDMI_MIC_A		2 	// 0 - 6 SOD board mic number	// explanation in bsp.c
#define SOD_HDMI_MIC_B		3 	// 0 - 6 SOD board mic number

#define MIC_GAINS			10
#define SUPPORT_NSBF		NO_NSBF

#define POST_PLL_DIVIDER_ENABLE 			1		// in optimized

#ifdef E_Q_D
#define EQD_ENABLE			1
#else
#define EQD_ENABLE			0
#endif

//---------

// comment if not required
//#define FLASH_SERVICE_

#ifdef FLASH_SERVICE_
#define FLASH_ENABLE		1
#define FLASH_HEADER_VP 	"melon/fw/vp/flash_header_2MB_flash.bin"

#else
#define FLASH_ENABLE		0
#define FLASH_HEADER_VP 	""
#endif

#define VESPER_MIC			0	// 1 = mic 1 is Vesper // 0 = non vesper

#endif

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

#ifdef INIT_TYPE_NNL_MELON_1		// Amazon NNL

// HW: triangle or hexagram or internal mic. QFN D10 board.

#define INIT_SEQUENCE		1

#define NNL_FW				1			// 0 - non NNL FW // 1 - NNL FW
#define FIRMWARE_IMAGE 		"melon/fw/melon_fw/Melon_D10_ver_4660_RC1_AmazonVT_1100_ASRP3_3973.bin"

#define AMAZON_MODEL	  	"melon/Amodels/NNL/amazon_model.bin"
#define GOOGLE_MODEL  		""
#define SED_MODEL  			""
#define CYBERON_MODEL		""
#define CYBERON_MODEL_NNL	""
#define RETUNE_MODEL		""

#define WAKE_ENGINE			2		// 2 = amazon 
// Dual case:
#define WAKE_ENGINE1		2
#define WAKE_ENGINE1_MODEL	AMAZON_MODEL
#define WAKE_ENGINE2		0		// Must be set to valid number in case WAKE_ENGINE = DUAL
#define WAKE_ENGINE2_MODEL	NO_MODEL

#define MODEL1_LOCATION		MEM_TYPE_AHB
#define MODEL2_LOCATION		0					// Not operative
#define ASRP_LOCATION		MEM_TYPE_AHB

#define AUDIO_BUFFER_SIZE	AUDIO_BUFFER_3_SEC
	
#define	TDK_ENABLE			0		// 0 (disable) // 1 (enable)
#define VESPER_VAD			0
#define LP_HW_VAD			0
#define HW_VAD				0
#define ENVIRO_ADAPTATION	0
#define HW_VAD_ENABLE_PRE_ROLL_EXTENTION 	0
#define ENABLE_HW_VAD_BUFFER_RECORDING 		0
#define AMAZON_SW_VAD_ENABLE			0
#define AMAZON_KWD_METADATA_ENABLE		0

//----------------

//		MIC configuration

#define NO_OF_MICS			2	// in all use cases
#define MIC_TYPE			1	// 1 = digital // 2 = analog

#define EXTERNAL_MICS		1 		// 0  no external mics // 1  external mics
#define USE_HEXAGRAM_MICS	0
#define SOD_HDMI_MIC_A		1 	// 0 - 6 SOD board mic number	// explanation in bsp.c
#define SOD_HDMI_MIC_B		2 	// 0 - 6 SOD board mic number

#define MIC_GAINS			13	

#define SUPPORT_NSBF		NO_NSBF
#define POST_PLL_DIVIDER_ENABLE 			1		// in optimized


#endif

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

#ifdef INIT_TYPE_RCU_MELON_2	// RCU 

#define INIT_SEQUENCE		2
#define NNL_FW				0			// 0 - non NNL FW // 1 - NNL FW

//--------------------------------------
//		Select Engine

// select a single Engine of RCU:
//#define RCU_AMAZON
#define RCU_CYBERON
//#define RCU_RETUNE
//#define RCU_GOOGLE

// comment if not required  (if comment: OK Synaptics and FindMy remote)
//#define CYBERON_OK_GOOGLE
//---------------------------------------

//	 		Select Mic configuration:

#define NO_OF_MICS			1	// in all use cases

// Mic type - Select a single option
#define ANALOG_INTERNAL_MICS
//#define DIGITAL_EXTERNAL_MICS_TRIANGLE
//#define DIGITAL_EXTERNAL_MICS_HEXAGRAM
//#define DIGITAL_VESPER_HEXAGRAM
//#define DIGITAL_VESPER_COUPON

//#define QFN48__   // enable for Vesper Coupon QFN48 only

//---------------------------------------
// VAD is defined below according to MIC type 
//---------------------------------------

//	 		Select Feature configuration:

// Select if E_Q_D is required -- 
//#define E_Q_D

#ifdef E_Q_D
#define EQD_ENABLE			1
#else
#define EQD_ENABLE			0
#endif

//---------

// comment if not required
//#define FLASH_SERVICE_

#ifdef FLASH_SERVICE_
#define FLASH_ENABLE		1
#define FLASH_HEADER_VP 	"melon/fw/vp/flash_header_2MB_flash.bin"

#else
#define FLASH_ENABLE		0
#define FLASH_HEADER_VP 	""
#endif

// uncomment only if 2MB_FLASH
//#define TWO_MB_FLASH

//---------

// Select if TDK or not
//#define TDK_

//--------------------------

#ifdef RCU_AMAZON
  #define ASRP_4020
  #ifdef TDK_
	#define	TDK_ENABLE		1	// 0 (disable) // 1 (enable)
	// TDK FW Version:
	#define FIRMWARE_IMAGE 		"melon/fw/tdk/Melon_RCU_D10_ver_46213_RC3_AmazonVT_290_ASRP3_3934_TDK_IH.bin"
	#define SUPPORT_NSBF 	NSBF_DUAL_INSTANCE
  #else
	#define	TDK_ENABLE		0	// 0 (disable) // 1 (enable)
	// Non TDK FW Version:

	#ifdef VAD_LP_HW_
		#define FIRMWARE_IMAGE 	"melon/fw/Melon_RCU_D10_ver_46213_D005_AmazonVT_290_ASRP3_3934.bin"
		#define SUPPORT_NSBF 	NSBF_DUAL_INSTANCE
	#else 
		#define FIRMWARE_IMAGE 	"melon/fw/RCU_D10_ver_4670_RC4_Amazon_290_ASRP_4023_HWV_Vesp_FLH_IH.bin"

		#define SUPPORT_NSBF 	NSBF_SINGLE_INSTANCE
  	#endif

  #endif

	#define WAKE_ENGINE			2	//{"0":"no VT", "1":"Sensory WWE", "2":"Amazon_WWE", "3":"Google WWE",
								// "4":"Samsung WWE", "5":"Sensing SED", "6":"T3T4", "7":"dual, "8": "Retune WWE"}
	#define WAKE_ENGINE1		2		// Must fit FW compilation 1st engine
	#define WAKE_ENGINE1_MODEL	AMAZON_MODEL

	#define MODEL1_LOCATION		MEM_TYPE_AHB
	#define MODEL2_LOCATION		0						// Not Operative
	#define ASRP_LOCATION		MEM_TYPE_DTCM

	#define AUDIO_BUFFER_SIZE	AUDIO_BUFFER_3_SEC

#endif

    //--------------------------

#ifdef RCU_GOOGLE
  #define ASRP_4020
  #ifdef TDK_
	#define	TDK_ENABLE		1	// 0 (disable) // 1 (enable)
	// TDK Version:
	#define FIRMWARE_IMAGE 		"melon/fw/tdk/Melon_RCU_D10_ver_46213_RC2_OKG_210124_ASRP3_3934_TDK_IH.bin"
	#define SUPPORT_NSBF 	NSBF_DUAL_INSTANCE
  #else
	#define	TDK_ENABLE		0	// 0 (disable) // 1 (enable)
	// Non TDK Version:
 	#define FIRMWARE_IMAGE 	"melon/fw/RCU_D10_ver_4670_RC4_Google_210124_ASRP_4023_HWV_Vesp_FLH_IH.bin"
 #endif

	#define WAKE_ENGINE			3	//{"0":"no VT", "1":"Sensory WWE", "2":"Amazon_WWE", "3":"Google WWE",

// "4":"Samsung WWE", "5":"Sensing SED", "6":"T3T4", "7":"dual, "8": "Retune WWE"}
	#define WAKE_ENGINE1		3		// Must fit FW compilation 1st engine
	#define WAKE_ENGINE1_MODEL	GOOGLE_MODEL

	#define MODEL1_LOCATION		MEM_TYPE_AHB
	#define MODEL2_LOCATION		0						// Not Operative
	#define ASRP_LOCATION		MEM_TYPE_DTCM

	#define SUPPORT_NSBF		NSBF_SINGLE_INSTANCE

	#define AUDIO_BUFFER_SIZE	AUDIO_BUFFER_3_SEC
#endif

#ifdef RCU_CYBERON
    #define ASRP_4020
	#define	TDK_ENABLE		0	// 0 (disable) // 1 (enable)

	#define FIRMWARE_IMAGE 		"melon/fw/RCU_D10_ver_4670_RC4_Cyberon_2214_ASRP_4023_HWV_Vesp_FLH_IH.bin"
	// ----------------------
	#define WAKE_ENGINE			9	//{"0":"no VT", "1":"Sensory WWE", "2":"Amazon_WWE", "3":"Google WWE",
	#define WAKE_ENGINE1		9		// Must fit FW compilation 1st engine
	#define WAKE_ENGINE1_MODEL	CYBERON_MODEL

	#define MODEL1_LOCATION		MEM_TYPE_AHB
	#define MODEL2_LOCATION		0						// Not Operative
	#define ASRP_LOCATION		MEM_TYPE_DTCM

	#define SUPPORT_NSBF 		NSBF_SINGLE_INSTANCE
	#ifdef CYBERON_OK_GOOGLE
	#define AUDIO_BUFFER_SIZE	AUDIO_BUFFER_3_SEC
	#else
		#define AUDIO_BUFFER_SIZE	AUDIO_BUFFER_1_SEC
	#endif

#endif

#ifdef RCU_RETUNE
	#define ASRP_4020
	#define	TDK_ENABLE		0	// 0 (disable) // 1 (enable)
	#ifdef VAD_LP_HW_
 		#define FIRMWARE_IMAGE 	"melon/fw/Melon_RCU_D10_ver_46216_RC31_RetuneVT_20210217_ASRP3_4021_HVAD_IH.bin"
		#define SUPPORT_NSBF	NSBF_SINGLE_INSTANCE
	#else	// HW_VAD:
		#define FIRMWARE_IMAGE 	"melon/fw/RCU_D10_ver_4670_RC4_Retune_20210217_ASRP_4023_HWV_Vesp_FLH_IH.bin"
	
	#define SUPPORT_NSBF 	NSBF_SINGLE_INSTANCE
	#endif
	
	#define WAKE_ENGINE			8	//{"0":"no VT", "1":"Sensory WWE", "2":"Amazon_WWE", "3":"Google WWE",
	#define WAKE_ENGINE1		8		// Must fit FW compilation 1st engine
	#define WAKE_ENGINE1_MODEL	RETUNE_MODEL

	#define MODEL1_LOCATION		MEM_TYPE_DTCM		// MEM_TYPE_AHB
	#define MODEL2_LOCATION		0					// Not Operative
	#define ASRP_LOCATION		MEM_TYPE_DTCM

	#define AUDIO_BUFFER_SIZE	AUDIO_BUFFER_2_5_SEC //AUDIO_BUFFER_2_SEC

#endif

    //--------------------------

// Dual case:						// Must fit FW compilation 2nd engine
#define WAKE_ENGINE2		0		// Must be set to valid number in case WAKE_ENGINE = DUAL
#define WAKE_ENGINE2_MODEL	NO_MODEL


//	 		Mic configuration - cont.

#ifdef ANALOG_INTERNAL_MICS
#define MIC_TYPE			2	// 1 = digital // 2 = analog
#define EXTERNAL_MICS		0 	// 0  no external mics // 1  external mics
#define USE_HEXAGRAM_MICS	0	// set to 0 when old 1.8 vesper mic is used.
#define VESPER_MIC			0	// 1 = mic 1 is Vesper // 0 = non vesper

#define VAD_HW_ 
#endif

#ifdef DIGITAL_EXTERNAL_MICS_TRIANGLE
#define MIC_TYPE			2	// 1 = digital // 2 = analog
#define EXTERNAL_MICS		0 	// 0  no external mics // 1  external mics
#define USE_HEXAGRAM_MICS	0	// set to 0 when old 1.8 vesper mic is used.
#define VESPER_MIC			0	// 1 = mic 1 is Vesper // 0 = non vesper

#define VAD_HW_ 
#endif

#ifdef DIGITAL_EXTERNAL_MICS_HEXAGRAM
#define MIC_TYPE			1	// 1 = digital // 2 = analog
#define EXTERNAL_MICS		1 	// 0  no external mics // 1  external mics
#define USE_HEXAGRAM_MICS	1	// set to 0 when old 1.8 vesper mic is used.
#define VESPER_MIC			0	// 1 = mic 1 is Vesper // 0 = non vesper

#define VAD_HW_ 
#endif

#ifdef DIGITAL_VESPER_COUPON
#define MIC_TYPE			1	// 1 = digital // 2 = analog
#define EXTERNAL_MICS		0 	// 0  no external mics // 1  external mics
#define USE_HEXAGRAM_MICS	0	// set to 0 when old 1.8 vesper mic is used.
#define VESPER_MIC			1	// 1 = mic 1 is Vesper // 0 = non vesper
#define VAD_VESPER_ 
	#ifdef QFN48__
	// nothing
	#else
			// Set host responsibility for coexistence of i2c and flash (QFN68 Vesper Coupon)
			#define HOST_COXIST_FLASH_I2C__
	#endif
#endif

#ifdef DIGITAL_VESPER_HEXAGRAM
#define MIC_TYPE			1	// 1 = digital // 2 = analog
#define EXTERNAL_MICS		1 	// 0  no external mics // 1  external mics
#define USE_HEXAGRAM_MICS	1	// set to 0 when old 1.8 vesper mic is used.
#define VESPER_MIC			1	// 1 = mic 1 is Vesper // 0 = non vesper
#define VESPER_HEXAGRAM_MICS	// use Vesper of hexagram (need to connect I2C wires on EVBL)

#define VAD_VESPER_ 
#endif

//------------------------------------
// 	VAD additional parameters:

#ifdef VAD_HW_
#define VESPER_VAD			0
#define LP_HW_VAD			0
#define HW_VAD				1
#define ENVIRO_ADAPTATION	1
#define HW_VAD_ENABLE_PRE_ROLL_EXTENTION 	1
#endif

#ifdef VAD_LP_HW_
#define VESPER_VAD			0
#define LP_HW_VAD			1 			
#define HW_VAD				0 					// Causion: for now HW_VAD flag is not set during LP_HW_VAD
#define ENVIRO_ADAPTATION	0 					// Causion! for now not for LP_HW_VAD
#define HW_VAD_ENABLE_PRE_ROLL_EXTENTION 	1	// Causion! not for LP_HW_VAD
#endif

#ifdef VAD_VESPER_
#define VESPER_VAD			1
#define LP_HW_VAD			0  
#define HW_VAD				0 
#define ENVIRO_ADAPTATION	0 	
#define HW_VAD_ENABLE_PRE_ROLL_EXTENTION 	0	
#endif

#ifndef VAD_HW_
#ifndef VAD_LP_HW_
#ifndef VAD_VESPER_
#define VESPER_VAD			0
#define LP_HW_VAD			0
#define HW_VAD				0
#define ENVIRO_ADAPTATION	0
#define HW_VAD_ENABLE_PRE_ROLL_EXTENTION 	1
#endif
#endif
#endif

#define ENABLE_HW_VAD_BUFFER_RECORDING 		0
#define AMAZON_SW_VAD_ENABLE				0

#define POST_PLL_DIVIDER_ENABLE 			1	// in optimized

//----------

#ifdef HOST_COXIST_FLASH_I2C__
#define HOST_COXIST_FLASH_I2C	1
#else	
#define HOST_COXIST_FLASH_I2C	0
#endif

//----------


/*   #defines of SOD_HDMI_MIC_A and SOD_HDMI_MIC_B are relevant only if USE_HEXAGRAM_MICS is 1.
		SOD_HDMI_MIC_A = 0	and    non used SOD_HDMI_MIC_B = 3
			in that case mic 0 (rising), 5 (falling) will work on DM0

		SOD_HDMI_MIC_A = 1	and    non used SOD_HDMI_MIC_B = 0
			in that case mic 1 (falling), 3 (rising) will work on DM0

		SOD_HDMI_MIC_A = 2	and    non used SOD_HDMI_MIC_B = 0
			in that case mic 2 (rising), 6 (falling) will work on DM0.	

	FOR VESPER HEXAGRAM we need to use mic 0,5 only! 
*/
// SOD_HDMI_MIC_A must be different from SOD_HDMI_MIC_B
#ifdef DIGITAL_VESPER_HEXAGRAM
#define SOD_HDMI_MIC_A		0 	// 0 - 6 SOD board DM0 mic number,	// explanation in bsp.c
#define SOD_HDMI_MIC_B		3 	// 0 - 6 SOD board DM1 mic number,
#else
#define SOD_HDMI_MIC_A		1 	// 0 - 6 SOD board DM0 mic number,	// explanation in bsp.c
#define SOD_HDMI_MIC_B		0 	// 0 - 6 SOD board DM1 mic number,
#endif

#define MIC_GAINS				0	 ////6

#endif

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

#ifdef INIT_TYPE_SENSING_VC_3			// SED NNL with Voice Call, with Hexagram vad mic

// HW: QFN board with hexagram VAD mic (2 wires on board) - VAD still not operative in FW

#define INIT_SEQUENCE		3

#define NNL_FW				1			// 0 - non NNL FW // 1 - NNL FW
//#define FIRMWARE_IMAGE 		"melon/fw/Melon_D10_ver_4660_RC5_T3T4_34_DspSense_111_ASRP3_3933.bin"	// with T3T4
#define FIRMWARE_IMAGE 		"melon/fw/melon_fw/Melon_D10_ver_4660_RC5_T3T4_34_DspSense_111_ASRP3_3933.bin"	// with T3T4
//#define FIRMWARE_IMAGE 		"melon/fw/Melon_D10_ver_4660_D012_T3T4_34_DspSense_111_ASRP3_3932.bin"	// with T3T4

#define AMAZON_MODEL  		""
#define GOOGLE_MODEL  		""
#define SED_MODEL	  		A_MODEL_SED_HW_GB
#define CYBERON_MODEL		""
#define CYBERON_MODEL_NNL	""
#define RETUNE_MODEL		""

#define WAKE_ENGINE			7		// Dual
// Dual case:
#define WAKE_ENGINE1		5		// 5 = sed // 6 = t3t4 // 7 = dual
#define WAKE_ENGINE1_MODEL	SED_MODEL
#define WAKE_ENGINE2		6		// Must be set to valid number in case WAKE_ENGINE = DUAL
#define WAKE_ENGINE2_MODEL	NO_MODEL

#define MODEL1_LOCATION		MEM_TYPE_AHB
#define MODEL2_LOCATION		0					// No tloaded
#define ASRP_LOCATION		MEM_TYPE_AHB
#define SUPPORT_NSBF		NO_NSBF

#define AUDIO_BUFFER_SIZE	AUDIO_BUFFER_3_SEC

//--------------------------------

//		MIC configuration:

#define NO_OF_MICS			2	// in all use cases
#define MIC_TYPE			1	// 1 = digital // 2 = analog

#define	TDK_ENABLE			0		// 0 (disable) // 1 (enable)
#define VESPER_VAD			0	// for now	
#define LP_HW_VAD			0
#define HW_VAD				0
#define ENVIRO_ADAPTATION	0
#define HW_VAD_ENABLE_PRE_ROLL_EXTENTION 	0
#define ENABLE_HW_VAD_BUFFER_RECORDING 		0
#define AMAZON_SW_VAD_ENABLE			0
#define AMAZON_KWD_METADATA_ENABLE		0

#define EXTERNAL_MICS		1 		// 0  no external mics // 1  external mics
#define USE_HEXAGRAM_MICS	0 	// set to 0 (and also set VESPER_VAD to 0) when old HDMI mic is used.
#define SOD_HDMI_MIC_A		1 	// 0 - 6 SOD board mic number		// explanation in bsp.c
#define SOD_HDMI_MIC_B		2 	// 0 - 6 SOD board mic number

#define MIC_GAINS			13

//#define MIC_DECIMATOR		// Comment to disable

#endif

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

// 	D10L Board GPIO configuration (for event / ready / I2C / UART debug / Sensor etc.

#ifdef QFN48__
	#define QFN48					1
	#define EVENTS_GPIO_NUM__		EVENTS_GPIO_NUM_7  	  // old:7  // new: 25
	#define READY_GPIO_NUM__ 		FW_READY_GPIO_NUM_9   // old: 9 // new: 26
	#define I2C_SDA_MST_GPIO__		I2C_SDA_GPIO_11 	  // old: 11 // new: 14 
#else // QFN68
    #define QFN48					0
    #define EVENTS_GPIO_NUM__		EVENTS_GPIO_NUM_14    // old:7  // new: 25
	#define READY_GPIO_NUM__ 		FW_READY_GPIO_NUM_1  // old: 9 // new: 26
	#define I2C_SDA_MST_GPIO__		I2C_SDA_GPIO_14 	  // old: 11 // new: 14 
#endif

// #define SPECIAL_GPIO_FOR_READY	//	Comment out if no special GPIO for ready

#ifdef INIT_TYPE_RCU_MELON_2

	#define EVENTS_GPIO_NUM			EVENTS_GPIO_NUM__  // old:7  // new: 25
	#define EVENTS_GPIO_POLARITY	EVENTS_GPIO_POL_HIGH	
	#define EVENTS_GPIO_DIRECTION	RISING
	#define EN_DIS_EVENTS_GPIO 		EN_EVENTS_GPIO
	#define EVENT_HAPPEN			1

	#ifdef SPECIAL_GPIO_FOR_READY

		#define READY_ACK				1	// 1 = FW ready GPIO is operative (for now not per chip) // 0 = not operative
		#define COMMON_READY_EVENT		0	// no common GPIO for both ready and event
		#define READY_GPIO_NUM 			READY_GPIO_NUM__  // old: 9 // new: 26
		#define EN_DIS_READY_GPIO 		DIS_FW_READY_GPIO		// enabled by default
		#define READY_GPIO_POLARITY 	FW_READY_GPIO_POL_HIGH
		#define READY_GPIO_DIRECTION	RISING
		#define READY_HAPPEN			1

	#else	// No Special Ready GPIO - GPIO is like Event but shifted left

		#define READY_ACK				0		// 1 = FW special ready GPIO is operative (for now not per chip) // 0 = not operative
		#define COMMON_READY_EVENT		1		// common GPIO for both ready and event
		#define READY_GPIO_NUM 			FW_READY_GPIO_NUM_25
		#define EN_DIS_READY_GPIO 		DIS_FW_READY_GPIO		// disabled by default
		#define READY_GPIO_POLARITY 	FW_READY_GPIO_POL_HIGH
		#define READY_GPIO_DIRECTION	RISING
		#define READY_HAPPEN			1

	#endif

	#define I2C_PULLUP_GPIO			16					// old: 6  // new: 16
	#define I2C_SDA_MST_GPIO		I2C_SDA_MST_GPIO__ 	// old: 11 // new: 14 

	#define SENSOR_INT_GPIO			18
	#define SENSOR_MUX_GPIO			31		// same as SENSOR_NO_GPIO
    #define BUZZER_GPIO				BUZZER_NO_GPIO

    #define UART_DEBUG_GPIO			UART_D2_0_UART_TDI_RTCK_D4_1_GPIO_14_15_D10_GPIO_12_13

	// Other parameters for init type 2
	#define SINGLE_THREAD_HOST		// all activitied with FW are synchonized into a single thread

	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

#else	// ready / event GPIOs for all other init types except 2

	#define I2C_SDA_MST_GPIO		I2C_SDA_GPIO_11

	#define EVENTS_GPIO_NUM			EVENTS_GPIO_NUM_14
	#define EVENTS_GPIO_POLARITY	EVENTS_GPIO_POL_HIGH	
	#define EVENTS_GPIO_DIRECTION	RISING
	#define EN_DIS_EVENTS_GPIO 		EN_EVENTS_GPIO
	#define EVENT_HAPPEN			1

// if no special GPIO for READY - set READY_ACK to 0 and COMMON_READY_EVENT to 1
	#ifdef SPECIAL_GPIO_FOR_READY

		#define READY_ACK				1	// 1 = FW ready GPIO is operative (for now not per chip) // 0 = not operative
		#define COMMON_READY_EVENT		0	// no common GPIO for both ready and event
		#define READY_GPIO_NUM 			FW_READY_GPIO_NUM_10
		#define EN_DIS_READY_GPIO 		EN_FW_READY_GPIO		// enabled by default
		#define READY_GPIO_POLARITY 	FW_READY_GPIO_POL_HIGH
		#define READY_GPIO_DIRECTION	RISING
		#define READY_HAPPEN			1

	#else	// No Special Ready GPIO - GPIO is like Event but shifted left

		#define READY_ACK				0		// 1 = FW special ready GPIO is operative (for now not per chip) // 0 = not operative
		#define COMMON_READY_EVENT		1		// common GPIO for both ready and event
		#define READY_GPIO_NUM 			FW_READY_GPIO_NUM_14
		#define EN_DIS_READY_GPIO 		DIS_FW_READY_GPIO		// disabled by default
		#define READY_GPIO_POLARITY 	FW_READY_GPIO_POL_HIGH
		#define READY_GPIO_DIRECTION	RISING
		#define READY_HAPPEN			1

	#endif

	#define UART_DEBUG_GPIO			UART_D2_0_UART_TDI_RTCK_D4_1_GPIO_14_15_D10_GPIO_12_13

	#define SENSOR_INT_GPIO			SENSOR_NO_GPIO
	#define SENSOR_MUX_GPIO			SENSOR_NO_GPIO

	#define BUZZER_GPIO				BUZZER_NO_GPIO

#endif

	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

#endif /* _DX_PARAMS_FLAVORS_H */

