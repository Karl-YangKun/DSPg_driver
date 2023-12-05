/**
 * dx_params.h  --  product parameters etc.
 *
 * Copyright (C) 2016 DSP Group
 * All rights reserved.
 *
 * Unauthorized copying of this file is strictly prohibited.
 */

#ifndef _DX_PARAMS_H
#define _DX_PARAMS_H

#include "dx_fw_defs.h"
#include "dx_gen.h"

#define CUSTOMER_HOST_VERSION "0.1"

#define DSPG_C_REFERENCE_HOST_VERSION "RCU_3.1.0"

#define I2C_DEVICE_ADDRESS 	0x3e  		// According to chip
//#define I2C_DEVICE_ADDRESS 	0x3f  		// According to chip

#define FW_MODE				MODE_RECORD	 // 1 = fw_fast // 2 = asrp/fw record // 3 = fw-debug
								 // If FW_MODE 2, OPTIMIZED	should be 0
#define NSBF         NO_NSBF
#define DISABLE_ASRP_RECORD   FALSE
#define AHB_DTCM_DUMP   1

#define REG_5_FW_RECORD		0x0	// set hex!
#define REG_6_FW_RECORD		0x0
#define REG_7_FW_RECORD		0x0


#define VT1_RECORD_EN	0
#define VT2_RECORD_EN	0

#define USE_HIGHER_GPIO_AT_3V3		0     // 0 - Use Higher GPIO at 3.3 V, 1 - Use Higher GPIO at 1.8 V
#define USE_HIGHER_GPIO				0     // 0 - Only Use GPIO from 0 to 18, 1 - Use GPIO from 0 to 18 with Higher GPIOs 19 to 30
#define TDM_CLK_SHARED           1 //shared should be enabled for 3.3v always.For 1.8V both options are available
#define BG_48_EC_DECIMATION_EN    0  //enable eco cancellation ref deimation for better music quality
#define TDM_CONNECT_TO_CODEC      0


#define DIGITAL_MIC_FREQ        1536
#define ANALOG_MIC_FREQ     1024
///////////////////////////////

/* Init type configuration */

// Select a single init sequence type
#define INIT_TYPE_MELON_0			// Original Melon (not NNL)
//#define INIT_TYPE_NNL_MELON_1		// Amazon NNL
// #define INIT_TYPE_RCU_MELON_2		// Melon RCU
//#define INIT_TYPE_SENSING_VC_3	// Sed / Voice call  	(T3T4 optional according to model)

#include "dx_params_flavor.h"		// Init Type related #defines

//		Unchanged parameters

#define TRASPORT_CHAN	1	// 1 = SPI // 2 = I2C  // 3 = UART	

#define EVENTS_TO_HOST	1	// 1 = FW sends events to host  0 = no events to host

#define CHIP_1			10	// 2 = DBM_D2  // 4 = DBM_D4  // 5 = DBM_D5 // 7 = DBM_D7 // 8 = DBM_D8 // 10 = DBM_D10 (defined in dx_gen.h)

#define MASTER_CLOCK	MASTER_CLOCK_32768

//#define transport type (SPI / I2C / UART) by comment 2 of next lines, and do full make:
#define SPI_TRANSPORT
//#define I2C_TRANSPORT
//#define UART_TRANSPORT

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

// 		Mic configuration:
//		MIC 1 -- MIC4 was moved to dx_params_flavor.h

#define SAR_FILTER	SAR_IIR_FILTER_1024// SAR_IIR_FILTER_128/SAR_IIR_FILTER_256/SAR_IIR_FILTER_512/SAR_IIR_FILTER_1024

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

// 		TDM Configuration
#define TDM_SAMPLE_RATE 	48000 	// 16000 = 16 KHz	// 48000 = 48 KHz
#define TDM_BIT_RATE 		16 	// 16 = 16 bit	// 32 = 32 bit
#define NO_OF_SPEAKERS		1	// 1 = one speaker		2 = 2 speakers
#define I2S_CHIP_MASTER		0	// 0 = Chip Slave	// 1 = Chip Master - required for INIT TYPE2

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


#if TDM_CLK_SHARED 
  #if USE_HIGHER_GPIO_AT_3V3
	  #define TDM0_3V3        0x2   //TDM0 at GPIO 19,20,21,22
	  #define TDM1_3V3        0x4   //TDM1 at GPIO 19,20,23,24
	  #define TDM2_3V3        0x8
  #else
	  #define TDM0_3V3        0x0   //TDM0 at GPIO 6,7,8,9
	  #define TDM1_3V3        0x0   //TDM1 at GPIO 6,7,15,16
	  #define TDM2_3V3        0x0
  #endif
