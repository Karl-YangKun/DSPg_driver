#include "D10L_utils.h"

uint16 g_reg6_fw_record=0;
// Command is sent to booter before FW init (preboot protocol)
#ifdef INIT_TYPE_RCU_MELON_2	// preboot for RCU
const char preboot_request[] = {
    0x5A,0x02,				// Header + opcode
    0x06,0x00,0x00,0x00,	// length (in words)
    0x02,0x02,0x00,0x00,	// Address
    0xCD,0xAB,				// magic num
    0x0f,0x00,				// Valid mask
    0x01,0x00,				// Reg 12
                            // reg 15:
    (EN_DIS_EVENTS_GPIO   | EVENTS_GPIO_POLARITY | EVENTS_GPIO_NUM),
    ((EN_DIS_READY_GPIO | READY_GPIO_POLARITY  | READY_GPIO_NUM) >> 8),
    INTERNAL_HOST,0x00,		// IH y/n
                            // reg 29:
    (HOST_INTERFACE & 0x00FF), (HOST_INTERFACE & 0xFF00) >> 8};

#else
const char preboot_request[] = {		// Non RCU
    0x5A,0x02,
    0x04,0x00,0x00,0x00,
    0x02,0x02,0x00,0x00,
    0xCD,0xAB,
    0x03,0x00,
    0x01,0x00,
    // additional 2 bytes for event and Ready configuration
    (EN_DIS_EVENTS_GPIO   | EVENTS_GPIO_POLARITY | EVENTS_GPIO_NUM),
    ((EN_DIS_READY_GPIO | READY_GPIO_POLARITY  | READY_GPIO_NUM) >> 8)};
#endif
#if(NNL_FW)
// nnl preboot:
const char preboot_nnl1[] = {0x5a, 0x04, 0x90, 0x00, 0x00, 0x03, 0xA5, 0x52, 0x55, 0x55};
const char preboot_nnl2[] = {0x5a, 0x04, 0x94, 0x00, 0x00, 0x03, 0xA5, 0x52, 0x55, 0x15};
const char preboot_nnl3[] = {0x5a, 0x04, 0x20, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00};
#endif
/******************************************************************************************/

/*************************chip procedures******************************/
/*when using uart to communication, synchronize the baud rate
 * or send 0 to synchronize d10l*/
bool D10L_Sync(void)
{
    comm_inf_t c_type = DSPg_GetCommType();
    if(c_type == UART)
    {
        char send_buf[16];
        DBM_DEBUG("--D10L_SyncBaudRate, sync baud rate");
        memset(send_buf,0,sizeof(send_buf));
        WRITE(send_buf, 16);
        uint8 data[4];
        char ok[4]={0x4f,0x4b,0x0a,0x0d};
        //read 5 times
        for(uint8 i=0; i<5; i++)
        {
            DELAY(10);
            READ(data,4);
            if (strncmp((char*)data,ok, 4) == 0)
                return TRUE;
        }
        return FALSE;
    }
    //only for d10l
    else if(c_type == SPI)
    {
        char send_buf[] = {0};
        WRITE(send_buf, sizeof(send_buf)); //sync for d10l
        DELAY(WRITE_DELAY);
        return TRUE;
    }
    return TRUE;
}

//read register to check if it have fw error
bool D10L_CheckError(void)
{
    uint16 data;
    
    WRITE_REG(0xd,0,r16d16);
    READ_REG(0x5,&data,r16d16);
    DBM_DEBUG("--fw_errors: %x",data);
    if(data!=0)
    {
        uint16 r6,r8;
        READ_REG(REG_IO_PORT_ADDR_HI_06,&r6,r16d16);
        READ_REG(REG_IO_PORT_VALUE_HI_08,&r8,r16d16);
        DBM_DEBUG("  reg 6 = %x  reg 8 = %x \n",r6,r8 );
        return FALSE;
    }
    return TRUE;
}

//send boot commands before loading fw
void D10L_PrebootRequest(void)
{
    DBM_DEBUG("--D10L_PreBootPowerUp, send boot commands");
    WRITE(preboot_request, sizeof(preboot_request));
    DELAY(WRITE_DELAY);

    //clean checksum
    char send_buf[2];
    send_buf[0] = 0x5A;
    send_buf[1] = 0x0F;
    WRITE(send_buf, 2);
    DELAY(WRITE_DELAY);

#if(NNL_FW)
    //TODO
#endif
}

void D10L_ReadID(void)
{
    uint8 id[6]={0};
    char RegSet[5] = {0};

    uint8 len=sprintf(RegSet, "%03xr", 0x19);
    RegSet[len]='\0';
    DBM_DEBUG("--Read chip id");
    WRITE(RegSet,len);
    READ(id,5);
    DBM_DEBUG("--ID:%s",(char*)&id[1]);
}

void D10L_ReadVersion(void)
{
    uint32 vers[3];
    DBM_DEBUG("--Read FW version");
    READ_REG(FW_VERSION_NUMBER_00,&vers[0],r16d16);
    READ_REG(FW_MINOR_VERSION_NUMBER_05,&vers[1],r16d16);
    READ_REG(FW_RC_VERSION_NUMBER_06,&vers[2],r16d16);
    DBM_DEBUG("--The version: %x.%x.%x", vers[0],vers[1],vers[2]);
}

void D10L_RunChip(void)
{
    char buf_run[] = {BOOT_PRE_OP_CODE, BOOT_RUN_FW};
    DBM_DEBUG("--D10L_RunChip, send run command");
    WRITE( buf_run, 2);
    DELAY(10);
}

bool D10L_VerifyCheckSum(uint8 *expect)
{
    char write_buf[] = {0x5A, 0x0E};
    uint8 c[7];
    uint8 start = 2;

    DBM_DEBUG("--Read checksum");
    WRITE( write_buf, 2);
    READ(c, 7);

    if(DSPg_GetCommType()==SPI)
        start = 3;

    if (strncmp((char*)&c[start],(char*)expect, 4) == 0)
    {
        DBM_DEBUG("--Checksum is right");
        D10L_RunChip();
        D10L_ReadVersion();
        D10L_ReadID();
        return TRUE;
    }
    else
    {
        DBM_DEBUG ("--Checksum error!!! got: %2x %2x %2x %2x expected: %2x %2x %2x %2x ",
                        c[start],c[start+1],c[start+2],c[start+3],expect[0],expect[1],expect[2],expect[3]);
        return FALSE;
    }
}

/***********************************************************************
* FUNCTION NAME: D10L_WiteHwPort32()
*
* DESCRIPTION:
*	using indirect commands
* PARAMETERS:
*	int chip, int32_t address, int32_t value
* RETURNS:
*	none.
***********************************************************************/
static void D10L_WiteHwPort32(uint32 address, uint32 val)
{
    WRITE_REG( address, val,r32d32);
}

/***********************************************************************
* FUNCTION NAME: D10L_ReadHwPort32()
*
* DESCRIPTION:
*	using indirect commands
* PARAMETERS:
*	int chip, int32_t address
* RETURNS: 	int32_t value
*	none.
***********************************************************************/
static uint32 D10L_ReadHwPort32(uint32 address)
{
    uint32 val;

    //DBM_DEBUG ("address high = %x, address low = %x\n", address_high, address_low);

    READ_REG( address, &val,r32d32);

    //DBM_DEBUG ("val_high = %x val_low = %x  val= %x\n", val_high, val_low, val);
    return val;
}

//static void D10L_StopBuffering(void)
//{
//    WRITE_REG(OPERATION_MODE_01, DETECTION_MODE,r16d16);
//}

// This function is activated from interrupt context
static uint16 D10L_PhraseTriggerData (uint32 event )
{
    uint16 VT_offset;
    uint16 id,length;

    if (event == VT1_DET)
        VT_offset = VT1_REGS_OFFSET;
    else if (event == VT2_DET)
        VT_offset = VT2_REGS_OFFSET;
    else
    {
        DBM_DEBUG("[%s]: VT_offset bad value.\n", __func__);
        return FALSE;
    }

    //TODO we need to phrase the data
    READ_REG(VT_offset | VT_REG_WORD_ID,&id,r16d16);
    DBM_DEBUG("word id %d", id);
    READ_REG(VT_offset | VT_REG_PHRASE_LENGTH_27,&length,r16d16);
    DBM_DEBUG("phrase length %d", length);

    return id;
}

static void D10L_SetMasterClock(uint32 clk)
{
    switch (clk)
    {
        case MASTER_CLOCK_32768:
            WRITE_REG(MASTER_CLOCK_FREQUENCY_1B, MCLK_32768Hz,r16d16);
        break;

        case MASTER_CLOCK_24576:
            WRITE_REG(MASTER_CLOCK_FREQUENCY_1B, MCLK_24576000Hz,r16d16);
        break;

        case MASTER_CLOCK_19200:
            WRITE_REG( MASTER_CLOCK_FREQUENCY_1B, MCLK_19200000Hz,r16d16);
        break;

        default:
            break;
    }
}

static void D10L_InitAdditionalSequence(uint8 senquence)
{
    // additional register according to init type
    switch(senquence)
    {
      case 0:
            WRITE_REG(MEMORY_CONFIG_33, NONE_OPTMIZED_MEM_CFG,r16d16);
            WRITE_REG(GENERAL_CONFIGURATION_2_23, 	FW_VAD_TYPE_NO_VAD		|
                                                                DDF_SAMPLE_WIDTH_16_BIT |
                                                                MIC_SAMPLE_RATE_16K,r16d16);	// Non SED
            WRITE_REG(VT_AUDIO_HISTORY_CFG_305, VT_NUM_NONE |	HIST_POINT_IS_SWITCH_TO_BUFFER,r16d16);
          break;
      case 1:
            WRITE_REG(MEMORY_CONFIG_33, 	0x86,r16d16);		// non optimized with MIC_AUDIO_BUF_LOC_DTCM
            WRITE_REG(GENERAL_CONFIGURATION_2_23, 	FW_VAD_TYPE_NO_VAD		|
                                                                DDF_SAMPLE_WIDTH_16_BIT |
                                                                MIC_SAMPLE_RATE_16K,r16d16);	// Non SED
            WRITE_REG(VT_AUDIO_HISTORY_CFG_305, VT_NUM_NONE |	HIST_POINT_IS_SWITCH_TO_BUFFER,r16d16);
          break;
      case 2:
            WRITE_REG(MEMORY_CONFIG_33, 	NONE_OPTMIZED_MEM_CFG,r16d16);
            WRITE_REG(GENERAL_CONFIGURATION_2_23, 	FW_VAD_TYPE_NO_VAD		|
                                                            DDF_SAMPLE_WIDTH_16_BIT |
                                                            MIC_SAMPLE_RATE_16K,r16d16);	// Non SED
            WRITE_REG(VT_AUDIO_HISTORY_CFG_305, VT_NUM_NONE |	HIST_POINT_IS_SWITCH_TO_BUFFER,r16d16);

            if (TDK_ENABLE)
            {
                // change Mux from Flash to I2C (TDK)
                WRITE_REG(0x5,  0x86,r16d16);	// Select GPIO 6
                WRITE_REG(0x18, 0x23,r16d16);	// Config
                // set back to low to speak with vesper
                // WRITE_REG( 0x18, 0x24);	// Set LOW (25) or HIGH (24)
                if (SENSOR_INT_GPIO != SENSOR_NO_GPIO)
                {
                        WRITE_REG( SENSOR_GPIO_CONFIGURATION_405, SENSOR_1_CONFIG 	|
                                                                        (			  1  << 11)	|	// MUX_GPIO_POL active high
                                                                        (SENSOR_MUX_GPIO << 6 )	|
                //														(			  1  << 5 )	|	// INT_GPIO POL active high
                                                                        (SENSOR_INT_GPIO << 0 ),r16d16	);
                    }
            }
            DELAY(20);
          break;
      case 3:
            // Memory Configuration
            WRITE_REG( MEMORY_CONFIG_33, 	0x186,r16d16);		// larger memory is required for VC
            //General Configuration_2 (start with 16K mics)
            WRITE_REG( GENERAL_CONFIGURATION_2_23, 	FW_VAD_TYPE_NO_VAD		|
                                                                DDF_SAMPLE_WIDTH_16_BIT |
                                                                MIC_SAMPLE_RATE_16K,r16d16);	// Non SED
          break;
      default :
          // no additional registers
          break;
    }
}

static void D10L_SetLdoOutput( int output_voltage, int input_voltage, bool disable)
{
//     int input_volt;
//     int ldo_step = 0;

//     if (disable == TRUE)
//     {
// //        WRITE_REG( LDO_CONFIG_2B, 0,r16d16);
//         return;
//     }

    // if (input_voltage <= 1200)
    //     input_volt = LDO_CAS_OUT_LEVEL_1_2_AND_BELLOW;
    // else if (input_voltage == 1800)
    //     input_volt = LDO_CAS_OUT_LEVEL_1_8;
    // else
    // {
    //     DBM_DEBUG("[%s]: input voltage is not supported!\n", __func__);
    //     return;
    // }

    // ldo_step = (((output_voltage - 707) *1000) / 4966 * (1800 / 1800));
    // DBM_DEBUG("ldo_step =%d\n", ldo_step);
    // DBM_DEBUG("[%s]: LDO Config - input %d, output - %d, ldo_step = %d\n", __func__, input_voltage, output_voltage, ldo_step);

    // if (ldo_step > 59)
    // {
    //     DBM_DEBUG("[%s]: Cannot support this ldo step!\n", __func__);
    //     return;
    // }
    // else
    //     WRITE_REG( LDO_CONFIG_2B, ldo_step << 4 | input_volt	| LDO_EN,r16d16);

    UNUSED(input_voltage);
    UNUSED(disable);
    if (output_voltage == 800) {
        WRITE_REG16( LDO_CONFIG_2B, 0x0d25);
    } else if (output_voltage == 880) {
        WRITE_REG16(  LDO_CONFIG_2B, 0x0e25);
    }

    D10L_ReadHwPort32(0x03000044);		// for testing
}

