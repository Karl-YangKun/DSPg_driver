#include  "DSPg_driver/DSPg.h"
#include "dspg_app.h"
#include <bitserial_api.h>
#include <panic.h>

#ifndef ADK6
#include <logging.h>
#include <stdio.h>
#else  //it's adk 6.x
#include "sink_debug.h"
#include <audio_plugin_common.h>
#endif


/**************************config********************************/
//io config
#define RESET_IO   60
#define SPI_CS_IO   5
#define SPI_CLK_IO   4
#define SPI_IN_IO   2
#define SPI_OUT_IO   3

#define I2S_CS_IO    17
#define I2S_CLK_IO   16
#define I2S_IN_IO       19
#define I2S_OUT_IO   18

#define USING_COMM    SPI
#define SPI_BLOCK_SIZE  0XFFFF  //the limmit block size for each writing using spi


/**************************functions********************************/
#define PIO2BANK(pio) ((uint16)((pio) / 32))
#define PIO2MASK(pio) (1UL << ((pio) % 32))



static bool dspgApp_Write(const uint8 *data,uint32 data_size);
static bool dspgApp_Read(uint8 *data,uint16 data_size);
static void dspgApp_DelayMs(uint16 ms);
static void dspgApp_SetPio(dspg_io_t map_io, bool high);
static void dspgApp_Debug( const char *format, uint8 n_args, va_list args);

bitserial_handle comm_handle;
TaskData task_data;

static interface_t intf=
{
    .comm = USING_COMM,
    .Write = dspgApp_Write,
    .Read = dspgApp_Read,
    .Delay = dspgApp_DelayMs,
    .Set_IO = dspgApp_SetPio,
    .Debug = dspgApp_Debug,
};


static bool dspgApp_SPIInit(void)
{
    bitserial_config bsconfig;
    uint16 bank;
    uint32 mask;

    /* Setup the PIOs mapping for Bitserial SPI use */
    bank = PIO2BANK(SPI_CS_IO);
    mask = PIO2MASK(SPI_CS_IO);
    PanicNotZero(PioSetMapPins32Bank(bank, mask, 0));
    bank = PIO2BANK(SPI_CLK_IO);
    mask = PIO2MASK(SPI_CLK_IO);
    PanicNotZero(PioSetMapPins32Bank(bank, mask, 0));
    bank = PIO2BANK(SPI_IN_IO);
    mask = PIO2MASK(SPI_IN_IO);
    PanicNotZero(PioSetMapPins32Bank(bank, mask, 0));
    bank = PIO2BANK(SPI_OUT_IO);
    mask = PIO2MASK(SPI_OUT_IO);
    PanicNotZero(PioSetMapPins32Bank(bank, mask, 0));

//    DBM_DEBUG("[%s]: current io mapping is %x.\n", __func__,PioGetMapPins32Bank(0));

    /* Setup the PIOs function for Bitserial SPI use*/
    PanicFalse(PioSetFunction(SPI_CS_IO, BITSERIAL_0_SEL_OUT));
    PanicFalse(PioSetFunction(SPI_CLK_IO, BITSERIAL_0_CLOCK_OUT));
    PanicFalse(PioSetFunction(SPI_IN_IO, BITSERIAL_0_DATA_IN));
    PanicFalse(PioSetFunction(SPI_OUT_IO, BITSERIAL_0_DATA_OUT));

    memset(&bsconfig, 0, sizeof(bsconfig));
    bsconfig.mode = BITSERIAL_MODE_SPI_MASTER;
    bsconfig.clock_frequency_khz = 2000;
    bsconfig.u.spi_cfg.sel_enabled = TRUE;
    bsconfig.u.spi_cfg.clock_sample_offset = 0;
    bsconfig.u.spi_cfg.select_time_offset = 0;
    bsconfig.u.spi_cfg.flags = BITSERIAL_SPI_MODE_0;
    comm_handle =  BitserialOpen((bitserial_block_index)BITSERIAL_BLOCK_0, &bsconfig);
    return (comm_handle !=BITSERIAL_HANDLE_ERROR);
}

static bool dspgApp_Write(const uint8 *data,uint32 data_size)
{
    bitserial_result result;

    if(data_size > SPI_BLOCK_SIZE)
    {
        uint32 written=0;
        do
        {
            if((data_size-written)>SPI_BLOCK_SIZE)
            {
                result = BitserialWrite(comm_handle,
                            BITSERIAL_NO_MSG,
                            data+written, SPI_BLOCK_SIZE,
                            BITSERIAL_FLAG_BLOCK );
                written += SPI_BLOCK_SIZE;
            }
            else
            {
                result = BitserialWrite(comm_handle,
                            BITSERIAL_NO_MSG,
                            data+written, data_size-written,
                            BITSERIAL_FLAG_BLOCK );
                written = data_size;
            }
        }while (written != data_size);
    }
    else
        result = BitserialWrite(comm_handle,
                            BITSERIAL_NO_MSG,
                            data, data_size,
                            BITSERIAL_FLAG_BLOCK );

    return(result == BITSERIAL_RESULT_SUCCESS);
}