#else
  #if USE_HIGHER_GPIO_AT_3V3
	  #define TDM0_3V3        0x2   //TDM0 at GPIO 19,20,21,22
	  #define TDM1_3V3        0x4   //TDM1 at GPIO 19,20,23,24
	  #define TDM2_3V3        0x8
  #else
	  #define TDM0_3V3        0x0   //TDM0 at GPIO 6,7,8,9
	  #define TDM1_3V3        0x4   //TDM1 at GPIO 19,20,23,24
	  #define TDM2_3V3        0x0
  #endif
#endif

//		SED / T3T4 parameters

#define SED_GAIN				25
#define T3T4_GAIN 				25
#define DNN_STEP_SIZE			DNN_STEP_SIZE_GB_HW
#define THUD_EN_DIS				FE_THUD_ENABLE
#define THUD_REPORT_EN_DIS		FE_THUD_REPORTING_ENABLE

#define NUM_OF_CLASSES			PP_NUM_OF_CLASSES   
#define CLASSES_TO_REPORT 		PP_CLASSES_TO_REPORT
#define DNN_OUT_SMOOTH  		PP_DNN_OUT_SMOOTH

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

	// Environment Adaptation configuration :

#if VESPER_VAD
                                            // PGA [dBSPL] = 107 + 10*log10(FW energy*(2^-31))3
	// For Vesper VAD ((for registers 341-345)): 
	#define SILENCE_LOW_THD             1913    // Correspond to 46.5dBSPL (30 PGA_Gain)
	#define SILENCE_HIGH_THD            8500    // Correspond to 53dBSPL (25.5 PGA_Gain)
	#define LONG_ACTIVE_PERIOD          75      // 7.5sec (time for the BF to converge)
	#define SHORT_ACTIVE_PERIOD         15      // 1.5sec
	#define RTC_MID_RATE_FACTOR         6       // Set RTC to fire every 6*33sec (medium environment noise)
	#define RTC_LOW_RATE_FACTOR         30      // Set RTC to fire every 30*33sec (clean enviroment less frequent updates)
	#define RTC_PERIOD_BASE_NUM         33      // 33sec Noisy environment need frequent adaptation
	// (for registers 34C-34D)
	#define RTC_ADAPT_RATE_HIGH_THD     28      // if PGA_MAX above this thershold use MID_RATE_FACTOR
	#define RTC_ADAPT_RATE_LOW_THD      25      // if PGA_MAX above this thershold use LOW_RATE_FACTOR
                                            // else use RTC_PERIOD_BASE_NUM
#else
    // For HW_VAD and LP_HW_VAD (for registers 341-345):
    #define SILENCE_LOW_THD             0xC     // Correspond to 46.5dBSPL (30 PGA_Gain) 341
    #define SILENCE_HIGH_THD            0x10    // Correspond to 53dBSPL (25.5 PGA_Gain) 342
    #define LONG_ACTIVE_PERIOD          49      // 4.9sec (time for the BF to converge) 343
    #define SHORT_ACTIVE_PERIOD         14      // 1.4sec
    #define RTC_MID_RATE_FACTOR         6       // Set RTC to fire every 6*3sec (medium environment noise) 344
    #define RTC_LOW_RATE_FACTOR         150     // Set RTC to fire every 150*3sec (clean environment less frequent updates)
    #define RTC_PERIOD_BASE_NUM         8       // 6sec Noisy environment need frequent adaptation
    // (for registers 34C-34D)
    #define RTC_ADAPT_RATE_LOW_THD      0xD     // RTC period adaptation rate high threshold -68dB 34c 
    #define RTC_ADAPT_RATE_HIGH_THD     0x3A    // RTC period adaptation rate low threshold -55dB 34d
#endif                                                                                                                                                                   // else use RTC_PERIOD_BASE_NUM

	// additional vesper VAD defines (for register 346):
	#define PGA_MAX_STEP_DOWN_DEC       1       // Steps to decrease PGA_MAX
	#define PGA_MAX_HIGH_BAUNDRY        30      // PGA_MAX_HIGH_BAUNDRY
	#define PGA_MAX_LOW_BAUNDRY         19      // PGA_MAX low baoudry

	// additional HW VAD defines (for registers 248-24B)
	#define RMS_MU				        0x38c3  // Average energy forgetting factor, used for smoothing the input signal energy 0.8869 348