static void D10L_ConfigureClocks(dspg_clock_config_t config)
{
    int8 tl3_div, ahb_div, apb_div;
    uint8 sleep_ms, multiplier, pll_step, x, div_ratio;
    uint16 reg_23, reg_23_orig, value, clk_config, low_clk_steps, ref_clk, post_pll_div, switch_clk, clk_src, i2s_sclk/*, master_spi_div*/;
    uint32 form_sel, hw_vad_min_freq, output_clk, max_clk, perph_clk, global_out=0, ahb_out=0, apb_out=0, tl3_out=0;

    max_clk = 300000;

    if (HW_VAD)
        hw_vad_min_freq= 4096000UL;  				// was 3072000;
    else
        hw_vad_min_freq= 1;
    UNUSED(hw_vad_min_freq);

    post_pll_div = 1;
    form_sel = 0;

    ref_clk = config.input_clk;

    if (config.wanted_pll == 0)
        config.wanted_pll = config.wanted_tl3_clk;
    else if (config.wanted_pll < ref_clk)
    {
        DBM_DEBUG("[%s]: wanted_pll = %d < ref_clk = %d Wanted PLL frequency cannot be lower than input CLK ERROR!\n", __func__, config.wanted_pll, ref_clk);
        return;
    }

    if( I2S_CHIP_MASTER)
        i2s_sclk = (TDM_SAMPLE_RATE * TDM_BIT_RATE * 2)/1000;
    else
        i2s_sclk = 1;

    if (LDO_ENABLE == 1) {		// LDO DISABLE case is configure at system init
        if (config.wanted_pll > 150000) {
            D10L_SetLdoOutput( 880, 1200, 0);
        } else {
            D10L_SetLdoOutput( 800, 1200, 0);
        }
    }

    if (config.input_clk != 32768)
    {
        if((config.input_clk * 1000) % 40000 == 0)
        {
            if((config.wanted_pll % 1000) == 0)
            {
                form_sel = FORM_SEL_2;
                ref_clk = 12000;
                low_clk_steps = 4000;
            }
            else
            {
                form_sel = FORM_SEL_1;
                ref_clk = 10240;
                low_clk_steps = 5120;
            }
        }
        else if((config.input_clk * 1000) % 51200 == 0)
        {
            form_sel = FORM_SEL_0;
            ref_clk = 12288;
            low_clk_steps = 4096;
        }
        else
        {
            DBM_DEBUG("[%s]: input clock is invalide ERROR!!!\n", __func__);
            return;
        }
    }
    else
        low_clk_steps = 4096;

    if(config.digital_mic_freq != 1) 
        config.digital_mic_freq *= 1000;

    if(config.analog_mic_freq != 1) 
        config.analog_mic_freq *= 1000;

    DBM_DEBUG("a: i2s_sclk=%d  max_clk=%d  ref_clk=%d  low_clk_steps=%d\n", i2s_sclk, max_clk, ref_clk, low_clk_steps);

    if(config.switch_to_tdm_clk)
    {
        ref_clk = (TDM_SAMPLE_RATE * TDM_BIT_RATE * 2)/1000;
        switch_clk = SRC_SET_CONFIG_CLK_ACCORDING_TO_CLK_SEL;
        clk_src = CLK_SEL_TDM0_SCLK;
        WRITE_REG( TDM_SCLK_CLOCK_FREQUENCY_1D, ref_clk,r16d16);
        DBM_DEBUG("b: I2S SCLK = %d ", ref_clk);

#ifdef D10L
        form_sel = FORM_SEL_0;
        ref_clk = 12288;
        low_clk_steps = 4096;
#endif
    }
    else if(config.switch_to_mclk)
    {
        switch_clk = SRC_SET_CONFIG_CLK_ACCORDING_TO_CLK_SEL;
        clk_src = CLK_SEL_MCLK_IN;
        if(ref_clk == 32768)
        {
            DELAY(40);
            WRITE_REG(MASTER_CLOCK_FREQUENCY_1B, 0x20,r16d16);
        }
        else
            WRITE_REG(MASTER_CLOCK_FREQUENCY_1B, config.input_clk,r16d16);
    }
    else
    {
        switch_clk = SRC_SET_KEEP_CUR_CLK_SRC;
        clk_src = CLK_SEL_MCLK_IN;
    }

    div_ratio = config.wanted_pll/ref_clk;

    if(config.wanted_pll < ref_clk)
        multiplier = 2;
    else
        multiplier = 1;

    output_clk = 0;

    DBM_DEBUG("c: div ratio=%d multiplier=%d \n",div_ratio, multiplier);

#ifndef D10L
    if ((ref_clk == 32768) || (ref_clk == 49152) || ((ref_clk == 12288) && (config.input_clk == 19200)))
#endif
    {
        if (multiplier * config.wanted_pll < ref_clk)
            pll_step = 0;
        else
        {
            if((multiplier * config.wanted_pll - ref_clk)%low_clk_steps == 0)
                pll_step =(multiplier * config.wanted_pll - ref_clk)/low_clk_steps;
            else
                pll_step = (multiplier * config.wanted_pll - ref_clk)/low_clk_steps + 1;
            DBM_DEBUG("pll_step = %d\n", pll_step);
        }

        while(output_clk/1000 < max_clk)
        {
            if (pll_step < 0) pll_step = 0;   //making sure a non negative number is used
            output_clk = ref_clk*1000 + ((pll_step) * low_clk_steps*1000);

#ifdef D10L
            (perph_clk = output_clk/2);
#else
            (perph_clk = output_clk);
#endif
            // Search for output_clk that is a multiplier of the below 3 frequencies (i.e. modulo operatopr % returns 0)
            if (!(perph_clk % i2s_sclk) && !(perph_clk % config.digital_mic_freq) && !(perph_clk % config.analog_mic_freq)/*&& !(output_clk % hw_vad_min_freq)*/) break;		// found!
            else
            {
                DBM_DEBUG ("no common divider - PLL step goes up from %d to %d\n", pll_step, pll_step+1);
                pll_step +=1;
            }
        }
    }
#ifndef D10L
    else
    {
        pll_step = div_and_ceil((multiplier*wanted_pll),(ref_clk));
        while (output_clk/1000 < max_clk)
        {
            output_clk = pll_step * ref_clk*1000;
                // Search for output_clk that is a multiplier of the below 3 frequencies (i.e. modulo operatopr % returns 0)
            if (chip == DBM_D10) perph_clk = output_clk/2;
            else perph_clk = output_clk;

            if (!(perph_clk % i2s_sclk) && !(perph_clk % config.digital_mic_freq) && !(perph_clk % config.analog_mic_freq)&& !(output_clk % hw_vad_min_freq))
            {
                break;		// found!
            }
            else
            {
                DBM_DEBUG ("no common divider - PLL step goes up from %d to %d\n", pll_step, pll_step+1);
                pll_step = pll_step+1;
            }
        }
    }
#endif
    if(config.use_pll_post_div)
    {
        post_pll_div = ((output_clk/1000)/config.wanted_tl3_clk);	// simple division = floor
        tl3_div = 1;
    }
    else
        tl3_div = (output_clk/1000) / config.wanted_tl3_clk;		// simple division = floor

    if(output_clk/1000 > (max_clk-40000))
    {
        DBM_DEBUG("!\n\n!!! MIPS ARE HIGH!!\n\n!\n");
        return;
    }
    DBM_DEBUG ("d: pll step = %d\n post_pll_div=%d\n tl3_div=%d\n ", pll_step, post_pll_div, tl3_div);

    tl3_div = tl3_div-1;

    if(config.wanted_ahb_clk == 0)
        config.wanted_ahb_clk = config.wanted_tl3_clk;

    if(config.wanted_apb_clk == 0)
        config.wanted_apb_clk = config.wanted_ahb_clk;

    ahb_div = (config.wanted_tl3_clk/config.wanted_ahb_clk)-1;
    apb_div = (config.wanted_ahb_clk/config.wanted_apb_clk)-1;
    // global_out =  output_clk/(post_pll_div);
    perph_clk =  output_clk/2;
    if (tl3_div == -1)
        tl3_div = 0;
    // tl3_out = global_out/(tl3_div+1);
    if (ahb_div == -1)
        ahb_div = 0;
    // ahb_out = tl3_out/(ahb_div+1);
    // apb_out = ahb_out/(apb_div+1);

    DBM_DEBUG("e: Selected Clock Frequencies are:\n   PLL = %d\n   PERIPH = %d\n   GLOBAL = %d\n   TL3 = %d\n   AHB = %d\n   APB = %d\n",
                                    output_clk, perph_clk, global_out, tl3_out, ahb_out, apb_out);
    DBM_DEBUG("f: TL3 frequency = %d MHz\n", tl3_out/1000000UL);

//    if (FLASH_ENABLE)
//    {
//        master_spi_div = (output_clk+(MASTER_SPI_MAX_CLOCK-1000000UL)) / MASTER_SPI_MAX_CLOCK; //make sure master spi clock will not exceed 10Mhz
//        DBM_DEBUG ("g: master_spi_div = %d\n ", master_spi_div);
//        WRITE_REG( FLASH_SPI_CONFIGURATION_360, MASTER_SPI_MODE_3 | (master_spi_div-1),r16d16);
//    }

//    if (LDO_ENABLE == 1 && tl3_out > 150000000UL) 		// LDO DISABLE case is configure at system init
//            D10L_SetLdoOutput(880, 1200, FALSE);

    READ_REG(GENERAL_CONFIGURATION_2_23,&reg_23,r16d16);
    reg_23_orig = reg_23;
    reg_23 &= 0xFFE0;

    if (config.use_pll_post_div)
        reg_23 |= (post_pll_div-1);

    if (reg_23_orig != reg_23)
        WRITE_REG( GENERAL_CONFIGURATION_2_23, reg_23,r16d16);

    if(ref_clk == 32768)
        sleep_ms = 40;
    else
    {
        if((config.input_clk != 24756) && (FW_MODE == MODE_DEBUG))
            sleep_ms =50;
        else
            sleep_ms = 15;
    }

    if(!config.switch_to_tdm_clk)
    {
        if(DSPg_GetCommType() != I2C) 	// send 32 bit register
            WRITE_REG( DSP_CLOCK_CONFIG_10, form_sel << 12				|
                                                         switch_clk << 12			|
                                                         clk_src << 12 				|
                                                         PLL_OSC_SEL_USE_PLL <<  12	|
                                                         pll_step << 12				|
                                                         tl3_div << 8				|
                                                         apb_div << 5				|
                                                         ahb_div,r16d32);
        else
        {
            WRITE_REG( DSP_CLOCK_CONFIG_10, USE_PLL_STEP_FROM_REG_0x1E	|
                                                                tl3_div << 8		|
                                                                apb_div	<< 5		|
                                                                ahb_div,r16d16);
            DELAY( sleep_ms);	// additional interim delay

            WRITE_REG( DSP_CLOCK_CONFIG_EXTENSION_1E, form_sel			|
                                                                switch_clk			|
                                                                clk_src				|
                                                                PLL_OSC_SEL_USE_PLL	|
                                                                pll_step,r16d16);
        }
        DELAY( sleep_ms);
    }
    else
    {
        if ((config.input_clk != 24756) && (FW_MODE == MODE_DEBUG))
            sleep_ms =100;
        else
            sleep_ms =100;   // for D10L switch to TDM   TODO ????
        for(x = 0; x < 100; x++)
        {
            if(DSPg_GetCommType() != I2C)
                WRITE_REG(DSP_CLOCK_CONFIG_10, form_sel << 12				|
                                                             switch_clk <<  12			|
                                                             clk_src << 12 				|
                                                             PLL_OSC_SEL_USE_PLL << 12	|
                                                             pll_step << 12				|
                                                             tl3_div << 8				|
                                                             apb_div << 5				|
                                                             ahb_div,r16d32);
            else
                WRITE_REG( DSP_CLOCK_CONFIG_EXTENSION_1E, form_sel			|
                                                                    switch_clk			|
                                                                    clk_src				|
                                                                    PLL_OSC_SEL_USE_PLL	|
                                                                    pll_step,r16d16);
            DELAY( sleep_ms);
        // verify actual clock switch
            READ_REG( DSP_CLOCK_CONFIG_EXTENSION_1E,&value,r16d16);
            clk_config = (CLK_SEL_TDM0_SCLK|PLL_OSC_SEL_USE_PLL) + pll_step;

            if(value == (clk_config & 0x7fff))
            {
                DBM_DEBUG("Successfull switch to TDM0 Clock\n");
                break;
            }
            else
            {
                //ERR_CNT = x;
                if (x == 99)
                {
                    DBM_DEBUG("Cannot switch to TDM0 CLOCK\n");
                    return;
                }
                else
                {
                    DBM_DEBUG("No Clock in TDM0 pin - Try Number %d\n", (x+1));
                    DELAY(50);		//to verify
                    return;
                }
            }
        }
    }

    if(config.switch_to_mclk)
    {
#ifdef D10L
        DELAY(20);		//to verify if required
#endif
        uint16 data=0;
        READ_REG( CHIP_ID_NUMBER_19,&data,r16d16);
        if ( (data& 0xff00) == 0xdb00)
            DBM_DEBUG("Successfull switch to MCLK Clock\n");
        else
        {
            DBM_DEBUG("Cannot switch to MCLK CLOCK\n");
            return;
        }
    }

//    if (LDO_ENABLE && (tl3_out <= 150000000UL) )
//            D10L_SetLdoOutput( 800, 1200, FALSE);

    DBM_DEBUG("================= finish configure clocks  <<============== \n\n");
}

static uint16 D10L_TranslateAnalogRate(uint16 amic_freq)
{
    switch (amic_freq)
    {
        case 128:
            return SAR_IIR_FILTER_128;
        case 256:
            return SAR_IIR_FILTER_256;
        case 512:
            return SAR_IIR_FILTER_512;
        case 1024:
            return SAR_IIR_FILTER_1024;
        default:
            return SAR_IIR_FILTER_512;
    }
}

static uint16 D10L_TranslateDigitalRate(uint16 dmic_freq)
{
        switch (dmic_freq)
        {
            case 512:
                    return DM_CLK_FREQ_512_SR_8KHz_16KHz;
            case 768:
                    return DM_CLK_FREQ_768_SR_8KHz_16KHz;
            case 1024:
                    return DM_CLK_FREQ_1024_1040_SR_16KHz_32KHz_48KHz;
            case 1536:
                    return DM_CLK_FREQ_1536_1200_SR_8KHz_16KHz_32KHz_48KHz;
            case 2304:
                    return DM_CLK_FREQ_2304_2352_SR_16KHz_32KHz_48KHz;
            case 3072:
                    return DM_CLK_FREQ_3072_SR_16KHz_32KHz_48KHz;
            default:
                    return DM_CLK_FREQ_1024_1040_SR_16KHz_32KHz_48KHz;
        }
}

static void D10L_GetDefaultClockConfig(dspg_clock_config_t *cfg)
{
    cfg->input_clk = MASTER_CLOCK;
    cfg->switch_to_mclk = NO_SWITCH_TO_MCLK;
    cfg->switch_to_tdm_clk = NO_SWITCH_TO_TDM;
    cfg->use_pll_post_div = NO_USE_PLL_POST_DIV;
    cfg->analog_mic_freq = ANALOG_MIC_FREQ;
    cfg->digital_mic_freq = DIGITAL_MIC_FREQ;
    cfg->wanted_tl3_clk = 0;
    cfg->wanted_ahb_clk = NO_WANTED_AHB_CLK;
    cfg->wanted_apb_clk = NO_WANTED_APB_CLK;
    cfg->wanted_pll = NO_WANTED_PLL;

    cfg->analog_mic_freq = DEFAULT_AMIC_FREQ;
    cfg->digital_mic_freq = DEFAULT_DMIC_FREQ;
}

