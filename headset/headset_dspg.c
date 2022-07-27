#include  "DSPg_driver/DSPg.h"
#include "headset_dspg.h"

#include <bitserial_api.h>
#include <panic.h>
#include <logging.h>
#include <stdio.h>

/**************************config********************************/
//io config
#define DSPG_RESET_IO   60
#define DSPG_SPI_CS_IO   15
#define DSPG_SPI_CLK_IO   20
#define DSPG_SPI_IN_IO   14
#define DSPG_SPI_OUT_IO   21

#define USING_COMM    SPI
#define SPI_BLOCK_SIZE  0XFFFF  //the limmit block size for each writing using spi


/**************************functions********************************/
#define PIO2BANK(pio) ((uint16)((pio) / 32))
#define PIO2MASK(pio) (1UL << ((pio) % 32))



static bool headsetDspg_Write(const uint8 *data,uint32 data_size);
static bool headsetDspg_Read(uint8 *data,uint16 data_size);
static void headsetDspg_DelayMs(uint16 ms);
static void headsetDspg_SetPio(dspg_io_t map_io, bool high);
static void headsetDspg_Debug( const char *format, uint8 n_args, va_list args);

bitserial_handle comm_handle;

static interface_t headset_inf=
{
    .comm = USING_COMM,
    .Write = headsetDspg_Write,
    .Read = headsetDspg_Read,
    .Delay = headsetDspg_DelayMs,
    .Set_IO = headsetDspg_SetPio,
    .Debug = headsetDspg_Debug,
};


static bool headsetDspg_SPIInit(void)
{
    bitserial_config bsconfig;
    uint16 bank;
    uint32 mask;

    /* Setup the PIOs mapping for Bitserial SPI use */
    bank = PIO2BANK(DSPG_SPI_CS_IO);
    mask = PIO2MASK(DSPG_SPI_CS_IO);
    PanicNotZero(PioSetMapPins32Bank(bank, mask, 0));
    bank = PIO2BANK(DSPG_SPI_CLK_IO);
    mask = PIO2MASK(DSPG_SPI_CLK_IO);
    PanicNotZero(PioSetMapPins32Bank(bank, mask, 0));
    bank = PIO2BANK(DSPG_SPI_IN_IO);
    mask = PIO2MASK(DSPG_SPI_IN_IO);
    PanicNotZero(PioSetMapPins32Bank(bank, mask, 0));
    bank = PIO2BANK(DSPG_SPI_OUT_IO);
    mask = PIO2MASK(DSPG_SPI_OUT_IO);
    PanicNotZero(PioSetMapPins32Bank(bank, mask, 0));

//    DBM_DEBUG("[%s]: current io mapping is %x.\n", __func__,PioGetMapPins32Bank(0));

    /* Setup the PIOs function for Bitserial SPI use*/
    PanicFalse(PioSetFunction(DSPG_SPI_CS_IO, BITSERIAL_0_SEL_OUT));
    PanicFalse(PioSetFunction(DSPG_SPI_CLK_IO, BITSERIAL_0_CLOCK_OUT));
    PanicFalse(PioSetFunction(DSPG_SPI_IN_IO, BITSERIAL_0_DATA_IN));
    PanicFalse(PioSetFunction(DSPG_SPI_OUT_IO, BITSERIAL_0_DATA_OUT));

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

static bool headsetDspg_Write(const uint8 *data,uint32 data_size)
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

static bool headsetDspg_Read(uint8 *data,uint16 data_size)
{
    bitserial_result result;
    result = BitserialRead(comm_handle,
                            BITSERIAL_NO_MSG,
                            data, data_size,
                            BITSERIAL_FLAG_BLOCK );
    return (result == BITSERIAL_RESULT_SUCCESS);
}

static void headsetDspg_DelayMs(uint16 ms)
{
     /* 1 ms */
    uint32 n = VmGetClock();
        while(VmGetClock() < ( n + ms ));
}

static void headsetDspg_SetPio(dspg_io_t map_io, bool high)
{
    uint32 io=0;
    if(map_io == reset_io)
        io=DSPG_RESET_IO;
    uint16 bank = PIO2BANK(io);
    uint32 mask = PIO2MASK(io);
    if(high)
        PanicNotZero(PioSet32Bank(bank, mask, mask));
    else
        PanicNotZero(PioSet32Bank(bank, mask, 0));
}

static void headsetDspg_Debug( const char *format, uint8 n_args, va_list args)
{
    hydra_log_firm_va_arg(format, n_args, args);
}

bool HeadsetDspg_Init(Task init_task)
{
    UNUSED(init_task);
    uint16 bank;
    uint32 mask;

    bank = PIO2BANK(DSPG_RESET_IO);
    mask = PIO2MASK(DSPG_RESET_IO);
    PanicNotZero(PioSetMapPins32Bank(bank, mask, mask));
    PanicNotZero(PioSetDir32Bank(bank, mask, mask));

    headset_inf.fw.data=fw_array;
    headset_inf.fw.data_size=HeadsetDspg_getSize (1);
    headset_inf.fw.config=fw_config;
    headset_inf.fw.config_size=HeadsetDspg_getSize (2);

    headset_inf.use_case[dspg_voice_call].data=model_array;
    headset_inf.use_case[dspg_voice_call].data_size=HeadsetDspg_getSize (3);
    headset_inf.use_case[dspg_voice_call].config=model_config;
    headset_inf.use_case[dspg_voice_call].config_size=HeadsetDspg_getSize (4);

    headsetDspg_SPIInit();
    DSPg_Init(headset_inf);

    return TRUE;
}

void HeadsetDspg_EnterVcMode(void)
{
    DSPg_SetMode(dspg_voice_call);
}

void HeadsetDspg_ExitVcMode(void)
{
    DSPg_SetMode(dspg_idle);
}