//	#define MIN_TH_DELTA_UP 	        0x1304  // HW-SAD factor to increase min. energy threshold 1.5dB 349
	#define MIN_TH_DELTA_UP 	        0x1000  // HW-SAD factor to increase min. energy threshold 0dB 349

	#define MIN_TH_LOW_LIMIT	        0x7  	// HW-SAD min. energy low threshold -73dB 34A
	#define MIN_TH_HIGH_LIMIT	        0x49  	// HW-SAD min. energy high threshold -53dB 34B

	// Vesper initial register
	#define WOS_HPF                     0x0     // 200hz
	#define WOS_LPF                     0x1     // 4000hz
	#define FAST_MODE_CNT               0x2    
	#define PGA_MIN_VAL                 0x12    // 64.5 dB
	#define PGA_MAX_VAL                 0x1F    // 45 dB                 
	#define VESPER_3011_WOS_THRESHOLD   0x4     // 14 dB

	// For both Vesper Vad and TDK (i.e. I2C)
	#define WDT_ENABLE                  0       // I2C watchdog disable
	#define WDT_DLY                     0       // watchdog time delay
	#define DOUT_RAW                    0       // Dout pin will be latched high upon trigger

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

// 		Specific use cases options

#define SUPPORT_RX_PROCESS			0	// during voice call:  	0 = disable // 1 = enable
#define GOGGLE_ASSISTANT_MODE		0	// during Barge in:		0 = disable // 1 =  enable
#define USE_SPI_IN_GA_MODE 			0	// 0 = False  //  1 = True  // buffer spi during google assist

#define BARGE_IN_TDM_BYPASS 		0	// 0 = no bypass // 1 = bypass

#define BARGE_IN_SINGLE_WWE_TO_LOAD	2

#define MICS_VOICE_CALL				1	// 1 mic // 2 mics // 3 mics

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

// 		Buffering session parameters

#ifdef E_Q_D
	#define BUFFERING_SECONDS		10	// for EQD tests you need longer buffering period
#else
	#define BUFFERING_SECONDS		5

#endif
		
#define BUFFERING_MAX_CHUNK			4096		// for NON SPI
#define BUFFERING_HOST_BUFFER_SIZE	15360

#define AUDIO_SAMPLE_RATE 			16000		
#define NO_OF_STREAM_CHANNELS		1	// how many channels to stream during buffering
#define BUFFERING_FRAME_LENGTH		960
#define DDF_VALID_CLKS				2

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

//		ASRP Parameters

#define ASRP_DELAY 					0
#define ASRP_BYPASS					0	// we should support both // 0 = non bypass // 1 = bypass

#define DYNAMIC_ASRP_FILTER			0
#define VC_FILTER_FILENAME 			"melon/asrp/Asrp_filterVoiceCall.bin"

//////////////////////////////////////////////////////////////////////////////

// 		D10L Board GPIO configuration and Vesper mic GPIOs

// UART_DEBUG_GPIO - moved to flavors.		
#define CHANGE_UART_BAUDRATE 	CHANGE_UART1_BAUDRATE
#define HOST_INTERFACE			(SPI_D2_TDM1_D4_GPIO_4_5_6_7_D10_GPIO_2_3_4_5 | UART_DEBUG_GPIO | UART_DEBUG_MODE_0)
#define USE_LOW_GPIOS

/////////////////////////////////////////////////////////////////////////////

// Vesper Mics GPIO 

//	FOR VESPER HEXAGRAM we need to use mic 0,5 only! 
#ifdef	VESPER_HEXAGRAM_MICS	// use Vesper of hexagram (need to connect I2C wires on EVBL)
		// remember to connect 2 wires SDA to GPIO 10 and SCL to GPIO 14 on board
		#define VESPER_DET_GPIO					DET_GPIO_NUM_27				// DOUT gpio: old: 18  new: 27				// for hexagram
		#define I2C_VESPER_MIC_SLAVE_ADDRESS  	I2C_VESPER_MIC_SLAVE_ADDRESS_RIGHT_CHANNEL	// 0x61 hexagram