static void D10L_ConfigureTdmHw(uint8 tdm_number, uint32 tdm_addr, 
                                tdm_direction tdm_dir, uint16 bit_depth, 
                                tdm_mode mode, uint8 num_of_channels, bool clk_dly)
{
    uint32 num_of_clk_dly = 0;
    uint32 value = 0;

    num_of_clk_dly = clk_dly ? 0x10000000UL : 0;

    if (tdm_dir == TDM_RX)
    {
        if (bit_depth == 16)
        {
            value = D10L_ReadHwPort32 (tdm_addr);
            value = value | 0x15UL;
            D10L_WiteHwPort32( tdm_addr, value);
            D10L_WiteHwPort32((tdm_addr + 4), 0x7);
            D10L_WiteHwPort32((tdm_addr + 6), 0xf001fUL | num_of_clk_dly);
        }
        else
        {
            //  Config TDM input to 32bit
            if (num_of_channels == 2)
            {
                    value = D10L_ReadHwPort32(tdm_addr);
                    value = value | 0x4055UL;
                    D10L_WiteHwPort32( tdm_addr, value);
                    D10L_WiteHwPort32( (tdm_addr + 4), 0x2064UL);
                    D10L_WiteHwPort32( (tdm_addr + 6), 0x3f003fUL | num_of_clk_dly);
                    D10L_WiteHwPort32( (tdm_addr + 10), 0x5);
            //4 16 bit channels
            }else{
                    value = D10L_ReadHwPort32( tdm_addr);
                    value = value | 0x1dUL;
                    D10L_WiteHwPort32( tdm_addr, value);
                    D10L_WiteHwPort32( (tdm_addr + 4), 0x73007UL);
                    D10L_WiteHwPort32( (tdm_addr + 6), 0x1f003fUL | num_of_clk_dly);
            }
        }
    }else{
            if (bit_depth == 16){
                if (mode == TDM_SLAVE){
                        value = D10L_ReadHwPort32(tdm_addr);
                        value = value | 0x217;
                        D10L_WiteHwPort32( tdm_addr, value);
                        D10L_WiteHwPort32( (tdm_addr + 4), 0x7);
                        D10L_WiteHwPort32( (tdm_addr + 6), 0xf001f | num_of_clk_dly);
                }else{
                        value = D10L_ReadHwPort32(tdm_addr);
                        value = (value & 0x1f000000) | 0x800052;
                        D10L_WiteHwPort32( tdm_addr, value);
                        D10L_WiteHwPort32((tdm_addr + 4), 0x241024);
                        D10L_WiteHwPort32((tdm_addr + 6), 0x100f001f | num_of_clk_dly);
                        D10L_WiteHwPort32((tdm_addr + 10), 0xf);
                }
            }else{
                    if (mode == TDM_SLAVE){
                            value = D10L_ReadHwPort32(tdm_addr);
                            value = value | 0x425f;
                            D10L_WiteHwPort32( tdm_addr, value);
                            D10L_WiteHwPort32( (tdm_addr + 4), 0x64);
                            D10L_WiteHwPort32( (tdm_addr + 6), 0x1f003f | num_of_clk_dly);
                            D10L_WiteHwPort32( (tdm_addr + 10), 0x5);
                    }else{
                            value = D10L_ReadHwPort32(tdm_addr);
                            value = (value & 0x1f000000) | 0x804052;
                            D10L_WiteHwPort32( tdm_addr, value);
                            D10L_WiteHwPort32( (tdm_addr + 4), 0x641064);
                            D10L_WiteHwPort32( (tdm_addr + 6), 0x1f003f | num_of_clk_dly);
                            D10L_WiteHwPort32( (tdm_addr + 10), 0x55);
                    }
            }
    }

    DBM_DEBUG("Configured chip=%d tdm=%d %s to bits %d mode %d\n", tdm_number, tdm_dir?"tx":"rx", bit_depth, mode);
}

static void D10L_ResetTdm(uint16 tdm_no)
{
	WRITE_REG16( TDM_ACTIVATION_CONTROL_31, tdm_no);
	WRITE_REG16( TDM_TX_CONFIG_37, 0);
	WRITE_REG16( TDM_RX_CONFIG_36, 0);
}

static void D10L_ConfigMics(mic_types mic_1_type, mic_types mic_2_type, mic_types mic_3_type, uint16 dmic_freq, uint16 amic_freq)
{
    uint8 sleep = 10;
    uint16 mic1_synch_flag = 0, mic2_synch_flag = 0, mic3_synch_flag = 0;
    uint16 dig_mic_config, analog_mic_config;
    uint32 dm_cfg_val = 0;

    DBM_DEBUG("   REGULAR   config_mics  \n");

    // Other cases - continue here
    if (mic_3_type != MIC_NONE)
    {
            mic1_synch_flag = SYNCED_START;
            mic2_synch_flag = SYNCED_START;
            mic3_synch_flag = NO_SYNCED_START;
    }
    else if (mic_2_type != MIC_NONE)
    {
            mic1_synch_flag = SYNCED_START;
            mic2_synch_flag = NO_SYNCED_START;
    }
    else
         mic1_synch_flag = NO_SYNCED_START;

    dig_mic_config = DDF_AUDIO_ATTENUATION_6dB_GAIN | D10L_TranslateDigitalRate(dmic_freq);
    analog_mic_config = DDF_AUDIO_ATTENUATION_MINUS_6dB | D10L_TranslateAnalogRate(amic_freq);

    if (mic_1_type == MIC_DIGITAL)
    {
        //changing valid_delay value for DDF (DM0)
        dm_cfg_val = D10L_ReadHwPort32(DDF_CTL1);
        dm_cfg_val = ((dm_cfg_val & 0xFFFFFC00UL) | DDF_VALID_CLKS);
        D10L_WiteHwPort32( DDF_CTL1, dm_cfg_val);
        WRITE_REG(FIRST_MICROPHONE_CONFIG_24,	mic1_synch_flag						|
                                                                                    dig_mic_config						|
                                                                                    CLOCK_POLARITY_RISING_EDGE			|
                                                                                    D10L_DDF_AND_DM_CONFIG_DDF0_DM0,r16d16);
    }
    else if (mic_1_type == MIC_ANALOG)
    {
            WRITE_REG( FIRST_MICROPHONE_CONFIG_24,	mic1_synch_flag									|
                                                                                        analog_mic_config								|
                                                                                        D10L_DDF_AND_DM_CONFIG_SAR0_D10L_DDF_SAR_ADC,r16d16);
            DELAY( sleep);
    }

    if (mic_2_type != MIC_NONE)
    {
        if (mic_2_type == MIC_DIGITAL && mic_1_type == MIC_DIGITAL)
            WRITE_REG(SECOND_MICROPHONE_CONFIG_25, mic2_synch_flag	|
                                                                            dig_mic_config						|
                                                                            CLOCK_POLARITY_FALLING_EDGE			|
                                                                            D10L_DDF_AND_DM_CONFIG_DDF1_DM0,r16d16);

        if(mic_2_type == MIC_DIGITAL && mic_1_type == MIC_ANALOG)
            WRITE_REG( SECOND_MICROPHONE_CONFIG_25, dig_mic_config	|
                                                                            CLOCK_POLARITY_RISING_EDGE			|
                                                                            D10L_DDF_AND_DM_CONFIG_DDF0_DM0,r16d16);
        if(mic_2_type == MIC_ANALOG && mic_1_type == MIC_ANALOG)
            WRITE_REG( SECOND_MICROPHONE_CONFIG_25, analog_mic_config | D10L_DDF_AND_DM_CONFIG_SAR1_D10L_DDF_SAR_ADC,r16d16);
        if(mic_2_type == MIC_ANALOG && mic_1_type == MIC_DIGITAL)
            WRITE_REG( SECOND_MICROPHONE_CONFIG_25, analog_mic_config | D10L_DDF_AND_DM_CONFIG_SAR0_D10L_DDF_SAR_ADC,r16d16);

        if (mic_3_type != MIC_NONE)
        {
            if (mic_3_type == MIC_ANALOG)
                    WRITE_REG( THIRD_MICROPHONE_CONFIG_26, mic3_synch_flag								|
                                                                                             analog_mic_config								|
                                                                                             D10L_DDF_AND_DM_CONFIG_SAR0_D10L_DDF_SAR_ADC,r16d16);
            else if (mic_3_type == MIC_DIGITAL)
                    WRITE_REG( THIRD_MICROPHONE_CONFIG_26, mic3_synch_flag								|
                                                                                                dig_mic_config						|
                                                                                                CLOCK_POLARITY_RISING_EDGE			|
                                                                                                D10L_DDF_AND_DM_CONFIG_DDF2_DM1,r16d16);
        }
    }
}

static void D10L_CloseMics(int mic_1_type, int mic_2_type, int mic_3_type)
{
    // close 3nd mic before 2st mic
    if (mic_3_type != MIC_NONE)
    {
            if (mic_3_type == MIC_ANALOG)
                    WRITE_REG( THIRD_MICROPHONE_CONFIG_26, CLOSING_MICS_SAR_LOW_AMP,r16d16);
            else
                    WRITE_REG(THIRD_MICROPHONE_CONFIG_26, CLOSING_MICS_NO_DM_CLOCK,r16d16);

    }

    // close 2nd mic before 1st mic
    if (mic_2_type != MIC_NONE)
    {
        if (mic_2_type == MIC_ANALOG)
                WRITE_REG(SECOND_MICROPHONE_CONFIG_25, CLOSING_MICS_SAR_LOW_AMP,r16d16);
        else
                WRITE_REG(SECOND_MICROPHONE_CONFIG_25, CLOSING_MICS_NO_DM_CLOCK,r16d16);
    }

    if (mic_1_type == MIC_ANALOG)
        WRITE_REG(FIRST_MICROPHONE_CONFIG_24, CLOSING_MICS_SAR_LOW_AMP,r16d16);
    else
        WRITE_REG(FIRST_MICROPHONE_CONFIG_24, CLOSING_MICS_NO_DM_CLOCK,r16d16);

}

static uint16 D10L_VtStatusRegAddress (uint16 vt_num)
{
    uint16 vt_reg;

    if (vt_num == LOAD_ENGINE_TYPE_VT1)
        vt_reg = VT_REG_INIT + VT1_REGS_OFFSET;
    else if (vt_num == LOAD_ENGINE_TYPE_VT2)
        vt_reg = VT_REG_INIT + VT2_REGS_OFFSET;
    else if (vt_num == LOAD_ENGINE_TYPE_VT3)
        vt_reg = VT_REG_INIT + VT3_REGS_OFFSET;
    else
    {
        DBM_DEBUG("[%s]: DBMDx Bad D10L_VtStatusRegAddress.\n", __func__);
        vt_reg =0;
    }
    return vt_reg;
}

static void D10L_ConfigNsbf(uint16 vt_offset)
{
    if (SUPPORT_NSBF== NSBF_DUAL_INSTANCE)
    {
        WRITE_REG(  vt_offset | VT_REG_RECOG_MODE, 	VT_RECOG_MODE_DUAL_INSTANCES	|
                                                                VT_INST_EN_INST_1				|
                                                                VT_INST_EN_INST_2				|
                                                                VT_INST_ACTIVE_PAUSE_INST_1		|
                                                                VT_INST_ACTIVE_PAUSE_INST_2,r16d16);

        WRITE_REG(  vt_offset | VT_REG_DETECTION_DELAY, NSBF_CH_DLY,r16d16);

    }
    else if (SUPPORT_NSBF == NSBF_SINGLE_INSTANCE)
        WRITE_REG(  vt_offset | VT_REG_RECOG_MODE, 	VT_RECOG_MODE_DUAL_INSTANCES	|
                                                                VT_INST_EN_INST_1				|
                                                                VT_INST_ACTIVE_PAUSE_INST_1,r16d16);
    else
        WRITE_REG(  vt_offset | VT_REG_RECOG_MODE, 	VT_RECOG_MODE_SINGLE_INSTANCE	|
                                                                VT_INST_EN_INST_1				|
                                                                VT_INST_ACTIVE_PAUSE_INST_1,r16d16);
}

static void D10L_SetAsrpOutputScaleGain(int8 tx_output_gain, int8 vcpf_output_gain, int8 rx_output_gain)
{
    uint16 tx1_out_gain=0, tx2_out_gain=0, tx3_out_gain=0;

    if (tx_output_gain != 0)
    {
        if (tx_output_gain > 0)
            tx1_out_gain = 0x059f;  //TODO
        else if (tx_output_gain < 0)
            DBM_DEBUG("ASRP Scaling Value cannot be bellow 0!\n");
        else
        {
            DBM_DEBUG("ASRP Tx Output is Muted!\n");
            tx1_out_gain = tx_output_gain;
        }
        WRITE_REG16( ASRP_OUTPUT_GAIN_DEST_SEL_10A, ASRP_OUTPUT_DEST_TX_1);
        WRITE_REG16( ASRP_OUTPUT_GAIN_DEST_VALUE_10B, tx1_out_gain);
        DBM_DEBUG("ASRP Tx Output increased by %d dB\n", tx_output_gain);
    }

    if (vcpf_output_gain != 0)
    {
        if (vcpf_output_gain > 0)
            tx2_out_gain = 0x059f;
        else if (vcpf_output_gain < 0)
            DBM_DEBUG("ASRP Scaling Value cannot be bellow 0!\n");
        else
        {
            DBM_DEBUG("ASRP VCPF Output is Muted!\n");
            tx2_out_gain = vcpf_output_gain;
        }
        WRITE_REG16( ASRP_OUTPUT_GAIN_DEST_SEL_10A, ASRP_OUTPUT_DEST_TX_2);
        WRITE_REG16( ASRP_OUTPUT_GAIN_DEST_VALUE_10B, tx2_out_gain);
        DBM_DEBUG("ASRP VCPF Output increased by %d dB\n", vcpf_output_gain);
    }

    if (rx_output_gain != 0)
    {
        if (rx_output_gain > 0)
            tx3_out_gain = 0x0169;  //???
        else if (rx_output_gain < 0)
            DBM_DEBUG("ASRP Scaling Value cannot be bellow 0!\n");
        else
        {
            DBM_DEBUG("ASRP RX Output is Muted!\n");
            tx3_out_gain = rx_output_gain;
        }
        WRITE_REG16( ASRP_OUTPUT_GAIN_DEST_SEL_10A, ASRP_OUTPUT_DEST_TX_3);
        WRITE_REG16( ASRP_OUTPUT_GAIN_DEST_VALUE_10B, tx3_out_gain);
        DBM_DEBUG("ASRP Rx Output increased by %d dB\n", rx_output_gain);
    }
}

static void D10L_ConfigRcuWwe ( wake_engine wwe, uint8 cp_0,  uint8 cp_1)
{
    // config loaded engine
    // for this project: we support for now only 1 model (VT1). Not dual.
    int history_length, history_point;

    if((wwe == WWE_AMAZON) || (wwe == WWE_GOOGLE) || (wwe == WWE_CYBERON) || (wwe == WWE_RETUNE))
    {
        WRITE_REG16( VT_GENERAL_CFG_304,  		VT_NUM_1	|
                                                        VT_EN_EN	|
                                                        VT_ACTIVE	|
                                                        SET_IN1_CP	|  	//SET_IN1_CP	|
                                                        (cp_1 << 5)	|
                                                        SET_IN0_CP	|
                                                        cp_0);			//SET_IN0_CP);

        if(wwe == WWE_AMAZON)
        {
            WRITE_REG16( VT1_REGS_OFFSET | VT_REG_META_DATA_EN, 1);
        }
            //TODO
//        if (wwe == WWE_CYBERON)
//        {
//            WRITE_REG16( VT1_REGS_OFFSET | VT_REG_GROUP_ID, 1);
//            WRITE_REG16( VT1_REGS_OFFSET | VT_REG_CMD_ID, 1);
//            WRITE_REG16( VT1_REGS_OFFSET | VT_WORD_ID_STREAM_28, 1);
//        }

        if (wwe == WWE_AMAZON)
            history_length = EXT_HIST_TIME_500MS; // EXT_HIST_TIME_500MS
        else if (wwe == WWE_RETUNE){
            history_length = EXT_HIST_TIME_1500MS;
        }
        else if (wwe == WWE_CYBERON)
        {
        #ifndef CYBERON_OK_GOOGLE
            history_length = NO_EXT_HIST_TIME;
        #else
            history_length = EXT_HIST_TIME_200MS;
        #endif
        }
        else
            history_length = NO_EXT_HIST_TIME;

        // due to Retune engine report bug, we calculate the buffer from enfd of phrase back.
        if (wwe == WWE_RETUNE){
            history_point = HIST_POINT_IS_WW_END;
        }
        else if (wwe == WWE_CYBERON)
        {
            #ifndef CYBERON_OK_GOOGLE
                history_point = HIST_POINT_IS_WW_END;
            #else
                history_point = HIST_POINT_IS_WW_START;
            #endif
        }
        else
            history_point = HIST_POINT_IS_WW_START;

        // config history point
        WRITE_REG16( VT_AUDIO_HISTORY_CFG_305,  VT_NUM_1				|
                                                        history_length		|
                                                        history_point);

        // for gain normalization - config phrase length of engine
        if(wwe == WWE_GOOGLE)
        {
            int phrase_len = USER_CFG_WWE_LENGTH_600_MS;  // google
            WRITE_REG16( VT1_REGS_OFFSET | VT_REG_PHRASE_LENGTH_27,  phrase_len);
        }

        DELAY( 20);
    }
}