static bool dspgApp_Read(uint8 *data,uint16 data_size)
{
    bitserial_result result;
    result = BitserialRead(comm_handle,
                            BITSERIAL_NO_MSG,
                            data, data_size,
                            BITSERIAL_FLAG_BLOCK );
    return (result == BITSERIAL_RESULT_SUCCESS);
}

static void dspgApp_DelayMs(uint16 ms)
{
     /* 1 ms */
    uint32 n = VmGetClock();
        while(VmGetClock() < ( n + ms ));
}

static void dspgApp_SetPio(dspg_io_t map_io, bool high)
{
    uint32 io=0;
    if(map_io == reset_io)
        io=RESET_IO;
    uint16 bank = PIO2BANK(io);
    uint32 mask = PIO2MASK(io);
    if(high)
        PanicNotZero(PioSet32Bank(bank, mask, mask));
    else
        PanicNotZero(PioSet32Bank(bank, mask, 0));
}

static void dspgApp_Debug( const char *format, uint8 n_args, va_list args)
{
#ifndef ADK6
    hydra_log_firm_va_arg(format, n_args, args);
#else
    UNUSED(n_args);
    vprintf(format,args);
    DEBUG(("\n"));
#endif
}

#ifndef ADK6
const pcm_config_t dspgApp_pcm_config =
{
    .sample_rate = 48000,       // lis25ba always runs with 16k. If general mic rate shall be used, set to 0
    .master_mode = 1,           // accelorometer should always be the slave
    .sample_size = 16,
    .slot_count = 2,
    .short_sync_enable = 1,
    .lsb_first_enable = 0,
    .sample_rising_edge_enable = 1,
    .sample_format = 0,         // 16 bits in 16 cycle slot duration
    .master_clock_source = CLK_SOURCE_TYPE_MCLK,
    .master_mclk_mult = 0,
};

const pcm_config_t *dspgApp_GetPcmInterfaceSettings(void)
{
    return &dspgApp_pcm_config;
}

static const pcm_callbacks_t dspgApp_Callbacks =
{
    /*! \brief Returns PCM interface settings for this device */
    .AudioPcmCommonGetPcmInterfaceSetting = dspgApp_GetPcmInterfaceSettings,
    /*! \brief Initialize I2C communication interface for this device */
    .AudioPcmCommonInitializeI2cInterface = NULL,
    /*! \brief Enable the device by writing  commands */
    .AudioPcmCommonEnableDevice = DspgApp_EnterVcMode,
    /*! \brief Disable the device by writing commands */
    .AudioPcmCommonDisableDevice = DspgApp_ExitVcMode,
};

const pcm_registry_per_user_t dspgApp_Registry =
{
    .user = pcm_user,
    .callbacks = &dspgApp_Callbacks,
};

static void dspgApp_MessageHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);
    UNUSED(message);
    switch (id)
    {
    case 0x1234:
        DSPg_SetMode(dspg_voice_call);
        break;
    default:break;

    }
}

bool DspgApp_Init(Task init_task)
#else
void DspgApp_Init(void)
#endif
{
    uint16 bank;
    uint32 mask;

    bank = PIO2BANK(RESET_IO);
    mask = PIO2MASK(RESET_IO);
    PanicNotZero(PioSetMapPins32Bank(bank, mask, mask));
    PanicNotZero(PioSetDir32Bank(bank, mask, mask));
    PanicNotZero(PioSet32Bank(bank, mask, mask));

    intf.fw.data=fw_array;
    intf.fw.data_size=DspgApp_getSize (1);
    intf.fw.config=fw_config;
    intf.fw.config_size=DspgApp_getSize (2);

    intf.use_case[dspg_voice_call].data=model_array;
    intf.use_case[dspg_voice_call].data_size=DspgApp_getSize (3);
    intf.use_case[dspg_voice_call].config=model_config;
    intf.use_case[dspg_voice_call].config_size=DspgApp_getSize (4);

    bank = PIO2BANK(I2S_IN_IO);
    mask = PIO2MASK(I2S_IN_IO);

//    PanicFalse(PioSetFunction(I2S_IN_IO, PIO));
    PioSetMapPins32Bank(bank, mask, mask);
    PioSet32Bank(bank, mask, 0);
    PioSetDir32Bank(bank, mask, 0);


    dspgApp_SPIInit();
    DSPg_Init(intf);

    PioSetMapPins32Bank(bank, mask, 0);
    PanicFalse(PioSetFunction(I2S_IN_IO, OTHER));
//    PanicFalse(PioSetFunction(I2S_IN_IO, PCM_IN));
//    PioSetFunction(I2S_OUT_IO, PCM_OUT);
//    PioSetFunction(I2S_CS_IO, PCM_SYNC);
//    PioSetFunction(I2S_CLK_IO, PCM_CLK);

#ifndef ADK6
    task_data.handler=dspgApp_MessageHandler;

    UNUSED(init_task);
    return TRUE;
#endif
}

void DspgApp_EnterVcMode(void)
{
    //enter later becase it need waiting clock
#ifndef ADK6
    MessageSendLater(&task_data,0x1234,0,200);
#else
    DSPg_SetMode(dspg_voice_call);
#endif
}

void DspgApp_ExitVcMode(void)
{
    DSPg_SetMode(dspg_idle);
}


