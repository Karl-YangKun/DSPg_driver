#ifndef DSPG_CONFIG_H_
#define DSPG_CONFIG_H_

//enable debug log 
#define ENABLE_DSPG_DEBUG

//config 
#define TIME_IN_RESET   10   //5
#define TIME_AFTER_RESET    100  //at least 80ms
#define TIME_IN_WAKEUP    60
#define ENABLE_RECONDIG


typedef enum
{
    dspg_voice_call,
    dspg_hibernate,
    dspg_idle,

    dspg_max_mode =dspg_idle,
} usecase_t;

typedef enum
{
    reset_io,
} dspg_io_t;

typedef enum
{
    SPI,
    UART,
    I2C,
} comm_inf_t;

typedef enum
{
    r16d16 = 0,  //reg 16 bits, data 16 bits
    r16d32,
    r32d32,
    read_r16d16 = 0x10,  //read reg 16bits
    read_r16d32,
    read_r32d32
} ins_t;

typedef struct
{
    ins_t   form;
    uint32  reg;
    uint32  data;
    uint16  delay;
} config_table_t;

typedef struct
{
    const uint8 *data;
    uint32      data_size;
    const config_table_t *config;
    uint16      config_size;
} source_t;

#endif /* DSPG_CONFIG_H_ */