static void D10L_ConfigWwe (wake_engine wwe, uint8 cp)
{
    // config loaded engine
    uint16 vt_num = 0;
    uint16 vt_offset = 0;

    DBM_DEBUG("config_wwe = %d\n",wwe);
    if(wwe == WWE_AMAZON || ((wwe == WWE_DUAL) && (WWE_AMAZON == WAKE_ENGINE1)))
    {
        WRITE_REG16(VT_GENERAL_CFG_304,  		VT_NUM_1				|
                                                        VT_EN_EN				|
                                                        VT_ACTIVE				|
                                                        SET_IN0_CP				|
                                                        cp);

        DELAY( 20);

        WRITE_REG16(VT_AUDIO_HISTORY_CFG_305,  VT_NUM_1				|
                                                        EXT_HIST_TIME_500MS		|
                                                        HIST_POINT_IS_WW_START);
        DELAY( 20);

        if(wwe != WWE_DUAL)
            WRITE_REG16(VT_GENERAL_CFG_304,	VT_NUM_2				|
                                                        VT_EN_DIS);

    }

    if(wwe == WWE_GOOGLE || ((wwe == WWE_DUAL) && (WWE_GOOGLE == WAKE_ENGINE2)))
    {
        WRITE_REG16(VT_GENERAL_CFG_304,  		VT_NUM_2			|
                                                        VT_EN_EN			|
                                                        VT_ACTIVE			|
                                                        SET_IN0_CP			|
                                                        cp);

        DELAY( 20);
        WRITE_REG16(VT_AUDIO_HISTORY_CFG_305,  VT_NUM_2			|
                                                        HIST_POINT_IS_WW_START);
        DELAY( 20);
        //WRITE_REG16((VT_REG_PHRASE_LENGTH_27 + VT2_REGS_OFFSET),  USER_CFG_WWE_LENGTH_750_MS);
        WRITE_REG16(VT2_REGS_OFFSET | VT_REG_PHRASE_LENGTH_27,  USER_CFG_WWE_LENGTH_750_MS);

        if(wwe != WWE_DUAL)
            WRITE_REG16(VT_GENERAL_CFG_304,	VT_NUM_1			|
                                                        VT_EN_DIS);

    }
    if ((wwe == WWE_CYBERON) || (( wwe == WWE_DUAL) && (WWE_CYBERON == WAKE_ENGINE1)))
    {
        if(WWE_CYBERON== WAKE_ENGINE1){
            vt_num = VT_NUM_1;
            vt_offset = VT1_REGS_OFFSET;
        }
        if(WWE_CYBERON== WAKE_ENGINE2){
            vt_num = VT_NUM_2;
            vt_offset = VT2_REGS_OFFSET;
        }
        WRITE_REG16(VT_GENERAL_CFG_304, vt_num | VT_EN_EN | VT_ACTIVE | SET_IN1_CP | IN1_CP_NUM_1 | SET_IN0_CP | cp);  //304w3a30

        WRITE_REG16(VT_AUDIO_HISTORY_CFG_305,  vt_num | EXT_HIST_TIME_500MS | HIST_POINT_IS_WW_START);   //305w21f6

        WRITE_REG16(vt_offset| VT_REG_GROUP_ID, 1);  //065w0001
        //WRITE_REG16(vt_offset| VT_REG_CMD_ID, 1);
        if(wwe != WWE_DUAL)
          WRITE_REG16(VT_GENERAL_CFG_304, VT_NUM_2|VT_EN_DIS);     //304w4000
    }
    else if (wwe == WWE_NONE) {
        WRITE_REG16(VT_GENERAL_CFG_304,  		VT_NUM_GLOBAL		|
                                                        VT_EN_DIS);
    }
}

static uint16 D10L_ConfigureVadType(int vad_type)
{
    uint16 reg_23_cur_val, reg_23_val;

    READ_REG(GENERAL_CONFIGURATION_2_23,&reg_23_cur_val,r16d16);
    reg_23_val = (reg_23_cur_val & 0xFF9F);
    reg_23_val |= vad_type;

    return reg_23_val;
}

static void D10L_ConfigureEnvironmentAdaptation(bool enable)
{
  // config Environment Adaptation algorithm
  if ( (HW_VAD) || (LP_HW_VAD)){

    uint16 env_adaptation = (enable == 1)? ENABLE_ENV_ADAPTION : DISABLE_ENV_ADAPTAION ;

//		configure_vad_type(chip, FW_VAD_TYPE_D6_HW_VAD1);

    WRITE_REG16( VAD_MIC_SILENCE_CONFIG_1_341, 			SIL_LOW_THD);
    WRITE_REG16( VAD_MIC_SILENCE_CONFIG_2_342, 			SIL_HIGH_THD);
    WRITE_REG16( VAD_MIC_ACTIVE_PERIOD_CONFIG_343, 		LONG_ACTIVE_THD |
                                                                SHORT_ACTIVE_THD);
    WRITE_REG16( VAD_MIC_RTC_CONFIG_1_344, 				MID_RATE_FACTOR |
                                                            LOW_RATE_FACTOR |
                                                            RTC_PERIOD_BASE);
    WRITE_REG16( VAD_RMS_MU_348,					RMS_MU);

    //-------

    WRITE_REG16( VAD_MINTH_DELTA_UP_349,			MIN_TH_DELTA_UP);
    WRITE_REG16( VAD_MIN_TH_LOW_LIMIT_34A,			MIN_TH_LOW_LIMIT);
    WRITE_REG16( VAD_MIN_TH_HIGH_LIMIT_34B,			MIN_TH_HIGH_LIMIT);

    // the 2 next registered are instead of 345 in Vesper
    WRITE_REG16( VAD_ADAPT_RATE_LOW_TH_34C,			RTC_ADAPT_RATE_LOW_THD);
    WRITE_REG16( VAD_ADAPT_RATE_HIGH_TH_34D,		RTC_ADAPT_RATE_HIGH_THD);

    // 340 should be sent after last EA settings
    WRITE_REG16( VAD_MIC_GENERAL_CONFIG_340, 		SIL_TIME_THD_1000MS	|
                                                            env_adaptation);


    DELAY( 20);

   }
}

static int D10L_CalculateDigitalMicGain (int mic_gain)
{

    if (MIC_TYPE == 1) {
        if ((USE_HEXAGRAM_MICS == 1) && (!VESPER_MIC))
        { mic_gain += 7;
        }
    }
    return mic_gain*16;		//Naveh - we already calculating mic gain so better already to have the correct units 1/16 dB
}

static void D10L_SetHwVadGain(uint32 base_addr, uint32 db_mode)
{
    uint32 cfg1;

    cfg1 = D10L_ReadHwPort32( base_addr + CFG1);
    // set db_mode into bits 4-6
    D10L_WiteHwPort32(base_addr + CFG1,	(cfg1 | (db_mode << 4)) );

}

static void D10L_SetFwLogStatus(uint8 which_log)
{
    static uint8 current_log;
    DBM_DEBUG("[%s] which_log = %d (=%s)\n", __func__, which_log, which_log==2 ? "audio record" : (which_log==3 ? "debug log" : "no log"));

    if (current_log == which_log)
    {
        DBM_DEBUG("fw log status already updated: %d\n",which_log);
        return;
    }
    else
        WRITE_REG16(FW_DEBUG_REGISTER_18, TOGGLE_UART_DEBUG_PRINTS);
    current_log = which_log;
}

static void D10L_AsrpRecordTurnOn(uint16 asrp_record_channels1, uint16 asrp_record_channels2, uint16 record_command)
{
    DBM_DEBUG("asrp_record_turn_on. record command=%x ->\n", record_command);

    if (!DISABLE_ASRP_RECORD)
    {
        /* turn on ASRP recording */
        WRITE_REG16( ASRP_RECORDS_CHANNELS_128, asrp_record_channels1);
        WRITE_REG16( ASRP_RECORDS_CHANNELS_2_129, asrp_record_channels2);
    }

    /* add also fw record if requested */
    WRITE_REG16( REG_FW_RECORD_A_05, REG_5_FW_RECORD);
    WRITE_REG16( REG_FW_RECORD_B_06, g_reg6_fw_record);
    WRITE_REG16( REG_FW_RECORD_C_07, REG_7_FW_RECORD);
    WRITE_REG16( UART_DEBUG_RECORDING_30, record_command | CONFIGURE_REC_MASK);
    WRITE_REG16( UART_DEBUG_RECORDING_30, ENABLE_RECORDING);

    D10L_SetFwLogStatus(FW_AUDIO_LOG);
}

static void D10L_AsrpRecordTurnOff(void)
{
    DBM_DEBUG("asrp_record_turn_off->\n");
    /* tuning off ASRP recording */

    WRITE_REG16(ASRP_RECORDS_CHANNELS_128, 0x0);
    WRITE_REG16(ASRP_RECORDS_CHANNELS_2_129, 0x0);

    /* tuning off FW recording */
    WRITE_REG16(REG_FW_RECORD_A_05, 0x0000);
    WRITE_REG16(REG_FW_RECORD_B_06, 0x0000);
    WRITE_REG16(REG_FW_RECORD_C_07, 0x0000);
    WRITE_REG16(UART_DEBUG_RECORDING_30, 0x0000);
    WRITE_REG16(UART_DEBUG_RECORDING_30, CONFIGURE_REC_MASK | RELEASE_REC_BUFF);

    D10L_SetFwLogStatus(FW_NO_LOG);
}

// this function is called only for use cases without ASRP
static void D10L_FwRecordTurnOn(uint16 r5_fw_record)
{
	WRITE_REG16( REG_FW_RECORD_A_05, r5_fw_record);
    WRITE_REG16( REG_FW_RECORD_B_06, g_reg6_fw_record);
    WRITE_REG16( REG_FW_RECORD_C_07, REG_7_FW_RECORD);
    WRITE_REG16(VT_RECORD_CFG_306, REC_IN0_DATA | VT_NUM_1);

	WRITE_REG16(UART_DEBUG_RECORDING_30, ALLOCATE_BUFFER_IN_AHB_MEMORY | ENLARGE_BUFFER_BY_2 | CONFIGURE_REC_MASK);
	WRITE_REG16(UART_DEBUG_RECORDING_30, ENABLE_RECORDING);
}

// this function is called only for use cases without ASRP
static void D10L_FwRecordTurnOff(void)
{
	/* tuning off mic recording */
	WRITE_REG16(UART_DEBUG_RECORDING_30, 0x0000);
}

static void D10L_InitLpHwVad(int base_addr)
{
    D10L_WiteHwPort32( base_addr+SAD_CFG1,				0x1D);
    D10L_WiteHwPort32( base_addr+CFG2,					0x00000000);
    D10L_WiteHwPort32( base_addr+0x0004,				0x4d5804b0UL);//SAD_LPSD_TUNE
    D10L_WiteHwPort32( base_addr+0x0006,				0x00000000);//SAD_BUFF_POINT
    //D10L_WiteHwPort32( base_addr+SAD_INT_EN,			0x00000000);
    //D10L_WiteHwPort32( base_addr+SAD_STAT,				0x00000000);
    D10L_WiteHwPort32( base_addr+SAD_SAT_MAP,			0x00000000);
    D10L_WiteHwPort32( base_addr+SAD_IIR1_B,			0xC1233EDDUL);
    D10L_WiteHwPort32( base_addr+SAD_IIR1_A,			0x0000C245UL);
    D10L_WiteHwPort32( base_addr+SAD_IIR2_B,			0xC0004000UL);
    D10L_WiteHwPort32( base_addr+SAD_IIR2_A,			0x00000000);
    D10L_WiteHwPort32( base_addr+SAD_IIR3_B,			0x0000400);
    D10L_WiteHwPort32( base_addr+SAD_IIR3_A,			0x00000000);
    D10L_WiteHwPort32( base_addr+SAD_DEC1_MM_TH,		0x05140003UL);
    D10L_WiteHwPort32( base_addr+SAD_DEC2_MM_TH,		0x05140003UL);
    D10L_WiteHwPort32( base_addr+SAD_DEC3_MM_TH,		0x05140003UL);
    D10L_WiteHwPort32( base_addr+SAD_DEC1_SENS_TH,		0x00003700UL);
    D10L_WiteHwPort32( base_addr+SAD_DEC2_SENS_TH,		0x000025A5);
    D10L_WiteHwPort32( base_addr+SAD_DEC3_SENS_TH,		0x000025A5);
    D10L_WiteHwPort32( base_addr+SAD_FAST_ATTACK1,		0x3C3903C7UL);
    D10L_WiteHwPort32( base_addr+SAD_FAST_DECAY1,		0x33330CCDUL);
    D10L_WiteHwPort32( base_addr+ SAD_SLOW_COEFF1,		0x3FAE0052UL);
    D10L_WiteHwPort32( base_addr+SAD_FAST_ATTACK2,		0x3C3903C7UL);
    D10L_WiteHwPort32( base_addr+SAD_FAST_DECAY2,		0x33330CCDUL);
    D10L_WiteHwPort32( base_addr+SAD_SLOW_COEFF2,		0x3FDA0026UL);
    D10L_WiteHwPort32( base_addr+SAD_FAST_ATTACK3,		0x3C3903C7UL);
    D10L_WiteHwPort32( base_addr+ SAD_FAST_DECAY3,		0x33330CCDUL);
    D10L_WiteHwPort32( base_addr+SAD_SLOW_COEFF3,		0x3FAE0052UL);
    D10L_WiteHwPort32( base_addr+SAD_LEN_SCALE,		0x00EF0089UL);
}

static bool D10L_LoadModel(const source_t file,uint16 model_type, uint16 mem_loc, uint16 vt_num)
{
    const uint8* ptr = file.data;
    char   header_buffer[MODEL_OR_ASRP_HEADER_LENGTH];
    uint32 num_of_words = 0;
    uint32 cur_ptr = 0;
    uint8 skip_bytys = 2;

    WRITE_REG(  LOAD_BINARY_FILE_0F, 	vt_num					|
                                                            OP_TYPE_LOAD_FILE		|
                                                            FILE_LOAD_INIT_OR_KILL_REL			|
                                                            BLK_START_NUM_1			|
                                                            mem_loc					|
                                                            model_type	,r16d16);
    DELAY(10);

    do
    {
        memcpy(header_buffer, ptr, MODEL_OR_ASRP_HEADER_LENGTH);
        ptr += MODEL_OR_ASRP_HEADER_LENGTH;
        cur_ptr += MODEL_OR_ASRP_HEADER_LENGTH;

        if(header_buffer[0] != 0x5A && (header_buffer[1] != 0x01 || header_buffer[1] != 0x02))
        {
            DBM_DEBUG("Load model file fail in %s, because of header error",__func__);
            return FALSE;
        }
        WRITE(header_buffer,MODEL_OR_ASRP_HEADER_LENGTH);
        DELAY(10);
        num_of_words =(uint8)header_buffer[5] * 0x1000000UL + (uint8)header_buffer[4] * 0x10000UL +(uint8)header_buffer[3] * 0x100UL + (uint8)header_buffer[2];
        WRITE(ptr,num_of_words*2);
        DELAY(10);
        ptr += num_of_words*2;
        cur_ptr += num_of_words*2;

        if(cur_ptr+skip_bytys == file.data_size)
            cur_ptr +=skip_bytys;//we send 0x5a 0x0b
    } while (cur_ptr < file.data_size);

    D10L_RunChip();
    DELAY(10);

    uint16 val;
    READ_REG(D10L_VtStatusRegAddress(vt_num),&val,r16d16);
    if(!val)
    {
        DBM_DEBUG("Acoustic model was not successfully uploaded %d",val);
        return FALSE;
    }
    else
        DBM_DEBUG("Acoustic model was successfully uploaded");

    return TRUE;
}