#else	// old Vesper Vad 1.8 mic. 		For 3.3 - different GPIO is relevant (GPIO 2)
		#ifdef QFN48__
			#define VESPER_DET_GPIO					DET_GPIO_NUM_14				// for coupon
		#else
			#define VESPER_DET_GPIO					DET_GPIO_NUM_27				// DOUT gpio: old: 18  new: 27				// for hexagram
		#endif
		#define I2C_VESPER_MIC_SLAVE_ADDRESS  	I2C_VESPER_MIC_SLAVE_ADDRESS_LEFT_CHANNEL	// 0x60 hexagram
#endif

//////////////////////////////////////////////////////////////////////////////

/* preparation for 3.3 board:
#define EVENTS_GPIO_NUM			EVENTS_GPIO_NUM_25			
#define EVENTS_GPIO_POLARITY	EVENTS_GPIO_POL_LOW
#define EVENTS_GPIO_DIRECTION	FALLING
#define EN_DIS_EVENTS_GPIO 		EN_EVENTS_GPIO
#define  EVENT_HAPPEN			0

#define READY_GPIO_NUM			FW_READY_GPIO_NUM_26	
#define READY_GPIO_POLARITY		FW_READY_GPIO_POL_LOW
#define READY_GPIO_DIRECTION	FALLING
#define EN_DIS_READY_GPIO 		EN_FW_READY_GPIO
#define READY_HAPPEN			0
*/
//#define REMOTE_APP					// comment if no remote app

//////////////////////////////////////////////////////////////////////////////

//			Host application parameters

#ifdef TDK_
#define OPTIMIZED			0				// 0 = not optimized // 1 = optimized
#else
#define OPTIMIZED			1				// 0 = not optimized // 1 = optimized
#endif

//#define REDUCED_CODE

#ifndef SINGLE_THREAD_HOST
#define READ_WRITE_LOCK			// Multi thread speaks with FW, requires CS
#endif
								// for now for all init types
#define BOOT_FROM_FLASH		0	// 1 = boot from flash	// 0 = boot from host
#define INTERNAL_HOST		0	// 1 = Internal host // 0 = External host        TODO what meaning
#define LPM_HIGH_SPD_SPI_BUFFERING	1


//////////////////////////////////////////////////////////////////////////////

/* GPIO pins on RPI board */
#define RESET_VT_GPIO			17
#define INTERRRUPT_TRIGGER_GPIO	23

#ifdef SPECIAL_GPIO_FOR_READY
#define INTERRRUPT_READY_GPIO	5
#else
#define INTERRRUPT_READY_GPIO	23		// same GPIO as event, used for testing actual GPIO status
#endif
#define WAKEUP_VT_GPIO 			14

#define RPI_LED_GPIO			0xFF		// 22 // set 0xFF to ignore

#define MATH				// to comment out if customer host doesn't support math fuctions for rms

/////////////////////////////////////////

#define TDM0_CLK_MINI_STREAMER		0	// 1 = TDM0 clock from MiniStreamer		0 = TDM0 clock from RPI default

#define HOST_COLLECT_FW_LOGS		// to comment in real product
#define FW_LOGS_ADD_TIME_STAMP		0	// 1 = add time stamp 	// 0 = skip tume stamp

#define CPLD_RST_GPIO 			26
#define DEBUG_GPIO	 			13	// for tests only

#define PLAY_VOLUME				100	// Normal volume

//////////////// Mics

// EXTERNAL_MICS				// moved to dx_param_flavor.h

// USE_HDMI_MICS was removed

/////////////////////////////////////////

	// QA and debug options

#define QA_FLAG 					0	// set to 1 for QA audio tests

#define KEEP_MODE_3_DURING_MODE_2	0	// continue fw logs even during audio record

#ifdef INIT_TYPE_RCU_MELON_2			// RCU 
#define OVERIDE_REMOTE_APP_USECASE	9	// RCU NR - uc9
#else
#define OVERIDE_REMOTE_APP_USECASE	0	// 0  -- do not overide // required usecase
#endif

#define AFTER_INIT_ENTER_UC			0	// specify uc number.

// WARNING: 
// QFN48: Before set LDO_ENABLE to 0 -- must connect j5 and disconnect j6 (to avoid crash) 
// QFN68: j6 always on; j5 to 0v8 (LDO = 0) or LDO (LDO = 1)
#define LDO_ENABLE		1

#define EXT_PINS_VDDIO	0				// 0 for 1.8V // 1 for 3.3V

#endif /* _DX_PARAMS_H */

