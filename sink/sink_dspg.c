#include  "DSPg_driver/DSPg.h"
#include "sink_dspg.h"
#include "sink_debug.h"

#include <bitserial_api.h>
#include <panic.h>

/**************************config********************************/
//io config
#define DSPG_RESET_IO   16
#define DSPG_SPI_CS_IO   17
#define DSPG_SPI_CLK_IO   18
#define DSPG_SPI_IN_IO   20
#define DSPG_SPI_OUT_IO   19

#define USING_COMM    SPI
#define SPI_BLOCK_SIZE  0XFFFF  //the limmit block size for each writing using spi


/**************************functions********************************/
#define PIO2BANK(pio) ((uint16)((pio) / 32))
#define PIO2MASK(pio) (1UL << ((pio) % 32))



static bool sinkWrite(const uint8 *data,uint32 data_size);
static bool sinkRead(uint8 *data,uint16 data_size);
static void sinkDelayMs(uint16 ms);
static void sinkSetPio(dspg_io_t map_io, bool high);
static void sinkDebug( const char *format, va_list args);

bitserial_handle spi_handle;

static interface_t sink_inf=
{
    .comm = USING_COMM,
    .Write = sinkWrite,
    .Read = sinkRead,
    .Delay = sinkDelayMs,
    .Set_IO = sinkSetPio,
    .Debug = sinkDebug,
};


static bool sinkSPIInit(void)
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
    spi_handle =  BitserialOpen((bitserial_block_index)BITSERIAL_BLOCK_0, &bsconfig);
    return (spi_handle !=BITSERIAL_HANDLE_ERROR);
}

static bool sinkWrite(const uint8 *data,uint32 data_size)
{
    bitserial_result result;

    if(data_size > SPI_BLOCK_SIZE)
    {
        uint32 written=0;
        do
        {
            if((data_size-written)>SPI_BLOCK_SIZE)
            {
                result = BitserialWrite(spi_handle,
                            BITSERIAL_NO_MSG,
                            data+written, SPI_BLOCK_SIZE,
                            BITSERIAL_FLAG_BLOCK );
                written += SPI_BLOCK_SIZE;
            }
            else
            {
                result = BitserialWrite(spi_handle,
                            BITSERIAL_NO_MSG,
                            data+written, data_size-written,
                            BITSERIAL_FLAG_BLOCK );
                written = data_size;
            }
        }while (written != data_size);
    }
    else
        result = BitserialWrite(spi_handle,
                            BITSERIAL_NO_MSG,
                            data, data_size,
                            BITSERIAL_FLAG_BLOCK );

    return(result == BITSERIAL_RESULT_SUCCESS);
}

static bool sinkRead(uint8 *data,uint16 data_size)
{
    bitserial_result result;
    result = BitserialRead(spi_handle,
                            BITSERIAL_NO_MSG,
                            data, data_size,
                            BITSERIAL_FLAG_BLOCK );
    return (result == BITSERIAL_RESULT_SUCCESS);
}

static void sinkDelayMs(uint16 ms)
{
     /* 1 ms */
    uint32 n = VmGetClock();
        while(VmGetClock() < ( n + ms ));
}

static void sinkSetPio(dspg_io_t map_io, bool high)
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

static void sinkDebug( const char *format, va_list args)
{
    vprintf(format,args);
    DEBUG(("\n"));
}

void sinkDspgInit(void)
{
    uint16 bank;
    uint32 mask;

    bank = PIO2BANK(DSPG_RESET_IO);
    mask = PIO2MASK(DSPG_RESET_IO);
    PanicNotZero(PioSetMapPins32Bank(bank, mask, mask));
    PanicNotZero(PioSetDir32Bank(bank, mask, mask));

    sink_inf.fw.data=fw_array;
    sink_inf.fw.data_size=SinkDspgConfig_getSize (1);
    sink_inf.fw.config=fw_config;
    sink_inf.fw.config_size=SinkDspgConfig_getSize (2);

    sink_inf.use_case[dspg_voice_call].data=model_array;
    sink_inf.use_case[dspg_voice_call].data_size=SinkDspgConfig_getSize (3);
    sink_inf.use_case[dspg_voice_call].config=model_config;
    sink_inf.use_case[dspg_voice_call].config_size=SinkDspgConfig_getSize (4);

    sinkSPIInit();
    DSPg_Init(sink_inf);
}

void sinkDspgEnterVcMode(void)
{
    DSPg_SetMode(dspg_voice_call);
}

void sinkDspgExitVcMode(void)
{
    DSPg_SetMode(dspg_idle);
}