static bool D10L_LoadAsrp(const source_t file,uint16 asrp_model_type, uint16 mem_loc, uint16 load_type)
{
    DBM_DEBUG("--D10L_LoadAsrp, load asrp data to dspg chip");
    DELAY(10);

    uint16 blk_start = (asrp_model_type == FILE_TYPE_DTE_LOAD_TO_PREALLOC)? BLK_START_NUM_1 : BLK_START_NUM_0;

        WRITE_REG( LOAD_BINARY_FILE_0F, 	load_type 			|
                                                    OP_TYPE_LOAD_FILE	|
                                                    FILE_LOAD_INIT_OR_KILL_REL			|
                                                    blk_start			|
                                                    mem_loc 			|
                                                    asrp_model_type	,r16d16);

    WRITE_REG16( FW_STATUS_INDEX_0D, 4);
    WRITE_REG16( FW_STATUS_INDEX_0D, 3);

    DELAY(20);
    //write data
    WRITE( file.data, file.data_size-2);
    DELAY(10);
    D10L_RunChip();
    DELAY(10);

    uint32 value[3];
    READ_REG(ASRP_LIB_VER_100, &value[0],r16d16);
    DBM_DEBUG("ASRP Loaded! lib Version: %x", value[0]);
    if (value[0] == 0)
    {
            DBM_DEBUG ("ASRP Params file failed to load\n");
            return FALSE;
    }
    else
    {
        if (asrp_model_type != FILE_TYPE_DTE_LOAD_TO_PREALLOC)
        {
            READ_REG(ASRP_TUNING_VER_101,&value[1],r16d16);
            READ_REG(ASRP_NUM_OF_ERRS_104,&value[2],r16d16);
            DBM_DEBUG("ASRP Loaded! lib Version: %x.%x.%x", value[0],value[1],value[2]);
        }
    }

    return TRUE;
}

static void D10L_SetGpioDir(int gpio, int direction)
{
    int32_t value, reg;
    int32_t iom;
    int gpio_offset;

    if (gpio < 15){
        iom = IOM1;
        gpio_offset = gpio;
    } else {
        iom = IOM2;
        gpio_offset = gpio-16;
    }
    value = D10L_ReadHwPort32(iom);
    value = value & (~(1<<(gpio_offset*2))) & (~(1<<((gpio_offset*2)+1))); // 2 bits per gpio
    D10L_WiteHwPort32(iom, value);

    reg = (direction==OUT) ? GPIO_DIR_OUT : GPIO_DIR_IN;
    D10L_WiteHwPort32( reg, (1<<gpio));

//	DBM_DEBUG("[%s] GPIO %d configured to direction %s\n", __func__, gpio, direction ? "OUT" : "IN");
    return ;
}

static void D10l_ConfigBuzzer(void)
{
    if(BUZZER_GPIO != BUZZER_NO_GPIO)
        D10L_SetGpioDir( BUZZER_GPIO, OUT);
}

static void D10l_SetDcValue(uint32 sar_cfg_addr, uint32 resistor_value)
{
	uint32 value;
		if (resistor_value == 180) {
			value = D10L_SAR_DC_FOR_180K_RES;
		} else if (resistor_value == 240) {
			value = D10L_SAR_DC_FOR_240K_RES;
		} else {
			DBM_DEBUG("[%s]: Resistor value is not supported for this chip type!\n", __func__);
			return;
		}

	D10L_WiteHwPort32(sar_cfg_addr, value);
}

static void D10L_InitConfigChip(void)
{
    uint16 host_interface;
    uint16 fw_ready_enable;
    uint16 max_number_of_mics;
    uint16 split;
    uint32 wanted_tl3_clk;
    uint16 r15,interrupt_events;
    dspg_clock_config_t clk_cfg;
    comm_inf_t c_type = DSPg_GetCommType();

    D10L_GetDefaultClockConfig(&clk_cfg);

    DBM_DEBUG("--D10L_InitConfigChip");
    if (c_type == SPI)
      host_interface = SPI_D2_TDM1_D4_GPIO_4_5_6_7_D10_GPIO_2_3_4_5;
    else if (c_type == I2C)
      host_interface = I2C_D2_ON_I2C_SDA_SCK_D4_D6_GPIO_1_2_D10_GPIO_3_4;
    else if (c_type == UART)
      host_interface = UART0_ENABLE_FOR_HOST_D2_TDM2_D4_D6_GPIO_17_18_D10_GPIO_2_5;
    else
    {
      DBM_DEBUG ("DBMDx interface not choosen.");
      return;
    }

    // Due to FW bug in FW Master branch, we skip reg 29 in case of boot from flash
    if (!BOOT_FROM_FLASH)
      WRITE_REG(HOST_INTERFACE_SUPPORT_29, host_interface | UART_DEBUG_MODE_0 | UART_DEBUG_GPIO,r16d16);

//    WRITE_REG16( UART_BAUD_RATE_0C, CHANGE_UART_BAUDRATE | UART1_UART_BAUD_RATE_3_Mhz);

    READ_REG(DETECTION_AND_SYSTEM_EVENTS_STATUS_14,&interrupt_events,r16d16);
    WRITE_REG( DETECTION_AND_SYSTEM_EVENTS_STATUS_14, interrupt_events,r16d16);
    DBM_DEBUG("--DETECTION_AND_SYSTEM_EVENTS_STATUS_14 0x%x",interrupt_events);

//    WRITE_REG(FW_DEBUG_REGISTER_18,TOGGLE_UART_DEBUG_PRINTS ,r16d16);

    //if  in optimized mode, turn off log output
    if(FW_MODE == MODE_OPT)
        WRITE_REG16(FW_DEBUG_REGISTER_18, TOGGLE_UART_DEBUG_PRINTS);

    WRITE_REG16( EVENTS_INDICATIONS_EN_CFG_12,	VT1_DET			|
														VT2_DET			|
														VT3_DET			|
														AEP_DET			|
	// future for DTK									SENSOR_DET		|
														WARNING_EVENT	|
														ERROR_EVENT);

    fw_ready_enable = EN_DIS_READY_GPIO | READY_GPIO_POLARITY | READY_GPIO_NUM;

    r15 = 	EN_DIS_EVENTS_GPIO	|
          EVENTS_GPIO_POLARITY	|
          EVENTS_GPIO_NUM 		|
          fw_ready_enable;

    WRITE_REG(EVENTS_INDICATION_GPIO_15,r15,r16d16);

    D10L_SetMasterClock(MASTER_CLOCK);

    // reg 9 should be configured before reg 22
    WRITE_REG(AUDIO_BUFFER_SIZE_09, AUDIO_BUFFER_SIZE,r16d16);

    // Set register 22 (once) according to max number of mics
    split = SPLIT_MIC_BUFFER;		// we set it now also for 1 mic
    if (NO_OF_MICS == 1)
    {
        max_number_of_mics = MAX_NUMBER_OF_MIC_IS_0_OR_1;
        split = NO_SPLIT_MIC_BUFFER;
    }
    else if (NO_OF_MICS == 2 )
        max_number_of_mics = MAX_NUMBER_OF_MIC_IS_2;
    else
        max_number_of_mics = MAX_NUMBER_OF_MIC_IS_4;

    //General Congfiguration (once)
    WRITE_REG(GENERAL_CONFIGURATION_1_22, 	split				|
                                                      DSP_CLK_GEN_PLL		|
                                                      max_number_of_mics,r16d16);
    D10L_InitAdditionalSequence(INIT_SEQUENCE);

    // Memory Configuration
    if (OPTIMIZED && (FW_MODE != MODE_RECORD))
      wanted_tl3_clk = 73728UL;
    else
      wanted_tl3_clk = 110000UL;

    // Disabe LDO if required. Other LDO setting - inside configure_clocks
    if (LDO_ENABLE == 0)
      D10L_SetLdoOutput(880, 1200,TRUE);	// Close LDO
    else
      D10L_SetLdoOutput(800, 1200,FALSE);

    clk_cfg.wanted_pll = 102001UL;
    clk_cfg.wanted_tl3_clk = wanted_tl3_clk;
    clk_cfg.digital_mic_freq = 1536;
    clk_cfg.analog_mic_freq = 1024;

    D10L_ConfigureClocks(clk_cfg);

    // initial debug UART baudrate after init:
    if (OPTIMIZED && (FW_MODE != MODE_RECORD))
        WRITE_REG(UART_BAUD_RATE_0C, CHANGE_UART_BAUDRATE | UART1_UART_BAUD_RATE_1_Mhz,r16d16);
    else
        WRITE_REG(UART_BAUD_RATE_0C, CHANGE_UART_BAUDRATE | UART1_UART_BAUD_RATE_3_Mhz,r16d16);

    if ((MIC_TYPE == MIC_ANALOG)) 
    {
		WRITE_REG( HIGH_PASS_FILTER_1A, DC_REMOVE_COARSE_EN |IIR_HPF_EN,r16d16);

		D10l_SetDcValue( D10L_SAR_0_CFG_2_ADDR, 180);
		if (NO_OF_MICS > 1) 
			D10l_SetDcValue( D10L_SAR_1_CFG_2_ADDR, 180);
		
	} else 
		WRITE_REG( HIGH_PASS_FILTER_1A, DC_REMOVE_COARSE_EN |IIR_HPF_EN,r16d16);
     
    D10L_WiteHwPort32( D10_AFE_CTRL2_ADDR, 0x1880); 

    D10L_ConfigMics(MIC_TYPE, NO_OF_MICS > 1 ? MIC_TYPE : 0 , NO_OF_MICS > 2 ? MIC_TYPE : 0, ANALOG_MIC_FREQ, DIGITAL_MIC_FREQ);
    DELAY(10); // TBD
    D10L_CloseMics(MIC_TYPE, NO_OF_MICS > 1 ? MIC_TYPE : 0, NO_OF_MICS > 2 ? MIC_TYPE : 0);
    WRITE_REG(AUDIO_ROUTING_CONFIGURATION_1F,	MUSIC_IN_TDM0 | TDM0_3V3 | TDM1_3V3 ,r16d16);

    DBM_DEBUG("[%s]: FINISH INIT CONFIG CHIP\n", __func__);
}

trigger_word_t D10L_VoiceTrigger(uint32 interrupt_events)
{
    uint16 word_id = D10L_PhraseTriggerData(interrupt_events);
    if (word_id < 2000)
    {
        // word_id -= 1001;
        DBM_DEBUG("Trigger ! - word_id %d\n", word_id);
    }
    else 
    {
//        word_id -= 2001;
        word_id -= 2000;
        DBM_DEBUG("Detecting Command \n");
        DELAY(100);
        WRITE_REG16(OPERATION_MODE_01,IDLE_MODE);
        WRITE_REG16(OPERATION_MODE_01,DETECTION_MODE);
    }

    return  word_id;
}

void D10L_AfterPowerUp(void)
{
    D10L_InitConfigChip();
    // verify again chip1 is alive by read version
    D10L_ReadVersion();
}

// Preparations before VT loading, and call Load_A_Model
void D10L_ModelLoading(const source_t file, wake_engine wwe)
{
    uint16 val;
    DBM_DEBUG("enter model_loading -->  wwe=%d  wwe_type = %s\n", wwe, (NNL_FW)? "NNL" : "NON NNL");

#if AHB_DTCM_DUMP
  WRITE_REG16( FW_STATUS_INDEX_0D, 3);
  WRITE_REG16( FW_STATUS_INDEX_0D, 4);
#endif

    if (wwe == WWE_AMAZON)
    {
        // If only amazon is required, erase (kill) previous vt2 model if already loaded:
        READ_REG(D10L_VtStatusRegAddress(LOAD_ENGINE_TYPE_VT2),&val,r16d16);
        if ((val != 0) && (val != 0xEEEE))
        {
            DBM_DEBUG("VT2 Model is Loaded but not needed - Releasing\n");
            WRITE_REG( VT_GENERAL_CFG_304, VT_NUM_2	| VT_EN_DIS,r16d16);
            WRITE_REG( LOAD_BINARY_FILE_0F, LOAD_ENGINE_TYPE_VT2			|
                                                                    OP_TYPE_RELEASE_FILE	|
                                                                    FILE_LOAD_INIT_OR_KILL_REL			|
                                                                    FILE_TYPE_DTE_PRIM_MODEL,r16d16);
        }

        D10L_ConfigNsbf( VT1_REGS_OFFSET);
        if (AMAZON_SW_VAD_ENABLE){
            WRITE_REG( VT1_REGS_OFFSET | VT_RECOG_MODE_LPSD_VAD, 1,r16d16);
        }

        // verify vt1 is not loaded already
        READ_REG(D10L_VtStatusRegAddress(LOAD_ENGINE_TYPE_VT1),&val,r16d16);
        if (val == 0){
            DBM_DEBUG("VT1 not initialized, assuming no model loaded. loading model\n");
            if (NNL_FW){
                D10L_LoadModel( file, FILE_TYPE_PRIM_AM_FIRST_BLOCK, MODEL1_LOCATION, LOAD_ENGINE_TYPE_VT1);
            } else {
                D10L_LoadModel(  file, FILE_TYPE_DTE_PRIM_MODEL, MODEL1_LOCATION, LOAD_ENGINE_TYPE_VT1);
            }
        }
        else{
            DBM_DEBUG("VT1 (AMAZON) already loaded, skip loading\n");
        }
    } else if (wwe == WWE_GOOGLE)
    {
        if (WWE_GOOGLE == WAKE_ENGINE1)
        { // load google to VT1 as a single model
            WRITE_REG( VT_GENERAL_CFG_304, VT_NUM_2 | VT_EN_DIS,r16d16);	 // disable other VT
            D10L_ConfigNsbf( VT1_REGS_OFFSET);
            // verify vt1 is not loaded already
            READ_REG(D10L_VtStatusRegAddress(LOAD_ENGINE_TYPE_VT1),&val,r16d16);
            if (val == 0){
                DBM_DEBUG("VT2 not initialized, assuming no model loaded. loading model\n");
                D10L_LoadModel( file,  FILE_TYPE_DTE_PRIM_MODEL, MODEL1_LOCATION, LOAD_ENGINE_TYPE_VT1);
            }
            else
                DBM_DEBUG("VT1 (GOOGLE) already loaded, skip loading\n");

        } else if (WWE_GOOGLE == WAKE_ENGINE2){ // load google to VT2 as a single model

            WRITE_REG( VT_GENERAL_CFG_304, VT_NUM_1 | VT_EN_DIS,r16d16);	 // disable other model

            // verify vt2 is not loaded already
            READ_REG(D10L_VtStatusRegAddress(LOAD_ENGINE_TYPE_VT2),&val,r16d16);
            if (val== 0){
                DBM_DEBUG("VT2 not initialized, assuming no model loaded. loading model\n");
                D10L_LoadModel( file, FILE_TYPE_DTE_PRIM_MODEL, MODEL2_LOCATION, LOAD_ENGINE_TYPE_VT2);
            }
            else{
                DBM_DEBUG("VT2 (GOOGLE) already loaded, skip loading\n");
            }
        }
    }
    else if (wwe == WWE_CYBERON)
    {
        // D10L_ConfigNsbf( VT1_REGS_OFFSET);

        // If only cyberon is required, erase (kill) previous vt2 model if already loaded:
        READ_REG(D10L_VtStatusRegAddress(LOAD_ENGINE_TYPE_VT2),&val,r16d16);
        if ((val != 0) && (val != 0xEEEE))
        {
            DBM_DEBUG("VT2 Model is Loaded but not needed - Releasing\n");
            WRITE_REG16(VT_GENERAL_CFG_304, VT_NUM_2	|
                                                     VT_EN_DIS);
            WRITE_REG16(LOAD_BINARY_FILE_0F, LOAD_ENGINE_TYPE_VT2			|
                                                                    OP_TYPE_RELEASE_FILE	|
                                                                    FILE_LOAD_INIT_OR_KILL_REL			|
                                                                    FILE_TYPE_DTE_PRIM_MODEL);
        }

        val = 0;
        // verify vt1 is not loaded already
        READ_REG(D10L_VtStatusRegAddress(LOAD_ENGINE_TYPE_VT1),&val,r16d16);
        if (val== 0)
        {
            DBM_DEBUG("VT1 not initialized, assuming no model loaded. loading model\n");
            WRITE_REG( VT1_REGS_OFFSET| VT_REG_ACTIVATE_NUM_GROUPS , VT_ACTIVATE_GROUP_1,r16d16);   //TODO log show the parameter is 2

            if (NNL_FW)
                WRITE_REG( VT1_REGS_OFFSET| VT_REG_ACTIVATE_MODEL_TYPE, VT_ACTIVATE_MODEL_NNL,r16d16);
            else
                WRITE_REG( VT1_REGS_OFFSET| VT_REG_ACTIVATE_MODEL_TYPE, VT_ACTIVATE_MODEL_DSP,r16d16);

            // WRITE_REG( VT1_REGS_OFFSET| VT_REG_PARAM_TIMEOUT, 5,r16d16);// 1 MS after command - back to detection

            DELAY(100);

            if(!NNL_FW)
                  D10L_LoadModel( file,  FILE_TYPE_DTE_PRIM_MODEL, MODEL1_LOCATION, LOAD_ENGINE_TYPE_VT1);
            else
                  D10L_LoadModel( file,  FILE_TYPE_DTE_PRIM_MODEL, MODEL1_LOCATION, LOAD_ENGINE_TYPE_VT1);
        }
        else
            DBM_DEBUG("VT1 (CYBERON) already loaded, skip loading\n");

    }
    else if (wwe == WWE_RETUNE){

        // verify vt1 is not loaded already
        READ_REG(D10L_VtStatusRegAddress(LOAD_ENGINE_TYPE_VT1),&val,r16d16);
        if (val== 0){

            DBM_DEBUG("VT1 not initialized, assuming no model loaded. loading model\n");

            D10L_ConfigNsbf( VT1_REGS_OFFSET);

            D10L_LoadModel( file,  FILE_TYPE_DTE_PRIM_MODEL, MODEL1_LOCATION, LOAD_ENGINE_TYPE_VT1);
        }
        else{
            DBM_DEBUG("VT1 (RETUNE) already loaded, skip loading\n");
        }

    } else if (wwe == WWE_NONE){

        READ_REG(D10L_VtStatusRegAddress(LOAD_ENGINE_TYPE_VT1),&val,r16d16);
        if (val!= 0)
        {
            WRITE_REG( VT_GENERAL_CFG_304, VT_NUM_1	|VT_EN_DIS,r16d16);
            DBM_DEBUG("VT1 Model - Releasing\n");
            WRITE_REG( LOAD_BINARY_FILE_0F, LOAD_ENGINE_TYPE_VT1	|
                                                                    OP_TYPE_RELEASE_FILE			|
                                                                    FILE_LOAD_INIT_OR_KILL_REL					|
                                                                    FILE_TYPE_DTE_PRIM_MODEL,r16d16);
        }
        READ_REG(D10L_VtStatusRegAddress(LOAD_ENGINE_TYPE_VT2),&val,r16d16);
        if (val!= 0)
        {
        WRITE_REG( VT_GENERAL_CFG_304, VT_NUM_2	|VT_EN_DIS,r16d16);
        DBM_DEBUG("VT2 Model - Releasing\n");
        WRITE_REG( LOAD_BINARY_FILE_0F, LOAD_ENGINE_TYPE_VT2			|
                                                    OP_TYPE_RELEASE_FILE		|
                                                    FILE_LOAD_INIT_OR_KILL_REL				|
                                                    FILE_TYPE_DTE_PRIM_MODEL,r16d16);
        }
    }
}

