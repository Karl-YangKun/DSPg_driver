#ifndef DSPG_PROCEDURES_H_
#define DSPG_PROCEDURES_H_

typedef bool (*initialize_peripheral)(void);
typedef bool (*load_firmware)(void);
typedef bool (*config_parameter)(void);
typedef bool (*load_usecase)(void);
typedef bool (*config_usecase)(void);
typedef bool (*exit_usecase)(void);


typedef struct
{
    initialize_peripheral init;

    load_firmware load_fw;

    config_parameter defalut_config;

    load_usecase load_uc;

    config_usecase set_config;

    exit_usecase exit_uc;
} handler_t;

handler_t DSPg_GetHander(void);

#endif /* DSPG_PROCEDURES_H_ */