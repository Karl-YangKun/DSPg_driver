#ifndef DSPG_CONFIG_H_
#define DSPG_CONFIG_H_

//feature enable
#define ENABLE_DSPG_DEBUG   //debug log
#define ENABLE_DSPG_REG_DEBUG  //debug read and write data
#define D10L                //d10l chip
#define DSPG_I2S     //using i2s
#define ENABLE_RECONDIG
#define ADK6

//config 
#define DELAY_IN_RESET   10   //5 in sdk
#define DELAY_AFTER_RESET    100  //at least 80ms
#define DELAY_IN_WAKEUP    60


typedef enum
{
    dspg_low_power,
    dspg_voice_call,
    dspg_voice_rcu,
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

typedef struct
{
    const uint8 *data;
    uint32      data_size;
} source_t;

typedef enum
{
    Answer_Call=1,
    Pause_Music,
    Reject_Call,
    Play_Music,
    Volume_up,
    Volume_down,
    Last_song,
    Next_song,
    Voice_Assist,
    XiaoqiXiaoqi=100,

    no_trigger=0xff
} trigger_word_t;

#endif /* DSPG_CONFIG_H_ */