// This function supports exit from uc2 and uc9 (RCU NR)
void D10L_UsecaseNrExit(bool keep_asrp_and_model_loaded)
{
    dspg_clock_config_t clk_cfg;
    uint16 reg_23;

    DBM_DEBUG("[%s]: Start. keep asrp and model files? %d\n", __func__, keep_asrp_and_model_loaded);

    WRITE_REG16(OPERATION_MODE_01,IDLE_MODE);
    DELAY(130);

    if (HW_VAD_ENABLE_PRE_ROLL_EXTENTION)
    {
        // Close pre roll extention BEFORE ASRP release memory
        READ_REG(GENERAL_CONFIGURATION_2_23,&reg_23,r16d16);
        WRITE_REG16( GENERAL_CONFIGURATION_2_23, (reg_23 & CLOSE_PRE_ROLL_EXTENTION));
    }

    WRITE_REG16(AUDIO_PROCESSING_CONFIGURATION_34,0x0);

    if (!keep_asrp_and_model_loaded)
        WRITE_REG16( LOAD_BINARY_FILE_0F,  	LOAD_ENGINE_TYPE_ASRP		|
                                                OP_TYPE_RELEASE_FILE		|
                                                FILE_LOAD_INIT_OR_KILL_REL	|
                                                FILE_TYPE_ASRP_PARAMS_PRIM_INIT);

    else
    {
    // disable and reinit asrp file for next time, do not erase the file
        WRITE_REG16( LOAD_BINARY_FILE_0F,  	LOAD_ENGINE_TYPE_ASRP		|
                                                OP_TYPE_RELEASE_FILE		|
                                                FILE_INIT_OR_KILL_ONLY		|	// do not release file
                                                FILE_TYPE_ASRP_PARAMS_PRIM_INIT);

    // activate primary ASRP init now, for next usage (ASRP is still not enabled)
        WRITE_REG16( LOAD_BINARY_FILE_0F,	LOAD_ENGINE_TYPE_ASRP	|
                                                OP_TYPE_LOAD_FILE		|
                                                FILE_INIT_OR_KILL_ONLY	|
                                                BLK_START_NUM_0			|
                                                FILE_TYPE_ASRP_PARAMS_PRIM_INIT);

    }
    WRITE_REG16( VT_GENERAL_CFG_304, VT_NUM_GLOBAL | VT_EN_DIS);

    // clean 23 HW VAD only after MICs are closed. TBD
    WRITE_REG16( GENERAL_CONFIGURATION_2_23, FW_VAD_TYPE_NO_VAD | MIC_SAMPLE_RATE_16K);

    if(FW_MODE == MODE_RECORD)
        D10L_AsrpRecordTurnOff();

    D10L_CloseMics(MIC_TYPE, NO_OF_MICS > 1 ? MIC_TYPE : 0, NO_OF_MICS > 2 ? MIC_TYPE : 0);

    if (!keep_asrp_and_model_loaded)
        D10L_ConfigWwe(WWE_NONE, 0);

    D10L_GetDefaultClockConfig(&clk_cfg);
    clk_cfg.wanted_tl3_clk = 102000;
    D10L_ConfigureClocks (clk_cfg);


    DBM_DEBUG("[%s]: EXIT USE CASE NR Dual Mic\n", __func__);
}

void D10L_UsecaseLpExit(void)
{
    dspg_clock_config_t clk_cfg;
	WRITE_REG16( OPERATION_MODE_01, IDLE_MODE);
	DELAY(80);		// may be reduced according to frame size

	if(FW_MODE == MODE_RECORD)
    {
        D10L_FwRecordTurnOff();  // record only fw (mic 1 only), no ASRP
    }

	D10L_CloseMics(MIC_TYPE, NO_OF_MICS > 1 ? MIC_TYPE : 0, NO_OF_MICS > 2 ? MIC_TYPE : 0);
	WRITE_REG16( AUDIO_PROCESSING_CONFIGURATION_34, 0x0000);

  if( VESPER_VAD)
	  WRITE_REG16( GENERAL_CONFIGURATION_2_23,FW_VAD_TYPE_FW_OR_VESPER |
													MIC_SAMPLE_RATE_16K);
  else
	  WRITE_REG16( GENERAL_CONFIGURATION_2_23,
													MIC_SAMPLE_RATE_16K);
	WRITE_REG16( AUDIO_ROUTING_CONFIGURATION_1F, MUSIC_IN_TDM0);

	// Disable model
	D10L_ConfigWwe(WWE_NONE, 0);

    D10L_GetDefaultClockConfig(&clk_cfg);
    clk_cfg.wanted_tl3_clk = 102001;
    D10L_ConfigureClocks (clk_cfg);

	// config_uart_speed(chip, CHANGE_UART0_BAUDRATE | UART0_D10_UART_BAUD_RATE_1_Mhz, 1000000);


	DBM_DEBUG("[%s]: EXIT USE CASE Low Power Mode\n", __func__);
}

void D10L_UsecaseBrageInExit(void)
{
    dspg_clock_config_t clk_cfg;

    WRITE_REG16( TDM_ACTIVATION_CONTROL_31, CONFIG_TDM_1);
	WRITE_REG16( TDM_ACTIVATION_CONTROL_31, CONFIG_TDM_0);

    D10L_GetDefaultClockConfig(&clk_cfg);
    clk_cfg.switch_to_tdm_clk = NO_SWITCH_TO_TDM;
    clk_cfg.switch_to_mclk = I2S_CHIP_MASTER? NO_SWITCH_TO_MCLK:SWITCH_TO_MCLK;
    clk_cfg.wanted_tl3_clk = 110592;
    D10L_ConfigureClocks (clk_cfg);
    DELAY(40);

    D10L_ResetTdm( CONFIG_TDM_1);			// for voice call Rx process
	D10L_ResetTdm( CONFIG_TDM_0);
    DELAY(40);

	if(FW_MODE == MODE_RECORD)
    {
        g_reg6_fw_record=0;
        D10L_FwRecordTurnOff();  // record only fw (mic 1 only), no ASRP
    }

    WRITE_REG16( AUDIO_PROCESSING_CONFIGURATION_34, 0x0);

    WRITE_REG16( AUDIO_STREAMING_SOURCE_SELECTION_13,   NO_STREAM_CH_4  |
                                                            NO_STREAM_CH_3  |
                                                            NO_STREAM_CH_2  |
                                                            NO_STREAM_CH_1);

	WRITE_REG16( LOAD_BINARY_FILE_0F, LOAD_ENGINE_TYPE_ASRP			|
												OP_TYPE_RELEASE_FILE		|
												FILE_LOAD_INIT_OR_KILL_REL	|
												FILE_TYPE_ASRP_PARAMS_PRIM_INIT);

    if(VESPER_VAD)
        WRITE_REG16(GENERAL_CONFIGURATION_2_23, FW_VAD_TYPE_FW_OR_VESPER | MIC_SAMPLE_RATE_16K);	// VAD command
    else
        WRITE_REG16(GENERAL_CONFIGURATION_2_23,	FW_VAD_TYPE_NO_VAD	|
                                                        MIC_SAMPLE_RATE_16K);

	D10L_CloseMics(MIC_TYPE, NO_OF_MICS > 1 ? MIC_TYPE : 0, NO_OF_MICS > 2 ? MIC_TYPE : 0);

	// Disable model
	D10L_ConfigWwe(WWE_NONE, 0);

	DBM_DEBUG("[%s]: EXIT USE CASE Barge In\n", __func__);
}


bool D10L_UsecaseBargeInEnter(const source_t file,wake_engine wwe)
{
    DBM_DEBUG("[%s]: Start.\n", __func__);

    bool optimized_case =OPTIMIZED && (FW_MODE != MODE_RECORD);
    dspg_clock_config_t clk_cfg;

    D10L_GetDefaultClockConfig(&clk_cfg);

    if (MIC_TYPE == MIC_ANALOG)
    {
        clk_cfg.analog_mic_freq = 1024;
        clk_cfg.digital_mic_freq = 1;
    }
    else
    {
        clk_cfg.analog_mic_freq = 1;
        clk_cfg.digital_mic_freq = 1536;		// temporary
    }

    if (optimized_case)
    {
        if (wwe == WWE_RETUNE)
        {
            clk_cfg.wanted_pll  = NO_WANTED_PLL;
            clk_cfg.wanted_tl3_clk = 60000;
        }
        else if (wwe == WWE_CYBERON)
        {
            // optimized cyberon
            clk_cfg.wanted_pll  = NO_WANTED_PLL;
            #ifdef CYBERON_OK_GOOGLE
            clk_cfg.wanted_tl3_clk = 65000;
            #else
            clk_cfg.wanted_tl3_clk = 98000;
            #endif
        }
        else if (wwe == WWE_GOOGLE)
        {
            // optimized Google
            clk_cfg.wanted_pll  = NO_WANTED_PLL;
            clk_cfg.wanted_tl3_clk = 50000;
        }
        else
        {
            // optimized Amazon
            clk_cfg.wanted_pll  = NO_WANTED_PLL;
            clk_cfg.wanted_tl3_clk = 50000;
        }

    }
    else
    {	// non optimized
        clk_cfg.wanted_pll  = 250000;
        clk_cfg.wanted_tl3_clk = clk_cfg.wanted_pll;
        clk_cfg.wanted_ahb_clk = clk_cfg.wanted_tl3_clk;
        clk_cfg.wanted_apb_clk = clk_cfg.wanted_ahb_clk;
    }

    DBM_DEBUG("wanted_tl3_clk = %d\n", clk_cfg.wanted_tl3_clk );
    clk_cfg.switch_to_tdm_clk = I2S_CHIP_MASTER? NO_SWITCH_TO_TDM:SWITCH_TO_TDM;
    D10L_ConfigureClocks(clk_cfg);

    if (optimized_case)
        WRITE_REG( UART_BAUD_RATE_0C, CHANGE_UART_BAUDRATE | UART1_UART_BAUD_RATE_1_Mhz,r16d16);


    D10L_LoadAsrp(file, FILE_TYPE_ASRP_PARAMS_PRIM_INIT, ASRP_LOCATION, LOAD_ENGINE_TYPE_ASRP);
    DELAY(15);

    D10L_ConfigWwe(wwe, IN0_CP_NUM_0);
    WRITE_REG16(BUFFERING_NORMAL_AMPLITUDE_38, NO_NORMALIZATION);
    // Config init2_asrp
    WRITE_REG( LOAD_BINARY_FILE_0F,	LOAD_ENGINE_TYPE_ASRP			|
                                                        OP_TYPE_LOAD_FILE		|
                                                        FILE_INIT_OR_KILL_ONLY	|
                                                        BLK_START_NUM_0			|
														MEM_TYPE_AHB			|	// no memory is allocated here
                                                        FILE_TYPE_ASRP_SEC_INIT,r16d16);

    WRITE_REG16(ASRP_USER_DEFINED_DELAY_107, 0);
    WRITE_REG( ASRP_OUTPUT_ROUTING_109, ASRP_OUTPUT_SRC_AEC_1 | ASRP_OUTPUT_DEST_TX_1,r16d16);
    if (EQD_ENABLE)
        // close EQD at end of ASRP session of FW
        WRITE_REG( ASRP_FORCE_INIT_STATE_102, ASRP_INIT_CHANNEL_SELECTION,r16d16);

    // if (SUPPORT_NSBF == NSBF_DUAL_INSTANCE)
        WRITE_REG( ASRP_OUTPUT_ROUTING_109, ASRP_OUTPUT_SRC_VCPF_1 | ASRP_OUTPUT_DEST_TX_2,r16d16);

    if (NO_OF_MICS < 2 && NO_OF_SPEAKERS == 1) 
    {
        // patch for ASRP 397 1 mic 1 spkr case:
        WRITE_REG16(ASRP_OUTPUT_GAIN_DEST_SEL_10A, ASRP_OUTPUT_DEST_TX_1);
        WRITE_REG16(ASRP_OUTPUT_GAIN_DEST_VALUE_10B, 0x25f);
    }
    else
    {
        D10L_SetAsrpOutputScaleGain( 15, 0, 0);
    }

    WRITE_REG( AUDIO_PROCESSING_CONFIGURATION_34,	POST_DETECTION_MODE_REMAIN_IN_DETECTION		|
                                                            ALGO1_EN_FW_MODE_1_AND_MODE_2,r16d16);

    uint16 is_tdm_clk_shared = TDM_CLK_SHARED ? TDM1_IOS_SHARED : TDM1_IOS_NOT_SHARED;
    if (I2S_CHIP_MASTER==0)
    {
        /*
        if ((tdm_sample_rate == 48000) && (tdm_bit_rate == 32)){
            clk_delay = TDM_SYNC_DELAY_7_CLKS_CYCLES;
        }else{
            clk_delay = TDM_SYNC_DELAY_5_CLKS_CYCLES;
        }
        */

        WRITE_REG16(AUDIO_ROUTING_CONFIGURATION_1F,	TDM_SYNC_RIGHT_CH		|
                                                            //clk_delay				| //not required for D10
                                                            USE_TDM_MUSIC_TO_SYNC	|
                                                            TDM0_3V3 |
                                                            TDM1_3V3 |
                                                            is_tdm_clk_shared |
                                                            MUSIC_IN_TDM0 | (1 << 2));       //01fw0c24   此处有异议
        // WRITE_REG16(AUDIO_ROUTING_CONFIGURATION_1F,0x0c24);

    }else{
        WRITE_REG16(AUDIO_ROUTING_CONFIGURATION_1F,	MUSIC_IN_TDM0 | TDM0_3V3 | TDM1_3V3 | is_tdm_clk_shared);
    }
    WRITE_REG16(AUDIO_ROUTING_CONFIGURATION_1F,	MUSIC_IN_TDM0 | TDM0_3V3 | TDM1_3V3 | is_tdm_clk_shared);
    WRITE_REG16(TDM_ACTIVATION_CONTROL_31,CONFIG_TDM_0);

    uint16 resampling_type =0, resampling_ratio = 0;
    if(TDM_SAMPLE_RATE == 48000)
    {
        if (BG_48_EC_DECIMATION_EN)
        {
            resampling_type  = RESAMPLE_TYPE_DECIMATION; //Macro value 0
            resampling_ratio = RESAMPLE_RATIO_NO_RESAMPLING;
        }
        else
        {
            resampling_type  = RESAMPLE_TYPE_DECIMATION;
            resampling_ratio = RESAMPLE_RATIO_3_1;
        }

        WRITE_REG16(TDM_RX_CONFIG_36,	DEMUX_MUX_ENABLE					|
                                                NUM_OF_CHANNELS_2_CH				|
                                                SAMPLE_WIDTH_16_BIT 				|
                                                INPUT_OUTPUT_SAMPLE_RATE_48_KHz		|
                                                RX_TX_I2S_CH_USE_I2S_STEREO			|
                                                HW_BLOCK_EN   |
                                                resampling_type |
                                                resampling_ratio |
                                                HW_BLOCK_EN							|
                                                RX_TX_CP2);
    }
    else if(TDM_SAMPLE_RATE == 16000)
    {
        WRITE_REG16(TDM_RX_CONFIG_36,	DEMUX_MUX_ENABLE					|
                                                NUM_OF_CHANNELS_2_CH				|
                                                SAMPLE_WIDTH_16_BIT 				|
                                                INPUT_OUTPUT_SAMPLE_RATE_16_KHz		|
                                                RX_TX_I2S_CH_USE_I2S_STEREO			|
                                                RESAMPLE_TYPE_DECIMATION			|
                                                RESAMPLE_RATIO_NO_RESAMPLING		|
                                                HW_BLOCK_EN							|
                                                RX_TX_CP2);
    }

    D10L_ConfigureTdmHw( 0, D10_TDM_0_RX_ADDR, TDM_RX, TDM_BIT_RATE, I2S_CHIP_MASTER, 2, 1);
    
    // TDM Tx config
    if (I2S_CHIP_MASTER==1)
    {
        WRITE_REG16(TDM_ACTIVATION_CONTROL_31,CONFIG_TDM_0);

        uint16 master_mode = TX_MASTER_CLOCK_ONLY;
        if (TDM_SAMPLE_RATE == 48000){
            WRITE_REG16(TDM_TX_CONFIG_37,	DEMUX_MUX_ENABLE					|
                                                    NUM_OF_CHANNELS_2_CH				|
                                                    SAMPLE_WIDTH_16_BIT 				|
                                                    INPUT_OUTPUT_SAMPLE_RATE_48_KHz		|
                                                    RX_TX_I2S_CH_USE_I2S_STEREO			|
                                                    master_mode							|
                                                    RESAMPLE_RATIO_NO_RESAMPLING		|
                                                    HW_BLOCK_EN							|
                                                    RX_TX_CP0);
        }else if(TDM_SAMPLE_RATE == 16000){
            WRITE_REG16(TDM_TX_CONFIG_37,	DEMUX_MUX_ENABLE					|
                                                    NUM_OF_CHANNELS_2_CH				|
                                                    SAMPLE_WIDTH_16_BIT 				|
                                                    INPUT_OUTPUT_SAMPLE_RATE_16_KHz		|
                                                    RX_TX_I2S_CH_USE_I2S_STEREO			|
                                                    master_mode							|
                                                    RESAMPLE_TYPE_DECIMATION			|
                                                    RESAMPLE_RATIO_NO_RESAMPLING		|
                                                    HW_BLOCK_EN							|
                                                    RX_TX_CP0);
        }
    }
    else
    {
        //chip slave
        WRITE_REG16(TDM_ACTIVATION_CONTROL_31,CONFIG_TDM_0);

        if (TDM_SAMPLE_RATE == 48000){
            WRITE_REG16(TDM_TX_CONFIG_37,	DEMUX_MUX_DISABLE					|
                                                    NUM_OF_CHANNELS_1_CH				|
                                                    SAMPLE_WIDTH_16_BIT 				|
                                                    INPUT_OUTPUT_SAMPLE_RATE_48_KHz 	|
                                                    TX_I2S_CH_SUM_HIGH_AND_LOW_INTO_ONE_SAMPLE 		|
                                                    RESAMPLE_TYPE_INTERPOLATION 		|
                                                    RESAMPLE_RATIO_3_1		|
                                                    HW_BLOCK_EN 						|
                                                    RX_TX_CP1);
        }else if(TDM_SAMPLE_RATE == 16000){
            WRITE_REG16(TDM_TX_CONFIG_37,	DEMUX_MUX_DISABLE					|
                                                    NUM_OF_CHANNELS_1_CH				|
                                                    SAMPLE_WIDTH_16_BIT 				|
                                                    INPUT_OUTPUT_SAMPLE_RATE_16_KHz 	|
                                                    TX_I2S_CH_SUM_HIGH_AND_LOW_INTO_ONE_SAMPLE 		|
                                                    RESAMPLE_TYPE_DECIMATION			|
                                                    RESAMPLE_RATIO_NO_RESAMPLING		|
                                                    HW_BLOCK_EN 						|
                                                    RX_TX_CP1);
        }

    }
    //delay_if_not_ready(chip, 5);
    D10L_ConfigureTdmHw( 0, D10_TDM_0_TX_ADDR, TDM_TX, TDM_BIT_RATE, I2S_CHIP_MASTER, 2, 1);
    if (FW_MODE == MODE_DEBUG){
        WRITE_REG16(FW_DEBUG_REGISTER_2_17, 0x1000);
    }

    // ASRP entries Config
    if (NO_OF_MICS > 1) 
    {
        WRITE_REG16(AUDIO_PROCESSING_ROUTING_11,  	IO_SET_0	|
                                                        IO_3N_2_NO_CP	|
                                                        IO_3N_1_NO_CP	|
                                                        IO_3N_0_CP_0);
    }
    else
    {
        WRITE_REG16(AUDIO_PROCESSING_ROUTING_11,  	IO_SET_0	|
                                                        IO_3N_2_NO_CP	|
                                                        IO_3N_1_CP_1	|
                                                        IO_3N_0_CP_0);
    }

    WRITE_REG16(AUDIO_PROCESSING_ROUTING_11,  	IO_SET_1 		|
                                                        IO_3N_2_NO_CP	|
                                                        IO_3N_1_NO_CP	|
                                                        IO_3N_0_NO_CP);

    WRITE_REG16(AUDIO_PROCESSING_ROUTING_11,   IO_SET_2 		|
                                                        IO_3N_2_CP_2	|
                                                        IO_3N_1_NO_CP	|
                                                        IO_3N_0_NO_CP);

    if (NO_OF_SPEAKERS == 2) {
        WRITE_REG16(AUDIO_PROCESSING_ROUTING_11,   IO_SET_3 		|
                                                        IO_3N_2_NO_CP	|
                                                        IO_3N_1_NO_CP	|
                                                        IO_3N_0_CP_3);
    } else {
        WRITE_REG16(AUDIO_PROCESSING_ROUTING_11,   IO_SET_3 		|
                                                        IO_3N_2_NO_CP	|
                                                        IO_3N_1_NO_CP	|
                                                        IO_3N_0_NO_CP);
    }

    WRITE_REG16(AUDIO_PROCESSING_ROUTING_11,   IO_SET_4 		|
                                                        IO_3N_2_NO_CP	|
                                                        IO_3N_1_NO_CP	|
                                                        IO_3N_0_NO_CP);
    if (NO_OF_SPEAKERS ==2 ){

        WRITE_REG16(AUDIO_PROCESSING_ROUTING_11,   IO_SET_5 		|
                                                            IO_3N_2_NO_CP	|
                                                            IO_3N_1_CP_1	|
                                                            IO_3N_0_CP_1);
    }else{
        WRITE_REG16(AUDIO_PROCESSING_ROUTING_11,   IO_SET_5 		|
                                                            IO_3N_2_NO_CP	|
                                                            IO_3N_1_CP_1	|
                                                            IO_3N_0_CP_1);
    }

    WRITE_REG16(AUDIO_PROCESSING_ROUTING_11,   IO_SET_6 		|
                                                        IO_3N_2_NO_CP	|
                                                        IO_3N_1_NO_CP	|
                                                        IO_3N_0_NO_CP);

    WRITE_REG16(AUDIO_PROCESSING_ROUTING_11,  	IO_SET_7 		|
                                                        IO_3N_2_NO_CP	|
                                                        IO_3N_1_NO_CP	|
                                                        IO_3N_0_NO_CP);


    /* Mic Config */
    D10L_ConfigMics(MIC_TYPE, NO_OF_MICS > 1 ? MIC_TYPE : 0, NO_OF_MICS > 2 ? MIC_TYPE : 0, clk_cfg.digital_mic_freq, clk_cfg.analog_mic_freq);

    if ((MIC_TYPE == MIC_ANALOG))
        WRITE_REG( MICROPHONE_ANALOG_GAIN_16,	SAR0_GAIN_EN		|
                                                                                    SAR1_GAIN_EN		|
                                                                                    SAR0_GAIN_0_DB	|
                                                                                    SAR1_GAIN_0_DB,r16d16);
    else
        WRITE_REG( DIGITAL_GAIN_04, GAIN_AFFECTS_ALL_MICROPHONES |D10L_CalculateDigitalMicGain( MIC_GAINS),r16d16);


    // Audio recording
    if(FW_MODE == MODE_RECORD)
    {
        if ((VT1_RECORD_EN == 1) && WAKE_ENGINE1)
        {
            WRITE_REG16(VT_RECORD_CFG_306, REC_IN0_DATA | VT_NUM_1);
        }
        if ((VT2_RECORD_EN == 1) && WAKE_ENGINE2)
        {
            WRITE_REG16(VT_RECORD_CFG_306, REC_IN0_DATA | VT_NUM_2);
        }
        uint16 asrp_record_channels1 = ASRP_REC_CH_TX_IN_1 | ASRP_REC_CH_RX_IN_1;

        if(NO_OF_MICS > 1 ) { // add also mic 2 recording
            asrp_record_channels1 |= ASRP_REC_CH_TX_IN_2 ;// | ASRP_REC_CH_RX_IN_2;
        }

        uint16 asrp_record_channels2 = ASRP_REC_CH_TX_OUT_1 | ASRP_REC_CH_TX_OUT_2;

        uint16 record_command = ALLOCATE_BUFFER_IN_AHB_MEMORY;

        D10L_AsrpRecordTurnOn( asrp_record_channels1, asrp_record_channels2, record_command);
    }

    //Print Memory info
    WRITE_REG16(FW_DEBUG_REGISTER_18, 0xa);

    //activate use case

    WRITE_REG16(TDM_ACTIVATION_CONTROL_31,0x0304);   //031w0304

    DBM_DEBUG("[%s]: FINISH ENTER USE CASE RCU NR - WWE = %d\n", __func__, wwe);

    return 0;
}

bool D10L_UsecaseRcuNrEnter(const source_t file,wake_engine wwe)
{
    DBM_DEBUG("[%s]: Start.\n", __func__);

    bool optimized_case =OPTIMIZED && (FW_MODE != MODE_RECORD);
    int stream_cp, reg_23_val;
    dspg_clock_config_t clk_cfg;

    D10L_GetDefaultClockConfig(&clk_cfg);

    if (MIC_TYPE == MIC_ANALOG)
    {
        clk_cfg.analog_mic_freq = 512;
        clk_cfg.digital_mic_freq = 1;
    }
    else
    {
        clk_cfg.analog_mic_freq = 1;
        clk_cfg.digital_mic_freq = 1024;		// temporary
    }

    if (optimized_case)
    {
        if (wwe == WWE_RETUNE)
        {
            clk_cfg.wanted_pll  = NO_WANTED_PLL;
            clk_cfg.wanted_tl3_clk = 60000;
        }
        else if (wwe == WWE_CYBERON)
        {
            // optimized cyberon
            clk_cfg.wanted_pll  = NO_WANTED_PLL;
            #ifdef CYBERON_OK_GOOGLE
            clk_cfg.wanted_tl3_clk = 65000;
            #else
            clk_cfg.wanted_tl3_clk = 98000;
            #endif
        }
        else if (wwe == WWE_GOOGLE)
        {
            // optimized Google
            clk_cfg.wanted_pll  = NO_WANTED_PLL;
            clk_cfg.wanted_tl3_clk = 50000;
        }
        else
        {
            // optimized Amazon
            clk_cfg.wanted_pll  = NO_WANTED_PLL;
            clk_cfg.wanted_tl3_clk = 50000;
        }

    }
    else
    {	// non optimized
        clk_cfg.wanted_pll  = NO_WANTED_PLL;
        clk_cfg.wanted_tl3_clk = NONE_OPTIMIZED_TL_FREQ;
    }

    DBM_DEBUG("wanted_tl3_clk = %d\n", clk_cfg.wanted_tl3_clk );

    D10L_ConfigureClocks(clk_cfg);

    if (optimized_case)
        WRITE_REG( UART_BAUD_RATE_0C, CHANGE_UART_BAUDRATE | UART1_UART_BAUD_RATE_1_Mhz,r16d16);

#ifdef TDM
    // Config TDM0 Tx but do not activate it for now
    WRITE_REG(TDM_ACTIVATION_CONTROL_31,CONFIG_TDM_0,r16d16);

    WRITE_REG(TDM_TX_CONFIG_37,	DEMUX_MUX_DISABLE					|
                                                NUM_OF_CHANNELS_1_CH				|
                                                SAMPLE_WIDTH_16_BIT 				|
                                                INPUT_OUTPUT_SAMPLE_RATE_16_KHz		|
                                                TX_I2S_CH_SUM_HIGH_AND_LOW_INTO_ONE_SAMPLE	|
                                                RESAMPLE_TYPE_DECIMATION			|
                                                RESAMPLE_RATIO_NO_RESAMPLING		|
                                                HW_BLOCK_EN							|
                                                RX_TX_CP0,r16d16);

    configure_tdm_hw(chip, 0, D10_TDM_0_TX_ADDR, TDM_TX, 16000, I2S_CHIP_MASTER, 2);
#endif

    D10L_LoadAsrp(file, FILE_TYPE_ASRP_PARAMS_PRIM_INIT, ASRP_LOCATION, LOAD_ENGINE_TYPE_ASRP);
    DELAY(15);

    // Config init2_asrp
    WRITE_REG( LOAD_BINARY_FILE_0F,	LOAD_ENGINE_TYPE_ASRP			|
                                                        OP_TYPE_LOAD_FILE		|
                                                        FILE_INIT_OR_KILL_ONLY	|
                                                        BLK_START_NUM_0			|
//														MEM_TYPE_AHB			|	// no memory is allocated here
                                                        FILE_TYPE_ASRP_SEC_INIT,r16d16);

    WRITE_REG( ASRP_OUTPUT_ROUTING_109, ASRP_OUTPUT_SRC_BFPF_0 | ASRP_OUTPUT_DEST_TX_1,r16d16);
    if (EQD_ENABLE)
        // close EQD at end of ASRP session of FW
        WRITE_REG( ASRP_FORCE_INIT_STATE_102, ASRP_INIT_CHANNEL_SELECTION,r16d16);

    if (SUPPORT_NSBF == NSBF_DUAL_INSTANCE)
        WRITE_REG( ASRP_OUTPUT_ROUTING_109, ASRP_OUTPUT_SRC_BFPF_1 | ASRP_OUTPUT_DEST_TX_2,r16d16);

    if (ASRP_BYPASS)
    {
        WRITE_REG( ASRP_BYPASS_EN_122, ASRP_FULL_BYPASS,r16d16);
        D10L_SetAsrpOutputScaleGain(1, 0, 0);
    }
    else
        D10L_SetAsrpOutputScaleGain(15, 0, 0);

    WRITE_REG( AUDIO_PROCESSING_CONFIGURATION_34,	POST_DETECTION_MODE_REMAIN_IN_DETECTION		|
                                                            ALGO1_EN_FW_MODE_1_AND_MODE_2,r16d16);
    // config loaded engine
    D10L_ConfigRcuWwe( wwe, IN0_CP_NUM_0, IN0_CP_NUM_1);
    D10l_ConfigBuzzer();

    // Config gain normalization according to engine:
    if (wwe == WWE_GOOGLE)
        WRITE_REG( BUFFERING_NORMAL_AMPLITUDE_38, USE_PHRASE_LEN_CONFIG_BY_HOST	|
                                                    USER_CONFIG_PHRASE_LEN_500_MS	|
                                                    NORMALIZE_TO_MINUS_6dB,r16d16);
    else
        WRITE_REG( BUFFERING_NORMAL_AMPLITUDE_38, USE_PHRASE_LEN_FROM_WWE	|
                                                        NORMALIZE_TO_MINUS_6dB,r16d16);

    // config ASRP entries

    WRITE_REG( AUDIO_PROCESSING_ROUTING_11,  	IO_SET_0		|
                                                        IO_3N_2_NO_CP	|
                                                        IO_3N_1_CP_1	|
                                                        IO_3N_0_CP_0,r16d16);
    WRITE_REG( AUDIO_PROCESSING_ROUTING_11,  	IO_SET_1 		|
                                                        IO_3N_2_NO_CP	|
                                                        IO_3N_1_NO_CP	|
                                                        IO_3N_0_NO_CP,r16d16);
    WRITE_REG( AUDIO_PROCESSING_ROUTING_11,   IO_SET_2 		|
                                                        IO_3N_2_NO_CP	|
                                                        IO_3N_1_NO_CP	|
                                                        IO_3N_0_NO_CP,r16d16);
    WRITE_REG( AUDIO_PROCESSING_ROUTING_11,   IO_SET_3 		|
                                                        IO_3N_2_NO_CP	|
                                                        IO_3N_1_NO_CP	|
                                                        IO_3N_0_NO_CP,r16d16);
    WRITE_REG( AUDIO_PROCESSING_ROUTING_11,   IO_SET_4 		|
                                                        IO_3N_2_NO_CP	|
                                                        IO_3N_1_NO_CP	|
                                                        IO_3N_0_NO_CP,r16d16);
    WRITE_REG( AUDIO_PROCESSING_ROUTING_11,   IO_SET_5 		|
                                                        IO_3N_2_NO_CP	|
                                                        IO_3N_1_CP_1	|
                                                        IO_3N_0_CP_0,r16d16);
    WRITE_REG( AUDIO_PROCESSING_ROUTING_11,   IO_SET_6 		|
                                                        IO_3N_2_NO_CP	|
                                                        IO_3N_1_NO_CP	|
                                                        IO_3N_0_NO_CP,r16d16);
    WRITE_REG( AUDIO_PROCESSING_ROUTING_11,  	IO_SET_7 		|
                                                        IO_3N_2_NO_CP	|
                                                        IO_3N_1_NO_CP	|
                                                        IO_3N_0_NO_CP,r16d16);
    WRITE_REG( AUDIO_PROCESSING_ROUTING_11,  	IO_SET_8 		|
                                                        IO_3N_2_NO_CP	| // Entry of bone mic 3
                                                        IO_3N_1_NO_CP	|
                                                        IO_3N_0_NO_CP,r16d16);

    stream_cp = (SUPPORT_NSBF == NSBF_DUAL_INSTANCE)? 1 : 0;

    WRITE_REG( AUDIO_STREAMING_SOURCE_SELECTION_13, 	NO_STREAM_CH_4	|
                                                                NO_STREAM_CH_3	|
                                                                NO_STREAM_CH_2	|
                                                                stream_cp,r16d16);

    // register 23 and preroll extention must be sent to FW AFTER ASRP init
    READ_REG(GENERAL_CONFIGURATION_2_23,&reg_23_val,r16d16);
    if (HW_VAD || LP_HW_VAD)
    { // set post divider and reg 23 for other VAD cases)
        if (OPTIMIZED && POST_PLL_DIVIDER_ENABLE)
        {
            WRITE_REG( REG_IO_PORT_ADDR_LO_05, ENABLE_POST_PLL_DIVIDER,r16d16);
        }
        else
        {
            DBM_DEBUG("!! DISABLE_POST_PLL_DIVIDER\n");
            WRITE_REG( REG_IO_PORT_ADDR_LO_05, DISABLE_POST_PLL_DIVIDER,r16d16);
        }
        reg_23_val = D10L_ConfigureVadType(FW_VAD_TYPE_D6_HW_VAD1);
        // configure_vad_type(chip, FW_VAD_TYPE_D6_HW_VAD1);

        // set this bit also for lp_hwv
        if (LP_HW_VAD)
        {
            reg_23_val |= ENABLE_LP_HW_VAD;
        }
        else
        {
            reg_23_val |= ENABLE_SECONDARY_HW_VAD_BUFFER;
        } // not for lp_hw
        if (HW_VAD_ENABLE_PRE_ROLL_EXTENTION && ((wwe == WWE_AMAZON) || (wwe == WWE_RETUNE) || (wwe == WWE_CYBERON)))
        {
            reg_23_val |= ENABLE_PRE_ROLL_EXTENTION;
        }

        // Enable EA for Amazon/Google/Retune
        if ((wwe == WWE_AMAZON) || (wwe == WWE_RETUNE) || (wwe == WWE_CYBERON))
            D10L_ConfigureEnvironmentAdaptation(ENVIRO_ADAPTATION); // config + enable or config + disable
    }

    // Config Reg 0x23
    DBM_DEBUG("Config Reg 0x23 = 0x%x\n", reg_23_val);
    WRITE_REG( GENERAL_CONFIGURATION_2_23, reg_23_val,r16d16);

    if (HW_VAD || LP_HW_VAD)
        WRITE_REG( VAD_START_STOP_350, VAD_START,r16d16);

    /* Mic Config */
    D10L_ConfigMics(MIC_TYPE, NO_OF_MICS > 1 ? MIC_TYPE : 0, NO_OF_MICS > 2 ? MIC_TYPE : 0, clk_cfg.digital_mic_freq, clk_cfg.analog_mic_freq);

    if ((MIC_TYPE == MIC_ANALOG))
        WRITE_REG( MICROPHONE_ANALOG_GAIN_16,	SAR0_GAIN_EN		|
                                                                                    SAR1_GAIN_EN		|
                                                                                    SAR0_GAIN_23_52_DB	|
                                                                                    SAR1_GAIN_23_52_DB,r16d16);
    else
        WRITE_REG( DIGITAL_GAIN_04, GAIN_AFFECTS_ALL_MICROPHONES |D10L_CalculateDigitalMicGain( MIC_GAINS),r16d16);

    if(HW_VAD)
    {
        // increase vad gain
        if (USE_HEXAGRAM_MICS == 1)
            D10L_SetHwVadGain( D10_HW_VAD1_BASE, VAD_GAIN_PLUS_6_DB);
    }

    if(LP_HW_VAD)
    {
        //g_lphw_vad setting should be written after reg 24 (as the FW also configure it)
        D10L_InitLpHwVad( D10_HW_VAD1_BASE);

        // increase vad gain
        if (USE_HEXAGRAM_MICS == 1)
            D10L_SetHwVadGain( D10_HW_VAD1_BASE, VAD_GAIN_PLUS_6_DB);
    }

    // Audio recording
    if(FW_MODE == MODE_RECORD)
    {
        uint16 asrp_record_channels1, asrp_record_channels2, record_command;
        uint16 vad_buffer = ((HW_VAD) && (DISABLE_ASRP_RECORD)) ? ADD_BUFFER_TO_HW_VAD : 0;	// for now: not for g_lphw_vad
        // enlarge buffer for amazon and google only, when audio buffer is 3
        uint16 enlarged = ((wwe == WWE_CYBERON) || (wwe == WWE_RETUNE)) ? 0 : ENLARGE_BUFFER_BY_2;

        WRITE_REG16( UART_BAUD_RATE_0C, CHANGE_UART_BAUDRATE | UART1_UART_BAUD_RATE_3_Mhz);

        if((ENABLE_HW_VAD_BUFFER_RECORDING) && (!DISABLE_ASRP_RECORD))
            g_reg6_fw_record = 0x6000;

        asrp_record_channels1 = ASRP_REC_CH_TX_IN_1	| ASRP_REC_CH_TX_IN_2;
        //Not clear why do we need the 2 RX - took from python:
        //asrp_record_channels1 |= ASRP_REC_CH_RX_IN_1	| ASRP_REC_CH_RX_IN_2;
        asrp_record_channels2 =  ASRP_REC_CH_TX_OUT_2 | ASRP_REC_CH_TX_OUT_1;
        record_command = ALLOCATE_BUFFER_IN_AHB_MEMORY | vad_buffer | enlarged;

        D10L_AsrpRecordTurnOn( asrp_record_channels1, asrp_record_channels2, record_command);
    }

    // TDM is activated only by user from menu
    WRITE_REG( OPERATION_MODE_01, DETECTION_MODE,r16d16);

    DBM_DEBUG("[%s]: FINISH ENTER USE CASE RCU NR - WWE = %d\n", __func__, wwe);

    return 0;
}

bool D10L_UsecaseLpEnter(wake_engine wwe)
{
    DBM_DEBUG("[%s]: Start.\n", __func__);

//    bool optimized_case =OPTIMIZED && (FW_MODE != MODE_RECORD);
    dspg_clock_config_t clk_cfg;

    D10L_GetDefaultClockConfig(&clk_cfg);

    if (MIC_TYPE == MIC_ANALOG)
    {
        clk_cfg.analog_mic_freq = 1024;
        clk_cfg.digital_mic_freq = 1;
    }
    else
    {
        clk_cfg.analog_mic_freq = 1;
        clk_cfg.digital_mic_freq = 1536;		// temporary
    }

    if (OPTIMIZED==1)
    {
        if (wwe != WWE_DUAL)
        {
            clk_cfg.wanted_pll  = 60000;
        }
        else
            clk_cfg.wanted_pll  = 102001; 

        clk_cfg.use_pll_post_div = FALSE;
        clk_cfg.wanted_tl3_clk = 60000;  
        clk_cfg.wanted_ahb_clk =  clk_cfg.wanted_tl3_clk;
        clk_cfg.wanted_apb_clk =  clk_cfg.wanted_ahb_clk;

    }
    else
    {	// non optimized
        clk_cfg.use_pll_post_div = FALSE;
        clk_cfg.wanted_pll  = NO_WANTED_PLL;
        clk_cfg.wanted_tl3_clk = NONE_OPTIMIZED_TL_FREQ;
    }

    DBM_DEBUG("wanted_tl3_clk = %d\n", clk_cfg.wanted_tl3_clk );

    D10L_ConfigureClocks(clk_cfg);

    if (FW_MODE != MODE_RECORD)
        WRITE_REG( UART_BAUD_RATE_0C, CHANGE_UART_BAUDRATE | UART1_UART_BAUD_RATE_1_Mhz,r16d16);

    D10L_ConfigWwe(wwe, IN0_CP_NUM_0);

    // WRITE_REG16(AUDIO_STREAMING_SOURCE_SELECTION_13, 	NO_STREAM_CH_4	|
	// 															NO_STREAM_CH_3	|
	// 															NO_STREAM_CH_2	|
	// 															STREAM_CH_1_CP_0);

	WRITE_REG16(BUFFERING_NORMAL_AMPLITUDE_38, USE_PHRASE_LEN_FROM_WWE	|
														NORMALIZE_TO_MINUS_6dB);

    WRITE_REG16(AUDIO_PROCESSING_CONFIGURATION_34, POST_DETECTION_MODE_REMAIN_IN_DETECTION);

    /* Mic Config */
    D10L_ConfigMics(MIC_TYPE, NO_OF_MICS > 1 ? MIC_TYPE : 0, NO_OF_MICS > 2 ? MIC_TYPE : 0, DM_CLK_FREQ_1536_1200_SR_8KHz_16KHz_32KHz_48KHz, SAR_IIR_FILTER_512);
   
    if (MIC_TYPE == MIC_ANALOG)
        WRITE_REG16( MICROPHONE_ANALOG_GAIN_16,	SAR0_GAIN_EN		|
														SAR1_GAIN_EN		|
														SAR0_GAIN_21_56_DB	|
														SAR1_GAIN_21_56_DB);
    WRITE_REG( DIGITAL_GAIN_04, GAIN_AFFECTS_ALL_MICROPHONES |D10L_CalculateDigitalMicGain( MIC_GAINS),r16d16);


    // Audio recording
    if(FW_MODE == MODE_RECORD)
    {
        D10L_FwRecordTurnOn(0x10);  // record only fw (mic 1 only), no ASRP
    }

    // TDM is activated only by user from menu
    WRITE_REG( OPERATION_MODE_01, DETECTION_MODE,r16d16);

    DBM_DEBUG("[%s]: FINISH ENTER USE CASE RCU NR - WWE = %d\n", __func__, wwe);

    return 0;
}


bool D10L_LoadFile(const source_t fw,uint8 skip_size)
{
    DBM_DEBUG("--D10L_LoadFile, load file data to dspg chip, skip % datas",skip_size);

    WRITE( fw.data, fw.data_size-4);

    //if read the checksum
    if(skip_size >= CHECKSUM_SIZE)
    {
        uint8 check[4];

        memcpy(check,&fw.data[fw.data_size-4],4);
        return D10L_VerifyCheckSum(check);
    }
    return TRUE;
}



